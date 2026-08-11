---
title: Weston（A720 Cluster Wayland 合成器）
tags:
  - AAOS
  - weston
  - Wayland
  - 跨SoC
  - 仪表
  - 座舱
platform: "gua / guav100 (AAOS)"
created: 2026-07-30
---

# Weston（A720 Cluster Wayland 合成器）

> **一句话**：Weston 是 **A720 Cluster（仪表，Linux/Yocto）** 侧的 **Wayland 合成器**——等价于 IVI(Android) 侧的 [[SurfaceFlinger|SF]]，负责把仪表内容合成上屏。

## 为什么需要它
座舱是**多颗 SoC**：中控 IVI 跑 Android(用 [[SurfaceFlinger]]+[[HWC]] 合成)，仪表 A720 跑 **Linux**，需要自己的合成器 → Weston。它既画 A720 本地的仪表 UI，也接收 IVI 经 [[composer_stub]] 跨 SoC 送来的帧（导航投屏等）。

## 核心概念
| 点 | 说明 |
|---|---|
| Wayland | 显示协议：client（应用）把 buffer 交给 compositor（Weston）合成 |
| Weston | Wayland 的**参考合成器**，Yocto/嵌入式 Linux 常用 |
| 进程管理 | 由 **systemd** 托管，`Restart=always` → 崩了自动拉起 |
| 跨 SoC 输入 | 经 [[composer_stub]] → [[GIPC]](控制) + [[SHMEM]](像素) 收 IVI 帧 |

## 类比
A720 仪表班的**班长**——对应 IVI 的 SF 班长。IVI 班长拼好要给仪表看的内容，走廊传递员([[composer_stub]])送过来，Weston 收下贴到仪表黑板。

## 故障恢复
- 进程名 `weston`（自研 compositor 则不同，用 `WESTON_PROC` 覆盖）
- kill 后 systemd 拉起；**IVI SF 不应被连累**（跨 SoC 进程隔离）
- 用例 → [[TC_CSOC_FAULT_005]]（kill weston 恢复）

## 📚 延伸阅读
- [Wayland / Weston 官方](https://wayland.freedesktop.org/)

## 关联
- 对应 IVI 合成器 → [[SurfaceFlinger]]｜跨 SoC 桥 → [[composer_stub]]｜数据面 → [[SHMEM]]
- 屏所在域 → [[仪表]]｜同类总览 → [[GFWK kill-恢复类测试]]｜上级 → [[000-GFWK图形框架总览]]
