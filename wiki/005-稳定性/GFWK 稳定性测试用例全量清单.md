---
title: "GFWK 稳定性测试用例全量清单 (55 用例)"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - GFWK
platform: "gua / guav100 (AAOS)"
created: 2026-07-31
---

# GFWK 稳定性测试用例全量清单 (55 用例)

> 来源：飞书[[GFWK 稳定性测试 — 需求规格说明书 v2]] §4。代码：`autocase/cases/MultiMedia/GPU/{Fault,Bound,Leak,Gralloc,Hwc,CrossSoc,Str,Gfwk,Stress,Car}`（branch `qi.zhu`）。
> **`❤️❤️` = 规格作者自标"最易挖出 bug"**。状态：✅实现 / 🧩骨架(pytest.skip 待填) / 🟡需 native 工具。

## 回归记录
- **2026-08-09 收严 5 个 kill 类用例 + 复用 common（SG0286, 各 5 轮）**：SF_FAULT_001 / HWC_FAULT_004 / STRESS_001 / CSOC_002 本地 5 轮 PASS，CSOC_005 经 GTMP(a720) PASS。GTMP 任务 98058 首轮 4/5（SF_FAULT_001 round5 偶发黑屏→加判黑重试修复）。**压出 3 发现**：[[BUG-SF-ctl-stopstart-显示不重建黑屏|F3 优雅重启SF显示永久黑]]、F1(kill-9 SF 级联重启 system_server)、F4(ctl.stop 须用 init 服务名 vendor.hwcomposer-3)。common 新增 `kill_proc/svc_autostart/svc_name_for/wait_render/wait_proc_alive`。提交 `086ab051`+`4ab84104`。详见 [[2026-08-09-GFWK收严杀进程压测]]。
- **2026-08-04 GFWK 全套 smoke 10/10 PASS（SG0286, 不刷机, 各 1 轮）**：GTMP 任务 95539，4 kill(SF/HWC/CSOC_composer_stub/weston) + 5 STR(001/002/004/005 + RECOVER_001) + **新 [[TC_GRAL_FAULT_001]] dma-heap OOM** 全通过（Pass Rate 100%, 14min）。经 `--params ROUNDS=1` 走新增 conftest `--params`→env 桥降 smoke。**TC_GRAL_FAULT_001 首次台架跑通不误报。**
- **2026-08-01 稳定集全绿（7/7 PASS, SG0286, fw 831）**：SF_FAULT_001/002、HWC_FAULT_003/004、CSOC_FAULT_002/004/005 经 `gfwk_fault_runner.sh`（含健康门）冒烟轮次全通过，`fail_count=0`、tombstone_delta=2（kill 噪声非 double-free）。HWC_FAULT_004 在 GTMP 上曾 FAIL（HWC DPMS 抖动），本地过健康门稳过。
  > ⚠️ GTMP 94087 标"刷机通过"但设备实际仍 831（#1194 未生效），本轮在 831 上跑；`build_20260731182538_1194` 待重刷验证。

## 状态一览

| 模块 | 目录 | 用例（❤️❤️=高 bug 产出） | 实现进度 |
|---|---|---|---|
| §3.1 SF/BufferQueue | Fault/Bound/Leak | FAULT_001✅ 002✅ 003 004；BOUND_001 002🟡 003🟡 004 005；LEAK_001/002/003 | 2/12 |
| §3.2 Gralloc/dma-buf | Gralloc/Leak | **FAULT_001✅❤️❤️(OOM,写完待验)**；CONF_001❤️❤️；FAULT_002❤️❤️(坏fd) 003/004；FUZZ_001❤️❤️；BOUND_001~004；LEAK_001/002；IFACE_001 | 1/13 **(历史 gap 最多)** |
| §3.3 HWC/合成 | Hwc | FAULT_002❤️❤️(坏Layer) 003❤️❤️(多屏隔离)；CONF_001；BOUND_001；FAULT_001；(+kill_hwc✅) | 0/3 +1 |
| §3.4 跨SoC | CrossSoc | FAULT_001✅(UAF) 002✅ **003✅ 004✅** 005✅；FAULT_004❤️❤️ SM_001❤️❤️ RECOVER_001/002❤️❤️；BOUND_002❤️；SM_002 RECOVER_003 BOUND_001/003 | 5/12 +2 |
| §3.5 STR | Str | STR_001~005（003/005 为 P0） | 0/5 |
| §3.6-3.8 并发·压力·实车 | Gfwk/Stress/Car | STRESS_001❤️❤️ 003❤️❤️；RACE_001 PERF_001 STRESS_002 CAR_001/002 | 0/7 |

