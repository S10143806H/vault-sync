---
title: 05 - Fence 与跨SoC同步（"画好了"举手 + 隔壁班）
tags:
  - AAOS
  - Fence
  - 跨SoC
  - composer_stub
  - sync_file
  - GFWK
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 05 - Fence 与跨SoC同步（"画好了"举手 + 隔壁班）

> 上级：[[000-GFWK图形框架总览]]

一句话：[[Fence]] 是跨环节"好了没"的同步信号；跨 SoC 靠 [[composer_stub]] 把画面送到仪表/[[智驾（ADAS）]]；fence 用错 = 花屏 / kernel panic(UAF)。（待展开）

- 前：[[HWC]]｜下一课：[[06-GFWK如何映射到测试用例]]
- 相关用例：TC_CSOC_*（其中 TC_CSOC_FAULT_001 = sync_file_poll UAF）
