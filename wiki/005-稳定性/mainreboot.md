---
title: mainreboot
tags:
  - 稳定性
  - AAOS
  - reboot
  - 座舱
  - 域隔离
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

# mainreboot

只把**某一个域的系统**重启，其他住户照常过日子。比如 `mainreboot_ivi` 只重启中控 Android——你会看到中控屏黑一下再起来，但**仪表还在正常显示车速**（开车时这点非常重要：中控崩了不能连累仪表）。

自检用例分了 `mainreboot_ivi / mainreboot_adas / mainreboot_cp1` 三个，挨个验证"单户重启不影响邻居"。

对比 [[socreboot]]（整栋楼）｜上级 [[座舱]]
