/*
  ameba_duck.ino — 用 Realtek AMB82-mini (Ameba Pro2, RTL8735B) 直接驱动 Open Duck Mini v2,
  在 MCU 上 50Hz 运行官方 RL 行走策略 (BEST_WALK_ONNX_2.onnx 导出的 MLP)。

  架构:
    控制线程 (高优先级, 50Hz): IMU -> 同步读 14 舵机位置/速度 -> 拼 101 维观测 -> MLP 推理
                               -> 目标角 -> 同步写 14 舵机; 同时做摔倒/掉线/低压/过温保护
    loop()                    : USB 串口命令行 + Wi-Fi UDP 遥控 (同一套命令), 舵机配置/标定向导

  与官方 Python runtime(Open_Duck_Mini_Runtime v2 分支 scripts/v2_rl_walk_mujoco.py)逐行对齐:
    观测顺序、action_scale=0.25、dof_vel*0.05、3 帧历史动作、步态相位(27 步/周期)、头部指令叠加。

  编译前:  1) 运行 tools/export_policy.py 生成 policy_weights.h / policy_selftest.h 到本目录
           2) 改 duck_config.h 里的 Wi-Fi、关节偏移、IMU 轴向
  Arduino IDE: 开发板选 "AMB82-MINI"; 串口监视器 115200, 换行符选 "NL" 或 "Both NL & CR"
  命令列表: 串口输入 help
*/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "duck_config.h"
#include "duck_policy.h"
#include "imu_mpu6050.h"
#include "sts_bus.h"

// ---------------------------------------------------------------------------
// 串口适配: 把 Ameba 的 Serial2 接到 sts::Bus
// ---------------------------------------------------------------------------
template <typename S>
class ArduinoStsPort : public sts::StsPort {
 public:
  explicit ArduinoStsPort(S& s) : s_(s) {}
  void write(const uint8_t* b, size_t n) override { s_.write(b, n); }
  void flushTx() override {}  // SDK 的 flush() 为空实现; 自动换向电路不需要等发送完成
  int readByte() override { return s_.available() > 0 ? s_.read() : -1; }
  void clearRx() override {
    while (s_.available() > 0) s_.read();
  }
  uint32_t micros() override { return ::micros(); }

 private:
  S& s_;
};

static ArduinoStsPort<decltype(DUCK_SERVO_SERIAL)> g_port(DUCK_SERVO_SERIAL);
static sts::Bus g_bus;
static ImuMpu6050 g_imu;
static WiFiUDP g_udp;
static bool g_wifi_ok = false;
static bool g_policy_ok = false;
static bool g_imu_ok = false;

// ---------------------------------------------------------------------------
// 共享状态 (loop 写请求, 控制线程执行)
// ---------------------------------------------------------------------------
enum Mode : uint8_t { MODE_OFF = 0, MODE_STAND, MODE_WALK, MODE_FAULT, MODE_MAINT };
static const char* const MODE_NAMES[] = {"OFF", "STAND", "WALK", "FAULT", "MAINT"};

static volatile uint8_t g_mode = MODE_OFF;      // 控制线程维护的实际模式
static volatile uint8_t g_req_mode = MODE_OFF;  // loop() 发出的请求
static volatile bool g_paused = false;
static volatile float g_cmd[7] = {0, 0, 0, 0, 0, 0, 0};  // vx vy wz neck_p head_p head_y head_r
static volatile uint32_t g_cmd_ms = 0;
static volatile bool g_cmd_sticky = false;  // 串口手输的指令不超时; UDP 指令超时清零
static volatile bool g_sprint = false;
static volatile uint8_t g_kp_legs = KP_LEGS;
static volatile uint8_t g_kp_head = KP_HEAD;
static char g_fault[80] = "";

// 运行统计
static volatile uint32_t g_t_imu = 0, g_t_read = 0, g_t_infer = 0, g_t_write = 0, g_t_total = 0,
                         g_t_max = 0, g_overruns = 0, g_steps = 0, g_bus_miss = 0;
static volatile float g_volt = 0, g_tilt_deg = 0;
static volatile int g_temp_max = 0;

// 策略状态
static float s_last_act[DUCK_NUM_JOINTS], s_last2_act[DUCK_NUM_JOINTS],
    s_last3_act[DUCK_NUM_JOINTS];
