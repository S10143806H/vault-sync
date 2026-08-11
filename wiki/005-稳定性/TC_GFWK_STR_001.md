---
title: "TC_GFWK_STR_001 — SF × STR 挂起唤醒"
tags: [稳定性, AAOS, 测试用例, STR, SurfaceFlinger]
case_id: TC_GFWK_STR_001
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# TC_GFWK_STR_001 — SF × STR 挂起唤醒

> §3.5，P1。代码：`cases/MultiMedia/GPU/Str/TC_GFWK_STR_001.py`。方法论 → [[GFWK STR 挂起唤醒稳定性测试]]。

**一句话**：反复 [[STR]] 挂起-唤醒，侧重 [[SurfaceFlinger|SF]]——验唤醒后 SF 在 + 主屏渲染，且 **SF layer 数 / fd 不随 STR 单调泄漏**。

## 侧重 / bug 假设
- 唤醒后 SF surface/buffer 未回收 → **layer / fd 单调增**（长稳后 OOM）
- 唤醒**首帧黑/花**（EGL surface 重建时序）

## 断言（三态 + 泄漏）
| 断言 | 判据 |
|---|---|
| 不崩/能醒 | `wait-for-device` 唤醒不掉线 |
| 恢复 | SF <`RESUME_SLA` 起 |
| 画面 | 主屏 screencap 非黑 |
| 泄漏 | 每 `CHECK_EVERY` 轮查 layer 增量 ≤`LAYER_TOL`、SF fd 增量 ≤`FD_TOL` |

## 如何自测
```bash
source ~/.virtualenvs/py312/bin/activate; cd ~/Documents/autocase
STR_001_ROUNDS=3 pytest cases/MultiMedia/GPU/Str/TC_GFWK_STR_001.py --bench=<yaml> -v
# 真内核 STR: STR_MODE=mem STR_001_ROUNDS=3 pytest ...（需唤醒源+继电器兜底）
```
手动：`input keyevent KEYCODE_SLEEP` / `KEYCODE_WAKEUP` 之间夹 `pidof surfaceflinger` + `dumpsys SurfaceFlinger --list | wc -l`(layer) + `ls /proc/<sf_pid>/fd | wc -l`(fd) 看是否涨。

## env
`STR_001_ROUNDS`(200) · `STR_MODE`(display/mem) · `STR_001_SUSPEND`(5) · `STR_001_RESUME_SLA`(30) · `STR_001_LAYER_TOL`(8) · `STR_001_FD_TOL`(24) · `STR_001_CHECK_EVERY`(20)

## 关联
- 方法 → [[GFWK STR 挂起唤醒稳定性测试]]｜样板 → [[TC_CSOC_RECOVER_001]]｜清单 → [[GFWK 稳定性测试用例全量清单]]
