---
title: "TC_GFWK_STR_002 — Gralloc × STR 挂起唤醒"
tags: [稳定性, AAOS, 测试用例, STR, Gralloc, dma-buf]
case_id: TC_GFWK_STR_002
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# TC_GFWK_STR_002 — Gralloc × STR 挂起唤醒

> §3.5，P1。代码：`cases/MultiMedia/GPU/Str/TC_GFWK_STR_002.py`。方法论 → [[GFWK STR 挂起唤醒稳定性测试]]。

**一句话**：反复 [[STR]] 挂起-唤醒，侧重 [[dma-buf heap|Gralloc/dma-buf]]——验唤醒后主屏渲染，且 **dma-buf 数 / 图形内存不随 STR 单调泄漏**。

## 侧重 / bug 假设
- 挂起瞬间 **pending dma-buf** 未回收 → dma-buf/图形内存单调增
- 唤醒**花屏**（buffer 复用/[[Fence|fence]] 处理不当）

## 断言（三态 + 泄漏）
| 断言 | 判据 |
|---|---|
| 不崩/能醒 | `wait-for-device` 唤醒不掉线 |
| 恢复 | SF <`RESUME_SLA` 起 |
| 画面 | 主屏 screencap 非黑 |
| 泄漏 | 每 `CHECK_EVERY` 轮查图形指标增量 ≤`GFX_TOL_PCT`% |

> 图形指标口径：优先全局 dma-buf 数（`/sys/kernel/debug/dma_buf/bufinfo`，需 root）；不可用回退 **SF 进程 RSS(KB)** 作代理。

## 如何自测
```bash
source ~/.virtualenvs/py312/bin/activate; cd ~/Documents/autocase
STR_002_ROUNDS=3 pytest cases/MultiMedia/GPU/Str/TC_GFWK_STR_002.py --bench=<yaml> -v
```

## env
`STR_002_ROUNDS`(200) · `STR_MODE`(display/mem) · `STR_002_SUSPEND`(5) · `STR_002_RESUME_SLA`(30) · `STR_002_GFX_TOL_PCT`(30) · `STR_002_CHECK_EVERY`(20)

## 关联
- 方法 → [[GFWK STR 挂起唤醒稳定性测试]]｜概念 → [[dma-buf heap]]｜[[Fence]]｜样板 → [[TC_CSOC_RECOVER_001]]｜清单 → [[GFWK 稳定性测试用例全量清单]]
