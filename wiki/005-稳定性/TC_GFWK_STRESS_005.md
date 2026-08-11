---
title: "TC_GFWK_STRESS_005 — 环境光/自动亮度压测"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - GFWK
  - 自动亮度
  - 环境光
platform: "gua / guav100 (AAOS)"
优先级: P2
维度: 压力(D3)
状态: 10/10 通过(2026-08-04, SG0286, 任务95603) — 附自动亮度未跟随观察
created: 2026-08-04
---

# TC_GFWK_STRESS_005 — 环境光/自动亮度压测

> 上级 [[GFWK 稳定性测试用例全量清单]] · 脚本来源 `simulate_ambient_light.sh`（GTMP 化）· 栈层 显示管线/背光
> 代码 `autocase/cases/MultiMedia/GPU/Stress/TC_GFWK_STRESS_005.py`（branch `qi.zhu`）

## 目标
反复经 VHAL inject-event 注入环境光 lux 阶梯（全黑→强光→回落），压**自动亮度调节链路**（环境光 sensor → 亮度策略 → 背光/DPU），验反复变光下亮度跟随、主屏不黑、不崩、不泄漏。

## 从 shell 脚本到 GTMP 用例

| `simulate_ambient_light.sh` | GTMP 用例 |
|---|---|
| 无限循环 + Ctrl+C 手停 | **有界轮次**（`AMBIENT_ROUNDS`，默认 10） |
| 只注入、打印成功/失败 | 注入后**采屏幕亮度序列 + 逐步判主屏非黑** |
| 无判定 | **三态断言**（崩溃/黑屏/亮度卡死/墓碑）+ adb 抖动防护 |
| cycle/ramp/random/list 模式 | lux 阶梯经 `AMBIENT_LUX` env 覆盖 |

## 四阶段

| 阶段 | 动作 |
|---|---|
| 1 基线 | 记墓碑 |
| 2 注入 | 扫 lux 阶梯 `0,10,...,100000` 升到顶再回落（VHAL prop `0x2140021A -i <lux>`） |
| 3 采集 | 每步读屏幕亮度（`dumpsys display` mBrightness / `settings get system screen_brightness`）+ 判主屏非黑 |
| 三态断言 | 不掉线；变光全程主屏不黑；(可选 `AMBIENT_STRICT=1`)扫描区间亮度极差 ≥ 阈值证自动亮度未卡死；无 double-free 墓碑 |

## 关键 env

| env | 默认 | 说明 |
|---|---|---|
| `AMBIENT_ROUNDS` | 10 | 轮次 |
| `AMBIENT_LUX` | `0,10,50,...,100000` | lux 阶梯 |
| `AMBIENT_INTERVAL` | 1 | 每步间隔秒 |
| `AMBIENT_PROP` | `0x2140021A` | 环境光 VHAL property |
| `AMBIENT_STRICT` / `AMBIENT_BRIGHT_DELTA` | 0 / 20 | 严格模式 + 亮度极差阈值 |

## 运行 / 回归
```bash
AMBIENT_ROUNDS=1 pytest cases/MultiMedia/GPU/Stress/TC_GFWK_STRESS_005.py --bench=<yaml> -v
```
- **2026-08-04 SG0286 任务 95603：10/10 PASS**。10 轮每轮采样 24 点，崩溃=0 黑屏=0 double-free=0，无崩溃/花屏。
  > ⚠️ **观察（非失败）**：亮度全程恒 **127（极差=0）**，注入 lux `0→100000` 屏幕亮度未变化。STRICT 关故未判失败。三种可能：① 台架自动亮度**未开启**（127 为固定手动亮度，注入无效，属预期）；② 亮度读取口径读的是手动设置非有效背光；③ 环境光 property `0x2140021A` 未接到亮度策略（潜在真缺口）。**下一步**：确认设备自动亮度开关状态，开启后再跑 `AMBIENT_STRICT=1` 复验是否跟随。

## 说明
- 严格亮度断言默认关闭：不同 SKU/设置下自动亮度可能被关或用 float 口径，硬断言易误报；需强验时 `AMBIENT_STRICT=1` 并按平台调 `AMBIENT_BRIGHT_DELTA`。
- 亮度读取多源兜底并把 float(0~1) 归一到 0-255。

## 📚 延伸阅读
- Android 自动亮度（Adaptive/Automatic brightness）：https://source.android.com/docs/core/display/auto-brightness
- Car UI / 车载 HMI 亮度：https://source.android.com/docs/automotive/hmi
- VehicleProperty（车辆属性注入）：https://source.android.com/docs/automotive/vhal

## 关联
- 同族 [[TC_GFWK_STRESS_004]]（后排屏开合）· 机制 [[后排屏]]
- 总览 [[000-GFWK图形框架总览]] · 映射 [[06-GFWK如何映射到测试用例]]
