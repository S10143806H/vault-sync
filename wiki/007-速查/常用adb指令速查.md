---
title: "常用 adb 指令速查 (GFWK 稳定性测试)"
tags: [adb, 速查, 稳定性, AAOS, 工具]
platform: "gua / guav100 (AAOS)"
created: 2026-08-01
---

# 常用 adb 指令速查（GFWK 稳定性测试）

> 本页汇总 GFWK 故障注入/稳定性测试常用 adb 指令。多台架同时在线时命令加 `-s <序列号>`（如 `-s A41AEC42`）。多数需先 `adb root`。

## ① 连接 / 基础
```bash
adb devices                       # 列出在线设备(序列号)
adb -s A41AEC42 root              # 提权(故障注入几乎都需要)
adb kill-server && adb start-server   # adb 抽风时重启服务
adb -s A41AEC42 wait-for-device   # 阻塞等设备回来(reboot/panic后)
adb shell echo alive              # 探活(掉线检测)
```

## ② firmware / 版本
```bash
adb shell getprop ro.build.version.incremental   # 增量版本号(如 831) 最常用
adb shell getprop ro.build.fingerprint           # 完整指纹
adb shell getprop ro.build.date                  # 构建时间
adb shell getprop ro.build.product               # 产品(guav100)
```

## ③ 进程发现 / kill（故障注入核心）
```bash
adb shell pidof surfaceflinger                    # 查某进程 pid
adb shell "ps -A -o NAME | grep -iE 'graphics.composer|hwc'"   # 模糊找进程名
adb shell kill -9 <pid>                           # 强杀(注入)
adb shell "ls /proc/<pid>/fd | wc -l"             # 数进程 fd(查泄漏)
```
| 逻辑角色 | 进程名 |
|---|---|
| SF | `surfaceflinger` |
| HWC | `android.hardware.composer.hwc3-service.gua`（kill 连带 SF 重启） |
| PQ(非HWC) | `vendor.gua.hardware.composer.pq-service.gua` |
| 跨SoC发送端/投屏桥 | `vendor.gua.hardware.cluster-service` + `gipc_sdd` |
| 跨SoC接收端 | `/bin/composer_stub`（在 [[Weston\|A720]]，走串口不走 adb） |

## ④ 显示 / 判黑 / 多屏
```bash
adb shell "dumpsys SurfaceFlinger --display-id"   # 列各物理屏 display id
adb shell screencap -d <display-id> -p /data/local/tmp/x.png   # 抓指定屏
adb shell "stat -c%s /data/local/tmp/x.png"       # PNG 字节数(>30KB≈非黑, 纯黑压缩后极小)
adb shell input keyevent KEYCODE_WAKEUP           # 唤醒主屏
# 后排/第3屏点亮(不会自动亮, 走 VHAL):
adb shell 'dumpsys android.hardware.automotive.vehicle.IVehicle/default --inject-event 560992868 -a 0 -b 0x344c'
#   -b 0x344c=WITHOUT_CANN开  0x314f=WITH_CANN开  0x324e=关
```

## ⑤ 框架健康门（跑测前必查, 防崩溃循环白跑）
```bash
adb shell getprop sys.boot_completed              # =1
adb shell pidof system_server                     # 隔4s查两次, 同pid=非崩溃循环 ⭐
adb shell "pm list packages | wc -l"              # >100 = AMS/PM 已注册
adb shell service check activity                  # 'found' 而非 'not found'
```
> `pm`/`am` 报 `Can't find service: package/activity` = system_server 没起到注册服务(崩溃循环), 不是 adb 问题 → 需 `adb reboot`。见 [[BUG-HWC-DPMS-SetPowerMode崩溃循环]]。

## ⑥ 墓碑 / crash / 日志
```bash
adb shell "ls /data/tombstones/ | grep -v .pb"    # 列墓碑(排除 .pb)
adb shell "su 0 cat /data/tombstones/tombstone_00 | head -40"   # 看墓碑头(signal/backtrace)
adb shell "su 0 cat /data/tombstones/tombstone_00" | grep -iE 'double.?free|use.after.free'  # 查 double-free
adb shell "dmesg | grep -iE 'sync_file_poll|segfault|Oops|Kernel panic|dpu.*fatal'"  # 内核故障
adb shell logcat -c                               # 清 logcat(施压前)
adb shell "logcat -d -b crash | grep -iE 'FATAL|ANR in|WATCHDOG'"  # 抓崩溃/ANR
adb shell "ls /data/anr/ | wc -l"                 # ANR 数
```

## ⑦ 重启 / panic（重, 会掉线）
```bash
adb -s A41AEC42 reboot                            # 常规重启
# IVI 强制 kernel panic(测跨SoC韧性, 会进 ramdump):
adb shell "echo 1 > /proc/sys/kernel/sysrq"
adb shell "echo c > /proc/sysrq-trigger"
adb wait-for-device                               # 等回来
```

## ⑧ Binder / HWC 畸形参数注入（service call, 不需 native）
```bash
# 戳 SF Binder 畸形事务(验拒绝不崩):
adb shell "service call SurfaceFlinger 1 i32 0xffffffff"
# 戳 HWC composer3 畸形 layer(首 i32 可传目标 display id):
adb shell "service call android.hardware.graphics.composer3.IComposer/default 1 i32 <display_id> i32 0xffffffff i32 0xdeadbeef"
```

## ⑨ 内存 / fd 压力
```bash
adb shell "cat /proc/meminfo | grep MemAvailable" # 可用内存
adb shell "cat /proc/sys/fs/file-nr"              # 全局 fd 使用
adb shell "stress-ng --vm 2 --vm-bytes 70% -t 60s &"   # 内存压力(需 stress-ng)
```

## ⑩ 串口侧（A720/CP1，非 adb）
> A720 Cluster、CP1 等**走串口不走 adb**。框架内经 `init_serial.cluster_uart` 操作；手动经台架 host 的 `/dev/ttyUSB*`。串口号随插拔变，按 gtmp config 的 USB 物理路径解析。A720 常用：`pidof weston`、`dmesg | grep composer_stub`、`reboot`。

## 关联
- 方法论 → [[GFWK kill-恢复类测试]]｜用例总览 → [[GFWK 稳定性测试用例全量清单]]
- 概念 → [[SurfaceFlinger]] [[HWC]] [[composer_stub]] [[GIPC]] [[SHMEM]]
- 缺陷 → [[BUG-kill-HWC-crashes-A720-composer_stub]] [[BUG-HWC-DPMS-SetPowerMode崩溃循环]]

## 📚 延伸阅读
- Android adb 官方：https://developer.android.com/tools/adb
- dumpsys：https://developer.android.com/tools/dumpsys
