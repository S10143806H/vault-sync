// duck_policy.cpp — 见 duck_policy.h
#include "duck_policy.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "policy_weights.h"
#include "policy_selftest.h"

#if POLICY_OBS_DIM != DUCK_OBS_DIM || POLICY_ACT_DIM != DUCK_ACT_DIM
#error "policy_weights.h 的维度与 duck_policy.h 不一致, 请检查导出的 ONNX"
#endif

static const float* s_w[POLICY_NUM_LAYERS];
static const float* s_b[POLICY_NUM_LAYERS];
// 中间结果用静态缓冲(2 x 2KB), 不占 FreeRTOS 堆(AmebaPro2 只有 120KB)
static float s_buf_a[POLICY_MAX_DIM];
static float s_buf_b[POLICY_MAX_DIM];

static const float* dup_to_ram(const float* src, size_t n) {
  float* dst = (float*)malloc(n * sizeof(float));
  if (!dst) return nullptr;
  memcpy(dst, src, n * sizeof(float));
  return dst;
}

bool duck_policy_init(bool copy_to_ram) {
  if (POLICY_OBS_DIM != DUCK_OBS_DIM) return false;
  if (POLICY_DIMS[POLICY_NUM_LAYERS] != 2 * DUCK_ACT_DIM) return false;

  for (int l = 0; l < POLICY_NUM_LAYERS; ++l) {
    const size_t n_in = POLICY_DIMS[l], n_out = POLICY_DIMS[l + 1];
    s_w[l] = POLICY_W[l];
    s_b[l] = POLICY_B[l];
    if (copy_to_ram) {
      const float* w = dup_to_ram(POLICY_W[l], n_in * n_out);
      const float* b = dup_to_ram(POLICY_B[l], n_out);
      if (w) s_w[l] = w;  // 拷贝失败就退回直接读 flash, 慢一些但能跑
      if (b) s_b[l] = b;
    }
  }
  return true;
}

// y[out] = b + x[in] @ W[in][out]; 外层按输入遍历, 让权重顺序访问(对缓存友好)
static void dense(const float* x, int n_in, const float* w, const float* b, int n_out,
                  float* y) {
  memcpy(y, b, n_out * sizeof(float));
  for (int i = 0; i < n_in; ++i) {
    const float xi = x[i];
    const float* wr = w + (size_t)i * n_out;
    for (int j = 0; j < n_out; ++j) y[j] += xi * wr[j];
  }
}

static inline float silu(float v) { return v / (1.0f + expf(-v)); }

void duck_policy_infer(const float* obs, float* act) {
  float* x = s_buf_a;
  float* y = s_buf_b;
  for (int i = 0; i < POLICY_OBS_DIM; ++i)
    x[i] = (obs[i] - POLICY_OBS_MEAN[i]) * POLICY_OBS_INV_STD[i];

  for (int l = 0; l < POLICY_NUM_LAYERS; ++l) {
    const int n_in = POLICY_DIMS[l], n_out = POLICY_DIMS[l + 1];
    dense(x, n_in, s_w[l], s_b[l], n_out, y);
    if (l < POLICY_NUM_LAYERS - 1)
      for (int j = 0; j < n_out; ++j) y[j] = silu(y[j]);
    float* t = x; x = y; y = t;  // 交换缓冲
  }
  // x 现在是最后一层输出 [28]: 前 14 维是动作均值, 过 tanh
  for (int j = 0; j < DUCK_ACT_DIM; ++j) act[j] = tanhf(x[j]);
}

float duck_policy_selftest() {
  float act[DUCK_ACT_DIM];
  float max_err = 0.f;
  for (int n = 0; n < SELFTEST_N; ++n) {
    duck_policy_infer(&SELFTEST_OBS[n * DUCK_OBS_DIM], act);
    for (int j = 0; j < DUCK_ACT_DIM; ++j) {
      float e = fabsf(act[j] - SELFTEST_ACT[n * DUCK_ACT_DIM + j]);
      if (e > max_err) max_err = e;
    }
  }
  return max_err;
}
