---
title: AMS (Activity Manager Service)
tags:
  - AAOS
  - AMS
  - system_server
  - WMS
  - Binder
  - ANR
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# AMS (Activity Manager Service)

住在 [[system_server]] 进程里，负责**所有 App 的生命周期**：启动、切后台、杀进程、权限管控。

## 关键职责

| 职责 | 说明 |
|---|---|
| App 启动 | `am start` 最终调到 AMS → 创建进程 → 启动 Activity |
| 后台限制 | 内存不足时 AMS 按优先级杀 App |
| [[ANR\|ANR]] 检测 | 主线程 5s 未响应 → AMS 触发 `ANR in <package>` |
| 与 WMS 共锁 | AMS lock ↔ [[WMS]] lock 可能形成死锁（TC_SF_FAULT_002 的注入目标） |

## 稳定性风险

AMS 与 [[WMS]] 共享锁，高并发 [[Binder IPC|Binder]] 请求（`dumpsys activity` × 200）可打满线程池 → `WATCHDOG` / [[ANR|ANR]]。
**注意**：AMS 死锁应只影响 [[system_server]]，不能波及 [[SurfaceFlinger]]（独立进程）。

## 相关
[[system_server]]｜[[WMS]]｜[[Binder IPC]]｜测试：[[TC_SF_FAULT_002]]
