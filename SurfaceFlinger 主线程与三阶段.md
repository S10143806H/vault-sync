---
title: SurfaceFlinger 主线程与三阶段
tags: [AAOS, SurfaceFlinger, vsync, 稳定性]
created: 2026-09-15
related: ["[[SurfaceFlinger]]", "[[VSync]]", "[[Fence]]", "[[HWC]]"]
---

# SurfaceFlinger 主线程与三阶段

> [!abstract] 一句话记住
> **中控是时钟源，远端屏是从属，SF 每帧只有 15.67ms。**

关联：[[SurfaceFlinger]] · [[VSync]] · [[Fence]] · [[HWC]]

## 抓取命令

```bash
# 看 layer 列表、display、fence
adb shell dumpsys SurfaceFlinger | head -200
```

---

## 1. 三块屏与它们的 ID

对应 bug 中的 `display=100`。

| HWC display | SF displayId          | 名称        | 角色                     |
| :---------: | --------------------- | ----------- | ------------------------ |
| `0`         | `4634679611807204096` | GUA0 中控   | **pacesetter（主时钟）** |
| `1`         | `4634679327297303554` | GUA2        | follower                 |
| `100`       | `4634679587309427457` | GUA1 远端屏 | follower                 |

> [!warning] 为什么远端屏最先「看起来」死
> bug 里的 `FENCE GAP display=100 gap=16.9s` 就是最后这块屏。
> - 它是 **follower**，`hwVsyncState=Disallowed`，自身没有硬件 vsync，跟着中控走。
> - 中控合成一卡 → 它的 `present` 整个停摆 → 因此远端屏最先表现出「死屏」。

---

## 2. SF 主循环的节拍

| 参数               | 值        | 说明                  |
| ------------------ | --------- | --------------------- |
| VSYNC period       | `16.67ms` | 一帧总时长            |
| `sf: workDuration` | `15.67ms` | SF 自己实际可用的时间 |

> [!note] 阈值来源
> 方案里「commit / composite 阈值 `16ms`」即源于此：一帧 `16.67ms`，SF 只能用 `15.67ms`。

---

## 3. 健康态基线

加心跳后，以下数值即为「正常」参照。

| 指标         | pacesetter | follower ×2 |
| ------------ | :--------: | :---------: |
| Total missed | `6`        | `0`         |
| HWC missed   | `4`        | `0`         |
| GPU missed   | `4`        | `0`         |

---

## 4. Fence 观测点

> [!tip] `Has 1 unfired fences`（pacesetter 的 VsyncController）
> - 正常态：偶尔 `1` 个未 signal 的 present fence 属正常。
> - bug 发生时：此处会**积压** —— 这是方案 `FENCE_STALL` 路径盯的数据。

---

## 5. VsyncGuaDispatch（厂商私有）

- AOSP **没有**此机制，是厂商追加的第二套 vsync 分发（`Gua` 前缀）。
- 读 `Scheduler` 代码时需留意它。
- 当前**空闲**：`mIntendedWakeupTime` 为极大值 = 没有排任务。

---

## 📚 延伸阅读

- [SurfaceFlinger — source.android.com](https://source.android.com/docs/core/graphics/surfaceflinger-windowmanager)
- [VSYNC — source.android.com](https://source.android.com/docs/core/graphics/implement-vsync)
- [Sync framework / Fence — source.android.com](https://source.android.com/docs/core/graphics/sync)
