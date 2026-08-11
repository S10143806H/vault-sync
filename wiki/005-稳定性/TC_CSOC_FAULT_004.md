---
title: "TC_CSOC_FAULT_004 — GIPC 通道断开/禁 SHMEM 降级 (教学版)"
tags: [稳定性, AAOS, 测试用例, 故障注入, GIPC, SHMEM, 跨SoC, 教学]
case_id: TC_CSOC_FAULT_004
platform: "gua / guav100 (AAOS)"
created: 2026-07-31
updated: 2026-08-01
status: passed
tested_on: 2026-08-01
---

# TC_CSOC_FAULT_004 — GIPC 通道断开 / 禁 SHMEM 降级 【已测通过】

> **【已测通过】 实测通过（2026-08-01, SG0286, fw 831）**：3 轮断 GIPC 均 0.1s 重连，A720 无新崩溃。

## ① 一句话
**把跨芯片的"传输管道"整个掐断，看两颗 SoC 是各自降级（画面停在最后一帧/不更新）还是直接崩，恢复后能否自动重连。**

## ② 原理：断的到底是什么
[[TC_CSOC_FAULT_002|上一个用例]] kill 的是"投屏桥业务进程"，这个用例更狠——断的是**传输通道本身**：

- **[[GIPC]]（控制面）**：跨芯片的信令总线。断了 = 两颗 SoC 之间"失联"，谁也不知道对面帧到没到、fence 有没有 signal。
- **[[SHMEM]]（数据面）**：跨芯片共享内存。断了 = 像素数据传不过去。
- 承载它们的守护进程：`gipc_sdd`（底层传输 daemon）+ `vendor.gua.hardware.cluster-service`（显示 HAL）。**同时 kill 这两个 = 通道断开**。

> **正确行为叫"降级不崩"**：通道断了，仪表屏应"冻在最后一帧"或显示降级画面，**不能整个 crash**；通道恢复后应**自动重连**继续投屏。这才是健壮的跨 SoC 设计。

## ③ 测试逻辑（流程图）
```mermaid
flowchart TD
    A["① 基线: GIPC进程pid + SF pid + A720崩溃计数"] --> B["② 注入: kill -9 gipc_sdd 和 cluster-service(同时)"]
    B --> C{"③ SLA<20s: 两进程都 respawn 重连?"}
    C -->|"否"| F["❌ 重连失败"]
    C -->|"是"| D{"④ 主屏降级不黑?"}
    D -->|"黑"| S["❌ 降级即崩(画面丢)"]
    D -->|"OK"| E{"A720 崩溃计数增加?"}
    E -->|"增加"| H["❌ 断IVI通道拖崩A720<br/>(跨SoC耦合缺陷)"]
    E -->|"没变"| I{"SF pid不变?"}
    I -->|"变"| J["❌ 连累IVI SF"]
    I -->|"不变"| P["【已测通过】 通过"]
    style F fill:#f8d7da
    style S fill:#f8d7da
    style H fill:#f8d7da
    style J fill:#f8d7da
    style P fill:#d4edda
```

## ④ 交互时序（时序图）
```mermaid
sequenceDiagram
    participant T as 测试脚本
    participant ADB as adb(IVI)
    participant G as GIPC进程(gipc_sdd+cluster-service)
    participant INIT as init
    participant A720 as A720(串口)
    T->>ADB: pidof gipc_sdd / cluster-service (基线)
    T->>A720: dmesg|grep 崩溃 (基线计数 a0)
    T->>ADB: kill -9 <两个pid>
    ADB->>G: SIGKILL (通道断)
    Note over A720: 仪表应冻在最后一帧(降级), 不崩
    INIT->>G: respawn 两进程
    loop 轮询<20s
        T->>ADB: pidof 两进程 → 都在 = 重连
    end
    T->>ADB: screencap 主屏 → 非黑(降级OK)
    T->>A720: dmesg|grep 崩溃 → 计数==a0 (没拖崩)
```

## ⑤ 本地复现（台架, 逐条 adb + 串口）
```bash
adb -s A41AEC42 root
adb -s A41AEC42 shell pidof gipc_sdd
adb -s A41AEC42 shell pidof vendor.gua.hardware.cluster-service
adb -s A41AEC42 shell kill -9 <pid1> <pid2>        # 同时断两个
adb -s A41AEC42 shell pidof gipc_sdd               # 等几秒→应 respawn
```
```bash
# a720 串口另查是否被拖崩：
dmesg | grep -iE 'composer_stub.*segfault|GIPC.*(panic|reset)'
```
自动化：`CSOC_GIPC_ROUNDS=3 pytest cases/MultiMedia/GPU/CrossSoc/TC_CSOC_FAULT_004.py --bench=<yaml> -v`

## ⑥ 易出 bug 的环节（重点）
| 环节 | 为什么易出 bug | 判据 |
|---|---|---|
| **降级 vs 崩溃** | 🎯 核心：通道断了，接收端等一个永远不来的帧/fence → 若无超时保护会**卡死或崩**（正确应降级冻屏） | 主屏非黑 + A720 不崩 |
| **跨 SoC 拖崩** | 断 IVI 侧通道，若 A720 侧对"上游消失"无保护 → segfault（同 composer_stub 那类耦合） | a720 dmesg 无新 segfault |
| **重连幂等** | respawn 后要能干净重建通道；若残留旧共享内存/fd → 重连失败或错帧 | <20s 两进程都回来 |
| **fence 泄漏** | 断链瞬间"已借出未 signal"的 fence 若不回收 → 长跑句柄泄漏 | 长轮次跑 + 查 fd |

> **一句话记住**：健壮的跨 SoC = **"断了要降级不崩、恢复要自动重连、且一侧断不拖崩另一侧"**。这三点任一破 = bug。

## 关联
- 同簇 → [[TC_CSOC_FAULT_002]](投屏桥) [[TC_CSOC_FAULT_003]](IVI panic) [[TC_CSOC_FAULT_005]](weston)
- 机制 → [[GIPC]] [[SHMEM]] [[composer_stub]]｜总览 [[GFWK 稳定性测试用例全量清单]]

## 📚 延伸阅读
- 共享内存 IPC 概念：https://man7.org/linux/man-pages/man7/shm_overview.7.html
- Android sync/fence（断链 fence 未 signal 的根因）：https://source.android.com/docs/core/graphics/sync
