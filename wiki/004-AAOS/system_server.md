# system_server

Android 最核心的**系统服务宿主进程**，由 `zygote` fork 出来，承载了几十个关键系统服务。

## 主要住户

| 服务 | 职责 |
|---|---|
| [[AMS]] | App 生命周期管理 |
| [[WMS]] | 窗口管理 |
| PackageManagerService | APK 安装/查询 |
| InputManagerService | 触摸/按键事件路由 |
| PowerManagerService | 亮屏/息屏/唤醒锁 |

## 稳定性特性

- **[[Binder IPC|Binder]] 线程池上限 15 条**：所有住户共享，高并发时互相竞争
- **AMS/WMS 共享锁**：两者之间锁序不当会死锁 → `ANR in system_server`（[[ANR|ANR]]）
- **WATCHDOG 监控**：system_server 内置 watchdog，30s 无响应 → 自杀重启（触发全局 App 重启）
- **与 SF 完全隔离**：system_server 崩溃/重启不应波及 [[SurfaceFlinger]]（独立进程）

## 相关
[[AMS]]｜[[WMS]]｜[[Binder IPC]]｜[[SurfaceFlinger]]｜测试：[[TC_SF_FAULT_002]]
