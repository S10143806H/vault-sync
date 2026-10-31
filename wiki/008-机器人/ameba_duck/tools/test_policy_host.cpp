// 主机端单测: 用 g++ 编译固件里的 duck_policy.cpp, 与 onnxruntime 期望输出对拍并测时。
// 运行: tools/run_host_tests.sh
#include <chrono>
#include <cstdio>

#include "duck_policy.h"

int main() {
  if (!duck_policy_init(true)) {
    std::printf("FAIL: duck_policy_init\n");
    return 1;
  }
  float err = duck_policy_selftest();
  std::printf("policy selftest max abs err vs onnxruntime = %.3e\n", err);

  float obs[DUCK_OBS_DIM] = {0};
  float act[DUCK_ACT_DIM];
  const int N = 2000;
  auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    obs[i % DUCK_OBS_DIM] += 1e-3f;
    duck_policy_infer(obs, act);
  }
  auto t1 = std::chrono::steady_clock::now();
  double us = std::chrono::duration<double, std::micro>(t1 - t0).count() / N;
  std::printf("host inference: %.1f us/step (x86, 仅作参考)\n", us);

  if (err > 1e-4f) {
    std::printf("FAIL: error too large\n");
    return 1;
  }
  std::printf("PASS\n");
  return 0;
}
