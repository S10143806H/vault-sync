---
title: Fence
tags:
  - AAOS
  - Fence
  - sync_file
  - dma-buf
  - 跨SoC
platform: gua / guav100 (AAOS)
created: 2026-07-28
---

# Fence

"画好了"举手：跨环节同步信号(dma-buf/sync_file)。没 signal 就取 buffer = 拿半成品 → 花屏 / UAF。

## 直觉：一个「画好了没」的举手牌 🙋

一帧图要经过 **GPU 画 → [[SurfaceFlinger|SF]] 合成 → 屏幕显示** 多手接力，各环节**并行抢时间**、不能干等。fence 就是环节间的握手信号：

```
GPU 拿到 buffer + 一个 fence（举手牌，初始放下）
   ↓ GPU 慢慢画，SF 先去忙别的（不干等）
   ↓ GPU 画完 → fence「举手」(signal)
SF 看到举手 → 才敢拿这块 buffer 去合成
```

- fence **signaled（举手）** = 数据 OK，放心用
- fence **未 signal（没举手）** = 还没画完，谁拿谁翻车

| 翻车方式 | 后果 |
|---|---|
| fence **永远不举手** | 下游死等 → **冻屏 / 卡死** |
| 没等举手就取 / 持已释放 fence | **花屏** 或 **崩溃**（[[SIGSEGV]]） |

## sync_file 是一个 fd（跨进程/跨域传递）
- fence 在用户态体现为 `anon_inode:sync_file` 的**文件描述符(fd)**，通过 [[Binder IPC|Binder]]/[[GIPC]] 在进程间传递。
- 跨 SoC：`weston`(A720) 产出的 sync_file fd 传给 [[composer_stub]] 等待 signal 后再合成。

## 两类 fence 相关缺陷/风险（本项目实测）
- **fence UAF（真崩溃）**：持已释放的 fence 还 `ppoll` → [[SIGSEGV]]/panic（GPU满载+重启SF/HWC 致信令不完整，KB G2；`sync_file_poll`+`remote fence timeout reclaimed`）
- **used fence 未 signal（冻屏）**：IVI panic 释放 IPC 资源时 used fence 没 signal → Cluster 等永不到的 fence 冻屏（KB G3）
- **SELinux 策略缺口**：实测 `composer_stub_t` `use` 属于 `weston_t` 的 `sync_file` fd 被 `avc: denied`（当前 [[SELinux|permissive]] 放行）；转 enforcing 会拦截跨 SoC fence 传递 → 需补 policy

详见 [[05-Fence与跨SoC同步]]｜跨核 [[composer_stub]]｜信号 [[SIGSEGV]]｜安全 [[SELinux]]｜判定 [[退出码101 vs SIGSEGV（composer_stub 崩溃判定）]]｜上级 [[000-GFWK图形框架总览]]
