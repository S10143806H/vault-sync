---
title: "BUG: HWC 在 DPMS SetPowerMode DRM ioctl 卡死→崩溃循环拖崩 system_server"
tags:
  - 稳定性
  - AAOS
  - 缺陷
  - HWC
  - DPMS
  - watchdog
platform: "gua / guav100 (AAOS)"
severity: 高(待确认)
status: 观察中(疑级联, 待确认是否独立缺陷)
created: 2026-07-31
---

# BUG：HWC DPMS SetPowerMode 崩溃 → system_server 崩溃循环

## 一句话
2026-07-31 台架(SG0286)测试前发现设备陷入**崩溃级联**：HWC(`android.hardware.composer.hwc3-service.gua`)在 **DPMS 灭/亮屏 `SetPowerMode` → `drmModeConnectorSetProperty` DRM ioctl** 处卡住，被 **pid 495(root,疑 hang 看门狗) SIGABRT** 打死 → 连带 SF → **system_server 反复重启**（pid 22810→23290→23696→24213…），**AMS/PM 服务始终注册不上** → `pm/am` 全 "Can't find service" → uiautomator 起不来 → **所有 adb 用例 setup 失败**。`adb reboot` 后恢复正常。

## 环境
- 版本：`meloui/goldencar/guav100:14/UP1A.231005.007.A1/831:userdebug`（G1.30.M）
- 台架：SG0286 / A41AEC42
- 触发：测试前置(uiautomator 拉起)阶段暴露；设备已 uptime ~14h

## 关键证据（tombstone_43, 14:47）
```
Cmdline: /vendor/bin/hw/android.hardware.composer.hwc3-service.gua
signal 6 (SIGABRT), code 0 (SI_USER from pid 495, uid 0)   ← 被 pid495 主动 kill
backtrace:
  #00 libc.so  __ioctl
  #02 libdrm.so  drmIoctl
  #03 libdrm.so  drmModeConnectorSetProperty
  #04 hwcomposer.gua.so  DrmAtomicStateManager::SetPowerStateForDpms(bool, PanelPowerNotify)
  #05 hwcomposer.gua.so  LocalHwcDisplay::SetPowerMode(int)
  #06 ComposerClient::setPowerMode(long, PowerMode)
```
- 现象：tombstone 短时间堆到 43（14:46-47 持续产生）；`system_server` pid 每几秒变一次。
- 内存/CPU 正常（5.7G 空闲、大量 idle）→ 非 OOM。

## 初步判断（待确认）
- HWC 在 DPMS 电源模式切换的 **DRM `drmModeConnectorSetProperty` ioctl 阻塞/挂死** → 被 hang 看门狗(pid 495) SIGABRT。
- HWC 反复被打死 → SF onrestart → system_server 依赖显示栈 → **崩溃循环**，AMS/PM 无法就绪。
- **待确认**：pid 495 是谁（看门狗/hybridscheduler?）；是**独立缺陷**（DRM ioctl 卡死）还是设备先前 wedged 的**级联症状**。需要开发确认 DRM/DPU 侧 `drmModeConnectorSetProperty` 为何会 hang。

## 影响
- 一旦触发即**整机 adb 不可测**（框架服务永不就绪），只能 reboot 恢复。生产上等价于**黑屏/无响应/需重启**级故障。

## 复现 / 下一步
- 目前为偶发观察（1 次），非稳定复现步骤。**下一步**：反复 DPMS 灭亮屏（`THIRD_SCREEN_OFF`/`ON` VHAL 或 `SetPowerMode`）压力，看能否稳定复现 HWC 在 `drmModeConnectorSetProperty` 卡死。
- 确认 pid 495 身份：`adb shell ps -A | grep 495` / 看门狗日志。

## 关联
- 机制 → [[composer_stub]]｜[[SurfaceFlinger|SF]]｜[[HWC]]
- 相邻缺陷 → [[BUG-kill-HWC-crashes-A720-composer_stub]]（kill HWC 拖崩 A720，不同路径）
- 触发场景用例 → [[TC_CSOC_FAULT_002]]（首跑因此 setup 失败）｜[[TC_HWC_FAULT_004]]
- 总览 → [[GFWK 稳定性测试用例全量清单]]

## 📚 延伸阅读
- DRM KMS 属性/DPMS：https://www.kernel.org/doc/html/latest/gpu/drm-kms.html
- Android HWC PowerMode/setPowerMode：https://source.android.com/docs/core/graphics/hwc
