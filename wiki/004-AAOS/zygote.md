---
title: "zygote（Java 进程母体 + SF 崩溃恢复的关键）"
tags:
  - AAOS
  - zygote
  - system_server
  - SurfaceFlinger
  - init
platform: "gua / guav100 (AAOS)"
created: 2026-08-10
updated: 2026-08-10
---

# zygote（Java 进程母体）

> 上级：[[07-Android架构分层]]｜关联：[[system_server]] · [[SurfaceFlinger]] · [[GFWK kill-恢复类测试]]

## 一句话
zygote 是所有 Android **Java 进程的母进程**：开机预加载好 JVM + framework 类，之后 `fork` 出 [[system_server]] 和每个 app 进程。**zygote 一重启，整个 Java 世界重来一遍。**

## 它管谁
```
init → zygote → system_server（AMS/WMS/PMS…）
             └→ 各 app 进程（SystemUI / launcher / 你的 App）
```
- native 层（[[SurfaceFlinger|SF]] / [[HWC]] / HAL）**不归 zygote**，由 init 直接拉起。

## 为什么它是"SF 崩溃恢复"的关键
`surfaceflinger.rc` 里有一行：
```
service surfaceflinger /system/bin/surfaceflinger
    class core animation
    onrestart restart --only-if-running zygote
```
`onrestart` = 当 SF **被 init 判为"意外死亡后 respawn"**时执行 → **连带 restart zygote** → 整个 Java 框架重建 → UI 恢复。

## kill-9 能恢复、stop/start 卡住的根因
| 操作 | 触发 onrestart? | zygote 重启? | 结果 |
|------|----------------|-------------|------|
| `kill -9 surfaceflinger` | ✅ init 判意外死亡→respawn→onrestart | ✅ | 整框架重init, **UI 恢复** |
| `stop; start surfaceflinger` | ❌ 主动停+全新起, 非"restart" | ❌ | 上层 Java 不恢复, **卡开机动画(蓝色小人)** |

> 所以「stop/start 后卡蓝色小人」是**预期行为，非缺陷**：只重启了 native 的 SF，没重启驱动它的 Java 框架（zygote/[[system_server]]/WMS）。详见 [[BUG-SF-ctl-stopstart-显示不重建黑屏]]。

## 对测试的含义
- **真故障 = 崩溃 = kill-9**：会走 onrestart→zygote→恢复，这才是要验的稳定性能力。
- **stop/start ≠ 有效故障注入**：它绕过 onrestart，测的是不支持的手动操作。用例 [[TC_SF_FAULT_001]] 对 stop/start 轮**只验 SF 进程重启、不判画面**。

## 关联
- [[system_server]]（zygote fork 出的第一个、最重要的 Java 进程）
- [[SurfaceFlinger]] · [[HWC]] · [[GFWK kill-恢复类测试]] · [[TC_SF_FAULT_001]]