static float s_targets[DUCK_NUM_JOINTS];
static float s_pos[DUCK_NUM_JOINTS], s_vel[DUCK_NUM_JOINTS];
static float s_gyro[3], s_acc[3] = {0, 0, 9.81f};
static float s_imitation_i = 0;
static float s_phase[2] = {0, 0};
static int s_bus_fail_streak = 0;
static int s_fall_streak = 0;
static float s_obs[DUCK_OBS_DIM];
static float s_act[DUCK_ACT_DIM];

static const float kZeroPose[DUCK_NUM_JOINTS] = {0};

// ---------------------------------------------------------------------------
// 舵机工具
// ---------------------------------------------------------------------------
static void writeAllByte(uint8_t addr, const uint8_t* vals) {
  g_bus.syncWrite(JOINT_IDS, DUCK_NUM_JOINTS, addr, vals, 1);
}
static void setTorqueAll(bool on) {
  uint8_t v[DUCK_NUM_JOINTS];
  memset(v, on ? 1 : 0, sizeof v);
  writeAllByte(sts::REG_TORQUE_ENABLE, v);
}
static void setKpAll(uint8_t legs, uint8_t head) {
  // 写 P 系数(寄存器 21)。Lock=1(上电默认)时只改 RAM 不写 EEPROM, 与官方 runtime 每次启动设 kp 的做法一致
  uint8_t v[DUCK_NUM_JOINTS];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) v[i] = (i >= 5 && i <= 8) ? head : legs;
  writeAllByte(sts::REG_P_COEF, v);
}
static void writePose(const float* pose) {
  float raw[DUCK_NUM_JOINTS];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) raw[i] = pose[i] + JOINT_OFFSETS[i];
  g_bus.syncWritePositions(JOINT_IDS, DUCK_NUM_JOINTS, raw);
}
// 读 14 路位置/速度(已减偏移); 读失败的关节沿用上次值。返回成功个数
static int readJoints(float* pos, float* vel) {
  float p[DUCK_NUM_JOINTS], v[DUCK_NUM_JOINTS];
  bool ok[DUCK_NUM_JOINTS];
  int good = g_bus.syncReadPosVel(JOINT_IDS, DUCK_NUM_JOINTS, p, v, ok);
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) {
    if (!ok[i]) continue;
    pos[i] = roundf((p[i] - JOINT_OFFSETS[i]) * 1000.0f) / 1000.0f;  // runtime 里 np.around(.., 3)
    vel[i] = roundf(v[i] * 1000.0f) / 1000.0f;
  }
  return good;
}
static void softStart(const float* pose) {
  // 与 HWI.turn_on() 相同: 低刚度 -> 移到目标姿态 -> 等 1s -> 正常刚度
  setKpAll(KP_SOFT, KP_SOFT);
  setTorqueAll(true);
  delay(1000);
  writePose(pose);
  delay(1000);
  setKpAll(g_kp_legs, g_kp_head);
}
static void enterFault(const char* why) {
  setTorqueAll(false);
  strncpy(g_fault, why, sizeof(g_fault) - 1);
  g_mode = MODE_FAULT;
  g_req_mode = MODE_FAULT;
  Serial.print("[FAULT] ");
  Serial.println(why);
}

// ---------------------------------------------------------------------------
// 控制线程
// ---------------------------------------------------------------------------
static void resetPolicyState() {
  memset(s_last_act, 0, sizeof s_last_act);
  memset(s_last2_act, 0, sizeof s_last2_act);
  memset(s_last3_act, 0, sizeof s_last3_act);
  memcpy(s_targets, INIT_POS, sizeof s_targets);
  s_imitation_i = 0;
  s_phase[0] = s_phase[1] = 0;  // runtime 初值为 [0, 0]
  s_bus_fail_streak = 0;
  s_fall_streak = 0;
}

static void currentCommands(float cmd[7]) {
  bool expired = !g_cmd_sticky && (millis() - g_cmd_ms > DUCK_CMD_TIMEOUT_MS);
  for (int i = 0; i < 7; ++i) cmd[i] = expired ? 0.0f : g_cmd[i];
}

