---
title: "TC_GFWK_STRESS_004 — 后排屏开合压测"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - GFWK
  - 后排屏
  - HWC
platform: "gua / guav100 (AAOS)"
优先级: P2
维度: 压力(D3)
状态: smoke 通过(2026-08-04, SG0286, 任务95599, 10轮)
created: 2026-08-04
---

# TC_GFWK_STRESS_004 — 后排屏开合压测

> 上级 [[GFWK 稳定性测试用例全量清单]] · 机制来源 [[后排屏]] · 栈层 [[HWC]]（第 3 屏 DPU 通路）
> 代码 `autocase/cases/MultiMedia/GPU/Stress/TC_GFWK_STRESS_004.py`（branch `qi.zhu`）

## 目标
反复经 VHAL inject-event 开/关后排屏（HWC 第 3 屏），压 DPU/HWC display 热插拔与后排背光通路，验反复开合后仍能可靠点亮/熄灭、不崩、不泄漏。

## 四阶段

| 阶段 | 动作 |
|---|---|
| 1 基线 | 先开屏一次确认**后排背光路径可读**（读不到则 skip 防误判），记墓碑 |
| 2 关屏 | `THIRD_SCREEN_OFF`(0x324e) → 等后排背光归 **0** |
| 3 开屏 | `THIRD_SCREEN_ON_WITHOUT_CANN`(0x344c) → 等后排背光 **>0** |
| 三态断言 | 不掉线/不进 ramdump；开<SLA 背光>0 且 关<SLA 背光=0；无 double-free 墓碑 |

## 判据说明（踩坑记录）
- **错口径（首版失败）**：用 `dumpsys display` 的 `mDisplayId=2` 是否消失判关屏 → **v1 未通过 0/1**。`mDisplayId=2` 只表示后排屏**已枚举/挂载**，开关屏后恒在，不反映上电态（"点亮 0.0s"即旁证）。
- **正确口径（v2 通过）**：用**后排背光** `cat sys/class/backlight/dp2_panel0/brightness`（复用 [[Gralloc|common.Display]] `check_rear_brightness` 口径），**>0=亮、0=灭**。
- 教训：显示类用例**先验证功能信号（背光/截图）再写断言**，别拿"枚举/挂载"当"上电"。

## 平台适配（V27 / T29 自动探测）
先 `getprop persist.gua.car.model` 判车型，再注入对应命令（命令详情见 [[后排屏]]）：

| 平台 | 开/关命令 | 验证方式 |
|---|---|---|
| **V27**（本台架） | VHAL inject-event `0x344c`开 / `0x324e`关 | **背光严格判**（>0/=0），已验 10/10 |
| **T29** | `am broadcast OPEN/CLOSE_REAR_SCREEN`（电机开合屏） | **best-effort**：下发+设备存活+不崩（背光/角度口径待 T29 台架确认后严格化） |

> 探测未识别时默认按 V27；`REAR_PLATFORM=V27\|T29` 可强制覆盖。V27 路径逻辑不变。
> 本台架为 V27，故 T29 分支只保证结构正确（探测→注入对应命令），亮灭态未在真机验证。

## 关键 env

| env | 默认 | 说明 |
|---|---|---|
| `REAR_TOGGLE_ROUNDS` | 10 | 轮次 |
| `REAR_PLATFORM` | 自动探测 | 强制平台 `V27`/`T29`（空=按 `persist.gua.car.model` 探测） |
| `REAR_BL_PATH` | `sys/class/backlight/dp2_panel0/brightness` | 后排背光路径 |
| `REAR_ON_CMD` / `REAR_OFF_CMD` | general_settings 常量 | 覆盖开/关命令（带 CAN 用 `THIRD_SCREEN_ON_WITH_CANN` 0x314f） |
| `REAR_LIT_SLA` / `REAR_OFF_SLA` | 6 | 点亮/熄灭 SLA |

## 运行 / 回归
```bash
REAR_TOGGLE_ROUNDS=1 pytest cases/MultiMedia/GPU/Stress/TC_GFWK_STRESS_004.py --bench=<yaml> -v
```
- **2026-08-04 SG0286 任务 95599：10/10 PASS**。基线背光 132；每轮 关→背光 0(0.0–0.1s)、开→背光 132(0.0s)；崩溃=0 开屏失败=0 关屏失败=0 double-free=0。
- **2026-08-04 SG0286 任务 95617（复跑）：10/10 PASS**。二次确认稳定，与 95599 一致。
- **2026-08-04 SG0286 任务 95638（平台探测回归）：10/10 PASS**。日志确认 `平台=V27 验证模式=backlight`，探测正确、注入 VHAL 命令，新增 T29/V27 分支未破坏 V27 路径。

## 📚 延伸阅读
- Android 多屏显示（Multi-display）：https://source.android.com/docs/core/display/multi_display
- 汽车多屏/乘客屏：https://source.android.com/docs/automotive/hmi/multi_display
- Linux backlight sysfs：https://docs.kernel.org/gpu/backlight.html

## 关联
- 机制/命令来源 [[后排屏]] · 同族 [[TC_GFWK_STRESS_005]]（环境光/自动亮度）
- 判黑/多屏方法 [[000-GFWK图形框架总览]] · 映射 [[06-GFWK如何映射到测试用例]]
