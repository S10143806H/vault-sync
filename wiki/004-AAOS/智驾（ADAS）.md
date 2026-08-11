---
title: "智驾（ADAS）"
tags:
  - AAOS
  - ADAS
  - 座舱
  - tombstone
  - SIGSEGV
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# 智驾（ADAS）

座舱这栋楼里的一户 → 见 [[座舱]]。运行 ADAS HAL 服务 `vendor.gua.hardware.adas-service`。

## 观察记录

- **2026-07-23**：跑 [[TC_SF_FAULT_001]]（kill SF）时，`/data/tombstones/`（[[tombstone（墓碑 验尸报告）|tombstone]]）出现多个 `vendor.gua.hardware.adas-service` 的 **SIGSEGV(signal 11)** 崩溃，fault addr `0x80`（空指针解引用）。
  - 一度疑似"kill SF 连累 adas"（时间戳对齐），但**干净复跑三轮 adas 0 崩溃** → 判定**巧合**，adas 本身**周期性 SIGSEGV**，与 SF 无直接因果。
  - 待办：单独观察 adas-service 自身崩溃频率，必要时上报 ADAS 团队。

## 关联
- 上级：[[座舱]]｜[[000-GFWK图形框架总览]]
- 相关用例：[[TC_SF_FAULT_001]]