static void healthCheck() {
  // 每 25 个周期(0.5s)轮询一个舵机的电压+温度(寄存器 62,63)
  static int k = 0;
  uint8_t d[2];
  if (g_bus.read(JOINT_IDS[k], sts::REG_PRESENT_VOLTAGE, 2, d)) {
    g_volt = d[0] * 0.1f;
    if ((int)d[1] > g_temp_max || k == 0) g_temp_max = d[1];
    if (g_volt > 1.0f && g_volt < DUCK_LOW_VOLTAGE_V) enterFault("battery low voltage");
    if (d[1] >= DUCK_SERVO_TEMP_LIMIT_C) enterFault("servo over temperature");
  }
  k = (k + 1) % DUCK_NUM_JOINTS;
}

static void walkStep() {
  uint32_t t0 = micros();
  float cmd[7];
  currentCommands(cmd);

  // 1) IMU
  float g[3], a[3];
  if (g_imu_ok && g_imu.read(g, a)) {
    memcpy(s_gyro, g, sizeof g);
    memcpy(s_acc, a, sizeof a);
  }
  uint32_t t1 = micros();

  // 2) 舵机状态
  int good = readJoints(s_pos, s_vel);
  if (good < DUCK_NUM_JOINTS) {
    ++g_bus_miss;
    if (++s_bus_fail_streak > DUCK_BUS_FAIL_LIMIT) {
      enterFault("servo bus: missing replies");
      return;
    }
  } else {
    s_bus_fail_streak = 0;
  }
  uint32_t t2 = micros();

  // 3) 观测 (顺序与 v2_rl_walk_mujoco.py get_obs() 完全一致, 共 101 维)
  int o = 0;
  for (int i = 0; i < 3; ++i) s_obs[o++] = s_gyro[i];
  for (int i = 0; i < 3; ++i) s_obs[o++] = s_acc[i];
  for (int i = 0; i < 7; ++i) s_obs[o++] = cmd[i];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_pos[i] - INIT_POS[i];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_vel[i] * DUCK_DOF_VEL_SCALE;
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_last_act[i];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_last2_act[i];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_last3_act[i];
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_obs[o++] = s_targets[i];
  s_obs[o++] = digitalRead(DUCK_PIN_FOOT_LEFT) == LOW ? 1.0f : 0.0f;
  s_obs[o++] = digitalRead(DUCK_PIN_FOOT_RIGHT) == LOW ? 1.0f : 0.0f;
  s_obs[o++] = s_phase[0];
  s_obs[o++] = s_phase[1];

  // 4) 步态相位 (在取观测之后更新, 与 runtime 一致)
  const float nb = (float)DUCK_GAIT_STEPS_PER_PERIOD;
  s_imitation_i += g_sprint ? 1.3f : 1.0f;  // LB 冲刺模式 = 1.3
  s_imitation_i = fmodf(s_imitation_i, nb);
  s_phase[0] = cosf(s_imitation_i / nb * 2.0f * (float)M_PI);
  s_phase[1] = sinf(s_imitation_i / nb * 2.0f * (float)M_PI);

  // 5) 推理
  duck_policy_infer(s_obs, s_act);
  uint32_t t3 = micros();

  // 6) 动作 -> 目标角
  memcpy(s_last3_act, s_last2_act, sizeof s_last3_act);
  memcpy(s_last2_act, s_last_act, sizeof s_last2_act);
  memcpy(s_last_act, s_act, sizeof s_last_act);
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) s_targets[i] = INIT_POS[i] + s_act[i] * DUCK_ACTION_SCALE;
  for (int i = 0; i < 4; ++i) s_targets[5 + i] += cmd[3 + i];  // 头部指令叠加
  writePose(s_targets);
  uint32_t t4 = micros();

  // 7) 摔倒检测: 加速度方向与机身 z 轴夹角
  float an = sqrtf(s_acc[0] * s_acc[0] + s_acc[1] * s_acc[1] + s_acc[2] * s_acc[2]);
  if (an > 1.0f) {
    float c = s_acc[2] / an;
    c = c > 1 ? 1 : (c < -1 ? -1 : c);
    g_tilt_deg = acosf(c) * 180.0f / (float)M_PI;
    s_fall_streak = (g_tilt_deg > DUCK_FALL_TILT_DEG) ? s_fall_streak + 1 : 0;
    if (s_fall_streak > 5) {
      enterFault("fall detected (tilt)");
      return;
    }
  }

  g_t_imu = t1 - t0;
  g_t_read = t2 - t1;
  g_t_infer = t3 - t2;
  g_t_write = t4 - t3;
  g_t_total = t4 - t0;
  if (g_t_total > g_t_max) g_t_max = g_t_total;
  ++g_steps;
}

