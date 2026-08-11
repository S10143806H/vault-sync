---
title: "tombstone（墓碑 验尸报告）"
tags:
  - 稳定性
  - AAOS
  - tombstone
  - 崩溃分析
  - SurfaceFlinger
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

- Android 里一个 native 进程（C/C++，如 [[SurfaceFlinger|surfaceflinger]]）异常崩溃（段错误 SIGSEGV、abort…）时，系统在 /data/tombstones/ 写一个文件，记录崩溃时的调用栈、寄存器、内存——给死掉的进程立块墓碑 + 死亡笔录，供工程师破案。
- 呼应你笔记：[[RAMdump]] 是"整机黑匣子"，tombstone 是"单个进程的死亡笔录"。
- 我们关心：恢复过程中有没有新的、非我们所杀的崩溃墓碑。这次多了 4 个，需人工看一条确认。
- [[SIGKILL （kill -9）]] 杀 SF 没有产生 surfaceflinger 的墓碑（SIGKILL 是"处决"不是"崩溃"，不留墓碑）。