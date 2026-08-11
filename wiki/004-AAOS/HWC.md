---
title: 04 - HWC 硬件混合渲染器
tags:
  - AAOS
  - HWC
  - SurfaceFlinger
  - overlay
  - DPU
  - GFWK
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 04 - HWC Hardware Composer

> 上级：[[000-GFWK图形框架总览]]

一句话：[[HWC]] 用硬件 overlay 把 [[SurfaceFlinger]] 合成的整版快速挂上墙，省 GPU；层数/格式超限则回退 GPU。（待展开）

> DPU 之后到远端屏(后排/仪表)走 [[SERDES链路]]（GMSL/FPD-Link 串行）；链路失锁/误码会黑屏/花屏但上层 display 仍枚举，需链路层监控兜底。

- 前：[[03-Gralloc与dma-buf]]｜下一课：[[05-Fence与跨SoC同步]]｜上级 [[000-GFWK图形框架总览]]

- 稳定性用例：
	- [[TC_HWC_FAULT_002]] — 非法 layer 参数拒绝（不 crash，🧩 骨架）
	- [[TC_HWC_FAULT_003]] — 多屏故障隔离（一屏坏不波及另一屏，🧩 骨架）
	- [[TC_HWC_FAULT_004]] — **kill HWC 恢复 SLA**（见下）

---

## 为什么要 kill HWC（故障注入）

HWC 是显示链路的**单点**——一个独立的合成 HAL 进程（本平台 `android.hardware.composer.hwc3-service.gua`）。稳定性测试主动"打晕"它，验证：崩了能不能被拉起、画面能不能恢复、有没有 double-free。它是 [[GFWK kill-恢复类测试]] 家族的一员，模板是 [[TC_SF_FAULT_001]]。

## kill HWC 会发生什么

kill composer HAL 进程后，是一条**连锁反应**：
1. SF 与 HWC 之间的 HWComposer **binder 连接断** → SF 判定合成器不可用 → SF `LOG_ALWAYS_FATAL` → **SF 连带一起重启**（实测 pid `502→5674`）
2. 屏幕**短暂黑一下**（合成链路瞬断）
3. `init` 把 composer HAL + [[SurfaceFlinger|SF]] 都重新拉起 → 画面恢复
4. **后排屏(HWC display 1) 不会自动亮**，需重新发 VHAL 命令唤醒（否则误判为"没恢复"）

> 所以 HWC 和 SF 是"绑在一起"的：kill HWC ≈ kill 半条显示链，SF 必然连坐。这与 kill [[composer_stub]]/[[Weston|weston]]（只影响跨 SoC 投屏、IVI SF 不受连累）**不同**。

## 我们预期什么（判据）

| 项 | 预期 |
|---|---|
| composer + SF 恢复 | 新 pid 且 `service check SurfaceFlinger` found，**< 10s** |
| 逐屏画面不黑 | 主屏 + 后排屏(**唤醒后**) + 仪表屏都有内容（非纯黑/纯色） |
| 无 double-free | 新增 tombstone 不含 `double free / UAF` |
| 服务自启动 | composer 是 init 托管、**非 oneshot**（否则不恢复属配置问题） |

> 完整用例与运行方法见 [[TC_HWC_FAULT_004]]；同类对比见 [[GFWK kill-恢复类测试]]

---



硬件混合渲染器 (HWC) HAL 用于确定通过可用硬件来合成缓冲区的最有效方法。作为 HAL，其实现是特定于设备的，而且通常由显示硬件原始设备制造商 (OEM) 完成。

当您考虑使用叠加平面时，很容易发现这种方法的好处，它会在显示硬件（而不是 GPU）中合成多个缓冲区。例如，假设有一部普通 Android 手机，其屏幕方向为纵向，状态栏在顶部，导航栏在底部，其他区域显示应用内容。每个层的内容都在单独的缓冲区中。您可以使用以下任一方法处理合成：

- 将应用内容渲染到暂存缓冲区中，然后在其上渲染状态栏，再在其上渲染导航栏，最后将暂存缓冲区传送到显示硬件。
- 将三个缓冲区全部传送到显示硬件，并指示它从不同的缓冲区读取屏幕不同部分的数据。

后一种方法可以显著提高效率。

显示处理器功能差异很大。叠加层的数量（无论层是否可以旋转或混合）以及对定位和重叠的限制很难通过 API 表达。为了适应这些选项，HWC 会执行以下计算：

1. [[SurfaceFlinger]] 向 HWC 提供一个完整的层列表，并询问“您希望如何处理这些层？”
2. HWC 的响应方式是将每个层标记为设备或客户端合成。
3. [[SurfaceFlinger]] 会处理所有客户端，将输出缓冲区传送到 HWC，并让 HWC 处理其余部分。

由于硬件供应商可以定制决策代码，因此可以在每台设备上实现最佳性能。

当屏幕上的内容没有变化时，叠加平面的效率可能会低于 GL 合成。当叠加层内容具有透明像素且叠加层混合在一起时，尤其如此。在此类情况下，HWC 可以为部分或全部层请求 GLES 合成，并保留合成的缓冲区。如果 SurfaceFlinger 要求合成同一组缓冲区，HWC 可以显示先前合成的暂存缓冲区。这可以延长闲置设备的电池续航时间。

Android 设备通常支持 4 个叠加平面。尝试合成的层数多于叠加层数会导致系统对其中一些层使用 GLES 合成，这意味着应用使用的层数会对能耗和性能产生重大影响。