static void handleTransition(uint8_t req) {
  uint8_t cur = g_mode;
  if (cur == MODE_FAULT && req != MODE_OFF) {  // 故障后必须先 off 确认, 不允许直接重新上力
    Serial.println("[FAULT] 请先输入 off 清除故障, 扶正后再 stand");
    g_req_mode = MODE_FAULT;
    return;
  }
  if (req == MODE_OFF) {
    setTorqueAll(false);
    g_fault[0] = 0;
    g_mode = MODE_OFF;
  } else if (req == MODE_STAND) {
    if (cur == MODE_OFF || cur == MODE_MAINT) {
      softStart(INIT_POS);
    } else {  // 从 WALK 回到站姿
      setKpAll(g_kp_legs, g_kp_head);
      writePose(INIT_POS);
    }
    g_mode = MODE_STAND;
  } else if (req == MODE_WALK) {
    if (!g_policy_ok) {
      Serial.println("[WALK] 拒绝: 策略自检未通过");
      g_req_mode = cur;
      return;
    }
    if (cur != MODE_STAND) {
      softStart(INIT_POS);
      delay(2000);  // runtime 在 start() 后同样等 2s
    }
    resetPolicyState();
    g_mode = MODE_WALK;
  }
  // MODE_FAULT / MODE_MAINT 由 enterFault()/loop() 直接设置
}

static void controlTask(const void* arg) {
  (void)arg;
  const uint32_t period_us = 1000000UL / DUCK_CONTROL_HZ;
  uint32_t next = micros();
  uint32_t tick = 0;
  for (;;) {
    uint8_t req = g_req_mode;
    if (req != g_mode && req != MODE_FAULT && req != MODE_MAINT) {
      handleTransition(req);
      next = micros();
    }
    uint8_t m = g_mode;
    if (m == MODE_WALK && !g_paused) {
      walkStep();
    }
    if ((m == MODE_WALK || m == MODE_STAND) && (++tick % 25 == 0)) healthCheck();

    next += period_us;
    int32_t wait = (int32_t)(next - micros());
    if (wait < 0) {
      ++g_overruns;
      next = micros();
    } else {
      if (wait > 1500) delay((wait - 500) / 1000);  // 让出 CPU 给 Wi-Fi / loop()
      while ((int32_t)(next - micros()) > 0) {
      }
    }
  }
}

// ---------------------------------------------------------------------------
// 命令行 (串口与 UDP 共用)
// ---------------------------------------------------------------------------
static bool requireIdle() {
  uint8_t m = g_mode;
  bool idle = (m == MODE_OFF || m == MODE_MAINT || m == MODE_FAULT);
  if (idle && g_req_mode == m) return true;  // 还有未执行的模式切换请求时也不允许
  Serial.println("先输入 off 卸力, 再执行该维护命令");
  return false;
}

static void printHelp() {
  Serial.println(F(
      "== 运行 ==\n"
      " stand            低刚度上电 -> 站姿\n"
      " walk             开始运行 RL 策略(先 stand 再 walk)\n"
      " pause / resume   暂停/恢复策略(也可按 D20 外接按键)\n"
      " off              全部卸力(也用于清除 FAULT)\n"
      " c vx vy wz [np hp hy hr]  速度指令 m/s,m/s,rad/s (+头部 rad), 串口输入的指令一直有效\n"
      " sprint 0|1       步频 x1.3\n"
      " kp <legs> <head> 运行时改 P 系数(默认 30 8; 腿抖动可降到 20)\n"
      " stat             状态/时序/电压/温度\n"
      "== 装配与标定 (需 off) ==\n"
      " scan             扫描总线上所有舵机 ID\n"
      " cfg <newid>      总线上只接 1 个新舵机: 写官方参数 + 改 ID + 回中(装舵盘用)\n"
      " setid <old> <new>\n"
      " center <id>      单个舵机转到 0 rad(2048)\n"
      " check            读 14 个关节: 位置/电压/温度\n"
      " imu              打印 3 秒 IMU 数据(检查轴向)\n"
      " feet             打印 3 秒足底开关\n"
      " zero             所有关节到 0 位(把鸭子悬空!), 进入 MAINT\n"
      " offset <i>       关节 i 卸力, 手动摆到正确零位后输入 offset <i> ok\n"
      " offsets          打印当前偏移(复制到 duck_config.h)\n"
      " bench            测推理和总线耗时\n"
      " selftest         策略数值自检"));
}

