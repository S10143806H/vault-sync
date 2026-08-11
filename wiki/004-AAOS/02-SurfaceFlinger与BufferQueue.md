---
title: "02 - SurfaceFlinger 与 BufferQueue（班长怎么收纸）"
tags:
  - AAOS
  - SurfaceFlinger
  - BufferQueue
  - backpressure
  - Fence
  - GFWK
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 02 - SurfaceFlinger 与 BufferQueue（班长怎么收纸）

> 上级：[[000-GFWK图形框架总览]] ｜ 前置：[[01-一帧画面是怎么上屏的]]
> 一句话：**[[Buffer Queue|BufferQueue]] = 收纸筐**，一头同学交纸、一头班长取纸；核心是**反压**。

## ① 收纸筐怎么运作

- 同学（App/[[生产者]]）画好一张 → `queueBuffer` **丢进筐**。
- 班长（[[SurfaceFlinger]]/[[Consumer|消费者]]）→ `acquireBuffer` **从筐取纸**去拼版。
- 取完用完的纸 → 还回去（`releaseBuffer`）循环利用，不是每张都新发。

这就是经典 **生产者-消费者模型**。

## ② 关键点：筐是有限格子的（slots）+ 反压

筐只有几个格子（slots，常见 3 个，跨屏投屏需 ≥5）。于是：

- **同学画太快、班长取太慢** → 筐满 → 同学**被迫等**（阻塞）= **反压 backpressure**。
  - 反压是**好事**：防止画一堆没人看的帧、内存爆掉。
- **班长快、同学慢** → 班长没纸可取 → 只能**重复贴上一张**（卡顿）。

## ③ 还要配合"举手"（Fence）

筐里的纸不一定画完了。每张纸带一个 [[Fence]]（"我画好了"举手）。**班长只能取已举手的纸**，否则拿到没画完的 → 花屏。

## ④ 挂到测试用例（都在测这个筐的极限）

| 用例 | 测什么 | 筐视角 |
|---|---|---|
| `TC_SF_BOUND_001` | slots=2/3/8/…/64 | 各种筐容量下会不会卡 |
| `TC_SF_BOUND_002` | Producer 30→1000fps | 同学狂交 → 反压是否正确触发 |
| `TC_SF_BOUND_003` | Consumer 极慢 | 班长偷懒 → 筐满是否死锁 |
| `TC_SF_BOUND_004` | 多屏各一个筐 | 筐之间是否互不干扰 |
| `TC_SF_LEAK_001` | 建/毁 Surface 1万次 | 筐反复拆装是否漏 fd/内存 |

> 反压相关（BOUND_002/003）正是你会议分工里的**"思考反压路径"**——暂不强制落地，但概念就在这。

## 下一步
- 纸从哪来 → [[03-Gralloc与dma-buf]]
- 班长罢工实战 → [[TC_SF_FAULT_001]]
