---
title: "TC_HWC_FAULT_002 — 坏 Layer 参数（HWC 拒绝非法参数，不 crash）"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - HWC
  - 故障注入
  - Binder
case_id: TC_HWC_FAULT_002
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

# TC_HWC_FAULT_002 — 坏 Layer 参数（HWC 拒绝非法参数，不 crash）

> 上级：[[000-GFWK图形框架总览]]
> 场景：故意喂给挂图工（[[HWC]]）**非法参数**，看它能不能拒绝请求、自己不崩，也不把 DPU 硬件寄存器打坏。
> 代码落点：`cases/MultiMedia/GPU/Hwc/TC_HWC_FAULT_002.py`（Ubuntu `~/Documents/autocase`）
> 参照结构：[[TC_SF_FAULT_001]]

## ① 这条用例到底测什么（一句话）

**喂给 HWC 4 种明显错误的 layer 参数 → 断言 HWC 进程不重启 + dmesg 无 composer crash / DPU fatal**。

墙报比喻：你故意给挂图工传一张"尺寸是负数"的纸、一张"旋转角度是 NaN（不是数）"的纸、一张"透明度 300%（超出 255 上限）"的纸……**挂图工应该把这些纸全部退回来，而不是自己当场晕倒、把整面墙弄坏**。

## ② 4 种非法 Binder 参数（实现方式与意图）

注入通过 `service call` 直接发 Binder transaction，传 4 组畸形 `i32` 值（绕过 SF 的上层校验）：

| 注入值 | 代表意图 | 为什么危险 |
|---|---|---|
| `0xffffffff 0xffffffff` | 全 bit 置 1（非法 dataspace/格式） | driver 可能路由到不存在的转换表 → null deref |
| `0x80000000 0x80000000` | INT_MIN（最小负整数） | 符号扩展 → 负坐标/负尺寸 → DPU 越界读写寄存器 |
| `0x00000000 0xdeadbeef` | 经典魔数（非法地址标记） | 若被解引用 → segfault；测试 HWC 是否校验指针类参数 |
| `0x7fffffff 0x7fffffff` | INT_MAX（最大正整数） | 超大 crop/alpha → driver 位运算溢出 |

> 原始概念意图（crop=(-1,-1)、transform=NaN、alpha=300、dataspace=非法）映射为上述 i32 边界值。`service call` 层面只有 `i32`，没有类型语义——HWC 必须对**任意非法输入**都能返回错误码而不崩。

## ③ 断言逻辑

| 断言 | 检查方式 | 为什么选它 |
|---|---|---|
| HWC 进程不重启 | `pidof android.hardware.composer.hwc3-service.gua` 前后对比，pid 不变 | 进程重启 = crash 被系统拉起，说明非法参数把 HWC 打死了 |
| 无 dmesg crash | `grep -iE 'composer.*(crash\|abort\|segv)\|dpu.*fatal'` | DPU fatal 说明参数已传到硬件层，寄存器可能被污染 |

没有"画面截图"断言——这条用例专门测"**拒绝非法请求**"，不测"合成结果是否正确"（那是 TC_HWC_CONF_001 的事）。

## ④ 当前状态：🟡 自动化已集成（service call Binder 注入）

注入方式从"HAL 层 harness"改为 **`service call` 直接戳 HWC Binder 接口**，无需编译 C++。

`hwc_layer_fuzz` shell 脚本**内嵌到 Python 用例**，运行时自动 deploy 到设备：
- 测试启动 → 写 fuzz 脚本到 `/data/local/tmp/hwc_layer_fuzz` → `chmod +x` → 执行
- 断言：输出含 `PASS` + HWC pid 不变 + dmesg 无 crash
- git: `b122e90c`（branch `qi.zhu`）

运行命令：
```bash
source ~/.virtualenvs/py312/bin/activate
cd /home/gua/Documents/autocase
pytest cases/MultiMedia/GPU/Hwc/TC_HWC_FAULT_002.py --serial A41AEC42 -v
```

## ⑤ 手动验证（无需 hwc_layer_fuzz，台架可直接跑）

### 实测（2026-07-27，台架 A41AEC42）
我们跑了 10 轮 [[SIGKILL （kill -9）|kill -9]] SF，HWC pid=499 全程不变 → 证明这台架的 BSP 正确实现了进程边界隔离。这是 D3（故障注入）验收的前提条件。

**Step 1：查进程名**
```bash
adb -s A41AEC42 shell ps -A | grep -i compos
```
```
system   499    1   11469272  16276 binder_ioctl_write_read 0 S android.hardware.composer.hwc3-service.gua
system   662    1   10883304   5284 binder_ioctl_write_read 0 S vendor.gua.hardware.composer.pq-service.gua
```
> 本台架 HWC 主进程：`android.hardware.composer.hwc3-service.gua`（pid=499）
> `pq-service` 是色彩后处理服务，不是 HWC 主体。

