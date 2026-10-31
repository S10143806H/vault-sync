// imu_mpu6050.h — 极简 MPU6050(GY-521) 寄存器级驱动, 只依赖 Arduino Wire。
// 为什么不用 BNO055: BNO055 的 I2C 依赖 clock stretching, AmebaPro2 社区移植(rkuo2000/AmebaPro_OpenDuck)
// 明确写了 "BNO055 currently not supported by AmebaPro2"。策略只用 gyro(rad/s) + acc(m/s²) 两个原始量,
// 用 6 轴 IMU + 开机零偏标定即可替代 (Microduck 本身也用 6 轴 LSM6DSV16X)。
#pragma once
#include <stdint.h>

class TwoWire;

class ImuMpu6050 {
 public:
  bool begin(TwoWire* wire, uint8_t addr = 0x68);
  // 静止状态下采样 n 次求陀螺零偏(机器人必须放稳不动)
  void calibrateGyro(int n = 400);
  // 读一次: gyro rad/s, acc m/s², 已做零偏和轴向映射(见 duck_config.h)
  bool read(float gyro[3], float acc[3]);
  uint8_t whoAmI() const { return who_; }

 private:
  bool readRaw(int16_t a[3], int16_t g[3]);
  bool writeReg(uint8_t reg, uint8_t val);
  TwoWire* wire_ = nullptr;
  uint8_t addr_ = 0x68;
  uint8_t who_ = 0;
  float gyro_bias_[3] = {0, 0, 0};
};
