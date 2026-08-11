---
title: "TC_GFWK_STR_004 — HWC × STR 挂起唤醒"
tags: [稳定性, AAOS, 测试用例, STR, HWC]
case_id: TC_GFWK_STR_004
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# TC_GFWK_STR_004 — HWC × STR 挂起唤醒

> §3.5，P1。代码：`cases/MultiMedia/GPU/Str/TC_GFWK_STR_004.py`。方法论 → [[GFWK STR 挂起唤醒稳定性测试]]。

**一句话**：反复 [[STR]] 挂起-唤醒，侧重 [[composer_stub|HWC]]/合成——验唤醒后 HWC 在 + **逐屏（含后排）非黑**。

## 侧重 / bug 假设
- 唤醒后 HWC 合成异常 / **花屏**（DPU/DP-PHY relock 失败）
- **后排屏不亮**（唤醒只点主屏，后排需再发 VHAL `THIRD_SCREEN_ON_WITHOUT_CANN`）

## 断言（三态）
| 断言 | 判据 |
|---|---|
| 不崩/能醒 | `wait-for-device` 唤醒不掉线 |
| 恢复 | HWC(`ps\|grep composer` 自动发现) <`RESUME_SLA` 起 |
| 画面 | **逐屏**（含后排，先 VHAL 唤醒）screencap 非黑 |

## 如何自测
```bash
source ~/.virtualenvs/py312/bin/activate; cd ~/Documents/autocase
STR_004_ROUNDS=3 pytest cases/MultiMedia/GPU/Str/TC_GFWK_STR_004.py --bench=<yaml> -v
```
手动：sleep/wakeup 后 `pidof <hwc>` + 逐屏 `screencap -d <id> -p ...`；后排先 `dumpsys ...IVehicle... --inject-event 560992868 -a 0 -b 0x344c`。

## env
`STR_004_ROUNDS`(200) · `STR_MODE`(display/mem) · `STR_004_SUSPEND`(5) · `STR_004_RESUME_SLA`(30) · `HWC_PROC`(覆盖 HWC 进程名)

## 关联
- 方法 → [[GFWK STR 挂起唤醒稳定性测试]]｜样板 → [[TC_CSOC_RECOVER_001]]｜同类 → [[TC_HWC_FAULT_004]]｜清单 → [[GFWK 稳定性测试用例全量清单]]
