---
title: GDC
tags:
  - AAOS
  - XG
  - 跨SoC
  - 诊断
  - 日志
platform: "gua / guav100 (AAOS)"
created: 2026-09-16
updated: 2026-09-16
source: "raw/articles/XG/CST SDK 接入文档.pdf (v1.0, 2026-06)"
---

# GDC（Global Diagnostic Communication）

**GDC** 是 Gua 平台的**全局诊断通信服务**（进程 `gdcd`，`post-fs` 启动），负责与各异构子系统（[[仪表|Cluster]]、ADSP、BPU、[[安全核|CP]] 等）收发日志、命令、二进制大数据。是 [[CST SDK]] **Debug 模块**的底层服务。

> ⚠️ Debug 模块 / GDC 仅 IVI 与 Cluster 可用，**PAD 设备不可用**。

## 子系统 ID

每个异构芯片子系统分配唯一 ID，用于标识日志/命令的来源/目标：

| ID | 常量 | 说明 |
|---|---|---|
| 1 | `CST_SUBSYS_ID_CLUSTER` | [[仪表]]盘 |
| 2–4 | `CST_SUBSYS_ID_ADSP1..3` | 音频 DSP |
| 5–8 | `CST_SUBSYS_ID_BPU1..4` | BPU 神经网络处理器 |
| 9–11 | `CST_SUBSYS_ID_CP0` / `CP1_C0` / `CP1_C1` | 功能[[安全核]] |

## 通道类型

| 通道 | 类 | 说明 |
|---|---|---|
| SHM | `ShmChannel` | 与 Cluster 走物理[[SHMEM|共享内存]] |
| FakeShm | `FakeShmChannel` | Binder 模拟共享内存（测试用） |
| HAL | `HalChannel` | HAL 层通信（ADSP） |
| UART | `UartChannel` | 串口 |
| GIPC | `GIPCChannel` | [[GIPC]] 进程间通信 |
| Node | `NodeChannel` | 设备文件读取（如 `/dev/log_bl31_cluster`） |
| PStore | `PStoreChannel` | Panic Store 读取（`/sys/fs/pstore/`），关联 [[RAMdump]] |
| GSCP | `GScpChannel` | 安全文件传输 |

## RawData 机制

子系统通过共享内存高效传输大块二进制（coredump、内存快照等）：`cst_debug_get_rawdata_buffer` 取缓冲 → `memcpy` → `cst_debug_submit_rawdata`；或直接 `cst_debug_submit_rawdata_file` 提交文件。

## CLI 工具

| 工具 | 用途 | 示例 |
|---|---|---|
| `glogcat` | 提取子系统日志 | `glogcat -b cluster --tag=tombstone` |
| `gremoteshell` | 远程命令/交互 shell | `gremoteshell -s cluster -c "ls /tmp"` |
| `gscp` | IVI↔Cluster 安全文件拷贝 | `gscp cluster:/tmp/f.txt /mnt/` |

## libgdc

独立于 CST SDK 的底层客户端库，直连 GDC，两种模式：
- **Socket 模式**（默认，Unix Domain Socket）：适合 CLI/独立进程，函数名无后缀。
- **Binder 模式**（`IGdcLibService` AIDL）：适合 Android 应用，Binder 安全隔离 + 死亡通知，函数名带 `_binder` 后缀。

## AIDL 接口

- `IGdcHalService`：`submitLog` / `submitCstBuffer` / `registerCallBack` / `submitResponse` / `submitRawdata[File]`。
- `IGdcLibService`：`subscribeLog` / `subscribeRawdata` / `sendCmd`。

## 关联

- 上层 SDK：[[CST SDK]]（Debug 模块）
- 跨SoC 基础：[[GIPC]]、[[SHMEM]]、[[安全核]]、[[仪表]]、[[SERDES链路]]
- 诊断产物：[[tombstone（墓碑 验尸报告）]]、[[RAMdump]]
- 通信：[[Binder IPC]]、[[vendor AIDL]]
