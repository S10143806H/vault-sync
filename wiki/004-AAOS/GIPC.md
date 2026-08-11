---
title: GIPC（Gua Inter-SoC 通信）
tags:
  - AAOS
  - GIPC
  - 跨SoC
  - IPC
  - 座舱
platform: "gua / guav100 (AAOS)"
created: 2026-07-30
---

# GIPC（Gua IPC）

> **一句话**：GIPC = 中控 IVI 与仪表 A720 **两颗 SoC 之间的控制通道**——相当于跨芯片版的 [[Binder IPC|Binder]]，传"有新帧、buffer 在哪、[[Fence]] 举牌没"这类**小消息**。

## 为什么需要它
Binder 只能在**同一颗 SoC 内**通信。IVI 和 A720 是**两颗物理芯片**，要协同投屏就需要一条跨芯片的 RPC/消息通道 → GIPC。像素本体太大不走它（走 [[SHMEM]]），GIPC 只传**控制信息**。

## 控制面 vs 数据面
| 面 | 跨 SoC | 传什么 |
|---|---|---|
| **控制面** | **GIPC** | buffer handle、fence 编号、连接/状态（小） |
| 数据面 | [[SHMEM]] | 像素本体（几 MB） |

> [[composer_stub]] 把像素放进 [[SHMEM]]，再用 GIPC 喊一句"内容在 N 号槽 + fence 编号"，A720 侧 [[Weston]] 收到就去 SHMEM 取像素上屏。

## 类比
两个教室之间的**对讲机**：大件放共用储物柜([[SHMEM]])，对讲机(GIPC)只喊"东西在 3 号柜、画完了(fence)"。

## 本平台真实进程
`gipc_sdd`（GIPC/SHMEM 传输 daemon）——跨 SoC 桥的底层传输；
上层显示 HAL 是 `vendor.gua.hardware.cluster-service`。

## 关联风险
- **GIPC 通道断开** → 投屏断、需重连（状态机：连接→断开→重连）
- **fence 跨 SoC 丢失** → A720 读到未 signal 的 buffer → 花屏/UAF（见 [[05-Fence与跨SoC同步]]）
- 相关用例：`TC_CSOC_SM_001`（GIPC 状态机 1000 轮）、`TC_CSOC_FAULT_004`（GIPC 通道断开）

## 关联
- 数据面搭档 → [[SHMEM]]｜发送桥 → [[composer_stub]]｜接收合成 → [[Weston]]
- 芯片内对应物 → [[Binder IPC]]｜机制 → [[Fence]]
- 上级 → [[000-GFWK图形框架总览]]｜跨 SoC 用例 → [[TC_CSOC_FAULT_002]] · [[TC_CSOC_FAULT_005]]
