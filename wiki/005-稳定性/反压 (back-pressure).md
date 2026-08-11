---
title: "反压 (back-pressure)"
tags:
  - 稳定性
  - AAOS
  - backpressure
  - BufferQueue
  - SurfaceFlinger
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

> [!note] 核心概念：反压 (back-pressure)
> **类比**：生产者=往传送带放包裹的人，消费者(SurfaceFlinger)=取包裹的人，传送带槽位(BufferQueue slot)有限。放得比取得快 → 槽位占满 → 放包裹的人被迫等。这个「被迫等的时长」就是反压强度。