## 已实现（可跑）
- [[TC_SF_FAULT_001]] kill SF｜[[TC_SF_FAULT_002]] AMS-WMS｜[[TC_SF_BOUND_002]] / [[TC_SF_BOUND_003]] 反压(需 native)
- [[TC_HWC_FAULT_004]] kill HWC(就绪门+跨SoC断言)
- [[TC_CSOC_FAULT_002]] kill composer_stub｜[[TC_CSOC_FAULT_005]] kill weston
- **[[TC_CSOC_FAULT_003]] IVI panic→Cluster**（2026-07-31 新）｜**[[TC_CSOC_FAULT_004]] GIPC 断/禁 SHMEM**（2026-07-31 新）
- TC_CSOC_FAULT_001 sync_file UAF (P0)（待台架验证）

## 压测用例矩阵（已实现，支持轮次压测）
> 已实现 = 非骨架、含内置循环轮次；轮次 env 可调大做长稳。GTMP 另可 `--infinite --repeat=N` 框架级重复任意用例（如 125299 用 `--repeat=50`）。统计（2026-08-03）：**~25 条 GFWK 压测用例已实现，其中 12 条已台架验证/smoke通过**（含 5 条 STR：STR_001/002/004/005 + RECOVER_001，均 1 轮 smoke PASS）。
>
> ⚠️ 说明：STR display 模式只验 IVI 侧；**A720 仪表真 STR 需 test_006(KL15，去 SG0285 有CAN那台)**。smoke=1轮仅证跑通/不误报，**泄漏类需长轮**(STR_001/002 CHECK_EVERY=20)。

| 模块 | 用例 | 轮次 env（默认） | 台架验证 |
|---|---|---|---|
| SF | [[TC_SF_FAULT_001]] kill SF | `SF_FAULT_ROUNDS=180` | ✅ 已验 |
| SF | [[TC_SF_FAULT_002]] AMS/WMS 突刺 | `SF_BINDER_BURST=200` | ✅ 已验 |
| SF | TC_SF_FAULT_003 EGL context lost | `SF_F003_ROUNDS=50` | 写完待验 |
| SF | TC_SF_FAULT_004 Binder 事务失败 | `SF_F004_ROUNDS=10` | 写完待验 |
| SF | [[TC_SF_BOUND_002]] Producer 反压 | native bq_producer 内循环 | 🟡 需 native |
| SF | [[TC_SF_BOUND_003]] Consumer 极限 | `SF_CONSUMER_ROUNDS=3` | 写完待验 |
| SF | TC_SF_LEAK_001 Surface 泄漏 | `SF_LEAK1_ROUNDS=200` | 写完待验 |
| HWC | TC_HWC_FAULT_001 PQ 持久化 | `HWC_F001_ROUNDS=3` | 写完待验 |
| HWC | TC_HWC_FAULT_003 多屏隔离 | `HWC_ISO_ROUNDS=3` | ✅ 已验 |
| HWC | **[[TC_HWC_FAULT_004]] kill HWC** | `HWC_FAULT_ROUNDS=50` | ✅ 已验(多台/多构) |
| 跨SoC | **TC_CSOC_FAULT_001 sync_file UAF (P0)** | `CSOC_UAF_ROUNDS=500` | 写完待验(长稳) |
| 跨SoC | [[TC_CSOC_FAULT_002]] kill composer_stub | `CSOC_FAULT_ROUNDS=100` | ✅ 已验 |
| 跨SoC | [[TC_CSOC_FAULT_003]] IVI panic→Cluster | `CSOC_P003_ROUNDS=3` | 写完待验(重/panic) |
| 跨SoC | [[TC_CSOC_FAULT_004]] GIPC 断 | `CSOC_GIPC_ROUNDS=20` | ✅ 已验 |
| 跨SoC | [[TC_CSOC_FAULT_005]] kill weston | `WESTON_ROUNDS=50` | ✅ 已验 |
| 跨SoC | [[TC_CSOC_RECOVER_001]] STR+跨SoC (=STR_003) | `CSOC_R001_ROUNDS=200` | ✅ smoke通过(08-03) |
| 跨SoC | TC_CSOC_RECOVER_002 A720重启→IVI | `CSOC_R002_ROUNDS=10` | 写完待验 |
| 跨SoC | TC_CSOC_SM_001 GIPC 状态机 | `CSOC_SM_ROUNDS=1000` | 写完待验 |
| STR | [[TC_GFWK_STR_001]] SF×STR | `STR_001_ROUNDS=200` | ✅ smoke通过(08-03) |
| STR | [[TC_GFWK_STR_002]] Gralloc×STR | `STR_002_ROUNDS=200` | ✅ smoke通过(08-03,已修图形口径) |
| STR | [[TC_GFWK_STR_004]] HWC×STR | `STR_004_ROUNDS=200` | ✅ smoke通过(08-03) |
| STR | [[TC_GFWK_STR_005]] 综合(P0) | `STR_005_ROUNDS=200` | ✅ smoke通过(08-03) |
| 综合 | TC_GFWK_RACE_001 SF+Gralloc+HWC 并发 | `RACE_ROUNDS=50` | 写完待验 |
| 综合 | TC_GFWK_STRESS_001 低内存+SF | `STRESS1_ROUNDS=20` | 写完待验 |
| 综合 | TC_GFWK_STRESS_002 FD 耗尽+Gralloc | `FD_HOG_N=8000` | 写完待验 |
| Gralloc | **[[TC_GRAL_FAULT_001]] dma-heap OOM** | `GRAL_F001_ROUNDS=10` | ✅ smoke通过(08-04, 注入可插拔) |
| 显示 | **[[TC_GFWK_STRESS_004]] 后排屏开合** | `REAR_TOGGLE_ROUNDS=10` | ✅ 10/10通过(08-04, 背光判据) |
| 显示 | **[[TC_GFWK_STRESS_005]] 环境光/自动亮度** | `AMBIENT_ROUNDS=10` | ✅ 10/10通过(08-04, 附亮度未跟随观察) |
| SF/HWC | **[[TC_GFWK_STRESS_006]] 多屏图层随机增删** | `STRESS_006_ROUNDS=10` | ✅ 三屏全命令通过(08-06,任务96839, IVI/Cluster/Rear×20层=60层, 跑满全部命令) |
| 显示/AIDL | **[[TC_GFWK_STRESS_008]] 后排屏 AIDL 电机压测** | `SCREENMOTOR_STRESS_DURATION=3600` | ✅ T29冒烟通过(08-07,任务97349,GFWK-T29-001,6阶段 ok=185/0fail; 原 GFWK-BASE-TEST-006/yue.shen) |

