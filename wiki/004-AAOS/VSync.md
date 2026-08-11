---
title: VSync
tags:
  - AAOS
  - VSync
  - SurfaceFlinger
  - DPU
  - tearing
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# VSync

换纸统一口令：只在屏幕"誊完整张、抬笔回顶"的消隐期(VBLANK)换 buffer，防撕裂(tearing)；供纸跟不上 → DPU underflow。

详见 [[01-一帧画面是怎么上屏的]]｜上级 [[000-GFWK图形框架总览]]
