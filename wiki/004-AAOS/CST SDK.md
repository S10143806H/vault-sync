---
title: CST SDK
tags:
  - AAOS
  - XG
  - 诊断
  - 故障
  - 维测
  - SDK
platform: "gua / guav100 (AAOS)"
created: 2026-09-16
updated: 2026-09-16
source: "raw/articles/XG/CST SDK 接入文档.pdf (v1.0, 2026-06)"
---

# CST SDK

**CST SDK（Cornerstone SDK）** 是 Gua 车载 [[中控 Android（IVI）|AAOS]] 平台的统一客户端开发套件，封装与底层三大服务的交互，给上层应用提供简洁的 C/C++ 和 Java API。一句话：**上报日志、故障、维护事件的"官方入口"**。

> ⚠️ 本页据 SDK 接入文档整理，属平台内部 SDK；三大模块分别对接 [[GDC]] / [[GFS]] / [[CST - CornerStone]] 三个底层服务。

## 三大模块 → 底层服务映射

| 模块 | 头文件 / Java 类 | 核心能力 | 底层服务 |
|---|---|---|---|
| **Debug** | `cst_debug.h` | 日志上报、命令注册与处理、RawData 传输 | [[GDC]]（`IGdcHalService`） |
| **Diag** | `cst_diag.h` / `cst_diag_ota.h` / `cst.diag.CstDiag` | 故障上报/恢复、订阅、查询、OTA 升级 | [[GFS]]（`IGfs` / `IDiagOtaManager`） |
| **Maint** | `cst_maint.h` / `cst.maint.CstMaint` | 事件上报、点位（Mark）上报 | [[CST - CornerStone]]（`ICornerStoneService`） |

## 平台差异（关键约束）

| 特性 | IVI | Cluster（[[仪表]]） | PAD |
|---|---|---|---|
| **Debug 模块** | ✅ | ✅ | ❌ 不可用 |
| Diag 模块 | ✅ | ✅ | ✅ |
| Maint 模块 | ✅ | ✅ | ✅ |
| Diag OTA | ✅ | ✅ | ✅ |

> ❗ **Debug 在 PAD 不可用**：Soong 插件 `cst.go` 编译时按 `deviceProduct` 判断，PAD（`gua1_pad`）不含 Debug 源码与 GDC 依赖，仅加 `-DGUA_PAD=1`。

## 接入要点

- **C/C++（Android.bp）**：`shared_libs: ["libcst", ...]`；按模块追加 `vendor.gua.hardware.gdc-V1-ndk`（Debug）/ `gua.gfs-V1-ndk`（Diag）/ `vendor.gua.cornerstone.transfer-V1-ndk`（Maint）。
- **Java / APK**：依赖 `cst-sdk.jar` + 对应 AIDL jar。SDK 用了隐藏 API（`android.os.ServiceManager`），APK 必须 **system/privileged 签名**，预装到 `system_ext` 分区，普通三方应用无法用。
- **签名限制**：Java `CstDiag` 内置 **GUA 证书签名校验**（`isGUASignature()`），非 GUA 签名调用被拒；`CstMaint` **无签名限制**。

## API 速记

- **Debug**：`cst_debug_submit_log[_with_tag]` / `_submit_log_buffer`（二进制）/ `_register_cmd_handler(s)`（同步 vs 异步）/ `_submit_response`（异步回结果）/ RawData 走共享内存或文件。子系统 ID 见 [[GDC]]。返回码 `0=OK`，`ERR_SERVICE_OFFLINE=7` 等。
- **Diag**：`CstDiag::createFault/submitFault/submitRecovery`；订阅 `subscribeFault(s)`；查询 `queryAllFaults/queryFaultInfo/queryFaultCount/queryFaultsByTime/isFaultOccurred`。严重级 `WARN(0)/ERROR(1)/FATAL(2)`——**FATAL 会同步内核驱动+云端**。⚠️ 纯 C API 只有上报/恢复，订阅查询需 C++/Java。
- **Maint**：`CstMaint.submitEvent(id, json[, files])` / `submitMark(id, json)`。**Event ID 必须 9 位**（`100000000~999999999`）。Mark 有令牌桶限流（1次/秒/EventID，桶容量 100），超频丢弃并自动上报 `eventId=100010006` 告警。

## SDK 内部机制

- **服务自动重连**：三模块均注册 `AServiceManager_registerForServiceNotifications` + `DeathRecipient`，服务崩溃→恢复后重建连接并 flush 缓存。
- **离线缓存**：服务离线期间自动缓存，恢复后自动 flush。容量：Maint Event 500 / Mark 1000；Debug 按 tag 分组队列；Diag 故障缓存重发。
- **限流保护**：见上 Maint Mark 令牌桶。

## CLI 工具（属 GDC）

`glogcat`（日志提取）/ `gremoteshell`（远程命令）/ `gscp`（IVI↔Cluster 安全文件拷贝）——详见 [[GDC]]。

## 配置文件参考

- **GDC**：`gdc_ivi.xml` / `gdc_cluster.xml` / `shm.xml`（共享内存布局）。
- **GFS**：`/data/vendor/gfs/stats/gfs_faults.db`（故障 SQLite）、`/data/vendor/gfs/evtfiles/`。
- **Cornerstone**：`components.json` / `events.json` / `actions.json` / `env.json` / `features.json` / `logstorage.json` / `network.json` / `export.json`。

## 关联

- 底层服务：[[GDC]]、[[GFS]]、[[CST - CornerStone]]
- 跨SoC 基础：[[GIPC]]、[[SHMEM]]、[[安全核]]、[[仪表]]、[[中控 Android（IVI）]]
- 接口/通信：[[vendor AIDL]]、[[Binder IPC]]
- 诊断产物：[[tombstone（墓碑 验尸报告）]]、[[RAMdump]]

## 📚 延伸阅读

- [Android Automotive OS 概览](https://source.android.com/docs/automotive/start/what_automotive) — AAOS 平台定位，理解 CST SDK 所处的车载系统层级。
- [AIDL for HALs](https://source.android.com/docs/core/architecture/aidl/aidl-hals) — CST 三模块底层均走 AIDL Binder，理解 HAL 接口机制。
