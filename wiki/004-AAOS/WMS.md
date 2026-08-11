---
title: WMS (WindowManagerService)
tags:
  - AAOS
  - WMS
  - system_server
  - AMS
  - Binder
  - SurfaceFlinger
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# WMS (WindowManagerService)

住在 [[system_server]] 进程里，负责**所有窗口的位置、层级、焦点、动画**。

## 关键职责

| 职责 | 说明 |
|---|---|
| 窗口层级 | 决定哪个窗口在最上面（z-order） |
| Display token | 向 [[SurfaceFlinger]] 分配 display token，SF 靠这个识别窗口 |
| 输入路由 | 与 InputDispatcher 配合，把触摸事件路由到正确窗口 |
| 与 AMS 共锁 | WMS lock ↔ [[AMS]] lock 可能死锁 |

## 与 SF 的关系

WMS 持有 display token，SF 的 transaction 需要从 WMS 拿 token。WMS 死锁时 SF 等待 token 可能超时（[[Binder IPC|Binder]] 5s 超时），超时后 SF 跳帧继续工作——**不应永久阻塞**。

## 相关
[[system_server]]｜[[AMS]]｜[[Binder IPC]]｜[[SurfaceFlinger]]｜测试：[[TC_SF_FAULT_002]]
