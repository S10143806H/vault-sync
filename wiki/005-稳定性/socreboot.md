---
title: socreboot
tags:
  - 稳定性
  - AAOS
  - reboot
  - 座舱
  - SoC
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

# socreboot

**SoC reboot**，整颗芯片所有域复位（软重启）——中控、仪表、智驾全部黑掉再重新启动。整芯片复位（IVI 侧 / CP0 侧触发），重启后查全部串口 + 中控/仪表屏点亮。最彻底的软重启，代价也最大（全部服务中断，要等整栋楼一户户亮起来）。

对比 [[mainreboot]]（单户）｜上级 [[座舱]]
