---
title: "澄清: stop/start SF 后卡开机动画 —— 预期行为(非缺陷)"
tags:
  - 稳定性
  - AAOS
  - 非缺陷
  - 已澄清
  - SurfaceFlinger
  - zygote
platform: "gua / guav100 (AAOS)"
severity: 无（非缺陷）
status: 已澄清（onrestart→zygote 机制；stop/start 卡 bootanim 属预期）
bench: "SG0286 (A41AEC42)"
created: 2026-08-09
updated: 2026-08-10
---

# 澄清：stop/start SF 后卡开机动画（蓝色小人）—— 预期行为

> ⚠️ **结论修正（2026-08-10）**：原记为缺陷 F3，经 `surfaceflinger.rc` 的 `onrestart restart zygote` 确认 —— **这是预期行为，非缺陷**。详见 [[zygote]]。
> 来源：2026-08-09 [[2026-08-09-GFWK收严杀进程压测|收严压测]]。用例 [[TC_SF_FAULT_001]]。

## 结论（先看这个）
`stop; start surfaceflinger` 只重启 native 的 SF，**不触发 `onrestart`→不重启 zygote→上层 Java 框架([[system_server]]/WMS)不恢复** → 停在开机动画。这是**手动操作绕过了崩溃恢复级联**，非产品缺陷。
- **真崩溃(kill-9/自然死)**：init respawn→`onrestart restart zygote`→整框架重init→**UI 正常恢复**。SF 崩溃恢复能力本身健全。
- 机制详解见 [[zygote]]。用例已改：stop/start 轮**只验 SF 进程重启、不判画面**。

## 一句话
用 `setprop ctl.stop surfaceflinger` + `ctl.start` **优雅重启** SF 后，**主屏永久黑**——但 SF 进程在跑、`service check SurfaceFlinger` = found、`dumpsys SurfaceFlinger --display-id` 正常枚举。而 `kill -9`（init respawn）**能正常重建显示**，且能**救活**已被 stop/start 卡黑的屏。

## 证据（A41AEC42 实测，PIL 灰度均值 mean）
| 操作 | 15s 后 | 40s+ / wake 后 |
|---|---|---|
| `kill -9` SF | mean 1.9（重建中） | **25s: mean 223（恢复）** |
| `ctl.stop`+`ctl.start` SF | mean 1.0（黑） | **仍 mean 0.9（黑，wake 无效）** |
| stop/start 黑屏后再 `kill -9` | — | **mean 217（救活）** |

- 判据：`check_image_black` 真像素判黑（mean<5=黑）。
- 复现率：stop/start 100% 黑（收严压测 2/2 轮）。

## 复现步骤
1. `adb -s <dev> shell "setprop ctl.stop surfaceflinger; sleep 2; setprop ctl.start surfaceflinger"`
2. 等 15~40s，`screencap` 拉图看主屏 → 黑
3. 对照：`kill -9 $(pidof surfaceflinger)` → ~25s 后主屏恢复

## 分析
- **进程恢复 ≠ 显示恢复**：SF 通过 ctl 优雅重启后进程/服务都在，但**未重新 acquire display / 重建 HWC 合成路径**，屏不亮。
- kill-9 走 init respawn，显示链路被完整重建 → 正常。
- 附带发现 [[2026-08-09-GFWK收严杀进程压测|F1]]：kill-9 SF 会级联重启 system_server，stop/start 不级联——两条路径 framework 行为不同，或与本 bug 同源。

## 待确认
- [ ] 图形团队：优雅重启 SF 不重建显示，是设计如此（不支持 ctl 重启）还是缺陷？
- [ ] 若为缺陷：定位 SF 重启后 display/HWC re-acquire 缺失点。

## 用例侧处理
[[TC_SF_FAULT_001]] 已按模式分流断言：kill-9 屏黑=硬失败（回归）；stop/start 屏黑=本 finding（默认告警，`SF_FAULT_STOPSTART_SCREEN_HARD=1` 升级硬失败）。

## 关联
- [[GFWK kill-恢复类测试]] · [[TC_SF_FAULT_001]] · [[SurfaceFlinger]] · [[HWC]] · [[GFWK稳定性-阅读脉络]]
- 同类跨SoC缺陷 → [[BUG-kill-HWC-crashes-A720-composer_stub]]