> 仍是骨架(pytest.skip 待填)：§3.2 `TC_GRAL_*` 多数、`TC_GFWK_CAR_*`、`TC_GFWK_STRESS_003` 等。（§3.5 `TC_GFWK_STR_001/002/004/005` + STR_003=RECOVER_001 均已实现）
> 仓内另有**既有 GPU 压测套**（`TC_GPU_*Stress*`/`TC_GPU_Stab_*`/`TC_DISP_GPUa_*Stress*`，非本 GFWK 工作，偏渲染/DFS/性能）。

## 下一批「最易测出问题」候选（❤️❤️ + 可注入）
1. ~~**TC_GRAL_FAULT_001** dma-heap OOM~~ ✅ 2026-08-03 写完待验（注入可插拔：设备内 allocator / stress-ng / am-churn）
2. **TC_CSOC_SM_001** GIPC 状态机 1000 轮 — 跨SoC，反复建链/断链找状态泄漏
3. **TC_HWC_FAULT_002** 坏 Layer 参数 — 需先造 native 工具喂坏参数
4. **GPU hang→reset / DPMS 抖动压测** — 栈层⑦缺口，对应已知 [[BUG-HWC-DPMS-SetPowerMode崩溃循环]]

> 图形栈分层→用例映射总图见 [[06-GFWK如何映射到测试用例]]（含 mermaid）。

## 关联
- 下一大类方法 → [[GFWK STR 挂起唤醒稳定性测试]]（§3.5，注入像 kill 一样轻、resume 最易暴露 bug）
- 自动化闭环 → [[GFWK 双bots自动挖bug闭环]]（飞书触发→跑→自动根因卡片）
- 方法论 → [[GFWK kill-恢复类测试]]｜四阶段套路见 gfwk-fault-case skill
- 总览 → [[000-GFWK图形框架总览]]｜规格 → [[GFWK 稳定性测试 — 需求规格说明书 v2]]
- 已知缺陷 → [[BUG-kill-HWC-crashes-A720-composer_stub]]
