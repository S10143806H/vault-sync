---
title: "BUG-STRESS006 仪表屏(Cluster)撕裂/花屏/闪屏"
tags:
  - 稳定性
  - GFWK
  - 缺陷
  - 花屏
  - 撕裂
  - 闪屏
  - 跨SoC
  - 仪表屏
platform: "gua / guav100 (AAOS)"
状态: 待提交(飞书链接待补)
created: 2026-08-07
---

# BUG-STRESS006 仪表屏(Cluster)撕裂/花屏/闪屏

> 飞书问题单链接：`<待补>`
> 来源用例 [[TC_GFWK_STRESS_006]] · GTMP 任务 97310 · 关联 [[SERDES链路]] / [[跨SoC]] / [[Weston]]

## 标题
`[GFWK][显示] 多屏图层压测(STRESS_006)下仪表屏(Cluster)撕裂/花屏/闪屏`

## 问题分类
花屏 + 撕裂(tearing) + 闪屏（Cluster/仪表屏，A720 侧）

## 严重级别 / 优先级
中（不崩，但影响仪表可视性；主屏正常）建议 P2；若行车中仪表撕裂可升 P1

## 测试环境
| 项 | 值 |
|---|---|
| 平台 | guav100 (AAOS) |
| 固件版本 | `<从 97310 日志 / getprop ro.build.display.id 补>` |
| 台架 / 设备 | bench 485 |
| 用例 | `cases/MultiMedia/GPU/Stress/TC_GFWK_STRESS_006.py`（分支 qi.zhu） |
| GTMP 任务 | 97310（子任务 129481） |
| 参数 | 默认（三屏 0,1,2 各 20 层=60 层，churn 80，含 chess/blur/corner） |

## 复现步骤
1. 台架安装 `MultiDisplayJavaDemo.apk`（平台签名）
2. 跑 `TC_GFWK_STRESS_006`（三屏高频 add/remove 图层 + rotate/flip/scale）
3. 运行中目视仪表屏（Cluster）

## 预期结果
三屏画面稳定，无撕裂/花屏/闪屏

## 实际结果
仪表屏（Cluster）出现**撕裂 / 花屏 / 闪屏**；IVI 主屏正常；用例断言全过（崩溃=0 恢复=0 主屏黑=0 double-free=0）——**自动化未能捕获该视觉缺陷**

## 出现概率
`<必现 / 偶现 X/Y，按实测填>`

## 初步分析（根因方向）
- Cluster 走 **A720 侧 Weston 合成 + 跨 SoC 通路（composer_stub→GIPC/SHMEM→Weston）+ [[SERDES链路]] 串行链路**，STRESS_006 对 #1(Cluster) 高频增删图层时该链路负载骤增。
- **撕裂** → A720 Weston 合成/上屏未与 Cluster panel VSync/tear-free 对齐，或 SERDES 帧不同步。
- **花屏** → 跨 SoC fence 丢失 / buffer 未就绪即上屏 / SERDES 误码-CRC。
- **闪屏** → 图层高频增删致 Cluster 侧帧丢/重复/黑帧交替。
- 排查建议：A720 Weston repaint/vsync 日志、composer_stub/GIPC/SHMEM fence 时序、SERDES 解串器 LOCK/误码计数（对应运行时段）。

## 附件（关键）
- 必须**录屏或手机拍摄仪表实屏**：撕裂/闪屏是合成/扫描时序伪影，`screencap` 抓 framebuffer **看不出撕裂**（这也是自动化判黑判不到的原因）。
- 任务 97310 日志、复现视频、A720 侧 dmesg/weston log（运行时段）。

## 测试增强（跟进，另立）
STRESS_006 判黑只查"非黑"，测不到撕裂/花屏/闪屏，且 Cluster 非主屏仅记录。可补：连续多帧 `screencap` 差分/SSIM（闪屏=帧间剧变、冻屏=帧间零变化）或录屏逐帧分析；撕裂需 panel 侧信号，更可靠是监控 SERDES/Weston vsync 计数（见 [[SERDES链路]] 用例）。

## 关联
- 用例 [[TC_GFWK_STRESS_006]] · 栈层 [[Weston]] / [[跨SoC]] / [[SERDES链路]] / [[HWC]]
- 总览 [[GFWK 稳定性测试用例全量清单]]
