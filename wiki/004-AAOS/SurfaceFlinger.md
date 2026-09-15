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
零基础学 SF + 启动项目的路线（约 2 周到能提第一个 MR）
- [[SurfaceFlinger 主线程与三阶段]]




`adb shell debuggerd -d $(adb shell pidof surfaceflinger)`   # 看一次健康态的主线程栈长什么样

`adb shell dumpsys SurfaceFlinger --timestats`

   目标：认识 main thread、commit/composite/present 三个词在栈里的样子。

SF 合成全跑在单一主线程，vsync 驱动，每帧顺序推进 commit → composite → present

| 概念 | 是什么 | 干什么 |
|---|---|---|
| **main thread** | SF 的单线程事件循环（`MessageQueue`/`onMessageReceived`），vsync 一到就醒来跑一帧 | 唯一操作图层树的线程；它一卡 → 全屏卡/黑。抓它的栈是定位掉帧/黑屏的起点 |
| **commit** | 帧的准备阶段：把 App 提交的事务（transaction）和新 buffer 锁进这一帧的状态 | 处理 [[SurfaceControl]] 事务、latch 最新 buffer、算可见区域/几何；决定“这帧长什么样” |
| **composite** | 合成阶段：把各 layer 按 z 序合成 | 决定每层走 [[HWC]] overlay（硬件叠加省电）还是 GPU/RenderEngine 客户端合成 |
| **present** | 上屏阶段：把合成结果交给 [[HWC]] 送显示屏 | 提交 present fence，等待上屏；`--timestats` 的 present time 即此步 |

调用链：`main thread → onMessageInvalidate → commit() → composite() → present`。
给 `composite()` 加 `sleep(3s)` → 主线程被拖住 → 下游 `dequeueBuffer failed -110` + `Skipped N frames`（Day 4 复现原理）。

2. Day 2-3：读 5 个文件，只读调用链（每个 1-2 小时）
   - frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp：onMessageInvalidate → commit() → composite()
   - frameworks/native/libs/gui/BufferQueueProducer.cpp：dequeueBuffer → waitForFreeSlotThenRelock 返回 -110 的地方
   - frameworks/native/libs/gui/BufferQueueConsumer.cpp：acquireBuffer / releaseBuffer（理解 SF 为什么「不还 buffer」）
   - services/surfaceflinger/ScreenCaptureOutput.cpp + SurfaceFlinger::captureScreen*：本 bug 的截图路径
   - services/surfaceflinger/Scheduler/：只看 vsync 怎么喂进主线程
     每读一个文件，写 10 行「谁调谁」的笔记。
3. Day 4：亲手复现一次 -110（半天）。在本地构建里给 composite() 加 sleep(3s) 的临时补丁，刷机后看 logcat 出现 dequeueBuffer failed -110 和 Skipped N frames。这一步让你把日志、代码、栈对上号。
4. Day 5-7：做 Action 1（抓栈触发器）。它在 drmhwc 侧改动最小：找 FENCE GAP ALERT 那行 ALOGE，旁边加调用 debuggerd -d 的采集函数，加 sysprop 开关和 30s 冷却。在台架用 Day 4 的 sleep 补丁验证能抓到栈。这就是你的第一个 MR。
5. Week 2：做 MR-2 BBQ 自爆。50 行以内，独立于其他 MR，正好用上 Day 2 读的 BufferQueue。

▎ 学习顺序的原则：先真机后代码、先复现后修改、先做能单独验证的最小改动。不要一上来读 Scheduler 和 RenderEngine。


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
