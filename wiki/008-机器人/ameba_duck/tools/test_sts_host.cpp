// 主机端单测: sts_bus 报文与 rustypot 测试向量逐字节对比 + 虚拟 STS3215 总线端到端测试。
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>

#include "sts_bus.h"

static int g_fail = 0;
#define CHECK(cond, msg)                         \
  do {                                           \
    if (!(cond)) {                               \
      std::printf("FAIL: %s\n", msg);            \
      ++g_fail;                                  \
    } else {                                     \
      std::printf("ok  : %s\n", msg);            \
    }                                            \
  } while (0)

// ---------- 虚拟舵机 + 虚拟总线 ----------
struct VirtualServo {
  uint8_t mem[256] = {0};
  explicit VirtualServo(uint8_t id) {
    mem[sts::REG_ID] = id;
    mem[sts::REG_P_COEF] = 32;
    mem[sts::REG_PRESENT_VOLTAGE] = 78;  // 7.8V
  }
  uint8_t id() const { return mem[sts::REG_ID]; }
};

struct VirtualBus : sts::StsPort {
  std::vector<VirtualServo> servos;
  std::vector<uint8_t> tx;
  std::deque<uint8_t> rx;
  uint32_t t_us = 0;
  bool echo = false;
  int drop_id = -1;  // 模拟某个舵机掉线

  void write(const uint8_t* b, size_t n) override { tx.insert(tx.end(), b, b + n); }
  void clearRx() override { rx.clear(); }
  int readByte() override {
    t_us += 10;  // 1 Mbps 下 1 字节 = 10us
    if (rx.empty()) return -1;
    int c = rx.front();
    rx.pop_front();
    return c;
  }
  uint32_t micros() override { return t_us; }
  void flushTx() override {
    if (echo) rx.insert(rx.end(), tx.begin(), tx.end());
    process(tx);
    tx.clear();
  }

  VirtualServo* find(uint8_t id) {
    for (auto& s : servos)
      if (s.id() == id && id != drop_id) return &s;
    return nullptr;
  }
  void status(uint8_t id, const uint8_t* p, size_t n) {
    uint8_t out[300];
    out[0] = 0xFF; out[1] = 0xFF; out[2] = id; out[3] = (uint8_t)(n + 2); out[4] = 0;
    if (n) std::memcpy(out + 5, p, n);
    out[5 + n] = sts::checksum(out + 2, 3 + n);
    rx.insert(rx.end(), out, out + 6 + n);
  }
  void process(const std::vector<uint8_t>& pkt) {
    if (pkt.size() < 6 || pkt[0] != 0xFF || pkt[1] != 0xFF) return;
    uint8_t id = pkt[2], len = pkt[3], ins = pkt[4];
    if (pkt.size() != (size_t)len + 4) return;
    if (pkt.back() != sts::checksum(&pkt[2], pkt.size() - 3)) return;
    const uint8_t* p = &pkt[5];
    size_t np = len - 2;
    if (ins == sts::INST_PING) {
      if (find(id)) status(id, nullptr, 0);
    } else if (ins == sts::INST_READ) {
      if (auto* s = find(id)) status(id, &s->mem[p[0]], p[1]);
    } else if (ins == sts::INST_WRITE) {
      if (auto* s = find(id)) {
        std::memcpy(&s->mem[p[0]], p + 1, np - 1);
        status(s->id(), nullptr, 0);
      }
    } else if (ins == sts::INST_SYNC_WRITE) {
      uint8_t addr = p[0], l = p[1];
      for (size_t i = 2; i + 1 + l <= np; i += 1 + l)
        if (auto* s = find(p[i])) std::memcpy(&s->mem[addr], p + i + 1, l);
    } else if (ins == sts::INST_SYNC_READ) {
      uint8_t addr = p[0], l = p[1];
      for (size_t i = 2; i < np; ++i)
        if (auto* s = find(p[i])) status(s->id(), &s->mem[addr], l);
    }
  }
};

