---
title: 消费者 (Consumer)
tags:
  - AAOS
  - Consumer
  - BufferQueue
  - SurfaceFlinger
  - backpressure
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 消费者 (Consumer)

**消费者 = 从队列取出画好的 buffer、用完归还槽位的一方**。经典**生产者-消费者模式**在图形栈的应用，队列是 [[Buffer Queue]]。班长 [[SurfaceFlinger|SF]] 就是 App 帧的消费者。

## API 循环（消费端 `IGraphicBufferConsumer`）

```
acquireBuffer()  取一张画好的稿
   ↓ 用（合成 / 显示）
releaseBuffer()  归还槽位
```

对应生产者 [[生产者|Producer]] 用 `dequeueBuffer → queueBuffer` 领槽交稿。取稿前须等 [[Fence]] 举牌"画完了"，防拿到半成品。

## 角色是相对的

[[SurfaceFlinger|SF]] 对 App 是消费者，但对 [[HWC]]/显示又是生产者——看**哪一段 BufferQueue** 而定。

## 反压：消费者太慢就占满槽位

消费者慢 → 迟迟不 `releaseBuffer` → 8 槽被占满 → 生产者 `dequeueBuffer` 阻塞。必须返回正确错误码、**不死锁、fd 不泄漏**。这正是 [[TC_SF_BOUND_003]] 验证的机制。

## 📚 延伸阅读
- [BufferQueue and gralloc（官方）](https://source.android.com/docs/core/graphics/arch-bq-gralloc)
- [Graphics architecture（官方）](https://source.android.com/docs/core/graphics/architecture)
- [Producer–consumer problem（Wikipedia）](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)

## 相关
详见 [[02-SurfaceFlinger与BufferQueue]]｜配对 [[生产者]]｜机制 [[Buffer Queue]]｜测试 [[TC_SF_BOUND_003]]｜上级 [[000-GFWK图形框架总览]]
