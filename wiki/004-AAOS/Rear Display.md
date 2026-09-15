---
title: 后排屏开合与亮灭控制（V27 / T29）
tags:
  - AAOS
  - GFWK
  - 后排屏
  - HWC
  - VHAL
platform: gua / guav100 (AAOS)
created: 2026-07
updated: 2026-08-04
---

# 后排屏开合与亮灭控制（V27 / T29）

> 后排屏 = HWC 第 3 屏（`mDisplayId=2`），跨车型控制方式不同。
> 用例落地见 [[TC_GFWK_STRESS_004]]（开合压测，含平台自动探测）。栈层见 [[HWC]] / [[000-GFWK图形框架总览]]。
> 新控制路径：开合已从 VHAL/CAN 改走 [[vendor AIDL]] `IGuaScreenControl`（电机 open/close/setAngle/getStatus），AIDL 压测见 [[TC_GFWK_STRESS_008]]。

## 平台探测

```bash
adb shell getprop persist.gua.car.model     # CHERY-V27 / CHERY-T29 / CHERY-ET(E0Y)
```
用例内以此判 V27/T29 再注入对应命令（复用 `common.Stability.device.device_info.get_car_model`）。`REAR_PLATFORM=V27|T29` 可强制覆盖。

---

## V27 — [[VHAL]] inject-event（信号量控制）
假装整车硬件下发了一条车辆属性
开屏约延迟 1s：

```bash
# VHAL 服务(车辆 HAL 的 AIDL 接口) android.hardware.automotive.vehicle.IVehicle/default 
# VehicleProperty ID (一个车辆属性,这里对应屏幕/电源类)
# -a area id
# -b 属性值(bytes/int 值)

# 开（不带 CAN）
adb shell "dumpsys android.hardware.automotive.vehicle.IVehicle/default --inject-event 560992868 -a 0 -b 0x344c"
# 关
adb shell "dumpsys android.hardware.automotive.vehicle.IVehicle/default --inject-event 560992868 -a 0 -b 0x324e"


```

`common/general_settings.py`

| 动作        | magic (-b) | 常量                             |
| --------- | ---------- | ------------------------------ |
| 开（不带 CAN） | `0x344c`   | `THIRD_SCREEN_ON_WITHOUT_CANN` |
| 开（带 CAN）  | `0x314f`   | `THIRD_SCREEN_ON_WITH_CANN`    |
| 关         | `0x324e`   | `THIRD_SCREEN_OFF`             |


---

## T29 — am broadcast（电机开合屏）
T29 后排屏为**电机驱动可开合**，用广播控制，另有角度接口：
```bash
# 打开后排屏（degree 由系统默认处理）
adb shell am broadcast -a com.gua.action.OPEN_REAR_SCREEN -p com.android.car

# 关闭后排屏
adb shell am broadcast -a com.gua.action.CLOSE_REAR_SCREEN -p com.android.car

# 设角度（示例 55 度）
adb shell service call vendor.gua.hardware.screenmotor.IGuaScreenControl/default 3 i32 2 i32 55  # setAngle 2：displayID
# 设目标角度（把 110 换成你要的度数）
adb shell settings put global rear_screen_target_angle 110

# 语音控制
adb shell dumpsys activity service com.gua.car.aiagent/.AiService e2e --text "打开后排屏"
```

---

## 验证亮/灭（判据）

| 信号 | 用途 | 说明 |
|---|---|---|
| **后排背光** `cat sys/class/backlight/dp2_panel0/brightness` | **亮/灭真值**（V27 已验） | >0 亮、=0 灭；复用 `check_rear_brightness` 口径 |
| `dumpsys display \| grep mDisplayId` | 仅判**已枚举/挂载** | 出现 `mDisplayId=2` = 后排屏已挂载；**开关屏后恒在，不反映上电态**，勿用作亮灭判据 |

> ⚠️ 踩坑：早期用 "mDisplayId=2 消失" 判关屏 → 误判（[[TC_GFWK_STRESS_004]] v1 失败复盘）。正确用**背光**。
> T29 电机开合平台的背光/角度口径待在 T29 台架确认后严格化。

## 📚 延伸阅读
- Android 多屏显示：https://source.android.com/docs/core/display/multi_display
- 车载多屏 / 乘客屏：https://source.android.com/docs/automotive/hmi/multi_display
- VehicleProperty（VHAL 属性）：https://source.android.com/docs/automotive/vhal

## 关联
- 用例 [[TC_GFWK_STRESS_004]]（后排屏开合压测，V27 背光判据 + T29 best-effort）
- 同族 [[TC_GFWK_STRESS_005]]（环境光/自动亮度）
- 总览 [[000-GFWK图形框架总览]] · 映射 [[06-GFWK如何映射到测试用例]]


---
## 20260831 Personal Notes

``` bash
# get ro prop
adb shell
getprop | grep "ro"

# serial  -> A41AEC42
adb -s A41AEC42 get-serialno
# car type: qrt29 / goldencar            
adb -s A41AEC42 shell getprop ro.product.name 

```

