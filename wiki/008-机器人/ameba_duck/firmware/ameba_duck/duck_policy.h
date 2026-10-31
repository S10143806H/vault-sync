// duck_policy.h — Open Duck Mini v2 行走策略 (MLP) 的纯 C/C++ 推理, 无第三方依赖。
// 权重来自 tools/export_policy.py 生成的 policy_weights.h。
#pragma once
#include <stdint.h>

#define DUCK_OBS_DIM 101
#define DUCK_ACT_DIM 14

// 初始化。AMB82-mini 上传 false: 其链接脚本把 .rodata 放在 128MB DDR(开机由 flash 载入),
// 权重常量本来就在 DDR, 无需拷贝; 而 FreeRTOS 堆只有 120KB, 拷贝反而会失败。
// copy_to_ram=true 仅用于 PC 主机测试或权重在 XIP flash 的其他 MCU。返回 false 表示维度不符。
bool duck_policy_init(bool copy_to_ram);

// obs[101] -> act[14], act 取值范围 [-1, 1]
void duck_policy_infer(const float* obs, float* act);

// 用 policy_selftest.h 中 onnxruntime 生成的向量自检, 返回最大绝对误差。
float duck_policy_selftest();
