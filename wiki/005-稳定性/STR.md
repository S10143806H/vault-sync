---
title: "STR (Suspend-to-RAM)"
tags:
  - 稳定性
  - AAOS
  - STR
  - 电源管理
  - 座舱
  - 时序
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

# STR (Suspend-to-RAM)

**S**uspend **T**o **R**AM，挂起到内存 
1. STR 是电脑/车机众多"睡眠"方式里的一种。在 AAOS 整车语境里，它对应文档说的"锁车 → 中控屏 1-2s 灭掉 → 系统休眠"。
2. 挂起时系统把"现场"（进程状态、打开的句柄、内核数据结构）**留在 RAM 里**，然后给几乎所有CPU/大部分SoC断电，只**保留 RAM 供电**（DRAM 进 self-refresh 自刷新，靠极低功耗维持电容里的数据不丢），把整个系统“冻”在内存里，唤醒时直接混服，不用重新开机。所以比“关机后再开机（冷启动）”快。
	- **为什么是 RAM**：RAM 能在 mW 级功耗下**保住内存内容**又能**瞬间读取**——所以唤醒不用重新加载系统，秒醒。
	- 对照：
		- **关机**：RAM 也断电，内容全丢 → 下次冷启动（慢、失忆）
		- **Suspend-to-Disk（休眠）**：把内存 dump 到磁盘再全断电 → 省电更彻底但唤醒慢
		- **STR**：只留 RAM → 省电 + 秒醒的折中，车机锁车场景最合适


车上对应场景：**锁车走人** → 整车下电（[[KL15]] 断），车机进 STR 省电；第二天解锁开门 → 秒亮，不用等开机动画。

**它和重启的本质区别：重启是失忆重来，STR 是睡一觉醒来接着干。**

是图形上**最大的单一故障场景**（DP HPD timing、PLL relock、fence timeout 等跨模块时序问题）。


## STR bug 的根源：RAM 记得，硬件忘了
关键矛盾——**RAM 里的软件状态被保住了，但被断电的硬件/外设状态没了**。唤醒时两者对不上就出错：
- DPU/GPU/DP-PHY/PLL 掉电 → 寄存器清零，**必须重新编程**（不是"按顺序起进程"就行）
- fence/dma-buf/EGL context/wayland proxy 等**跨挂起的句柄**可能失效，代码若不校验、直接用悬空引用 → 崩（如 [[composer_stub]] 的 `wl_proxy` 段错误）
- 挂起瞬间**在途**的合成/传输任务没排空 → 唤醒后状态错乱

## 对 GFWK 的具体影响
| 环节                         | 挂起丢了什么                  | 唤醒没接好会怎样            |
| -------------------------- | ----------------------- | ------------------- |
| DPU/DP-PHY                 | 时序/HPD/PLL 状态           | 无信号/花屏/黑屏           |
| [[SurfaceFlinger\|SF]]/EGL | surface/context         | 首帧黑、layer 泄漏        |
| [[Fence]]                  | 挂起前 used fence 未 signal | 消费端死等 → **冻屏**      |
| [[GIPC]]/[[SHMEM]] 跨SoC    | 通道连接                    | 投屏丢、A720 收不到帧       |
| 后排屏                        | 点亮状态                    | 唤醒只亮主屏，后排需再 VHAL 唤醒 |

## 顺序一致就够了吗？——不够
> "只要保证进程休眠顺序 / 开启顺序一致就不出错" 是**必要不充分**。顺序对只是第一步，真正难的是：
> 1. **状态恢复**（不只是"起来"，是硬件寄存器/PLL/时序要重新配对）
> 2. **时序竞态**（顺序对，但 HPD 去抖、PLL relock、fence signal 有时间窗，消费端可能早于生产端就绪）
> 3. **跨 SoC 异步**（IVI 与 A720 各自唤醒，GIPC 重连握手会 race，跨挂起的 fence 可能变孤儿）
> 4. **句柄再校验**（buffer/fence/proxy 跨挂起后要判有效性，不能假设还在）
> 5. **在途任务排空**（挂起点的 pending 合成/dma-buf 要安全收尾）
>
> 所以才要**反复挂起-唤醒压测**（[[GFWK STR 挂起唤醒稳定性测试]]）——顺序类问题一两次就露，但**竞态/泄漏/句柄失效**只有跑几十上百轮才现形。

## 官方电源适配要求（IVI 整车电源休眠唤醒）
来源：飞书《IVI整车电源休眠唤醒流程》。核心——**凡涉及资源占用/状态维护/数据处理的模块，都要监听系统电源状态**，在对应阶段做对：
- **进入 STR / 关机前**：资源释放、任务停止、**数据持久化**（清理干净再睡）
- **从 STR 恢复(Resume)后**：状态恢复、**资源重建**、业务恢复（醒来重新接好）

> 这就是 GFWK 侧 STR bug 的根因框架：某模块没监听电源状态 / 睡前没清理 / 醒后没重建 → 冻屏、泄漏、[[composer_stub]] 悬空 proxy 崩。测试就是反复睡醒，逼出没适配好的模块。

## KL15 下电 = adb 断连（判断"真睡"的标志）
| 睡法 | Android/内核 | **adb** | A720 仪表 |
|---|---|---|---|
| **KL15 拉低**（整车真睡） | IVI suspend、CP0/CP1 深睡 | **断连**（设备 offline） | **一起睡** |
| `echo mem`（内核 STR） | 内核 suspend | **断连** | 视电源域，不一定 |
| `input keyevent SLEEP`（显示灭） | Android 照常跑 | **不断**（还在） | **没睡** |

- **KL15 下电必然带 adb 断连**——test_006 步骤3 专门 `检查 adb 已断连`，这是"整车真进 STR"的判据；没断=没真睡。
- 反过来：`keyevent SLEEP` 只是灭屏，adb 一直在 → 所以 display 模式**测不到真 STR / 仪表屏**。

| 目标            | 主要函数                                             |
| ------------- | ------------------------------------------------ |
| KL15 高/低      | `set_vehicle_awake(True/False)`                  |
| Deep sleep 进入 | CP0：`checked_cmd_output(..., "sleep_mode deep")` |
| Deep sleep 等待 | `wait_domains_suspended(...)`                    |

关联：[[座舱]]｜[[000-GFWK图形框架总览]] | https://guatechltd.feishu.cn/wiki/AaMkwfOE5i4WLWkSpZNcglFanRc?hyperlink_open_type=lark.open_in_browser&disposable_login_token=eyJ1c2VyX2lkIjoiNzY0MDA2Mzg4MjI1MzM1NjIzNCIsImRldmljZV9sb2dpbl9pZCI6Ijc2NDEwNTg0NzM3NzM3OTYzMTgiLCJ0aW1lc3RhbXAiOjE3ODU3Mzg5MzYsInVuaXQiOiJldV9uYyIsInB3ZF9sZXNzX2xvZ2luX2F1dGgiOiIxIiwidmVyc2lvbiI6InYzIiwidGVuYW50X2JyYW5kIjoiZmVpc2h1IiwicGtnX2JyYW5kIjoi6aOe5LmmIiwiY2xpZW50X3NjaGVtYSI6IngtZmVpc2h1In0%3D.8d2bf34a5f6db42e4508a7f3ced948e22ba890cc4ddb595b45e26bca70b142ef
