---
title: CornerStone
tags:
  - 稳定性
  - AAOS
  - CornerStone
  - 日志
  - 崩溃分析
  - 维测
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
updated: 2026-09-16
source: "raw/articles/XG/CST SDK 接入文档.pdf (v1.0, 2026-06)"
---

CornerStone 是你们车机上的**持续日志落盘系统**——可以理解为整车的"黑匣子"：一个常驻服务，把系统运行日志（各域的 logcat、内核、崩溃信息等）持续写到设备磁盘的固定目录里，跨重启保留，出问题时可以追溯事发前的完整历史，不是 Android 标准组件。

> 📌 **补全（据 [[CST SDK]] 接入文档）**：日志落盘只是 CornerStone 的一个功能组件（`logstorage.json`）。它实际是平台的**维护服务全貌**——见下文。

从你们日志里能看到测试框架和它的三种交互：

1. **测试前拉取存档**：`Pulled cornerstone log to ... cornerstone_log_20260715_..._car0_pre-test.tgz`（39MB）——把测试开始前设备上已积累的 CornerStone 日志先拉回台架保存，作为"事前基线"，这样测试中出的问题可以和历史区分开
2. **测试中监控**：monkey 压测时每 5 秒一条 `CornerStone dir check: total 218248`——盯着日志目录的大小，确认日志服务还活着、还在正常写入（目录大小不涨可能意味着日志服务挂了或磁盘满了）
3. **收集清单里的一项**：之前 zip 版机器人的 `xg_get_all_log.bat` 采集范围就包括 CornerStone/系统/应用/崩溃日志——失败后打包带回分析

和之前讲的 [[RAMdump]]/pstore 的关系：pstore 是"崩溃瞬间"的最后快照（几MB），CornerStone 是"日常连续"的完整记录（几十MB到GB级）。查崩溃根因时两个配合用——CornerStone 看崩溃前发生了什么，pstore 看崩溃那一刻死在哪。

## 维护服务全貌（SDK 视角）

CornerStone 是 [[CST SDK]] **Maint 模块**的底层服务，双进程架构：`system_cornerstoned`（`ICornerStoneService/system`）+ `vendor_cornerstoned`（`IVendorCornerStoneService/vendor`），均 `post-fs-data` 启动。核心功能：

- **事件/标记处理**：接收 Maint 上报的 Event / Mark，经插件系统处理后执行预定义动作（命令/脚本/服务调用）。
- **云端同步**：事件和标记自动上传，支持定时/即时上传与断点续传。
- **USB 导出**：U 盘插入时自动导出系统日志，带 RSA 签名验证、音频提示、安全卸载。
- **日志落盘**（`logstorage`）：即本页开头讲的"黑匣子"，是其功能组件之一。

### 客户端上报（Maint 模块）

- **Event**：记录重要状态变化，`submitEvent(id, json[, files])`，无限流；离线队列容量 500。**Event ID 必须 9 位**（`100000000~999999999`）。
- **Mark**：标记关键时间节点，`submitMark(id, json)`，**令牌桶限流**（1次/秒/EventID，桶容量 100），超频丢弃并自动上报 `eventId=100010006` 告警；离线队列容量 1000。
- `CstMaint` **无 GUA 签名限制**，任何系统应用均可用（区别于 [[GFS]] 的 `CstDiag`）。

### 配置文件

`components.json`（组件开关）/ `events.json`（事件定义）/ `actions.json`（动作）/ `env.json`（服务器地址、磁盘限制、线程池）/ `features.json` / `logstorage.json`（日志源）/ `network.json`（上传）/ `export.json`（USB 导出）。

## 关联

- 上层 SDK：[[CST SDK]]（Maint 模块）
- 兄弟服务：[[GFS]]（故障，FATAL 会转发给 CornerStone 云端同步）、[[GDC]]（诊断通信）
- 崩溃分析：[[RAMdump]]、[[tombstone（墓碑 验尸报告）]]