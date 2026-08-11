---
title: "TC_GFWK_STR_005 — 综合 (GPU+Disp+跨SoC+Audio) × STR"
tags: [稳定性, AAOS, 测试用例, STR, 跨SoC, 综合]
case_id: TC_GFWK_STR_005
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# TC_GFWK_STR_005 — 综合 × STR（P0）

> §3.5，**P0**。代码：`cases/MultiMedia/GPU/Str/TC_GFWK_STR_005.py`。方法论 → [[GFWK STR 挂起唤醒稳定性测试]]。

**一句话**：**GPU 高负载(+可选 Audio)常驻**下反复 [[STR]] 挂起-唤醒，压**多子系统同时唤醒的竞态**——验 [[SurfaceFlinger|SF]]+[[GIPC]] 重连、逐屏(含后排)非黑、GPU 负载存活、A720 无崩。

## 侧重 / bug 假设
多子系统（GPU 渲染 + 显示 + 跨SoC 投屏 + Audio）**同时唤醒竞态** → 黑屏/花屏/崩溃/投屏丢/GPU 负载被杀。这是 STR 最全的一条，最容易在竞态下暴露问题。

## 断言（三态，跨SoC 用 `A720CNT=` 抗串口噪声）
| 断言 | 判据 |
|---|---|
| 不崩/能醒 | `wait-for-device` 唤醒不掉线 |
| 恢复 | SF + `gipc_sdd` + `cluster-service` <`RESUME_SLA` 重连 |
| 画面 | 逐屏（含后排）非黑 |
| 负载 | GPU 负载进程唤醒后仍存活 |
| 跨SoC | a720 dmesg 无新 composer_stub segfault/fence timeout/GIPC panic |

## 如何自测（需 a720 串口）
```bash
source ~/.virtualenvs/py312/bin/activate; cd ~/Documents/autocase
STR_005_ROUNDS=3 pytest cases/MultiMedia/GPU/Str/TC_GFWK_STR_005.py --bench=/home/gua/data/apps/gtmp-client/data/config/ECU_V27_10.78.20.5_gua-SG0286.yaml -v
# 用 GFXBench 当负载: GFX_ACTIVITY=<pkg/act> STR_005_ROUNDS=3 pytest ...
# 加音频负载: AUDIO_ACT=<pkg/act> ...
```
> GPU 负载默认 `GpuBaseAw.stress_gpu_load()`（glmark2 offscreen）；无 glmark2 时会告警继续（负载=none）。

## env
`STR_005_ROUNDS`(200) · `STR_MODE`(display/mem) · `STR_005_SUSPEND`(5) · `STR_005_RESUME_SLA`(30) · `GFX_ACTIVITY`(切 GFXBench) · `AUDIO_ACT`(可选音频)

## 关联
- 方法 → [[GFWK STR 挂起唤醒稳定性测试]]｜样板 → [[TC_CSOC_RECOVER_001]]（=STR_003）｜概念 → [[GIPC]]｜[[Fence]]｜[[composer_stub]]｜清单 → [[GFWK 稳定性测试用例全量清单]]