static void printOffsets() {
  Serial.print("static float JOINT_OFFSETS[DUCK_NUM_JOINTS] = {");
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) {
    Serial.print(JOINT_OFFSETS[i], 4);
    Serial.print(i < DUCK_NUM_JOINTS - 1 ? "f, " : "f};\n");
  }
}

static void cmdScan() {
  Serial.print("scan: ");
  int n = 0;
  for (int id = 0; id < 254; ++id) {
    if (g_bus.ping((uint8_t)id)) {
      Serial.print(id);
      Serial.print(' ');
      ++n;
    }
  }
  Serial.print("\n共 ");
  Serial.print(n);
  Serial.println(" 个");
}

static void cmdCfg(int new_id) {
  // 对应官方 scripts/configure_motor.py: 新舵机默认 ID 1, 找不到就扫描
  int cur = -1;
  if (g_bus.ping(1)) cur = 1;
  for (int id = 0; id < 254 && cur < 0; ++id)
    if (g_bus.ping((uint8_t)id)) cur = id;
  if (cur < 0) {
    Serial.println("cfg: 没找到舵机, 检查供电/接线/波特率");
    return;
  }
  uint8_t id = (uint8_t)cur;
  bool ok = true;
  ok &= g_bus.writeByte(id, sts::REG_LOCK, 0);  // 解锁 EEPROM
  ok &= g_bus.writeByte(id, sts::REG_MODE, 0);  // 位置模式
  ok &= g_bus.writeByte(id, sts::REG_MAX_ACC, 0);  // 寄存器85 出厂常为 50, 不改则比仿真慢约 3 倍
  ok &= g_bus.writeByte(id, sts::REG_ACC, 0);
  ok &= g_bus.writeByte(id, sts::REG_P_COEF, 32);
  ok &= g_bus.writeByte(id, sts::REG_I_COEF, 0);
  ok &= g_bus.writeByte(id, sts::REG_D_COEF, 0);
  ok &= g_bus.writeByte(id, sts::REG_ID, (uint8_t)new_id);
  delay(50);
  id = (uint8_t)new_id;
  ok &= g_bus.writeByte(id, sts::REG_LOCK, 1);
  ok &= g_bus.writeByte(id, sts::REG_TORQUE_ENABLE, 1);
  ok &= g_bus.writeWord(id, sts::REG_GOAL_POSITION, 2048);  // 回中, 此时装舵盘
  Serial.print(ok ? "cfg OK: " : "cfg 有写入失败: ");
  Serial.print(cur);
  Serial.print(" -> ");
  Serial.print(new_id);
  Serial.println(", 已回中(2048), 现在对齐安装舵盘");
}

static void cmdCheck() {
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) {
    uint8_t id = JOINT_IDS[i];
    uint16_t raw = 0;
    uint8_t vt[2] = {0, 0};
    bool ok = g_bus.readWord(id, sts::REG_PRESENT_POSITION, &raw) &&
              g_bus.read(id, sts::REG_PRESENT_VOLTAGE, 2, vt);
    Serial.print(JOINT_NAMES[i]);
    Serial.print(" id=");
    Serial.print(id);
    if (!ok) {
      Serial.println("  无应答!");
      continue;
    }
    Serial.print(" pos=");
    Serial.print(sts::rawToRad(raw) - JOINT_OFFSETS[i], 3);
    Serial.print("rad  V=");
    Serial.print(vt[0] * 0.1f, 1);
    Serial.print("  T=");
    Serial.println(vt[1]);
  }
}

static void cmdImu() {
  if (!g_imu_ok) {
    Serial.println("IMU 未初始化");
    return;
  }
  Serial.println("期望: 静止直立 acc≈(0,0,+9.8); 低头 acc_x<0; 右侧朝下 acc_y>0; 原地左转 gyro_z>0");
  for (int k = 0; k < 30; ++k) {
    float g[3], a[3];
    if (g_imu.read(g, a)) {
      Serial.print("gyro ");
      for (int i = 0; i < 3; ++i) { Serial.print(g[i], 3); Serial.print(' '); }
      Serial.print(" acc ");
      for (int i = 0; i < 3; ++i) { Serial.print(a[i], 2); Serial.print(' '); }
      Serial.println();
    }
    delay(100);
  }
}