**Step 2：记录初始 pid**
```bash
adb -s A41AEC42 shell pidof android.hardware.composer.hwc3-service.gua
# 499
```

**Step 3：kill SF 10轮，观察 HWC pid**
```bash
for i in $(seq 1 10); do
  adb -s A41AEC42 shell su 0 kill -9 $(adb -s A41AEC42 shell pidof surfaceflinger)
  sleep 3
  echo "Round $i HWC pid: $(adb -s A41AEC42 shell pidof android.hardware.composer.hwc3-service.gua)"
done
```
```
Round 1 HWC pid: 499
Round 2 HWC pid: 499
Round 3 HWC pid: 499
kill: missing argument (see "kill --help")
Round 4 HWC pid: 499
Round 5 HWC pid: 499
Round 6 HWC pid: 499
kill: missing argument (see "kill --help")
Round 7 HWC pid: 499
Round 8 HWC pid: 499
kill: missing argument (see "kill --help")
Round 9 HWC pid: 499
Round 10 HWC pid: 499
```
> `kill: missing argument`（第3/6/9轮）= SF 正处于重启窗口，`pidof` 返回空，kill 无参数 → 无害，HWC pid 仍然不变。

**Step 4：检查 dmesg**
```bash
adb -s A41AEC42 shell dmesg | grep -iE 'hwc3|composer|dpu.*fatal'
```
```
[ 8664.050508] servicemanager: Found android.hardware.graphics.composer3.IComposer/default in device VINTF manifest.
```
> 只有正常的 servicemanager 查找日志，无 crash / abort / DPU fatal。

### 结论：**SF-HWC 故障隔离正常** ✅

| 验证项 | 结果 |
|---|---|
| HWC pid 全程 | 499（10轮不变） |
| dmesg crash/fatal | 0 条 |
| SF 恢复 | 正常（kill: missing argument = SF 重启中，属预期） |

## ⑥ hwc_layer_fuzz 脚本说明（内嵌于 Python 用例，自动 deploy）


```sh
#!/system/bin/sh
HWC_SVC="android.hardware.graphics.composer3.IComposer/default"  # Binder 服务名
HWC_PROC="android.hardware.composer.hwc3-service.gua"            # 本台架 HWC 进程名

pid_before=$(pidof $HWC_PROC)     # ① 注入前记录 pid

# ② 发 4 次畸形 Binder transaction（code=1，传非法 i32 对）
service call $HWC_SVC 1 i32 0xffffffff i32 0xffffffff  # 全非法位
service call $HWC_SVC 1 i32 0x80000000 i32 0x80000000  # INT_MIN
service call $HWC_SVC 1 i32 0x00000000 i32 0xdeadbeef  # 魔数
service call $HWC_SVC 1 i32 0x7fffffff i32 0x7fffffff  # INT_MAX

pid_after=$(pidof $HWC_PROC)      # ③ 注入后记录 pid

if [ "$pid_before" = "$pid_after" ]; then
  echo PASS: HWC pid unchanged $pid_after  # ④ pid 不变 = 没崩 = PASS
  exit 0
else
  echo FAIL: HWC crashed before=$pid_before after=$pid_after
  exit 1
fi
```

Python 用例在 `_deploy_fuzz()` 中把上述每行用 `adb shell echo 'line' >> /data/local/tmp/hwc_layer_fuzz` 写入设备，然后 `chmod +x` 后执行，检查输出含 `PASS` 作为断言。

**实测返回**：每次 `service call` 返回 `fffffff8`（= `-8` = `EX_ILLEGAL_ARGUMENT`）→ HWC 识别并拒绝，进程不崩。

## ⑦ 和 TC_SF_FAULT_001 的对比（理解 D3 维度）

|        | TC_SF_FAULT_001         | TC_HWC_FAULT_002        |
| ------ | ----------------------- | ----------------------- |
| 故障注入对象 | [[SurfaceFlinger]]（班长） | [[HWC]]（挂图工）            |
| 注入方式   | `kill -9`（进程级，[[SIGKILL （kill -9）\|SIGKILL]]）  | 非法参数（接口级，协议违反）          |
| 期望行为   | 进程重启后自愈                 | 拒绝请求、进程不崩               |
| 需要的工装  | adb root（已有）            | service call（已有，无需编译）   |
| 当前状态   | ✅ 验收通过（180轮/41min）      | 🟡 自动化已集成（`b122e90c`）   |

两条用例合起来覆盖了"进程级崩溃恢复"和"接口级鲁棒性"——是故障注入维度(D3)的两个主要子方向。

## 关联
- 被测组件 → [[HWC]]
- 上级地图 → [[000-GFWK图形框架总览]]
- 模式参照 → [[TC_SF_FAULT_001]]
- 多屏隔离扩展 → [[TC_HWC_FAULT_003]]（一屏坏参数不波及另一屏）
- 重点用例集 → [[重点用例-高亮]]
