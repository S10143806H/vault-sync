// sts_bus.h — 飞特 Feetech STS3215 串行总线舵机驱动(半双工 TTL, 默认 1 Mbps)
// 协议与 Pollen 官方 rustypot 的 Feetech/Dynamixel-v1 实现一致:
//   指令包: FF FF ID LEN INSTR PARAMS... CHK,  LEN = 参数个数 + 2,  CHK = ~(ID+LEN+INSTR+ΣPARAMS)
//   应答包: FF FF ID LEN ERR   PARAMS... CHK
// 不依赖 Arduino 头文件: 串口通过 StsPort 接口注入(固件里用 ArduinoStsPort, 主机单测里用虚拟总线)。
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef STS_SYNC_READ_CHUNK
#define STS_SYNC_READ_CHUNK 7  // 每批同步读的舵机数: 7 x 10 字节 = 70 字节 < 128 字节接收缓冲
#endif

namespace sts {

// ---- 指令 ----
enum : uint8_t {
  INST_PING = 0x01,
  INST_READ = 0x02,
  INST_WRITE = 0x03,
  INST_SYNC_READ = 0x82,
  INST_SYNC_WRITE = 0x83,
};
static const uint8_t BROADCAST_ID = 0xFE;

// ---- STS3215 控制表(地址取自 rustypot sts3215.rs) ----
enum : uint8_t {
  REG_ID = 5,
  REG_BAUD = 6,               // 0 = 1Mbps
  REG_RETURN_DELAY = 7,       // 应答延时, 单位 2us
  REG_MIN_ANGLE = 9,          // u16
  REG_MAX_ANGLE = 11,         // u16
  REG_P_COEF = 21,            // u8, 位置环 P (Open Duck 用 32/30, 头部 8)
  REG_D_COEF = 22,
  REG_I_COEF = 23,
  REG_OFFSET = 31,            // u16, 零位偏移(符号位 bit11)
  REG_MODE = 33,              // 0 = 位置模式
  REG_TORQUE_ENABLE = 40,
  REG_ACC = 41,               // 0 = 不限加速度
  REG_GOAL_POSITION = 42,     // u16
  REG_GOAL_TIME = 44,
  REG_GOAL_SPEED = 46,
  REG_TORQUE_LIMIT = 48,
  REG_LOCK = 55,              // 0 = EEPROM 区写入掉电保存; 1 = 不保存(只改 RAM)
  REG_PRESENT_POSITION = 56,  // u16, 与 58 连续, 可一次读 4 字节
  REG_PRESENT_SPEED = 58,     // u16, bit15 为符号位, 单位 step/s
  REG_PRESENT_LOAD = 60,
  REG_PRESENT_VOLTAGE = 62,   // u8, 单位 0.1V
  REG_PRESENT_TEMP = 63,      // u8, 摄氏度
  REG_MOVING = 66,
  REG_PRESENT_CURRENT = 69,
  REG_MAX_ACC = 85,           // u8
};

// ---- 单位换算(与 rustypot 一致: 2048 步 = 0 rad, 4096 步/圈) ----
float rawToRad(uint16_t raw);         // 位置
uint16_t radToRaw(float rad);         // 位置, 截断到 [0, 4095]
float rawSpeedToRadS(uint16_t raw);   // 速度, sign-magnitude(bit15)

// ---- 串口抽象 ----
struct StsPort {
  virtual ~StsPort() {}
  virtual void write(const uint8_t* buf, size_t n) = 0;
  virtual void flushTx() = 0;           // 阻塞到最后一个 bit 发完
  virtual int readByte() = 0;           // 无数据返回 -1
  virtual void clearRx() = 0;
  virtual uint32_t micros() = 0;
  virtual void setTxMode(bool tx) { (void)tx; }  // 方向脚(DIR)控制, 自动方向板无需实现
};

// ---- 报文工具(静态, 便于单测) ----
uint8_t checksum(const uint8_t* from_id, size_t n);  // 对 ID..最后一个参数求和取反
size_t buildPacket(uint8_t* out, uint8_t id, uint8_t instr, const uint8_t* params,
                   size_t nparams);

class Bus {
 public:
  // echo=true: 单线回环接法(TX/RX 并在一根线上), 会收到自己发出的字节, 驱动自动丢弃
  void begin(StsPort* port, bool echo = false, uint32_t timeout_us = 3000);

  bool ping(uint8_t id);
  bool write(uint8_t id, uint8_t addr, const uint8_t* data, uint8_t len);
  bool writeByte(uint8_t id, uint8_t addr, uint8_t v) { return write(id, addr, &v, 1); }
  bool writeWord(uint8_t id, uint8_t addr, uint16_t v);
  bool read(uint8_t id, uint8_t addr, uint8_t len, uint8_t* out);
  bool readByte(uint8_t id, uint8_t addr, uint8_t* v) { return read(id, addr, 1, v); }
  bool readWord(uint8_t id, uint8_t addr, uint16_t* v);

  // 同步写: data 为 n 个舵机各 len_each 字节, 依次排列。广播, 无应答。
  void syncWrite(const uint8_t* ids, uint8_t n, uint8_t addr, const uint8_t* data,
                 uint8_t len_each);
  // 同步读: out 为 n*len_each 字节; ok[i] 表示第 i 个舵机是否成功应答。返回成功个数。
  int syncRead(const uint8_t* ids, uint8_t n, uint8_t addr, uint8_t len_each, uint8_t* out,
               bool* ok);

  // 高层便捷: 14 个关节一次写目标角(rad) / 一次读位置(rad)+速度(rad/s)
  void syncWritePositions(const uint8_t* ids, uint8_t n, const float* rad);
  int syncReadPosVel(const uint8_t* ids, uint8_t n, float* pos_rad, float* vel_rads, bool* ok);

  uint32_t errorCount() const { return errors_; }
  uint8_t lastServoError() const { return last_servo_err_; }

 private:
  void send(const uint8_t* pkt, size_t n);
  // 读一个应答包到 params, 返回参数长度, 失败返回 -1
  int receiveStatus(uint8_t expect_id, uint8_t* params, uint8_t max_params, uint32_t timeout_us);
  int receiveAny(uint8_t* id_out, uint8_t* params, uint8_t max_params, uint32_t timeout_us);

  StsPort* port_ = nullptr;
  bool echo_ = false;
  uint32_t timeout_us_ = 3000;
  uint32_t errors_ = 0;
  uint8_t last_servo_err_ = 0;
};

}  // namespace sts
