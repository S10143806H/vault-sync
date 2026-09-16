---
title: CST - CornerStone
aliases:
  - CornerStone
  - Cornerstone
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

# 🗄️ CornerStone — 整车维护服务与"黑匣子"

> [!abstract] 一句话
> CornerStone 是车机上的**持续日志落盘系统**，可理解为整车的"**黑匣子**"：一个常驻服务，把各域运行日志（logcat、内核、崩溃信息等）持续写到设备磁盘固定目录，**跨重启保留**，出问题时可追溯事发前的完整历史。**非 Android 标准组件。**

> [!note] 定位补全（据 [[CST SDK]] 接入文档）
> 日志落盘只是 CornerStone 的一个功能组件（`logstorage.json`）。它实际是平台的**维护服务全貌**——详见下文。

---

## 🔍 测试框架中的三种交互

| # | 交互 | 日志示例 | 作用 |
|---|------|---------|------|
| 1 | **测试前拉取存档** | `Pulled cornerstone log to ... _pre-test.tgz`（39MB） | 把测试开始前设备已积累的日志拉回台架，作为"**事前基线**"，将测试中的问题与历史区分开 |
| 2 | **测试中监控** | 每 5s 一条 `CornerStone dir check: total 218248` | monkey 压测时盯日志目录大小，确认日志服务**存活且在写入**（不涨 = 服务挂了或磁盘满） |
| 3 | **失败收集清单** | `xg_get_all_log.bat` 采集范围含 CST/系统/应用/崩溃日志 | 失败后打包带回分析 |

---

## 🆚 与 [[RAMdump]] / pstore 的关系

> [!tip] 两者互补，查根因时配合使用
> - **pstore** —— 崩溃**瞬间**的最后快照（几 MB）→ 看"死在哪"
> - **CornerStone** —— 日常**连续**的完整记录（几十 MB ~ GB 级）→ 看"崩溃前发生了什么"

---

## 🏗️ 维护服务全貌（SDK 视角）

CornerStone 是 [[CST SDK]] **Maint 模块**的底层服务，采用**双进程架构**（均于 `post-fs-data` 启动）：

| 进程 | 服务接口 |
|------|---------|
| `system_cornerstoned` | `ICornerStoneService/system` |
| `vendor_cornerstoned` | `IVendorCornerStoneService/vendor` |

**核心功能：**

- 📤 **事件/标记处理** —— 接收 Maint 上报的 Event / Mark，经插件系统处理后执行预定义动作（命令 / 脚本 / 服务调用）
- ☁️ **云端同步** —— 事件和标记自动上传，支持定时 / 即时上传与断点续传
- 💾 **USB 导出** —— U 盘插入时自动导出系统日志，带 RSA 签名验证、音频提示、安全卸载
- 🗄️ **日志落盘**（`logstorage`）—— 即开头讲的"黑匣子"，是其功能组件之一

### 📲 客户端上报（Maint 模块）

| 类型 | 用途 | API | 限流 | 离线队列 | 关键约束 |
|------|------|-----|------|---------|---------|
| **Event** | 记录重要状态变化 | `submitEvent(id, json[, files])` | 无限流 | 500 | **ID 必须 9 位**（`100000000~999999999`） |
| **Mark** | 标记关键时间节点 | `submitMark(id, json)` | **令牌桶** 1 次/秒/EventID，桶容量 100 | 1000 | 超频丢弃并自动上报 `eventId=100010006` 告警 |

> [!info] 权限
> `CstMaint` **无 GUA 签名限制**，任何系统应用均可用（区别于 [[GFS]] 的 `CstDiag`）。

### ⚙️ 配置文件

| 文件 | 作用 |
|------|------|
| `components.json` | 组件开关 |
| `events.json` | 事件定义 |
| `actions.json` | 动作 |
| `env.json` | 服务器地址、磁盘限制、线程池 |
| `features.json` | 特性开关 |
| `logstorage.json` | 日志源 |
| `network.json` | 上传 |
| `export.json` | USB 导出 |

---

## 🔗 关联

- **上层 SDK**：[[CST SDK]]（Maint 模块）
- **兄弟服务**：[[GFS]]（故障，FATAL 会转发给 CornerStone 云端同步）、[[GDC]]（诊断通信）
- **崩溃分析**：[[RAMdump]]、[[tombstone（墓碑 验尸报告）]]
