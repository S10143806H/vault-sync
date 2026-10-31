// duck_config.h — 机器人参数。所有数值与 Open_Duck_Mini_Runtime(v2 分支) / Open_Duck_Playground 对齐,
// 改动前先看操作手册「标定」章节。⚠ 标注的项每台鸭子都不一样, 必须自己测。
#pragma once
#include <stdint.h>

// ======================= 1. 引脚 (AMB82-mini) =======================
// 以下引脚均核对自官方 SDK: ameba-arduino-pro2/Arduino_package/hardware/variants/ameba_amb82-mini/variant.h
// 舵机总线: Serial2 -> 自动换向半双工电路(Waveshare Bus Servo Adapter(A) 或自制载板) -> 14 个 STS3215
//   Serial2: TX = D19 (PD_15), RX = D18 (PD_16)
//   ⚠ SDK 的 Serial2.flush() 是空函数, 无法用"方向脚"判断发送结束, 必须用自动换向电路
#define DUCK_SERVO_SERIAL Serial2
#define DUCK_SERVO_BAUD 1000000
// IMU: Wire -> MPU6050 / GY-521, 地址 0x68。AMB82-mini 的 Wire: SDA = D12 (PE_4), SCL = D13 (PE_3)
//   (D12~D15 同时是 SPI0, 本项目不用 SPI0)
#define DUCK_IMU_ADDR 0x68
// 足底开关(按下 = 接地 = 着地), 使用内部上拉。D16/D17 为 UART2 的 CTS/RTS, 未启用流控时可作普通 GPIO
#define DUCK_PIN_FOOT_LEFT 16
#define DUCK_PIN_FOOT_RIGHT 17
// 暂停/恢复按键(可选): 外接轻触开关到 D20 (PD_14) 与 GND, 按下 = LOW
//   (板载 PUSH_BTN=D29 的电平极性 SDK 未说明, 故不用)
#define DUCK_PIN_BUTTON 20
// 状态灯: 板载 LED_G (D24) 绿 = 正常, LED_B (D23) 蓝 = 行走中; 故障时绿灯快闪
#define DUCK_PIN_LED_OK LED_G
#define DUCK_PIN_LED_WALK LED_B
// 可选: 电池电压分压采样 (100k/33k 分压 -> A0 = D11), 不接也行, 固件优先读舵机自带电压寄存器

// ======================= 2. 关节 / 舵机 ID =======================
// 顺序 = 策略输入输出顺序, 不能改 (与 rustypot_position_hwi.py 中 self.joints 一致)
#define DUCK_NUM_JOINTS 14
static const char* const JOINT_NAMES[DUCK_NUM_JOINTS] = {
    "left_hip_yaw",  "left_hip_roll",  "left_hip_pitch",  "left_knee",  "left_ankle",
    "neck_pitch",    "head_pitch",     "head_yaw",        "head_roll",
    "right_hip_yaw", "right_hip_roll", "right_hip_pitch", "right_knee", "right_ankle"};
static const uint8_t JOINT_IDS[DUCK_NUM_JOINTS] = {20, 21, 22, 23, 24, 30, 31, 32, 33,
                                                   10, 11, 12, 13, 14};

// 默认站姿 (rad), 策略输出是相对它的偏移
static const float INIT_POS[DUCK_NUM_JOINTS] = {
    0.002f, 0.053f, -0.63f, 1.368f, -0.784f,  // 左腿
    0.0f,   0.0f,   0.0f,   0.0f,              // 脖子+头
    -0.003f, -0.065f, 0.635f, 1.379f, -0.796f  // 右腿
};

// ⚠ 关节零位软偏移 (rad): 用串口命令 `offsets` 向导测出后填这里
//    (对应 duck_config.json 的 joints_offsets; 读回位置 = 舵机读数 - offset, 写入 = 目标 + offset)
static float JOINT_OFFSETS[DUCK_NUM_JOINTS] __attribute__((unused)) = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// 位置环 P 系数 (写舵机寄存器 21)。官方: 腿 30, 头 8。
// 社区经验: 若站立时腿 5Hz 左右抖动, 腿降到 20~24。
static const uint8_t KP_LEGS = 30;
static const uint8_t KP_HEAD = 8;
static const uint8_t KP_SOFT = 2;  // 上电缓慢就位用的"软"刚度

// ======================= 3. 策略参数 (与训练一致, 勿改) =======================
#define DUCK_CONTROL_HZ 50
#define DUCK_ACTION_SCALE 0.25f
#define DUCK_DOF_VEL_SCALE 0.05f
#define DUCK_GAIT_STEPS_PER_PERIOD 27  // polynomial_coefficients.pkl: period 0.54s * fps 50
// 速度指令范围 (来自 xbox_controller.py / joystick.py)
#define DUCK_CMD_VX_MAX 0.15f   // m/s 前后
#define DUCK_CMD_VY_MAX 0.20f   // m/s 左右
#define DUCK_CMD_YAW_MAX 1.0f   // rad/s 转向

// ======================= 4. IMU 轴向映射 =======================
// 机身坐标系(与 MuJoCo 模型 imu site 一致): x 向前, y 向左, z 向上。
// 期望读数: 静止直立 acc ≈ (0, 0, +9.8); 低头(前倾) acc_x < 0; 右侧向下 acc_y > 0; 原地左转 gyro_z > 0
// ⚠ 按你的 IMU 安装方向修改: body_axis[i] = sign[i] * chip_axis[map[i]]
static const uint8_t IMU_AXIS_MAP[3] = {0, 1, 2};
static const int8_t IMU_AXIS_SIGN[3] = {1, 1, 1};

// ======================= 5. 安全 =======================
#define DUCK_FALL_TILT_DEG 55.0f       // 机身倾角超过即判定摔倒 -> 卸力
#define DUCK_BUS_FAIL_LIMIT 10         // 连续 N 个周期总线读失败 -> 卸力
#define DUCK_LOW_VOLTAGE_V 6.6f        // 2S 锂电 3.3V/节, 低于即停机
#define DUCK_CMD_TIMEOUT_MS 500        // 遥控指令超时 -> 指令清零(原地踏步)
#define DUCK_SERVO_TEMP_LIMIT_C 65     // 任一舵机超温 -> 停机降温

// ======================= 6. 遥控 (Wi-Fi UDP) =======================
// 在固件里填你的 Wi-Fi(手机热点即可), PC/手机用 tools/duck_teleop.py 发指令
#define DUCK_WIFI_SSID "your-ssid"
#define DUCK_WIFI_PASS "your-password"
#define DUCK_UDP_PORT 8888