static float s_offset_ref[DUCK_NUM_JOINTS];

static void cmdOffset(int i, bool confirm) {
  if (g_mode != MODE_MAINT) {
    Serial.println("先执行 zero");
    return;
  }
  if (i < 0 || i >= DUCK_NUM_JOINTS) return;
  float pos[DUCK_NUM_JOINTS], vel[DUCK_NUM_JOINTS];
  memset(pos, 0, sizeof pos);
  if (readJoints(pos, vel) < DUCK_NUM_JOINTS) {
    Serial.println("读舵机失败");
    return;
  }
  if (!confirm) {
    s_offset_ref[i] = pos[i];
    g_bus.writeByte(JOINT_IDS[i], sts::REG_TORQUE_ENABLE, 0);
    Serial.print(JOINT_NAMES[i]);
    Serial.println(" 已卸力: 手动摆到正确的零位(对照 CAD/装配图), 然后输入 offset <i> ok");
  } else {
    JOINT_OFFSETS[i] += pos[i] - s_offset_ref[i];
    writePose(kZeroPose);
    delay(300);
    g_bus.writeByte(JOINT_IDS[i], sts::REG_TORQUE_ENABLE, 1);
    Serial.print(JOINT_NAMES[i]);
    Serial.print(" offset = ");
    Serial.println(JOINT_OFFSETS[i], 4);
    printOffsets();
  }
}

static void cmdBench() {
  float obs[DUCK_OBS_DIM] = {0}, act[DUCK_ACT_DIM];
  uint32_t t0 = micros();
  for (int k = 0; k < 50; ++k) duck_policy_infer(obs, act);
  uint32_t t1 = micros();
  Serial.print("policy infer: ");
  Serial.print((t1 - t0) / 50);
  Serial.println(" us/次 (预算 < 8000)");
  float p[DUCK_NUM_JOINTS], v[DUCK_NUM_JOINTS];
  t0 = micros();
  int good = 0;
  for (int k = 0; k < 20; ++k) good += readJoints(p, v);
  t1 = micros();
  Serial.print("sync read 14 joints: ");
  Serial.print((t1 - t0) / 20);
  Serial.print(" us/次, 成功率 ");
  Serial.print(good * 100 / (20 * DUCK_NUM_JOINTS));
  Serial.println("%");
}

static void cmdStat() {
  Serial.print("mode=");
  Serial.print(MODE_NAMES[g_mode]);
  if (g_paused) Serial.print("(paused)");
  if (g_fault[0]) { Serial.print(" fault="); Serial.print(g_fault); }
  Serial.print("  steps=");
  Serial.print(g_steps);
  Serial.print("  overruns=");
  Serial.print(g_overruns);
  Serial.print("  bus_miss=");
  Serial.println(g_bus_miss);
  Serial.print("us: imu=");
  Serial.print(g_t_imu);
  Serial.print(" read=");
  Serial.print(g_t_read);
  Serial.print(" infer=");
  Serial.print(g_t_infer);
  Serial.print(" write=");
  Serial.print(g_t_write);
  Serial.print(" total=");
  Serial.print(g_t_total);
  Serial.print(" max=");
  Serial.println(g_t_max);
  Serial.print("V=");
  Serial.print(g_volt, 1);
  Serial.print("  Tmax=");
  Serial.print(g_temp_max);
  Serial.print("  tilt=");
  Serial.print(g_tilt_deg, 1);
  Serial.print("  cmd=");
  for (int i = 0; i < 3; ++i) { Serial.print(g_cmd[i], 2); Serial.print(' '); }
  if (g_wifi_ok) { Serial.print(" ip="); Serial.print(WiFi.localIP()); }
  Serial.println();
}

