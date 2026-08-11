---
title: RAMdump
tags:
  - 稳定性
  - AAOS
  - ramdump
  - pstore
  - 崩溃分析
  - kernel panic
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

Ramdump（内存转储）就是**系统崩溃瞬间把内存原样拷贝保存下来的"案发现场快照"**。

类比：程序日志是"目击者口述"，ramdump 是"整个案发现场原封不动封存"——崩溃那一刻内存里的所有东西（内核数据结构、调用栈、寄存器、各进程状态）都被完整保留，事后可以用调试工具（如 Crash Utility、QCAT）加载分析，精确还原"崩溃时 CPU 正在执行哪行代码、哪个变量是什么值"。

为什么需要它：严重崩溃（kernel panic、硬件异常）发生时**系统已经死了**，来不及写日志——日志能记录的是崩溃**之前**的事，而 ramdump 记录的是崩溃**那一刻**的事。对于"设备突然挂死/重启"这类问题，往往只有 ramdump 能给出最终答案。

对应到你们的流程（113446 那次）：

1. IVI 域崩溃后，设备重启时 BootLoader 检测到"上次是异常死亡"，不直接正常开机，而是把崩溃现场保护起来
2. 台架框架把设备切到 fastboot，用 `fastboot fetch ivi-pstore / cluster-pstore` 把现场拉回来——你日志里的 "Ramdump fetch" 就是这一步
3. **pstore** 是 ramdump 的轻量版：一小块跨重启不丢的内存区域，专门存内核最后的 console 输出和 panic 调用栈（console-ramoops）。完整 ramdump 可能几个GB，pstore 只有几MB，但通常够定位 panic 原因

所以之前说"要定 ramdump 根因，把 pstore 文件发我"——意思就是：宿主机日志只看到设备死了这个"果"，pstore 里的 panic 栈才写着"因"。


https://guatechltd.feishu.cn/wiki/PDOcwdzaIiDz3ykfgMXc9iRmnNh