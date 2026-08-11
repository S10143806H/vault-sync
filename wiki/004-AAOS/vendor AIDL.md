---
title: vendor AIDL(厂商 HAL 接口)
tags:
  - AAOS
  - AIDL
  - Binder
  - HAL
  - vendor
platform: gua / guav100 (AAOS)
created: 2026-08-07
---

# vendor AIDL(厂商 HAL 接口)

## 一句话
**AIDL** = Android 定义 **Binder 跨进程接口**的语言；**vendor AIDL** = **厂商(OEM/SoC 厂)自定义的 stable AIDL HAL 接口**（区别于 Google AOSP 标准 AIDL），放 vendor 分区、带版本、`@VintfStability`，注册到 servicemanager 供跨进程调用。

## 为什么有它
Android 11+ HAL 从 **HIDL** 全面转 **stable AIDL**。厂商把自家硬件能力（如后排屏电机、氛围灯）定义成 **vendor 命名空间**的 AIDL 接口对外提供。相比旧的 VHAL(property)/CAN 广播"发信号"，vendor AIDL 是**强类型、带返回值+回调**的 HAL 接口，能读状态、收故障。

## 本平台实例：后排屏电机
`vendor.gua.hardware.screenmotor.IGuaScreenControl` —— Gua(OEM) 定义的**后排屏电机控制** vendor AIDL，**服务端跑在 HWC3 进程**。后排屏开合**从旧 VHAL/CAN 改走这套 AIDL**（见 [[后排屏]]）。

## 调用机制(shell 手动打 Binder)
| 环节 | 命令/说明 |
|---|---|
| 注册发现 | `service check vendor.gua.hardware.screenmotor.IGuaScreenControl/default` → `found` |
| 手动调用 | `service call <svc> <code> i32 <arg>...`（shell 直发 [[Binder IPC\|Binder]] 事务） |
| **transaction code** | 按接口方法声明顺序编号：`1=open 2=close 3=setAngle 4=getStatus 5=registerCallback 6=unregisterCallback` |
| 参数 | `i32 <displayId> i32 <degree>`（如 open displayId=0 degree=110 → `service call ... 1 i32 0 i32 110`） |
| 返回 Parcel | **首个 8-hex 字 = exception code**（`00000000`=成功），其后是返回数据 |
| getStatus 解析 | Parcel 后续 hex 字依次 = `exception, parcelable_flag, data_len, state, angle, fault, target` |
| 回调 | `registerCallback` 后服务端主动推 `onStatusChanged`/`onFault` 给客户端 |

> ★ 判成功坑：**别用 `"0x00000000:" in out`**（只匹配多行 getStatus，会把单行 `Parcel(00000000)` 的 open/close 全判 fail）。应取 **Parcel 首个 8-hex 字段 == 00000000**。这是 [[TC_GFWK_STRESS_008]] 用例修过的点。

## AIDL vs HIDL vs 旧 VHAL/CAN
| | 传输 | 类型/能力 | 本平台后排屏 |
|---|---|---|---|
| VHAL inject-event / CAN 广播 | property / CAN | 发信号, 无返回 | 旧路径(V27/T29, 见 [[TC_GFWK_STRESS_004]]) |
| HIDL | Binder(hwbinder) | 强类型, Android 8~10 | 已淘汰 |
| **vendor AIDL** | Binder | **强类型 + 返回值 + 回调 + 版本** | 新路径 IGuaScreenControl(见 [[TC_GFWK_STRESS_008]]) |

## 📚 延伸阅读
- AIDL for HALs：https://source.android.com/docs/core/architecture/aidl/aidl-hals
- Stable AIDL：https://source.android.com/docs/core/architecture/aidl/stable-aidl
- AIDL 语言参考：https://developer.android.com/develop/background-work/services/aidl

## 关联
- 底层 [[Binder IPC]] · HAL 承载进程 [[HWC]]
- 后排屏机制 [[后排屏]] · 用例 [[TC_GFWK_STRESS_008]](AIDL 电机压测) / [[TC_GFWK_STRESS_004]](旧 VHAL/broadcast 开合)
