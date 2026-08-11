---
title: "TC_Melo2_AVM_Gear_Func_001 — AVM 环视随挡位切换"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - AVM环视
  - Camera
  - 座舱
  - CAN
case_id: TC_Melo2_AVM_Gear_Func_001
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

> 上级：[[座舱]]｜模块：[[相机（Camera）]] / [[AVM环视]]｜范式参考：[[TC_SF_FAULT_001]]
> 场景：通过 CAN（TsMaster 同星盒子）下发挡位 R→D→P，验证 [[AVM环视]] 窗口随挡位正确切换。
> 作者：jiahui.li（2026.03.12）｜类型：功能测试（CAN 驱动）｜fixture：`avm` + `can`
> 代码落点：`cases/MultiMedia/Camera/Func/TC_Melo2_AVM_Gear_Func_001.py`（函数 `test_avm_gear_infinite_loop`）

```Plaintext
用例标题: 验证 [R-D-P] 挡位切换时，AVM 窗口是否符合挡位状态

预置条件:
  1. 车机上电
  2. 连接 TsMaster 同星盒子（CAN 注入）

测试步骤:
  1. 遍历 R/D/P 挡，通过 CAN 信号下发切换挡位，检查 AVM 窗口是否符合挡位状态

预期结果:
  1. 各挡位下 AVM 窗口状态正确（见下方映射表）
```
> ⚠️ 源码 docstring 有 copy-paste 残留（用例编号误写成 `TC_Camera_ASIC_Func_012`、预期结果留有相机用例文字）；本文档按**实际代码**校正。

## ① 这条用例到底测什么（一句话）

用 CAN 模拟换挡 `R→D→P`，[[断言（assert）]] [[AVM环视]] 窗口**随挡位联动**：倒车出后视、前进出前视、驻车退出。

## ② 机制（CAN 下发 + AVM 断言）

- **信号注入**：`VehicleAction`（`can`）经 TsMaster 同星盒子下发 `set_gear()` / `set_turn_signal()`。
- **窗口校验**：`AVMAction`（`avm`）的 `check_avm_active(mode)`（AVM 是否拉起）+ `check_avm_window(mode)`（对应视图 Tag 是否在）。

**挡位 → AVM 映射**：

| 挡位 | CAN | 期望 AVM 状态 | 断言 |
|---|---|---|---|
| R 倒车 | `set_gear("R")` | 拉起 + **后视(reverse)** | `check_avm_active("reverse")` 且 `check_avm_window("reverse")` |
| D 前进 | `set_gear("D")` | 切到 **前视(drive)** | `check_avm_active("drive")` 且 `check_avm_window("drive")` |
| P 驻车 | `set_gear("P")` | **窗口退出** | `not check_avm_active("drive")` |

> 用例前置还拨了转向灯 `LEFT/RIGHT/OFF`（可触发侧视 AVM 场景）。

## ③ 断言（对应代码）

1. R 挡：AVM 拉起 + 后视 Tag 在
2. D 挡：AVM 切前视 + 前视 Tag 在
3. P 挡：AVM 窗口正常退出（不再 active）

## ④ 落地状态

- **已实现**（jiahui.li），非骨架；依赖 **CAN 注入工装**（TsMaster 同星盒子）+ AVM 模组。
- 类型：功能正确性（D1），验证"挡位↔视图"业务逻辑，非故障注入。
- 备注：函数名 `test_avm_gear_infinite_loop` 但实际是一次 R-D-P，非无限循环；如需长稳可包 loop。

## 关联
- 模块 [[相机（Camera）]] / [[AVM环视]]｜座舱域 [[座舱]]｜断言 [[断言（assert）]]

### 运行用例
```
pytest cases/MultiMedia/Camera/Func/TC_Melo2_AVM_Gear_Func_001.py --serial 4AC9EC42 
# 需 TsMaster 同星盒子接入 CAN + AVM 模组
```