int main() {
  // 1) 与 rustypot v1 协议单测向量逐字节比对
  {
    uint8_t out[32];
    const uint8_t params[] = {0x1E, 0x02, 0x0B, 0x0C};
    size_t n = sts::buildPacket(out, 0xFE, sts::INST_SYNC_READ, params, sizeof(params));
    const uint8_t expect[] = {0xFF, 0xFF, 0xFE, 0x06, 0x82, 0x1E, 0x02, 0x0B, 0x0C, 0x42};
    CHECK(n == sizeof(expect) && std::memcmp(out, expect, n) == 0, "sync_read 报文 == rustypot 向量");
  }
  {
    uint8_t out[32];
    const uint8_t params[] = {0x1E, 0x02, 0x0B, 0x00, 0x00, 0x0C, 0x0A, 0x14};
    size_t n = sts::buildPacket(out, 0xFE, sts::INST_SYNC_WRITE, params, sizeof(params));
    const uint8_t expect[] = {0xFF, 0xFF, 0xFE, 0x0A, 0x83, 0x1E, 0x02, 0x0B,
                              0x00, 0x00, 0x0C, 0x0A, 0x14, 0x1F};
    CHECK(n == sizeof(expect) && std::memcmp(out, expect, n) == 0, "sync_write 报文 == rustypot 向量");
  }
  // 2) 单位换算与 rustypot 一致
  CHECK(sts::radToRaw(0.0f) == 2048, "0 rad -> 2048");
  CHECK(std::fabs(sts::rawToRad(2048)) < 1e-6f, "2048 -> 0 rad");
  CHECK(std::fabs(sts::rawToRad(sts::radToRaw(1.368f)) - 1.368f) < 2e-3f, "rad<->raw 往返误差 < 1 步");
  CHECK(std::fabs(sts::rawSpeedToRadS(0x8000 | 100) + 2 * 3.14159265f * 100 / 4095) < 1e-5f,
        "速度符号位(bit15)解析");

  // 3) 虚拟总线端到端: 14 个舵机, ID 与 Open Duck Mini v2 相同
  const uint8_t ids[14] = {20, 21, 22, 23, 24, 30, 31, 32, 33, 10, 11, 12, 13, 14};
  for (bool echo : {false, true}) {
    VirtualBus vb;
    vb.echo = echo;
    for (uint8_t id : ids) vb.servos.emplace_back(id);
    sts::Bus bus;
    bus.begin(&vb, echo);
    const char* tag = echo ? "[单线回环]" : "[方向脚/自动方向]";
    char msg[128];

    std::snprintf(msg, sizeof msg, "%s ping id=22", tag);
    CHECK(bus.ping(22), msg);
    std::snprintf(msg, sizeof msg, "%s ping 不存在的 id=99 返回 false", tag);
    CHECK(!bus.ping(99), msg);

    uint8_t v = 0;
    std::snprintf(msg, sizeof msg, "%s 读电压寄存器", tag);
    CHECK(bus.readByte(23, sts::REG_PRESENT_VOLTAGE, &v) && v == 78, msg);

    float tgt[14], pos[14], vel[14];
    bool ok[14];
    for (int i = 0; i < 14; ++i) tgt[i] = -1.0f + 0.15f * i;
    bus.syncWritePositions(ids, 14, tgt);
    // 让虚拟舵机"到位": present = goal, speed = -37 step/s
    for (auto& s : vb.servos) {
      s.mem[sts::REG_PRESENT_POSITION] = s.mem[sts::REG_GOAL_POSITION];
      s.mem[sts::REG_PRESENT_POSITION + 1] = s.mem[sts::REG_GOAL_POSITION + 1];
      s.mem[sts::REG_PRESENT_SPEED] = 37;
      s.mem[sts::REG_PRESENT_SPEED + 1] = 0x80;
    }
    int good = bus.syncReadPosVel(ids, 14, pos, vel, ok);
    float max_err = 0;
    for (int i = 0; i < 14; ++i) max_err = std::fmax(max_err, std::fabs(pos[i] - tgt[i]));
    std::snprintf(msg, sizeof msg, "%s sync 写 14 路目标 -> sync 读回, 14/14 应答, 误差 %.4f rad", tag, max_err);
    CHECK(good == 14 && max_err < 2e-3f, msg);
    std::snprintf(msg, sizeof msg, "%s 速度读回为负值", tag);
    CHECK(vel[0] < 0 && std::fabs(vel[0] + 2 * 3.14159265f * 37 / 4095) < 1e-5f, msg);

    vb.drop_id = 13;  // right_knee 掉线
    good = bus.syncReadPosVel(ids, 14, pos, vel, ok);
    std::snprintf(msg, sizeof msg, "%s 一个舵机掉线时其余 13 个仍可读, 且能定位掉线 ID", tag);
    CHECK(good == 13 && !ok[12] && ok[13], msg);
  }

  // 4) 1 Mbps 下的总线时间预算(纯计算)
  {
    const int n = 14;
    int sw_bytes = 6 + 2 + n * 3;             // sync write 位置
    int sr_req = 6 + 2 + n;                   // sync read 请求
    int sr_resp = n * (6 + 4);                // 每个舵机应答 10 字节
    double us = (sw_bytes + sr_req + sr_resp) * 10.0;  // 10 bit/字节 @1Mbps
    std::printf("info: 每控制周期总线字节 = %d, 纯传输 %.0f us (不含舵机应答延时)\n",
                sw_bytes + sr_req + sr_resp, us);
  }

  if (g_fail) {
    std::printf("%d CHECK(S) FAILED\n", g_fail);
    return 1;
  }
  std::printf("PASS\n");
  return 0;
}
