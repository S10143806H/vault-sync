---
title: SurfaceFlinger
tags:
  - AAOS
  - SurfaceFlinger
  - BufferQueue
  - HWC
  - Binder
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

[[Buffer Queue]]管理

图形管线合成引擎"班长/总管"：收集各 App 的 layer → 合成一帧 → 交 [[HWC]] 上屏。是**黑屏总闸**（它一倒立刻全黑）

- 稳定性用例：[[TC_SF_FAULT_001]]（kill -9 恢复 SLA，真机 ~1s 恢复）｜[[TC_SF_FAULT_002]]（[[Binder IPC|Binder]] 突刺，SF 不被 [[AMS]]/[[WMS]] 连累）
- 图层句柄：[[SurfaceControl]]（特权侧直接操作图层树上某层的 z/位置/alpha/屏，绕过 [[WMS]]）
- 上级地图：[[000-GFWK图形框架总览]]

---

## kill SF vs stop/start SF（重启方式的区别）

本质：**骤死（模拟崩溃）** vs **优雅生命周期管理（受控开关机）**。GFWK 故障注入用前者。

| 维度 | `kill -9 $(pidof surfaceflinger)` | `adb shell stop/start surfaceflinger` |
|---|---|---|
| 信号 | **SIGKILL** 立即，进程骤死 | init 发 **SIGTERM → 超时才 SIGKILL**，有序退出 |
| 谁在操作 | 直接杀进程，**init 被动**发现子进程死 → 重生 | **init 主动**管生命周期（底层走 `ctl.stop`/`ctl.start` 属性） |
| 资源清理 | **无析构/无释放**（fence、dma-buf、GL context 骤断） | 有序析构、释放资源 |
| 恢复 | init 按 `.rc` **自动 respawn**；SF 常配 `onrestart` 连带**重启 zygote/框架** | `stop` 后**保持停止不回来**，需手动 `start` |
| 崩溃循环 | 快速反复 kill 可触发 `critical` 服务 → **整机 reboot** | 不会 |
| 用途 | **故障注入**：测异常死亡的恢复路径 | **受控重启/置位**：测计划内重启、置到已知态 |

> 补充：`adb shell stop`/`start`（**不带服务名**）= 停/起整个框架（zygote）；带 `surfaceflinger` = 只操作 SF 服务。另有 `setprop ctl.restart surfaceflinger`。

### 为什么故障用例用 kill 不用 stop/start
bug 藏在**骤死路径**里——`stop` 优雅退出会正常释放、掩盖问题；`kill -9` 才暴露：
- **fence / sync_file UAF**（消费方还在读，生产方被骤杀）
- **dma-buf / SurfaceControl 泄漏**（无析构）
- **跨 SoC 连坐**：IVI SF 骤死是否拖崩 A720 [[composer_stub]]
- **就绪门**：`pid 回来 ≠ 画面恢复`（[[TC_SF_FAULT_001]] 的三态断言正是测这个）

一句话：**`kill -9` = 模拟真实崩溃（测健壮性）；`stop/start` = 受控开关机（测流程）**。前者才可能挖出稳定性缺陷。方法论详见 [[GFWK kill-恢复类测试]]。

## 📚 延伸阅读
- Android init 语言（service / `oneshot` / `onrestart` / `critical`）：https://android.googlesource.com/platform/system/core/+/master/init/README.md
- SurfaceFlinger 架构：https://source.android.com/docs/core/graphics/arch-sf-hwc
- `adb shell stop/start`（框架重启）：https://source.android.com/docs/core/graphics/implement-vsync