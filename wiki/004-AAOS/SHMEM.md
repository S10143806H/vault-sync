---
title: SHMEM（跨 SoC 共享内存）
tags:
  - AAOS
  - SHMEM
  - 跨SoC
  - 共享内存
  - GIPC
platform: "gua / guav100 (AAOS)"
created: 2026-07-30
---

# SHMEM（跨 SoC 共享内存）

> **一句话**：SHMEM = 两颗 SoC(IVI / A720)**都能访问的共享物理内存**，专门用来搬跨 SoC 投屏的**像素数据**（数据面）。

## 为什么需要它
跨 SoC 投屏一帧像素几 MB，**不可能塞进控制消息**逐字节传。所以像素放进一块两颗芯片共享的物理内存(SHMEM)，控制通道(GIPC)只喊一句"内容在 N 号槽 + fence 编号"，接收侧([[Weston]])直接从 SHMEM 读像素。

## 控制面 vs 数据面（跨 SoC 版）
和芯片内 [[Binder IPC|Binder]]+[[dma-buf heap|dma-buf]] 同一套路，搬到两颗芯片之间：

| 面 | 芯片内 | 跨 SoC | 传什么 |
|---|---|---|---|
| 控制面 | Binder | **[[GIPC]]** | handle、fence、状态（小） |
| 数据面 | dma-buf | **SHMEM** | 像素本体（几 MB） |

## 类比
两个教室共用的**储物柜**：大件东西放柜子，不用手搬过走廊；只用对讲机(GIPC)喊一句"东西在 3 号柜、我放好了(fence)"。

## 关联风险
- 跨 SoC **fence 必须 signal** 后接收侧才能读 SHMEM，否则花屏/UAF（见 [[05-Fence与跨SoC同步]]）
- A720 重启时共享内存里的 fence 未 signal → IVI 侧冻屏/UAF（[[TC_CSOC_FAULT_002]] / composer_stub 风险）

## 关联
- 传输控制面 → GIPC｜发送桥 → [[composer_stub]]｜接收合成 → [[Weston]]
- 芯片内对应物 → [[dma-buf heap]]｜机制 → [[Buffer Queue]]｜[[Fence]]
- 上级 → [[000-GFWK图形框架总览]]｜跨 SoC 用例 → [[TC_CSOC_FAULT_002]]
