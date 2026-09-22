---
title: "设备 SN(序列号)查询与恢复速查"
tags: [速查, SN, 序列号, NV, uboot, GTMP, AAOS, guav100]
platform: "V27/T29 域控(goldencar/qrt29), NV 存于 NOR flash /dev/nv0"
created: 2026-09-22
---

# 设备 SN(序列号)查询与恢复速查

> SN 是域控唯一身份(adb/fastboot 识别设备用)。存于 **NOR flash 的 NV 分区**(`/dev/nv0`,by-name → `mtdblock0pXX`)。**全格式化会清空 SN**。实测于 V27(SH0296)。来源:飞书「NV分区/设备序列号恢复SOP」。

## ① 查询 SN
```bash
adb -s <dev> shell getprop ro.serialno          # 系统 SN(= adb serial)
adb -s <dev> shell getprop ro.boot.serialno      # bootloader 传入的 SN
adb devices                                       # serial 列;ecub样有效 SN 以 "a/A" 开头
# NV 原始(权威来源),格式 offset:key:len:value
adb -s <dev> root
adb -s <dev> shell "su 0 cat /dev/nv0 | head -1"  # 0:sn:<len>:<SN>
# 只取纯 SN 值:
adb -s <dev> shell "su 0 sh -c \"cat /dev/nv0 | tr ' ' '\n' | grep -oE '^0:sn:[0-9]+:[A-Za-z0-9]+' | cut -d: -f4\""
```
- serial 是 `00000000` 或非 "a/A" 开头 → **SN 已破坏/未 provisioning**;`/dev/nv0` 若是 `abcdef0123456789` 是固件占位符。

## ② 写自定义 SN(先备份!)
```bash
adb -s <dev> root
# 备份 NV(出错可还原)
adb -s <dev> shell "su 0 dd if=/dev/block/by-name/nv of=/data/local/tmp/nv_backup.img bs=1M"
adb -s <dev> pull /data/local/tmp/nv_backup.img ./nv_backup.img
# 写 SN(⚠️ 格式是 offset:值,值里不要带 "sn"!)
adb -s <dev> shell "su 0 sh -c 'echo 0:A0SH0296 > /dev/nv0'"
# 回读确认(应 0:sn:8:A0SH0296)
adb -s <dev> shell "su 0 cat /dev/nv0 | head -1"
# 可选 VIN(offset 20 key=carvin):  echo 20:carvin<VIN> > /dev/nv0
```
> **坑1(格式)**:`echo 0:snA0SH0296` 会写成值=`snA0SH0296`(多了 sn 前缀)。正确 `echo 0:A0SH0296`(offset 0 的 key 固定是 sn,只给值)。
> SN 选 **"A" 开头 + 唯一**(别撞真实/别台架 SN,否则冲突);8 位(如 `A0SH0296`)。

## ③ 让 SN 真正生效 —— 必须进 uboot 写(关键坑2)
> **`echo > /dev/nv0` 只更新内核侧 NV,重启后 `ro.boot.serialno` 不变、`adb devices` 还是旧的**——因为 **bootloader 从它自己的 NV 读 SN 并经 cmdline `androidboot.serialno=` 传入**。必须在 **uboot** 写才生效。
```bash
adb -s <dev> reboot bootloader        # 进 uboot(debug 版可操作;release 有超时监控)
# 开 IVI 串口控制台(SH0296: /dev/ttyUSB8, baud 115200; 乱码试 1500000):
picocom -b 115200 /dev/ttyUSB8
#   → uboot 倒计时时【狂敲回车/空格】打断 autoboot 拿到 => 提示符
nv write sn A0SH0296
nv read sn                            # 回读确认
reset
# 起来后验证:
adb devices                           # serial = A0SH0296
adb -s A0SH0296 shell getprop ro.boot.serialno
```
- uboot 也写失败 = NV 分区损坏 → GFT 单镜像刷版本包里的 `nv.bin` 后再 `nv write sn`。

## ④ 还原 SN
```bash
adb -s <dev> shell "su 0 dd if=/data/local/tmp/nv_backup.img of=/dev/block/by-name/nv bs=1M"
# 或进 uboot: nv write sn <原SN>
adb -s <dev> reboot
```

## ⑤ ⚠️ 改 SN 后必做:同步更新 GTMP 台架绑定(坑3,血泪)
> 改 SN 后,**GTMP 台架 car0 仍绑旧 serial → 所有该台架的 GTMP 任务在 power cycle/恢复时报 `RuntimeError: car0 not restored after power cycle` 而失败**(实测 task 182916)。
```bash
# 1. 改 bench yaml 的 car0 绑定
sed -i 's/adb,car0,\^<旧SN>/adb,car0,^<新SN>/' \
  ~/data/apps/gtmp-client/data/config/ECU_*_<bench>.yaml
# 2. GTMP 管理端把该 testbench 注册的设备 serial 也改成新 SN(否则会从注册信息重生成 yaml 覆盖)
```

## 要点
- SN 在 `/dev/nv0`(NOR flash NV 分区);格式 `offset:key:len:value`,SN=offset 0(key sn),VIN=offset 20(key carvin)
- 写值格式 `echo <offset>:<值> > /dev/nv0`(值不含 key 前缀)
- **adb serial 由 bootloader 从 NV 读 → 只有 uboot `nv write sn` 才改得动**;`echo /dev/nv0` 不够
- 改 SN = 改设备身份,**必须同步 GTMP 台架绑定**,否则 GTMP 任务 power cycle 后失败
