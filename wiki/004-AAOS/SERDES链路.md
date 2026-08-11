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

## 怎么读(需台架确认实际路径)
| 读法 | 说明 |
|---|---|
| **I2C 读 SER/DES 寄存器** | LOCK bit / error counter / line-fault; 需芯片 I2C 地址 + 寄存器 map(按 GMSL/FPD-Link 数据手册) |
| **内核 sysfs / debugfs** | 驱动若暴露 `locked`/`link_status`/错误计数 |
| **dmesg 关键字** | `gmsl`/`fpd`/`link (up\|down)`/`lock`/`unlock`/`retrain`/`line fault` |

## 可做成的稳定性用例
| 用例 | 注入/动作 | 判据 | 维度 |
|---|---|---|---|
| **链路长稳监控** | 周期读 LOCK+误码(挂其它压测旁路) | 不掉 LOCK; 误码增量 ≤ 阈值 | 长稳/监控 |
| **链路故障注入恢复** | I2C 写 SER 关输出 / GPIO 断链(拔插为人工) | 自动 retrain 重锁 < SLA; 屏恢复不崩 | 故障注入 |
| **热插拔/上电时序恢复** | 配合 [[TC_GFWK_STRESS_007]] 显示开关 / reboot / STR | 每次上电链路重锁**且屏检测到** | 恢复 |
| **BER/CRC 监控 + 花屏关联** | 视频压力下读 DES 错误计数 | 误码 ≤ 阈值; 花屏/闪屏时抓链路状态做根因 | 压力/根因 |
| **多链路隔离** | 断一条(后排)链路 | 不连累仪表/AVM 链路 | 隔离 |
| **相机 SERDES 链路** | 倒车/AVM 启用监控相机链路 | 相机链路不丢锁/无 line-fault | 摄像头 |

> 最适合做**旁路探针**: 把链路状态采集嵌进已有 kill/显示用例(如 [[TC_HWC_FAULT_004]] kill HWC、[[TC_GFWK_STRESS_007]] 开关屏)同时采, 一箭双雕。可在 `common/Gpu/gfwk_stress_util.py` 加 `serdes_link_status()` 复用(读法确认后)。

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
- 后排屏机制 [[后排屏]] · 显示开关恢复 [[TC_GFWK_STRESS_007]]
- 跨 SoC(仪表侧 [[Weston]]/[[composer_stub]])[[跨SoC]]
- 用例总览 [[GFWK 稳定性测试用例全量清单]]
