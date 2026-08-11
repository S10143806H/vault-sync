---
title: CornerStone
tags:
  - 稳定性
  - AAOS
  - CornerStone
  - 日志
  - 崩溃分析
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

CornerStone 是你们车机上的**持续日志落盘系统**——可以理解为整车的"黑匣子"：一个常驻服务，把系统运行日志（各域的 logcat、内核、崩溃信息等）持续写到设备磁盘的固定目录里，跨重启保留，出问题时可以追溯事发前的完整历史，不是 Android 标准组件。

从你们日志里能看到测试框架和它的三种交互：

1. **测试前拉取存档**：`Pulled cornerstone log to ... cornerstone_log_20260715_..._car0_pre-test.tgz`（39MB）——把测试开始前设备上已积累的 CornerStone 日志先拉回台架保存，作为"事前基线"，这样测试中出的问题可以和历史区分开
2. **测试中监控**：monkey 压测时每 5 秒一条 `CornerStone dir check: total 218248`——盯着日志目录的大小，确认日志服务还活着、还在正常写入（目录大小不涨可能意味着日志服务挂了或磁盘满了）
3. **收集清单里的一项**：之前 zip 版机器人的 `xg_get_all_log.bat` 采集范围就包括 CornerStone/系统/应用/崩溃日志——失败后打包带回分析

和之前讲的 [[RAMdump]]/pstore 的关系：pstore 是"崩溃瞬间"的最后快照（几MB），CornerStone 是"日常连续"的完整记录（几十MB到GB级）。查崩溃根因时两个配合用——CornerStone 看崩溃前发生了什么，pstore 看崩溃那一刻死在哪。