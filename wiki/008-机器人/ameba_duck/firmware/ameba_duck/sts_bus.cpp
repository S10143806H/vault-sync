// sts_bus.cpp — 见 sts_bus.h
#include "sts_bus.h"

#include <math.h>
#include <string.h>

namespace sts {

static const float kPi = 3.14159265358979f;

float rawToRad(uint16_t raw) {
  int v = (raw & 0x8000) ? -(int)(raw & 0x7FFF) : (int)raw;  // sign-magnitude(bit15)
  return 2.0f * kPi * (float)v / 4096.0f - kPi;
}

uint16_t radToRaw(float rad) {
  float v = 4096.0f * (kPi + rad) / (2.0f * kPi);
  if (v < 0.0f) v = 0.0f;
  if (v > 4095.0f) v = 4095.0f;
  return (uint16_t)v;  // 截断, 与 rustypot `as i16` 一致
}

float rawSpeedToRadS(uint16_t raw) {
  int v = (raw & 0x8000) ? -(int)(raw & 0x7FFF) : (int)raw;
  return 2.0f * kPi * (float)v / 4095.0f;
}

uint8_t checksum(const uint8_t* from_id, size_t n) {
  uint32_t s = 0;
  for (size_t i = 0; i < n; ++i) s += from_id[i];
  return (uint8_t)(~s & 0xFF);
}

size_t buildPacket(uint8_t* out, uint8_t id, uint8_t instr, const uint8_t* params,
                   size_t nparams) {
  out[0] = 0xFF;
  out[1] = 0xFF;
  out[2] = id;
  out[3] = (uint8_t)(nparams + 2);
  out[4] = instr;
  if (nparams) memcpy(out + 5, params, nparams);
  out[5 + nparams] = checksum(out + 2, 3 + nparams);
  return 6 + nparams;
}

void Bus::begin(StsPort* port, bool echo, uint32_t timeout_us) {
  port_ = port;
  echo_ = echo;
  timeout_us_ = timeout_us;
}

void Bus::send(const uint8_t* pkt, size_t n) {
  port_->clearRx();
  port_->setTxMode(true);
  port_->write(pkt, n);
  port_->flushTx();
  port_->setTxMode(false);
  if (echo_) {  // 单线回环: 先把自己发出的 n 个字节吃掉
    uint32_t t0 = port_->micros();
    size_t got = 0;
    while (got < n && (port_->micros() - t0) < timeout_us_) {
      if (port_->readByte() >= 0) ++got;
    }
  }
}

int Bus::receiveStatus(uint8_t expect_id, uint8_t* params, uint8_t max_params,
                       uint32_t timeout_us) {
  uint8_t id = 0;
  int np = receiveAny(&id, params, max_params, timeout_us);
  if (np < 0) return -1;
  if (id != expect_id) { ++errors_; return -1; }
  return np;
}

int Bus::receiveAny(uint8_t* id_out, uint8_t* params, uint8_t max_params,
                    uint32_t timeout_us) {
  // 状态机: 找 FF FF -> ID -> LEN -> ERR -> PARAMS -> CHK
  uint8_t buf[260];
  size_t n = 0;
  size_t need = 4;  // 先收到 LEN 为止
  uint32_t t0 = port_->micros();
  while ((port_->micros() - t0) < timeout_us) {
    int c = port_->readByte();
    if (c < 0) continue;
    buf[n++] = (uint8_t)c;
    if (n == 1 && buf[0] != 0xFF) { n = 0; continue; }
    if (n == 2 && buf[1] != 0xFF) { n = 0; continue; }
    if (n == 3 && buf[2] == 0xFF) { n = 2; continue; }  // 多余的 FF 前导
    if (n == 4) {
      if (buf[3] < 2) { n = 0; continue; }
      need = 4 + buf[3];
    }
    if (n >= 4 && n == need) {
      if (buf[n - 1] != checksum(buf + 2, n - 3)) { ++errors_; return -1; }
      *id_out = buf[2];
      last_servo_err_ = buf[4];
      int np = (int)buf[3] - 2;
      if (np > max_params) np = max_params;
      if (np > 0 && params) memcpy(params, buf + 5, np);
      return np;
    }
    if (n >= sizeof(buf)) n = 0;
  }
  ++errors_;
  return -1;
}

bool Bus::ping(uint8_t id) {
  uint8_t pkt[8];
  size_t n = buildPacket(pkt, id, INST_PING, nullptr, 0);
  send(pkt, n);
  return receiveStatus(id, nullptr, 0, timeout_us_) >= 0;
}

bool Bus::write(uint8_t id, uint8_t addr, const uint8_t* data, uint8_t len) {
  uint8_t params[64];
  if (len > sizeof(params) - 1) return false;
  params[0] = addr;
  memcpy(params + 1, data, len);
  uint8_t pkt[80];
  size_t n = buildPacket(pkt, id, INST_WRITE, params, 1 + len);
  send(pkt, n);
  if (id == BROADCAST_ID) return true;
  return receiveStatus(id, nullptr, 0, timeout_us_) >= 0;
}

bool Bus::writeWord(uint8_t id, uint8_t addr, uint16_t v) {
  uint8_t d[2] = {(uint8_t)(v & 0xFF), (uint8_t)(v >> 8)};  // 小端
  return write(id, addr, d, 2);
}

bool Bus::read(uint8_t id, uint8_t addr, uint8_t len, uint8_t* out) {
  uint8_t params[2] = {addr, len};
  uint8_t pkt[8];
  size_t n = buildPacket(pkt, id, INST_READ, params, 2);
  send(pkt, n);
  return receiveStatus(id, out, len, timeout_us_) == len;
}

bool Bus::readWord(uint8_t id, uint8_t addr, uint16_t* v) {
  uint8_t d[2];
  if (!read(id, addr, 2, d)) return false;
  *v = (uint16_t)(d[0] | (d[1] << 8));
  return true;
}

void Bus::syncWrite(const uint8_t* ids, uint8_t n, uint8_t addr, const uint8_t* data,
                    uint8_t len_each) {
  uint8_t params[2 + 32 * 5];
  size_t np = 0;
  params[np++] = addr;
  params[np++] = len_each;
  for (uint8_t i = 0; i < n && np + 1 + len_each <= sizeof(params); ++i) {
    params[np++] = ids[i];
    memcpy(params + np, data + i * len_each, len_each);
    np += len_each;
  }
  uint8_t pkt[sizeof(params) + 6];
  size_t len = buildPacket(pkt, BROADCAST_ID, INST_SYNC_WRITE, params, np);
  send(pkt, len);  // 广播指令, 舵机不应答
}

int Bus::syncRead(const uint8_t* ids, uint8_t n, uint8_t addr, uint8_t len_each, uint8_t* out,
                  bool* ok) {
  uint8_t params[2 + 32];
  size_t np = 0;
  params[np++] = addr;
  params[np++] = len_each;
  for (uint8_t i = 0; i < n && np < sizeof(params); ++i) params[np++] = ids[i];
  uint8_t pkt[sizeof(params) + 6];
  size_t len = buildPacket(pkt, BROADCAST_ID, INST_SYNC_READ, params, np);
  send(pkt, len);
  // 舵机按 ID 列表顺序逐个应答; 某个舵机掉线时后面的照常应答,
  // 所以按应答包里的 ID 归位, 而不是假设第 k 个包一定属于 ids[k]
  for (uint8_t i = 0; i < n; ++i) ok[i] = false;
  int good = 0;
  uint8_t tmp[64];
  for (uint8_t k = 0; k < n; ++k) {
    uint8_t rid = 0;
    int r = receiveAny(&rid, tmp, sizeof(tmp), timeout_us_);
    if (r < 0) continue;  // 超时(该舵机掉线)或校验错, 继续等下一个
    for (uint8_t i = 0; i < n; ++i) {
      if (ids[i] == rid && !ok[i] && r == len_each) {
        memcpy(out + i * len_each, tmp, len_each);
        ok[i] = true;
        ++good;
        break;
      }
    }
    if (good == n) break;
  }
  return good;
}

void Bus::syncWritePositions(const uint8_t* ids, uint8_t n, const float* rad) {
  uint8_t data[32 * 2];
  for (uint8_t i = 0; i < n && i < 32; ++i) {
    uint16_t raw = radToRaw(rad[i]);
    data[2 * i] = (uint8_t)(raw & 0xFF);
    data[2 * i + 1] = (uint8_t)(raw >> 8);
  }
  syncWrite(ids, n, REG_GOAL_POSITION, data, 2);
}

int Bus::syncReadPosVel(const uint8_t* ids, uint8_t n, float* pos_rad, float* vel_rads,
                        bool* ok) {
  uint8_t data[32 * 4];
  // AmebaPro2 Serial2 接收环形缓冲只有 128 字节(RingBuffer.h SERIAL_BUFFER_SIZE),
  // 14 个舵机一次应答 140 字节会有溢出风险, 所以分批读, 每批 STS_SYNC_READ_CHUNK 个
  int good = 0;
  for (uint8_t s = 0; s < n; s += STS_SYNC_READ_CHUNK) {
    uint8_t m = (uint8_t)((n - s) < STS_SYNC_READ_CHUNK ? (n - s) : STS_SYNC_READ_CHUNK);
    good += syncRead(ids + s, m, REG_PRESENT_POSITION, 4, data + 4 * s, ok + s);  // 56..59: 位置+速度
  }
  for (uint8_t i = 0; i < n; ++i) {
    if (!ok[i]) continue;
    uint16_t p = (uint16_t)(data[4 * i] | (data[4 * i + 1] << 8));
    uint16_t v = (uint16_t)(data[4 * i + 2] | (data[4 * i + 3] << 8));
    pos_rad[i] = rawToRad(p);
    vel_rads[i] = rawSpeedToRadS(v);
  }
  return good;
}

}  // namespace sts
