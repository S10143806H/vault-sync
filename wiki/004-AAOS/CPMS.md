---
title: CPMS（Car Power Management Service）
tags:
  - AAOS
  - CPMS
  - 电源
  - STR
platform: gua / guav100 (AAOS)
created: 2026-08-03
---

# CPMS（Car Power Management Service）

AAOS 框架层的**车辆电源管理服务**，负责在 ON / SHUTDOWN / [[STR]] 等电源状态间协调切换，并驱动 App/Service 有序收尾与恢复。

## 对 App/Service 的接口
- **CarPowerManager**：注册监听电源状态变化。
- **CarPowerStateListener**：仅监听（日志/展示），无需通知完成。
- **CarPowerStateListenerWithCompletion**：需在清理完成后调用 `future.complete()`，CPMS 才继续流转。

## 状态机主干
```
WAIT_FOR_VHAL → ON → SHUTDOWN_PREPARE → WAIT_FOR_FINISH → SUSPEND(DEEP_SLEEP)
```
- `STATE_SHUTDOWN_PREPARE`：**唯一适合做耗时清理**的阶段（可 postpone、等 listener 完成）。
- `STATE_SUSPEND_ENTER`：不允许 listener completion，不可回头。

## 电源策略（Power Policy）
CPMS 下发 [[电源策略]] 控制组件开关：
- `gua_policy_powersave`：关机/进 STR。
- `gua_policy_on`：开机/退出 STR。
- `gua_policy_sentinel_enter/exit`：进/出 [[哨兵模式]]。
- ⚠️ CPMS 会**全量上报**策略（含未变化的），Native 侧需做状态过滤去重。

## 调试命令
```bash
adb shell cmd car_service apply-power-policy gua_policy_powersave   # 模拟进 STR
adb shell cmd car_service apply-power-policy gua_policy_on          # 模拟退 STR
```

## 📚 延伸阅读
[AAOS Power management](https://source.android.com/docs/automotive/power/power?hl=zh-cn)

## 相关
[[STR]]｜[[哨兵模式]]｜[[KL15]]｜[[安全核]]
