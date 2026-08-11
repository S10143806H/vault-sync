---
title: STR（整车休眠 / Suspend-to-RAM）
tags:
  - AAOS
  - STR
  - 电源
  - CPMS
  - 座舱
platform: gua / guav100 (AAOS)
created: 2026-08-03
---

# STR（Suspend-to-RAM，整车休眠）

**一句话**：把整个系统"现场"冻结在 [[RAM]] 里，主 SoC 断电，只留 [[安全核]] + [[KL15]] 检测电路靠常电（KL30）盯着唤醒源；解锁后从内存秒速恢复，不用冷启动。

> STR 不是"一个动作"，而是一条跨 **安全岛(CP0/CP1) → Android 用户态(CPMS/App) → Kernel** 的**流水线**。

## 流程图
🔴Android · 🟡Kernel · 🔵整车状态

```mermaid
flowchart TD
    ON[整车 ON] --> LOCK[锁车 KL15 拉低]
    LOCK --> S1["1 安全岛/Native<br/>powerSM 上报 STR·功放下电·吸顶屏关"]
    S1 --> S2["2 VHAL<br/>关背光·延迟 15s 通知·gua_policy_powersave"]
    S2 --> S3["3 Native 策略<br/>日志·性能·audio 关闭"]
    S3 --> S4["4 CPMS 收尾<br/>灭屏·关 wifi/蓝牙·等 App 完成·杀三方"]
    S4 --> FREEZE[/"Kernel 冻结到内存"/]
    FREEZE --> SLEEP(((休眠 STR)))
    SLEEP -. KL15 拉高 解锁 .-> WAKE[/"Kernel 唤醒用户态"/]
    WAKE --> S5["5 Native + App<br/>wifi/蓝牙/input 恢复·重建状态"]
    S5 --> S6["6 VHAL 通过 PowerSM<br/>通知安全岛 ivi 唤醒"]
    S6 --> ON2[整车 ON]

    classDef sec fill:#fde2e4,stroke:#e5989b;
    classDef ker fill:#fff3c4,stroke:#e0a800;
    classDef sta fill:#dbe4ff,stroke:#748ffc;
    class S1,S2,S3,S4,S5,S6 sec;
    class FREEZE,WAKE ker;
    class ON,LOCK,SLEEP,ON2 sta;
```

**下电 1→6 从用户态往下压到 Kernel 冻结；唤醒反向从 Kernel 往上恢复，对称。**

## 关键区分（最容易踩坑的三点）

### 1. 电源状态 vs 电源策略
| | 电源状态 (Power State) | 电源策略 (Power Policy) |
|---|---|---|
| 含义 | 系统整体档位：ON / SHUTDOWN / **SUSPEND(STR)** | 哪些组件开/关（display、wifi、audio…）|
| 谁管 | Kernel 真断电/冻内存 | [[CPMS]] 下发，**CPU 活着才能执行** |

### 2. "系统等你" vs "你等系统"（清理为何必须放 SHUTDOWN_PREPARE）
- `SHUTDOWN_PREPARE`：**会主动等你**——`canPostpone=true`，可反复 postpone，等所有 listener `future.complete()`。弹性可达数分钟。
- `SUSPEND_ENTER`：**不等你**——`state(7) doesn't allow listener completion`，不可回头点，清理会被从中间截断 → 唤醒后状态不一致/coredump。
- **口诀**：耗时清理/存数据 **必须放 `STATE_SHUTDOWN_PREPARE`**。

### 3. STR vs [[哨兵模式]]（互斥）
- 哨兵要监控 → **SoC 保持唤醒**，只用 `gua_policy_sentinel_enter` 关屏/音频，留摄像头算力。
- STR → **SoC 真睡全冻结**，任何 process 停摆。
- **要哨兵就不进 STR；进了 STR 哨兵也一起冻。** 锁车瞬间按"要不要监控"**分岔二选一**：

```mermaid
flowchart TD
    L[锁车 KL15 拉低] --> Q{需要监控?}
    Q -->|是| SENTRY["哨兵模式<br/>SoC 醒着·摄像头录<br/>gua_policy_sentinel_enter<br/>费电(约 1 英里续航/小时)"]
    Q -->|否| STR["STR 休眠<br/>SoC 断电·全冻结<br/>gua_policy_powersave<br/>近 0 耗电"]
    SENTRY -->|CP0 输入 work / set_ready_status 0| WAKE[恢复 ON]
    STR -->|KL15 拉高 或 重新上电冷启动| WAKE

    classDef sentry fill:#ffe8cc,stroke:#f08c00;
    classDef str fill:#d3f9d8,stroke:#2f9e44;
    class SENTRY sentry;
    class STR str;
```

> 物理互斥：**不能既 CPU 断电极省电，又 CPU 跑算法监控**。省电 ↔ 监控，二选一。

## CPMS 状态机主干
```
WAIT_FOR_VHAL → ON → SHUTDOWN_PREPARE → WAIT_FOR_FINISH → SUSPEND(DEEP_SLEEP) → [冻结]
              ON ← WAIT_FOR_VHAL ← Resuming ←──────────────────────────── KL15↑
```

## 逐行日志的时间证据
| 阶段 | 耗时 | 说明 |
|---|---|---|
| VHAL 延迟通知 | 15 s | 给收尾留缓冲 |
| `SHUTDOWN_PREPARE`（含 GarageMode + postpone） | **~218 s** | 系统"等你"，弹性 |
| `SUSPEND_ENTER → Entering Suspend-to-RAM` | **~3 ms** | "你等系统"，不可回头 |

关键行：`Entering Suspend-to-RAM`（冻内存）/ `Resuming after suspending`（唤醒）。

## 台架测试要点
- 必须找到 **绿色 KL15 信号线**（外接继电器模拟点火/熄火）：`KL15 拉低=锁车`、`KL15 拉高=解锁`。
- 触发 STR：`adb shell dumpsys vendor.gua.hardware.power.IPower/default --report 2`（关机用 `--report 1`）。
- ⚠️ 安全岛暂未实现软唤醒 → **进 STR 后只能重新上电冷启动**。
- 模拟策略：`adb shell cmd car_service apply-power-policy gua_policy_powersave`（醒：`gua_policy_on`）。
- reboot 测试可覆盖关机测试：CP0 串口输入 `reboot`。
- 出问题 dump：CP0 `sm_state`；CP1 `sm_state` / `dfx_dump plc|ss|is|ipc`。

## 📚 延伸阅读
- [AAOS Power management](https://source.android.com/docs/automotive/power/power?hl=zh-cn)
- [AAOS Power policy](https://source.android.com/docs/automotive/power/power_policy?hl=zh-cn)

## 相关
[[CPMS]]｜[[哨兵模式]]｜[[KL15]]｜[[安全核]]｜[[中控 Android（IVI）]]｜[[座舱]]
