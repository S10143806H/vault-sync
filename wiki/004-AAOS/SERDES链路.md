---
title: SERDES 链路(显示/摄像头串行链路)
tags:
  - AAOS
  - SERDES
  - GMSL
  - FPD-Link
  - 显示
  - 摄像头
  - 稳定性
platform: gua / guav100 (AAOS)
created: 2026-08-07
---

# SERDES 链路(显示/摄像头串行链路)

## 一句话
**SERDES(Serializer/Deserializer, 串行器/解串器)= 主 SoC/DPU 与远端屏(后排/仪表)及摄像头(AVM/DMS)之间的高速串行视频链路**——把并行像素/相机数据串行化经同轴/STP 传到远端再还原。座舱常见 **GMSL2**(Maxim/ADI, `MAX9671x` 序列器 + `MAX9296/96712` 解串器)或 **FPD-Link III/IV**(TI, `DS90UBxxx`)。

## 为什么要监控它
远端屏/相机不在主板上, 全靠这条串行链路。链路一旦 **失锁(LOCK loss)/误码/line-fault/需 retrain**, 表现就是 **黑屏/花屏/闪屏/相机丢图**——但上层 [[HWC]]/[[SurfaceFlinger]] 可能"以为屏还在"(display 仍枚举)。所以**链路层监控比上层枚举更早、更根因**: [[TC_GFWK_STRESS_007]] 查"屏被检测到", SERDES 监控查"底层链路真锁上"。

## 关键状态信号
| 信号 | 含义 |
|---|---|
| **LOCK / LOSS** | 链路是否锁定(解串器锁上序列器时钟/数据) |
| **误码 / CRC 计数** | 传输错误累计(BER 指标) |
| **line-fault** | 线缆开路/短路/对地故障 |
| **retrain / re-lock** | 链路抖动后重训练次数 |
| **热插拔** | 远端上下电导致的 link up/down |

## 怎么读 —— ✅ 台架(A41AEC42)实测确认
本平台 = **GMSL**：序列器 **MAX96855**(ser@0x40) + 解串器 **MAX96772**(des@0x48)。i2c 被内核驱动占用(`i2cdetect` 显 `UU`)**不能直读寄存器**, 改走 debugfs + dmesg:

| 源 | 命令 | 含义 |
|---|---|---|
| **锁定值** | `cat /sys/kernel/debug/dri/0/<link>/serdes_status`(**会 printk 到 dmesg**) | `[SERDES]...gmsl lock[0x8a]` = 锁定; `0x8a`=des reg0x13 锁定值 |
| **故障事件** | `dmesg \| grep '[ser:i2c-<bus>]'` | `linklock error`/`pixel clock error`/`LINK_A_LCTRL2=0x0` = 掉链 |
| **恢复事件** | 同上 | `linklock recovered (was faulty for N checks)` |
| **DP 重训** | 同上 | `dp train link`→`Link training successful`(重训=瞬断重连) |
| **内建监控** | `cat .../<link>/fmg_enable`(=1) | 驱动自带 fault monitor, 勿关 |
| **官方旋钮文档** | `cat .../<link>/help` | 列全部 debugfs |

> 读窗隔离: 先 `echo <marker> > /dev/kmsg` 打标记, 再 `dmesg | sed -n '/<marker>/,$p'`, 避免吃历史 ring buffer 噪声。

**故障注入(安全可逆)**: `echo 1 > /sys/kernel/debug/dri/0/<link>/link/training` —— 官方 debugfs 机制, 触发 DP 全链路重训(瞬断重连)后自愈重锁, **不连累 IVI [[SurfaceFlinger]]**(pid 不变)。
> 注: DP 重训**不必然**级联出 GMSL `linklock error`(取决于 fmg 轮询是否撞上瞬断窗口), 故恢复判据以 **重训成功 + GMSL 重锁** 为准。

### 台架链路映射 / 已知基线
| link(debugfs) | ser 总线 | 屏 | 状态 |
|---|---|---|---|
| `dp2` | i2c-13 | 主屏(panel saf407db1, 2560x1600) | **已锁 0x8a** |
| `dp1` | i2c-15 | 后排 | 本台架**未接** → `can't read reg 0x13`(ret=-121) 持续报错 = **预期基线, 非缺陷** |

## 已落地用例 ✅
| 用例 | 注入/动作 | 判据 | 状态 |
|---|---|---|---|
| [[TC_SERDES_FAULT_002]] | `echo 1 > link/training` 强制重训(瞬断重连) | 重训成功+GMSL 重锁 < SLA; IVI SF pid 不变; 无 double-free | ✅ 实测通过 |
| [[TC_SERDES_STRESS_003]] | 负载/长稳下周期读 LOCK+故障事件 | 锁定值恒 0x8a; 零自发故障; 主屏非黑 | ✅ 实测通过 |

> 公共件 `common/Gpu/gfwk_stress_util.py` 已加 `serdes_lock / serdes_train / serdes_events_since / serdes_retrain_result / serdes_mark`。可做**旁路探针**嵌进 [[TC_HWC_FAULT_004]] kill HWC / [[TC_GFWK_STRESS_007]] 开关屏同时采, 一箭双雕。未做: BER/CRC 关联花屏、相机 SERDES 链路、多链路隔离(后排未接)。

## 与显示栈的关系
```
App -> [[SurfaceFlinger]] -> [[HWC]] -> DPU -> [SERDES 序列器] --串行链路--> [解串器] -> 远端屏(后排/仪表)
                                                                            相机 -> [序列器] --> [解串器] -> ISP/V4L2
```
链路在 **DPU 之后、物理屏之前**(相机则在 sensor 之后、SoC 之前), 是显示/相机通路的"最后一公里/第一公里"。

## 📚 延伸阅读
- GMSL(Analog Devices/Maxim)：https://www.analog.com/en/product-category/gigabit-multimedia-serial-link.html
- TI FPD-Link SerDes：https://www.ti.com/interface/fpd-link-serdes/overview.html
- Android 多屏显示(上层对照)：https://source.android.com/docs/core/display/multi_display
- Android 图形架构 SF/HWC：https://source.android.com/docs/core/graphics/arch-sf-hwc

## 关联
- 显示栈 [[SurfaceFlinger]] · [[HWC]] · [[SurfaceControl]]
- 后排屏机制 [[wiki/004-AAOS/Rear Display]] · 显示开关恢复 [[TC_GFWK_STRESS_007]]
- 跨 SoC(仪表侧 [[Weston]]/[[composer_stub]])[[跨SoC]]
- 用例总览 [[GFWK 稳定性测试用例全量清单]]
