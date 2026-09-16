---
title: GFS
tags:
  - 稳定性
  - AAOS
  - XG
  - 故障
  - 诊断
platform: "gua / guav100 (AAOS)"
created: 2026-09-16
updated: 2026-09-16
source: "raw/articles/XG/CST SDK 接入文档.pdf (v1.0, 2026-06)"
---

# GFS（Global Fault Service）

**GFS** 是 Gua 平台的**全局故障服务**（进程 `gfsd`，`post-fs-data` 启动），负责故障（Fault）的全生命周期管理——上报、恢复、订阅、查询。是 [[CST SDK]] **Diag 模块**的底层服务。

## 故障严重等级

| 值 | 常量 | 说明 |
|---|---|---|
| 0 | `CST_DIAG_SEVERITY_WARN` | 警告 |
| 1 | `CST_DIAG_SEVERITY_ERROR` | 错误 |
| 2 | `CST_DIAG_SEVERITY_FATAL` | 致命——**同步至内核驱动 + 云端** |

## 故障数据流（4 方向分发）

客户端 `IGfs.reportOccur()` 上报 → `GfsDispatcher` 分发到 4 个目标：

1. **StorageManager** — 持久化到 SQLite（`/data/vendor/gfs/stats/gfs_faults.db`）+ 事件文本文件（`/data/vendor/gfs/evtfiles/`）
2. **GfsSubscription** — 通知所有订阅该故障的回调
3. **GfsHub** — FATAL 级写入内核驱动 `/dev/gfs`
4. **TargetCornerstone** — 转发到 [[CST - CornerStone]] 维护服务（事件上报 + 云端同步）

故障恢复时同样通知这 4 个方向。

## Diag OTA

`IDiagOtaManager` 处理 UDS 诊断协议触发的 ECU 固件升级。OTA 进程实现 `CstDiagOtaCallback` 抽象接口并注册。结果码：`RESULT_PENDING(-1)` / `OK(0)` / `FAILED` / `TIMEOUT` / `BUSY` / `INVALID_PARAM` / `NOT_SUPPORTED` / `CONDITION_NOT_MET` / `SECURITY_DENIED`。同步在 `onEvent()` 直接返回，异步返回 `RESULT_PENDING` 后 `reportResult()` 上报。

## AIDL 接口

- `IGfs`：`reportOccur` / `reportRecovery` / `subscribe(s)` / `unsubscribe(s)` / `queryFaultsAll` / `queryFaultInfo` / `queryFaultCount` / `queryFaultsByTime`。
- `IDiagOtaManager`：`registerCallback` / `reportResult` / `confirmBlockReceived` / `releaseChannel`。

## 关联

- 上层 SDK：[[CST SDK]]（Diag 模块，`CstDiag`）
- 下游：[[CST - CornerStone]]（FATAL 故障转发 + 云端同步）
- 诊断产物：[[tombstone（墓碑 验尸报告）]]、[[RAMdump]]
- 故障类用例：[[TC_SF_FAULT_001]]、[[TC_HWC_FAULT_004]]、[[TC_CSOC_FAULT_003]]

## 📚 延伸阅读

- [Android car diagnostics / VHAL](https://source.android.com/docs/automotive/vhal) — AAOS 车辆诊断与属性上报机制，对照理解 GFS 故障通道定位。
