---
title: "‌⁣⁢​⁡​⁢‌⁢​﻿⁤﻿​‌⁢​‌‌⁡⁤⁢‬⁢‍⁤⁡⁡​⁣‍‌⁢‌⁣⁡‬⁣⁤‬⁡⁤‌‍‌⁢​⁢‍﻿48 SF wdt 维测方案调研 - 飞书云文档"
source: "https://guatechltd.feishu.cn/wiki/CGZWwH22ciyjxvkZfPecOSwznNd"
author:
published:
created: 2026-09-16
description:
tags:
  - "clippings"
---
- [48 SF wdt 维测方案调研](#ZeIPdagK4o2GHtxsyaLcN6Dqndq)
- [1\. 背景与问题陈述](#doxcnp4vNQlUk1AX17A2zVThpee)
- [1.1 现象](#doxcnWiFgbjSuPttgwqtTfSRBpS)
- [1.2 当前可观察性缺口](#doxcnnpRYhJYf0flwfX3iRsbAyd)
- [1.3 目标与非目标](#doxcnyo4sbczeqpuKPpinz5Litc)
- [2\. 总体架构](#doxcndgII4akCTtpqXDMdUk7Uvd)
- [3\. 组件设计](#doxcnUIAUXnKYYJfyDCZZa9sErh)
- [3.1 HeartbeatMonitor(心跳仲裁器)](#doxcnZmModgVl3GuiNUJxqh3uHh)
- [3.2 MainThreadStackTracer(主线程堆栈+指纹)](#doxcnS89KdbRb9hDTIaJGADJn3b)
- [3.3 DumpOrchestrator(分段 dump 协调器)](#doxcniqUo4IcvTZ8LHq3SkXlyBg)
- [3.4 HeartbeatWatchBuffer(RCU 风格只读快照)](#doxcnjl2a0fNXcwqHKU7MTGGBMb)
- [3.5 BBQ 自爆挂钩(libs/gui)](#doxcn3WefGKgbWjANI4u1vkMkEb)
- [3.6 HeartbeatRingBuffer(黑匣子,事后取证)](#doxcnoRwdNFHaX9dRTsZuqiRKbc)
- [为什么必须有它](#doxcnqKc4rLQB4tFbVew4zGoCPg)
- [dump 输出示例](#doxcnhPTsF33vgStoMEPQaZL4Ec)
- [成本对比](#doxcnOOFCGr0O3GpOpbnRtnf6HE)
- [3.7 控制开关(sysprop)](#doxcn7EbgrOWFGTYdasxUePZ3gc)
- [3.8 HeartbeatMetricsReporter(补强 A:线上 metrics 通道)](#doxcnEfEWI8q0yKvPEVbjrSSHWf)
- [3.9 JankAttributor(补强 B:慢帧分层归因)](#doxcn9NtbFgXzF162aXjka1x5Ue)
- [4\. 数据流与时序](#doxcn65vgz4FfJme0t6oE3KGuKd)
- [4.1 正常帧(绿色路径)](#doxcn5bg7GSLN5wU9OvlOYkm3Kb)
- [4.2 异常帧(连续 4 帧 commit 慢 → 触发 dump)](#doxcnCpekiDT5m7tSq7wu5qVlOf)
- [4.3 异常帧(present fence 永远 stall,红色路径)](#doxcnZxIns6xQdi6KE1SPwXAQAp)
- [4.4 BBQ 自爆路径(libs/gui)](#doxcnu4GqJBbHQkLvLXWKA9lCDf)
- [logcat 对齐效果](#doxcn9s0dRtikTKiLndBl4felpc)
- [5\. 时序图](#doxcntlwYwQjl8DJYEpy4Oz6NHc)
- [5.1 正常帧时序](#doxcnwiwwAIAuatcJV8l1z3QgBd)
- [5.2 异常帧时序(4 帧连续 commit 慢触发 dump)](#doxcnzMb42kjyTh1Ma0yrBeFJee)
- [5.3 异常帧时序(present fence 永远 stall)](#doxcnvHY2cIfch0f2uteMO534vf)
- [5.4 watchBuffer 快照更新(关键并发约束)](#doxcnOeMUyOk8E6rujDqjCIaqVe)
- [5.5 启动与关停](#doxcn1u8dLMiBsZUFtWkTz6S4sh)
- [6\. Watchdog 并发与死锁约束](#doxcnTGqJfNHYkDmzOseVTgJ24f)
- [7\. 错误处理与边界](#doxcnFqC8GXm9BfwfqcT7MNtRke)
- [8\. 测试计划](#doxcnWAI3MQHNsd9SR0DECIlH0c)
- [8.1 单元测试(新)](#doxcniq1AS7C09iA9xKMJGKHsLb)
- [8.2 集成测试](#doxcnujEKm6JDw3djM4ds9yuPza)
- [8.3 真机回归](#doxcnQeKFnG2LSzKwTr3jjMK1Db)
- [9\. 改动文件清单](#doxcnPSWyifa0rCBHUN27o5Jwoe)
- [10\. 默认值与决策记录](#doxcnqESOv2vTVPmpdDvNPe8Ckg)
- [11\. 里程碑拆分](#doxcnlJc6ONPFqP0qCHYSdrckHd)
- [12\. 开放问题(未来)](#doxcnSkNVsRaLvB9AMbgiCltJbe)
- [附录 A.术语表](#doxcnFDEAwN2UkL6nmfzNHlPcgO)

输入“/”快速插入内容

本期项目：

优先考虑获取足够多的日志，辅助debug,暂时不考虑相关的异常处理，可以预留处理接口

主要考虑的异常的场景的就是hang 的场景

项目分工：

满煜璇

主导一下框架的设计 需要基于解耦框架来实现

Zhu Qi

熟悉完整的维测的流程后续慢慢把整个功能维护起来

沈王雄

确认如何使用完善的工具来 dump 堆栈

如果缺乏人力考虑让

丁盈月

沈悦

支持

周四对齐一次；

📌

一句话概括:为 SurfaceFlinger 主循环 (commit → composite → present) 和 BufferQueue 增加心跳监控与维测能力,异常时主动 dump 主线程堆栈 + 全链路时序快照到 logcat,解决 dequeueBuffer -110 (ETIMEDOUT) 类问题"看不见根因"的痛点。

评审范围:V1.1 完整方案(含补强 A~E:线上 metrics / 归因 / 指纹 / 采样率 / perfetto 通道)

关联文档:[《Android 卡顿监控:从 Choreographer 到线上 FPS 监测》](https://mp.weixin.qq.com/s/qONDZWnye0vGpASz845osg) (本文档 §3.8/§3.9 借鉴其线上化思想)

1\. 背景与问题陈述

1.1 现象

W OpenGLRenderer: dequeueBuffer failed, error = -110; switching to fallback

I Choreographer: Skipped 238+ frames!

\-110 是 ETIMEDOUT,来自 BufferQueueProducer::dequeueBuffer 内 waitForFreeSlotThenRelock 返回超时。多 app 同时触发且呈周期性,指向 SF/BBQ 消费侧持续卡住,不是单一 app 渲染慢。

1.2 当前可观察性缺口

<table><tbody><tr><td rowspan="1" colspan="1"><p></p><p>维度</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>现状</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>缺口</p><p></p></td></tr></tbody></table>

<table><colgroup><col width="180"> <col width="180"> <col width="180"></colgroup><tbody><tr><td rowspan="1" colspan="1"><p></p><p>SF 主循环耗时</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>仅事后 dumpsys 才能看 frame timeline</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>无法知道"现在是否卡"</p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p>BBQ 队列状态</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>dumpsys 有但需主动触发</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>app 报错时 SF 端无同步快照</p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p>Layer 级 buffer 状态</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>字段存在但部分被 DEBUG 屏蔽</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>app logcat 看不到</p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td><td rowspan="1" colspan="1"><p></p><p></p></td></tr></tbody></table>

评论（3）

跳转至首条评论

7,043 字

- 上传日志

- 联系客服

- 功能更新

- 帮助中心

- 快捷键

连续按下 Ctrl + A 以选中全文