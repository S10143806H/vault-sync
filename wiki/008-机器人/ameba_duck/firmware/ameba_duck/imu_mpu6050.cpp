// imu_mpu6050.cpp — 见 imu_mpu6050.h
#include "imu_mpu6050.h"

#include <Arduino.h>
#include <Wire.h>

#include "duck_config.h"

// 量程: 陀螺 ±500 dps (65.5 LSB/dps), 加速度 ±4 g (8192 LSB/g), DLPF 44 Hz, 采样 200 Hz
static const float GYRO_LSB_TO_RADS = (1.0f / 65.5f) * 3.14159265f / 180.0f;
static const float ACC_LSB_TO_MS2 = 9.80665f / 8192.0f;

bool ImuMpu6050::writeReg(uint8_t reg, uint8_t val) {
  wire_->beginTransmission(addr_);
  wire_->write(reg);
  wire_->write(val);
  return wire_->endTransmission() == 0;
}

bool ImuMpu6050::begin(TwoWire* wire, uint8_t addr) {
  wire_ = wire;
  addr_ = addr;
  wire_->begin();
  wire_->setClock(400000);

  wire_->beginTransmission(addr_);
  wire_->write(0x75);  // WHO_AM_I
  if (wire_->endTransmission() != 0) return false;
  if (wire_->requestFrom((int)addr_, 1) != 1) return false;
  who_ = wire_->read();  // 正品 0x68; 部分兼容片为 0x70/0x72/0x98, 寄存器相同即可用

  bool ok = true;
  ok &= writeReg(0x6B, 0x80);  // 复位
  delay(100);
  ok &= writeReg(0x6B, 0x01);  // 唤醒, 时钟取 X 轴陀螺 PLL
  ok &= writeReg(0x19, 0x04);  // SMPLRT_DIV: 1kHz/(1+4) = 200Hz
  ok &= writeReg(0x1A, 0x03);  // CONFIG: DLPF 44Hz
  ok &= writeReg(0x1B, 0x08);  // GYRO_CONFIG: ±500 dps
  ok &= writeReg(0x1C, 0x08);  // ACCEL_CONFIG: ±4 g
  delay(50);
  return ok;
}

bool ImuMpu6050::readRaw(int16_t a[3], int16_t g[3]) {
  wire_->beginTransmission(addr_);
  wire_->write(0x3B);  // ACCEL_XOUT_H 起连续 14 字节
  if (wire_->endTransmission() != 0) return false;
  if (wire_->requestFrom((int)addr_, 14) != 14) return false;
  uint8_t b[14];
  for (int i = 0; i < 14; ++i) b[i] = wire_->read();
  for (int i = 0; i < 3; ++i) {
    a[i] = (int16_t)((b[2 * i] << 8) | b[2 * i + 1]);
    g[i] = (int16_t)((b[8 + 2 * i] << 8) | b[8 + 2 * i + 1]);
  }
  return true;
}

void ImuMpu6050::calibrateGyro(int n) {
  double sum[3] = {0, 0, 0};
  int got = 0;
  int16_t a[3], g[3];
  for (int k = 0; k < n; ++k) {
    if (readRaw(a, g)) {
      for (int i = 0; i < 3; ++i) sum[i] += g[i];
      ++got;
    }
    delay(5);
  }
  if (got > 0)
    for (int i = 0; i < 3; ++i) gyro_bias_[i] = (float)(sum[i] / got);
}

bool ImuMpu6050::read(float gyro[3], float acc[3]) {
  int16_t a[3], g[3];
  if (!readRaw(a, g)) return false;
  float gc[3], ac[3];
  for (int i = 0; i < 3; ++i) {
    gc[i] = ((float)g[i] - gyro_bias_[i]) * GYRO_LSB_TO_RADS;
    ac[i] = (float)a[i] * ACC_LSB_TO_MS2;
  }
  for (int i = 0; i < 3; ++i) {  // 芯片坐标 -> 机身坐标 (x 前, y 左, z 上)
    gyro[i] = IMU_AXIS_SIGN[i] * gc[IMU_AXIS_MAP[i]];
    acc[i] = IMU_AXIS_SIGN[i] * ac[IMU_AXIS_MAP[i]];
  }
  return true;
}
