---
title: Buffer Queue
tags:
  - AAOS
  - BufferQueue
  - SurfaceFlinger
  - Producer
  - Consumer
  - backpressure
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# Buffer Queue

收纸筐：[[生产者]] 交纸(queue)、[[Consumer|消费者]] [[SurfaceFlinger]] 取纸(dequeue) 的有限格子队列；核心是**反压**。

详见 [[02-SurfaceFlinger与BufferQueue]]｜上级 [[000-GFWK图形框架总览]]
