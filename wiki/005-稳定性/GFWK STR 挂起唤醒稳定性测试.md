---
title: "GFWK STR 挂起-唤醒稳定性测试（§3.5 方法与用例）"
tags:
  - 稳定性
  - AAOS
  - GFWK
  - STR
  - 故障注入
  - 跨SoC
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# GFWK STR 挂起-唤醒稳定性测试（§3.5）

> **先读这篇再看代码。** 讲清：为什么 STR 是图形最大单一故障场景、怎么像 kill 一样注入、验什么、5 个用例怎么排、bug 长什么样。概念本身见 [[STR]]。

## 一句话
反复让座舱**挂起→唤醒**（STR），每轮验证唤醒后 [[SurfaceFlinger|SF]]/[[GIPC]]/投屏/三屏/fence 全部正确重建，A720 不崩、无资源泄漏——**resume 路径是图形链最脆的一环**。

## 为什么优先做 STR（高优先级 + 易挖 bug）
- **注入像 kill 一样轻**：纯 adb 一条命令即可挂起/唤醒，无需造 native 工具
- **resume 是 bug 重灾区**：唤醒瞬间 DP HPD 时序、PLL relock、fence timeout、跨 SoC 重连全挤在一起（见 [[STR]]「最大单一故障场景」）
- **规格 P0**：`STR_003`(Cross-SoC×STR)、`STR_005`(综合) 均 P0
- **有现成模板**：[[TC_CSOC_RECOVER_001]](STR+跨SoC) 已实现，STR_001/002/004/005 照抄改注入/断言点

## 两种注入模式（env `STR_MODE`）
| 模式 | 命令 | 特点 | 风险 |
|---|---|---|---|
| **display**（默认，安全） | `input keyevent KEYCODE_SLEEP` → `KEYCODE_WAKEUP` | 显示挂起-恢复代理，快、稳、CI 友好 | 不覆盖真内核 suspend 路径 |
| **mem**（真内核 STR） | `echo +N >/sys/class/rtc/rtc0/wakealarm; echo mem >/sys/power/state` | 走完整 suspend/resume，最真实 | 需 RTC/串口唤醒源，**有不醒风险**（测前备好 power_relay 兜底） |

> 建议：CI 常态用 display 模式跑量；mem 模式定期专项（带唤醒源 + 继电器兜底）。

## 四阶段套路（套 [[GFWK kill-恢复类测试|kill 同款]]）
1. **基线**：SF/GIPC pid、显示屏枚举（含后排）、A720 崩溃计数、图形内存/layer/fd 基线
2. **注入**：`STR_MODE` 挂起 `SUSPEND_SEC` 秒 → 唤醒
3. **恢复判定**：`adb wait-for-device` 醒来 → **就绪门**（主屏 `wait_screen_render` 真渲染）再判，别一醒就查
4. **三态断言**（缺一不可）：
   - **不崩/能醒**：设备唤醒、不掉线（不醒=硬失败，power_relay 兜底）
   - **重建**：SF+GIPC <SLA 重连；投屏恢复
   - **画面/资源**：逐屏非黑（**后排需 VHAL 唤醒**）、A720 无新崩溃、layer/fd/mem 不单调泄漏

## 用例（§3.5 + 已实现的 STR 类）
> ⭐ **[[TC_CSOC_RECOVER_001]](STR+跨SoC恢复) 已实现**，本质就是 §3.5 的 `STR_003`(Cross-SoC×STR)——**两者等同，不重复造**：STR_003 直接用 RECOVER_001（或在其上补投屏细节断言），其余 STR_* 复用它当模板。

| 用例 | 压什么 | bug 假设 | 优先级 | 状态 |
|---|---|---|---|---|
| **TC_CSOC_RECOVER_001** STR+跨SoC恢复 | STR + 跨SoC 投屏 + 三屏(含后排) | 唤醒后 [[GIPC]]/[[Fence\|fence]] 未重建 → 仪表冻屏/投屏丢/后排不亮 | **P0** | ✅ **已实现**(=STR_003;`CSOC_R001_ROUNDS=200`) |
| TC_GFWK_STR_003 Cross-SoC×STR | 同上 | 同上 | **P0** | ⭐ 用 RECOVER_001 覆盖 |
| **[[TC_GFWK_STR_005]]** 综合 | STR + GPU+Disp+跨SoC+Audio | 多子系统唤醒竞态 → 黑屏/崩溃 | **P0** | ✅ 写完待验 |
| [[TC_GFWK_STR_001]] SF×STR | STR + SF | 唤醒 SF layer/buffer 泄漏、首帧黑 | P1 | ✅ 写完待验 |
| [[TC_GFWK_STR_004]] HWC×STR | STR + HWC | 唤醒 HWC 合成异常、后排屏不亮 | P1 | ✅ 写完待验 |
| [[TC_GFWK_STR_002]] Gralloc×STR | STR + dma-buf pending | 挂起时 pending buffer 处理不当 → 泄漏/花屏 | P1 | ✅ 写完待验 |

## 常见 STR bug 模式（重点盯）
- **冻屏**：resume 后 used [[Fence|fence]] 没 signal，消费端死等（对照 KB G3）
- **投屏丢**：GIPC 通道唤醒未重连，A720 收不到帧
- **后排屏不亮**：唤醒只点主屏，后排需再发 VHAL `THIRD_SCREEN_ON_WITHOUT_CANN`
- **首帧黑/花**：SF/EGL surface 重建时序问题
- **泄漏累积**：每次 STR 泄一点 layer/fd/图形内存 → 长稳后 OOM
- **不醒**（mem 模式）：suspend 后无唤醒源或 relock 失败 → 设备失联

## 实施计划
1. **先验 [[TC_CSOC_RECOVER_001]]（=STR_003, 已实现）**：上台架 `CSOC_R001_ROUNDS=3` 单轮取证，确认 `_suspend_resume`/`_a720_crashes`(已含 `A720CNT=` 抗噪) 工作、能抓冻屏/投屏丢/后排不亮；必要时补跨SoC投屏细节断言。这就是 STR 样板。
2. **再建 STR_005（综合, P0）**：在样板上加 GPU+Audio 负载，压多子系统唤醒竞态
3. 批量补 STR_001(SF) / STR_004(HWC) / STR_002(Gralloc)（同模板，换负载/断言侧重）
4. 结果回填到 [[GFWK 稳定性测试用例全量清单]] 压测矩阵；RECOVER_001 从"写完待验"→验证状态

## 关联
- 概念 → [[STR]]｜[[Fence]]｜[[GIPC]]｜[[SHMEM]]｜[[composer_stub]]
- 模板/同类 → [[TC_CSOC_FAULT_003]]（IVI panic 恢复）｜[[GFWK kill-恢复类测试]]
- 清单 → [[GFWK 稳定性测试用例全量清单]]｜闭环 → [[GFWK 双bots自动挖bug闭环]]
- 上级 → [[000-GFWK图形框架总览]]｜规格 → [[GFWK 稳定性测试 — 需求规格说明书 v2]]

## 📚 延伸阅读
- Linux 内核电源状态/suspend：https://docs.kernel.org/admin-guide/pm/sleep-states.html
- Android 电源管理（doze/suspend）：https://source.android.com/docs/core/power
- Android sync/fence（resume fence 未 signal 根因）：https://source.android.com/docs/core/graphics/sync
