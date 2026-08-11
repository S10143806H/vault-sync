# Index — 内容目录

> 本 vault 的全站目录, 按域分类。karpathy LLM-Wiki 模式: 人读, LLM 维护。
> 新增/改动记 [[log]]; schema 见根 `CLAUDE.md`。

**共 114 篇**

## 000-GenerativeAI — 生成式 AI 基础

_(空)_

## 001-Agent — Agent / 编码助手  (4)

- [[Claude Code]]
- [[Codex]] — 第一章： Codex简介
- [[Deep sleep]]
- [[OpenAI Codex 使用]]

## 002-MCP — MCP 工具  (2)

- [[feishu-cli]]
- [[playwright]]

## 003-Skills — Skills 技能  (3)

- [[AGENTS.md]]
- [[skill-creator]] — 参考资料
- [[teach]]

## 004-AAOS — AAOS 图形栈 / 跨SoC (显示知识库)  (50)

- [[000-GFWK图形框架总览]] — GFWK 图形框架总览
- [[01-一帧画面是怎么上屏的]] — 01 - 一帧画面是怎么上屏的（大图景）
- [[02-SurfaceFlinger与BufferQueue]] — 02 - SurfaceFlinger 与 BufferQueue（班长怎么收纸）
- [[03-Gralloc与dma-buf]] — 03 - Gralloc 与 dma-buf（发纸的 + 纸库存）
- [[05-Fence与跨SoC同步]] — 05 - Fence 与跨SoC同步（"画好了"举手 + 隔壁班）
- [[06-GFWK如何映射到测试用例]] — 06 - GFWK 如何映射到测试用例
- [[07-Android架构分层]] — 07 - Android 架构分层总览
- [[AAOS+Cluster-学习地图]] — AAOS + Cluster 学习地图（从哪入手）
- [[AMS]] — AMS (Activity Manager Service)
- [[Android Framework]]
- [[Binder IPC]]
- [[Buffer Queue]]
- [[CI构建与产物流程]] — CI 构建与产物流程（Jenkins → JFrog Artifactory）
- [[CPMS]] — CPMS（Car Power Management Service）
- [[Consumer]] — 消费者 (Consumer)
- [[Fence]]
- [[GIPC]] — GIPC（Gua Inter-SoC 通信）
- [[Gralloc]]
- [[GraphicBuffer]]
- [[HWC]] — 04 - HWC 硬件混合渲染器
- [[KL15]]
- [[SELinux]] — SELinux（强制访问控制 / avc denied）
- [[SERDES链路]] — SERDES 链路(显示/摄像头串行链路)
- [[SHMEM]] — SHMEM（跨 SoC 共享内存）
- [[SIGSEGV]] — SIGSEGV（段错误 / signal 11）
- [[STR]] — STR（整车休眠 / Suspend-to-RAM）
- [[SurfaceControl]]
- [[SurfaceFlinger]]
- [[VSync]]
- [[WMS]] — WMS (WindowManagerService)
- [[Weston]] — Weston（A720 Cluster Wayland 合成器）
- [[composer_stub]]
- [[dma-buf heap]]
- [[system_server]]
- [[vendor AIDL]] — vendor AIDL(厂商 HAL 接口)
- [[zygote]] — zygote（Java 进程母体 + SF 崩溃恢复的关键）
- [[中控 Android（IVI）]]
- [[代码编译]]
- [[仪表]] — 仪表（Cluster / a720）
- [[后排屏]] — 后排屏开合与亮灭控制（V27 / T29）
- [[哨兵模式]] — 哨兵模式（Sentry Mode）
- [[安全核]] — 安全核（CP0 / CP1）
- [[座舱]]
- [[显示链路]]
- [[智驾（ADAS）]]
- [[生产者]] — 生产者 (Producer)
- [[硬件拓扑]]
- [[编译native工具-BufferQueue 反压测试]] — BufferQueue 反压测试 (bq_producer)
- [[退出码101 vs SIGSEGV（composer_stub 崩溃判定）]]
- [[重点用例-高亮]] — 重点用例 ❤️（SF / HWC / GFWK 黄色高亮）

## 005-稳定性 — 稳定性测试 (用例·故障·BUG)  (46)

