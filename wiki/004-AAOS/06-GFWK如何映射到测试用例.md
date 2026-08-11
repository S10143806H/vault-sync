---
title: 06 - GFWK 如何映射到测试用例
tags:
  - AAOS
  - GFWK
  - 测试用例
  - SurfaceFlinger
  - HWC
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 06 - GFWK 如何映射到测试用例

> 上级：[[000-GFWK图形框架总览]]

一句话：管线每个环节的故障 → 对应一个测试维度/用例。代码在 `D:\Dev_Code\autocase`（本地）与 Ubuntu `~/Documents/autocase`（真机权威副本）。

| 环节 | 维度 | 用例前缀 |
|---|---|---|
| [[SurfaceFlinger]]/[[Buffer Queue]] | FAULT/BOUND/LEAK | TC_SF_* |
| [[Gralloc]]/[[dma-buf heap]] | CONF/FAULT/LEAK | TC_GRAL_* |
| [[HWC]] | CONF/FAULT | TC_HWC_* |
| [[Fence]]/[[composer_stub]] | FAULT/SM/RECOVER | TC_CSOC_* |

- 实战起点：[[TC_SF_FAULT_001]]

---

## 图形栈分层 → 压测用例映射（mermaid）

> 一帧从 App 到像素，纵向穿过每一层；每层的故障模式对应一类注入用例。右列括号为 `autocase` 前缀。STR（挂起-唤醒）是**横切**维度，与每层相乘。

```mermaid
graph LR
    subgraph STACK["图形栈分层（一帧的旅程）"]
        direction TB
        L1["① App / EGL·GLES·Vulkan<br/>渲染 API"]
        L2["② BufferQueue / libgui<br/>生产者-消费者 8 槽状态机"]
        L3["③ Gralloc / dma-buf heap<br/>图形内存分配"]
        L4["④ SurfaceFlinger<br/>z-order 合成"]
        L5["⑤ HWC composer HAL<br/>硬件 overlay 合成"]
        L6["⑥ Fence / sync_file<br/>帧生产-消费同步"]
        L7["⑦ GPU 驱动 / DPU·DRM-KMS<br/>渲染·扫描上屏"]
        L8["⑧ GIPC + SHMEM → composer_stub → Weston<br/>跨 SoC 投屏(仪表 a720)"]
        L1 --> L2 --> L3 --> L4 --> L5 --> L6 --> L7 --> L8
    end

    subgraph CASES["对应压测/故障注入用例"]
        direction TB
        C1["GPU 高负载·反复建销 Surface<br/>(TC_GPU_Stress_* · STR_005)"]
        C2["高频提交·反压·槽位极限<br/>(TC_SF_BOUND_001~005)"]
        C3["dma-heap OOM·坏 fd·泄漏<br/>(TC_GRAL_FAULT_001❤️ 002 · LEAK_001/002)"]
        C4["kill SF·Surface 泄漏·Binder 突刺<br/>(TC_SF_FAULT_001/002 · LEAK_001)"]
        C5["kill HWC·坏 Layer·多屏隔离<br/>(TC_HWC_FAULT_002/003/004)"]
        C6["sync_file UAF·fence 超时<br/>(TC_CSOC_FAULT_001 · BOUND_002)"]
        C7["DPMS 抖动·GPU hang→reset<br/>(BUG-HWC-DPMS · 待补)"]
        C8["kill composer_stub/weston·GIPC 断·SHMEM 压<br/>(TC_CSOC_FAULT_002/004/005 · RECOVER_001)"]
    end

    STR["⊗ STR 横切维度<br/>display / echo mem / KL15 深睡<br/>每层 × 反复挂起-唤醒<br/>(TC_GFWK_STR_001/002/004/005)"]

    L1 -.-> C1
    L2 -.-> C2
    L3 -.-> C3
    L4 -.-> C4
    L5 -.-> C5
    L6 -.-> C6
    L7 -.-> C7
    L8 -.-> C8
    STACK === STR

    classDef gap fill:#ffe0b2,stroke:#e65100,color:#000
    class C7 gap
```

| 栈层 | 故障模式 | 用例 | 状态 |
|---|---|---|---|
| ① API/GPU | 驱动泄漏·hang | `TC_GPU_Stress_*` / [[TC_GFWK_STR_005]] | ✅ / 🟠 hang→reset 待补 |
| ② BufferQueue | 反压·槽位·突刺 | `TC_SF_BOUND_001~005` | 部分需 native |
| ③ Gralloc/dma-buf | **OOM**·坏 fd·泄漏 | **[[TC_GRAL_FAULT_001]]**（新，写完待验）· `002` · `LEAK_001/002` | 🟠 gap 最多 |
| ④ SurfaceFlinger | 崩溃·泄漏·死锁 | [[TC_SF_FAULT_001]] / [[TC_SF_FAULT_002]] | ✅ |
| ⑤ HWC | 崩溃·坏 Layer·多屏 | [[TC_HWC_FAULT_004]] / `002` / `003` | ✅ |
| ⑥ Fence | UAF·超时 | `TC_CSOC_FAULT_001`（P0） | 写完待验 |
| ⑦ GPU驱动/DPU | **DPMS 抖动·hang→reset** | [[BUG-HWC-DPMS-SetPowerMode崩溃循环]] | 🟠 待立用例 |
| ⑧ 跨 SoC | kill 桥·断链·压带宽 | [[TC_CSOC_FAULT_002]] / `004` / [[TC_CSOC_FAULT_005]] / [[TC_CSOC_RECOVER_001]] | ✅ |
| ⊗ STR 横切 | 挂起-唤醒累积 | [[TC_GFWK_STR_001]]/`002`/`004`/`005` | ✅ smoke 通过 |

> 🟠 = 高价值待补：③ dma-heap OOM 已落 [[TC_GRAL_FAULT_001]]；⑦ GPU hang→reset 与 DPMS 压测为下一批（见 [[BUG-HWC-DPMS-SetPowerMode崩溃循环]]）。

## 📚 延伸阅读
- Android Graphics 架构总览：https://source.android.com/docs/core/graphics
- BufferQueue 与 Gralloc：https://source.android.com/docs/core/graphics/arch-bq-gralloc
- SurfaceFlinger 与 HWC：https://source.android.com/docs/core/graphics/arch-sf-hwc
- Fence / 同步框架：https://source.android.com/docs/core/graphics/sync
- Kernel DMA-BUF：https://docs.kernel.org/driver-api/dma-buf.html