static void execLine(char* line, bool from_udp) {
  char* argv[10];
  int argc = 0;
  for (char* t = strtok(line, " \t\r\n"); t && argc < 10; t = strtok(nullptr, " \t\r\n"))
    argv[argc++] = t;
  if (argc == 0) return;
  const char* c = argv[0];

  if (!strcmp(c, "c")) {  // 高频遥控指令, 不回显
    for (int i = 0; i < 7; ++i) g_cmd[i] = (argc > 1 + i) ? (float)atof(argv[1 + i]) : 0.0f;
    g_cmd[0] = constrain(g_cmd[0], -DUCK_CMD_VX_MAX, DUCK_CMD_VX_MAX);
    g_cmd[1] = constrain(g_cmd[1], -DUCK_CMD_VY_MAX, DUCK_CMD_VY_MAX);
    g_cmd[2] = constrain(g_cmd[2], -DUCK_CMD_YAW_MAX, DUCK_CMD_YAW_MAX);
    g_cmd_ms = millis();
    g_cmd_sticky = !from_udp;
    return;
  }
  if (!strcmp(c, "help")) printHelp();
  else if (!strcmp(c, "off")) { g_paused = false; g_req_mode = MODE_OFF; }
  else if (!strcmp(c, "stand")) g_req_mode = MODE_STAND;
  else if (!strcmp(c, "walk")) { g_paused = false; g_req_mode = MODE_WALK; }
  else if (!strcmp(c, "pause")) g_paused = true;
  else if (!strcmp(c, "resume")) g_paused = false;
  else if (!strcmp(c, "sprint") && argc > 1) g_sprint = atoi(argv[1]) != 0;
  else if (!strcmp(c, "stat")) cmdStat();
  else if (!strcmp(c, "kp") && argc > 2) {
    g_kp_legs = (uint8_t)atoi(argv[1]);
    g_kp_head = (uint8_t)atoi(argv[2]);
    if (g_mode == MODE_OFF || g_mode == MODE_MAINT) setKpAll(g_kp_legs, g_kp_head);
    else Serial.println("kp 将在下次 stand 时生效(运行中改请先 off)");
  } else if (!strcmp(c, "selftest")) {
    Serial.print("policy max err = ");
    Serial.println(duck_policy_selftest(), 7);
  } else if (from_udp) {
    // 维护类命令只允许串口执行
  } else if (!strcmp(c, "scan")) { if (requireIdle()) cmdScan(); }
  else if (!strcmp(c, "cfg") && argc > 1) { if (requireIdle()) cmdCfg(atoi(argv[1])); }
  else if (!strcmp(c, "setid") && argc > 2) {
    if (requireIdle()) {
      uint8_t o = (uint8_t)atoi(argv[1]), n = (uint8_t)atoi(argv[2]);
      bool ok = g_bus.writeByte(o, sts::REG_LOCK, 0) && g_bus.writeByte(o, sts::REG_ID, n);
      delay(50);
      ok = ok && g_bus.writeByte(n, sts::REG_LOCK, 1);
      Serial.println(ok ? "setid OK" : "setid 失败");
    }
  } else if (!strcmp(c, "center") && argc > 1) {
    if (requireIdle()) {
      uint8_t id = (uint8_t)atoi(argv[1]);
      g_bus.writeByte(id, sts::REG_TORQUE_ENABLE, 1);
      g_bus.writeWord(id, sts::REG_GOAL_POSITION, 2048);
    }
  } else if (!strcmp(c, "check")) { if (requireIdle()) cmdCheck(); }
  else if (!strcmp(c, "imu")) { if (requireIdle()) cmdImu(); }
  else if (!strcmp(c, "feet")) {
    for (int k = 0; k < 30; ++k) {
      Serial.print("left=");
      Serial.print(digitalRead(DUCK_PIN_FOOT_LEFT) == LOW);
      Serial.print(" right=");
      Serial.println(digitalRead(DUCK_PIN_FOOT_RIGHT) == LOW);
      delay(100);
    }
  } else if (!strcmp(c, "zero")) {
    if (requireIdle()) {
      softStart(kZeroPose);
      g_mode = MODE_MAINT;
      g_req_mode = MODE_MAINT;
      Serial.println("已到 0 位 (MAINT)。逐个关节执行 offset <i> 标定, 完成后 off");
    }
  } else if (!strcmp(c, "offset") && argc > 1) {
    cmdOffset(atoi(argv[1]), argc > 2 && !strcmp(argv[2], "ok"));
  } else if (!strcmp(c, "offsets")) printOffsets();
  else if (!strcmp(c, "bench")) { if (requireIdle()) cmdBench(); }
  else {
    Serial.print("未知命令: ");
    Serial.println(c);
  }
}

