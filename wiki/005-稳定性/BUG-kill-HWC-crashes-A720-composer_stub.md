---
title: "BUG: kill IVI HWC 拖崩 A720 composer_stub (SIGSEGV @ libwayland)"
tags:
  - 稳定性
  - AAOS
  - 缺陷
  - 跨SoC
  - composer_stub
  - HWC
platform: "gua / guav100 (AAOS)"
severity: 中高
status: 疑似已修复（20260803 日构不复现，待确认上游是否修复）
created: 2026-07-30
updated: 2026-08-03
---

# BUG：kill IVI HWC → A720 `/bin/composer_stub` SIGSEGV

## ⚠ 2026-08-03 复核更新（重要）
> **在新日构 `20260803` + 修正检测后，此缺陷不再复现**（5 轮 kill HWC，composer_stub 全部 `exit status 101` 干净退出 + init/GWDT 自恢复，**0 segfault / 0 killed-by-signal**，用例 PASS）。

- **原 07-30 证据仍有效**：旧构 831 上确有 5 个 SIGSEGV tombstone（真实缺陷，见下）。→ 结论：**当前构疑似已修复/不复现**，需向图形团队确认 831→20260803 之间是否落了修复。
- **期间几次"composer_stub 崩溃 N 次"报告作废**（task 124206 / 08-03 首次重跑的"2 次"）：经 cornerstone 内核日志核对，实为**测试检测的两处误报**，非真崩溃：
  1. **B2 adb server 错乱**（`Wrong data in check_okay`）被崩溃防护误当"设备崩溃" → 已修（`adb kill/start-server` 恢复后重判）
  2. **a720 串口共享控制台噪声**（时间戳/PID 数字）被 `re.search(\d+)` 当成 `grep -c` 计数 → 误报崩溃次数 → 已修（`A720CNT=` 唯一标记隔离）
- 修复提交：autocase `b9fc05da`(B2) / `e7373ab4`(串口计数) / `7b38e44a`(汇总三态措辞)；分析 bot KB 加 **G5** 排除项（exit 101=健康自恢复）。

## 原始发现 831
kill 掉 IVI(Android) 侧的 HWC(composer HAL) 后，**A720(仪表, Linux) 侧的 `/bin/composer_stub` 立即 SIGSEGV 崩溃**（挂在 libwayland-client 的悬空 proxy 上），**每 kill 一次崩一次，1:1 稳定复现**。IVI 本机显示能自恢复，但跨 SoC 接收端不做保护、直接段错误。

## 环境
- 版本：`guav100_dailybuild_userdebug_202607211449_831`（G1.30.M）
- 台架：A41AEC42
- 用例：`TC_HWC_FAULT_004`（kill HWC 恢复）

## 复现步骤
1. IVI 侧 `kill -9 $(pidof android.hardware.composer.hwc3-service.gua)`
2. 观察 A720（a720 串口）`/bin/composer_stub` 是否崩 / 产生 tombstone
3. 重复 N 次

**实测**：5 轮 kill → A720 产生 5 个 `CLUSTER_tombstone`，时间戳与 5 次 kill 逐一对应（1:1）。

## 关键证据（tombstone）
```
Cmdline: /bin/composer_stub
pid: 950  name: binder:950_3  >>> /bin/composer_stub <<<
ABI: 'arm64'
signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x0000aaa5348ebb02
backtrace:
  #00  /usr/lib/libwayland-client.so.0.22.0 (wl_proxy_get_version+4)
  #01  /bin/composer_stub
```
kill 与墓碑时间戳对应表：

| 轮 | kill 时刻 | CLUSTER_tombstone |
|---|---|---|
| 1 | 18:15:47 | tombstone_0 18:15:47 |
| 2 | 18:16:24 | tombstone_1 18:16:24 |
| 3 | 18:16:59 | tombstone_2 18:16:58 |
| 4 | 18:17:35 | tombstone_3 18:17:34 |
| 5 | 18:18:00 | tombstone_4 18:17:59 |

## 初步判断
- A720 的 `/bin/composer_stub` 是接收 IVI 投屏的 **Wayland 客户端**（见 [[composer_stub]]）。
- IVI HWC 重启时，跨 SoC 链路（GIPC/Wayland）中断，composer_stub 持有的 **wayland proxy 变悬空**，下次 `wl_proxy_get_version` 解引用 → SIGSEGV。
- **根因方向**：composer_stub 未处理"上游(IVI HWC/合成源)重启"事件，缺少 proxy 有效性校验 / 断线重连保护。

## 影响
- 单次 kill：composer_stub 崩后应会被拉起，投屏短暂中断；但**每次 IVI 显示栈抖动都让仪表侧崩一次**，累积风险（内存/句柄泄漏、极端时序下更严重故障）。
- 对比：IVI 本机三屏 kill 后能自恢复（就绪门验证过），A720 weston kill 也能自恢复；**唯独 composer_stub 直接段错误**。

## 建议
1. A720 `composer_stub` 增加 wayland proxy 有效性校验 + 上游断线的重连/优雅降级，别直接解引用悬空 proxy。
2. 用例侧已加**跨 SoC 崩溃断言**（`TC_HWC_FAULT_004` 经 a720 串口查 composer_stub segfault），把这类漏网崩溃纳入 CI。

## 现场日志
- tombstone：cornerstone post-test 包 `cluster/CLUSTER_tombstone_0..4`（`.zst` 为非标准帧，`_4` 为明文）
- 参考本地：`C:\Users\XGTech_ZHU\Downloads\cornerstone_log_20260730_181833_car0_post-test.tgz`

## 飞书问题单
- 文档：https://guatechltd.feishu.cn/docx/OOFZdiHMYohIVgxvjWUcDz4KnIh （2026-07-30 创建）

## 关联
- 机制 → [[composer_stub]]｜[[Weston]]｜[[GIPC]]｜[[SHMEM]]
- 用例 → [[TC_HWC_FAULT_004]]｜[[TC_CSOC_FAULT_002]]｜[[GFWK kill-恢复类测试]]