- [[2026-08-09-GFWK收严杀进程压测]] — 2026-08-09 GFWK 收严杀进程压测（5用例×5轮）
- [[ANR]] — ANR - Application Not Responding
- [[BUG-HWC-DPMS-SetPowerMode崩溃循环]] — BUG: HWC 在 DPMS SetPowerMode DRM ioctl 卡死→崩溃循环拖崩 system_server
- [[BUG-SF-ctl-stopstart-显示不重建黑屏]] — 澄清: stop/start SF 后卡开机动画 —— 预期行为(非缺陷)
- [[BUG-STRESS006-Cluster撕裂花屏闪屏]] — BUG-STRESS006 仪表屏(Cluster)撕裂/花屏/闪屏
- [[BUG-kill-HWC-crashes-A720-composer_stub]] — BUG: kill IVI HWC 拖崩 A720 composer_stub (SIGSEGV @ libwayland)
- [[CornerStone]]
- [[GFWK STR 挂起唤醒稳定性测试]] — GFWK STR 挂起-唤醒稳定性测试（§3.5 方法与用例）
- [[GFWK kill-恢复类测试]] — GFWK kill-恢复类测试（SF / HWC / composer_stub / weston）
- [[GFWK 双bots自动挖bug闭环]] — GFWK 自动挖 bug 闭环（feishu-gtmp-bot + gtmp-analyze-bot）
- [[GFWK 稳定性测试 — 需求规格说明书 v2]]
- [[GFWK 稳定性测试用例全量清单]] — GFWK 稳定性测试用例全量清单 (55 用例)
- [[GFWK稳定性-阅读脉络]] — GFWK 稳定性 — 阅读脉络 (MOC)
- [[OS reboot]]
- [[RAMdump]]
- [[SIGKILL （kill -9）]]
- [[STR]] — STR (Suspend-to-RAM)
- [[TC_CSOC_FAULT_002]] — TC_CSOC_FAULT_002 — kill composer_stub 跨SoC投屏恢复 (教学版)
- [[TC_CSOC_FAULT_003]] — TC_CSOC_FAULT_003 — IVI panic → Cluster 恢复
- [[TC_CSOC_FAULT_004]] — TC_CSOC_FAULT_004 — GIPC 通道断开/禁 SHMEM 降级 (教学版)
- [[TC_CSOC_FAULT_005]] — TC_CSOC_FAULT_005 — kill weston (A720 合成器) 恢复 (教学版)
- [[TC_CSOC_RECOVER_001]] — TC_CSOC_RECOVER_001 — STR + 跨SoC 恢复（=STR_003）
- [[TC_GFWK_STRESS_004]] — TC_GFWK_STRESS_004 — 后排屏开合压测
- [[TC_GFWK_STRESS_005]] — TC_GFWK_STRESS_005 — 环境光/自动亮度压测
- [[TC_GFWK_STRESS_006]] — TC_GFWK_STRESS_006 — 多屏图层随机增删压测
- [[TC_GFWK_STRESS_008]] — TC_GFWK_STRESS_008 — 后排屏 AIDL 电机持续压测
- [[TC_GFWK_STR_001]] — TC_GFWK_STR_001 — SF × STR 挂起唤醒
- [[TC_GFWK_STR_002]] — TC_GFWK_STR_002 — Gralloc × STR 挂起唤醒
- [[TC_GFWK_STR_004]] — TC_GFWK_STR_004 — HWC × STR 挂起唤醒
- [[TC_GFWK_STR_005]] — TC_GFWK_STR_005 — 综合 (GPU+Disp+跨SoC+Audio) × STR
- [[TC_GRAL_FAULT_001]] — TC_GRAL_FAULT_001 — dma-heap OOM
- [[TC_HWC_FAULT_002]] — TC_HWC_FAULT_002 — 坏 Layer 参数（HWC 拒绝非法参数，不 crash）
- [[TC_HWC_FAULT_003]] — TC_HWC_FAULT_003 — 多屏 Layer 故障隔离 (教学版)
- [[TC_HWC_FAULT_004]] — TC_HWC_FAULT_004 — kill HWC 恢复 (教学版)
- [[TC_Melo2_AVM_Gear_Func_001 — AVM 环视随挡位切换]]
- [[TC_SERDES_FAULT_002]] — TC_SERDES_FAULT_002 — SERDES(GMSL) 链路掉链-重训自愈 (教学版)
- [[TC_SERDES_STRESS_003]] — TC_SERDES_STRESS_003 — SERDES(GMSL) 链路压力/长稳监控 (教学版)
- [[TC_SF_BOUND_002]] — TC_SF_BOUND_002 — Producer 極限
- [[TC_SF_BOUND_003]] — TC_SF_BOUND_003 — Consumer 速率極限（Slots 耗盡 / 無死鎖）
- [[TC_SF_FAULT_001]] — TC_SF_FAULT_001 — kill SurfaceFlinger 恢复 SLA (教学版)
- [[TC_SF_FAULT_002]] — TC_SF_FAULT_002 — AMS/WMS Binder 突刺（SF 不被连累）
- [[mainreboot]]
- [[socreboot]]
- [[tombstone（墓碑 验尸报告）]]
- [[反压 (back-pressure)]]
- [[断言（assert）]]

## 006-屏幕异常算法 — 屏幕异常检测算法  (4)

- [[DL模型训练]]
- [[屏幕异常检测算法总览]]
- [[屏幕异常模型训练方案]]
- [[黑屏检测算法说明]]

## 007-速查 — 速查 / cheatsheet  (3)

- [[Ubuntu ↔ Windows 传文件]]
- [[常用adb指令速查]] — 常用 adb 指令速查 (GFWK 稳定性测试)
- [[常用linux 指令速查]] — check bot services

## 未归类

- [[claudecode 飞书]]
- [[未命名]]