// ---------------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Ameba Duck (Open Duck Mini v2 on AMB82-mini) ===");

  pinMode(DUCK_PIN_FOOT_LEFT, INPUT_PULLUP);
  pinMode(DUCK_PIN_FOOT_RIGHT, INPUT_PULLUP);
  pinMode(DUCK_PIN_BUTTON, INPUT_PULLUP);
  pinMode(DUCK_PIN_LED_OK, OUTPUT);
  pinMode(DUCK_PIN_LED_WALK, OUTPUT);

  // 1) 策略
  g_policy_ok = duck_policy_init(false);  // 权重 .rodata 已在 DDR, 不拷贝
  float err = g_policy_ok ? duck_policy_selftest() : 1e9f;
  g_policy_ok = g_policy_ok && err < 1e-3f;
  Serial.print("[policy] selftest max err = ");
  Serial.print(err, 7);
  Serial.println(g_policy_ok ? "  OK" : "  FAIL (walk 将被禁止)");

  // 2) 舵机总线
  DUCK_SERVO_SERIAL.begin(DUCK_SERVO_BAUD);
  g_bus.begin(&g_port, false, 3000);
  int found = 0;
  for (int i = 0; i < DUCK_NUM_JOINTS; ++i) found += g_bus.ping(JOINT_IDS[i]) ? 1 : 0;
  Serial.print("[servo] ");
  Serial.print(found);
  Serial.println("/14 个关节舵机应答 (装配阶段可忽略)");

  // 3) IMU
  g_imu_ok = g_imu.begin(&Wire, DUCK_IMU_ADDR);
  Serial.print("[imu] MPU6050 ");
  Serial.print(g_imu_ok ? "OK, WHO_AM_I=0x" : "未找到");
  if (g_imu_ok) {
    Serial.println(g_imu.whoAmI(), HEX);
    Serial.println("[imu] 陀螺零偏标定 2 秒, 保持静止...");
    g_imu.calibrateGyro(400);
  } else {
    Serial.println();
  }

  // 4) Wi-Fi (可选)
  if (strcmp(DUCK_WIFI_SSID, "your-ssid") != 0) {
    static char ssid[] = DUCK_WIFI_SSID;
    static char pass[] = DUCK_WIFI_PASS;
    for (int k = 0; k < 3 && WiFi.status() != WL_CONNECTED; ++k) WiFi.begin(ssid, pass);
    g_wifi_ok = WiFi.status() == WL_CONNECTED;
    if (g_wifi_ok) {
      g_udp.begin(DUCK_UDP_PORT);
      Serial.print("[wifi] IP = ");
      Serial.print(WiFi.localIP());
      Serial.print("  UDP 端口 ");
      Serial.println(DUCK_UDP_PORT);
    } else {
      Serial.println("[wifi] 连接失败, 仅串口控制");
    }
  }

  // 5) 控制线程
  os_thread_create_arduino(controlTask, nullptr, osPriorityHigh, 8192);
  Serial.println("就绪。输入 help 查看命令");
}

void loop() {
  // USB 串口命令
  static char line[96];
  static size_t n = 0;
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    if (ch == '\n' || ch == '\r') {
      if (n) {
        line[n] = 0;
        execLine(line, false);
        n = 0;
      }
    } else if (n < sizeof(line) - 1) {
      line[n++] = ch;
    }
  }

  // UDP 遥控 (tools/duck_teleop.py): 每个包一行命令, 如 "c 0.10 0 0.3"
  if (g_wifi_ok) {
    int len = g_udp.parsePacket();
    if (len > 0) {
      char buf[128];
      int r = g_udp.read((unsigned char*)buf, sizeof(buf) - 1);
      if (r > 0) {
        buf[r] = 0;
        execLine(buf, true);
      }
    }
  }

  // 板载按键: 暂停/恢复
  static bool last_btn = false;
  bool btn = digitalRead(DUCK_PIN_BUTTON) == LOW;
  if (btn && !last_btn && g_mode == MODE_WALK) g_paused = !g_paused;
  last_btn = btn;

  // 状态灯
  bool fault = g_mode == MODE_FAULT;
  digitalWrite(DUCK_PIN_LED_OK, fault ? ((millis() / 150) % 2) : HIGH);
  digitalWrite(DUCK_PIN_LED_WALK, (g_mode == MODE_WALK && !g_paused) ? HIGH : LOW);

  delay(2);
}
