---
title: Ameba 鸭子操作手册——用 AMB82-mini 一步一步复刻 Open Duck Mini v2
tags: [机器人, 操作手册, AmebaPro2, AMB82-mini, OpenDuckMini, STS3215]
created: 2026-10-31
updated: 2026-10-31
source:
  - https://github.com/apirrone/Open_Duck_Mini/tree/v2/docs
  - https://github.com/apirrone/Open_Duck_Mini_Runtime/tree/v2
  - https://ameba-doc-arduino-sdk.readthedocs-hosted.com (AMB82-mini Getting Started)
---

# Ameba 鸭子操作手册

> [!tip] 怎么用这份手册
> - 共 **11 个阶段**，每个阶段都写了**预计耗时**、**通过标准**和**常见坑**。按顺序做，每过一关打个勾。
> - 背景原理、为什么这样设计，见 [[Ameba鸭子-可行性调研与详细设计]]。
> - 代码在 vault 的 `wiki/008-机器人/ameba_duck/` 目录。
> - 下文命令里的 `$DUCK` 指这个目录。

**总进度（复制到你的日记里打勾）**

- [ ] 阶段 0：电脑上先看到鸭子走（30 分钟）
- [ ] 阶段 1：采购（下单 1 小时，到货 3–7 天）
- [ ] 阶段 2：3D 打印（机器约 50 小时，人工约 3 小时）
- [ ] 阶段 3：搭建 Arduino 环境并烧录固件（1 小时）
- [ ] 阶段 4：逐个配置 14 个舵机（1.5 小时）
- [ ] 阶段 5：机械组装（6–10 小时）
- [ ] 阶段 6：接线与上电检查（2 小时）
- [ ] 阶段 7：板上自检（30 分钟）
- [ ] 阶段 8：关节零位标定（1 小时）
- [ ] 阶段 9：站立（15 分钟）
- [ ] 阶段 10：行走与调参（1–3 小时）

---

## 阶段 0：电脑上先看到鸭子走（30 分钟，可选但强烈推荐）

目的是先在电脑上确认两件事：策略本身能走；本方案的 C++ 移植和官方完全一致。

1. 克隆两个仓库：
   ```bash
   git clone https://github.com/apirrone/Open_Duck_Mini
   git clone https://github.com/apirrone/Open_Duck_Playground
   ```
2. **跑本方案的主机自测**。需要 Python 3.10+ 和 g++；Windows 用 WSL。
   ```bash
   pip install onnx onnxruntime numpy
   bash $DUCK/tools/run_host_tests.sh Open_Duck_Mini/BEST_WALK_ONNX_2.onnx
   ```
   ✅ **通过标准**：最后一行打印 `ALL HOST TESTS PASSED`，策略误差约 `1e-6`。
3. （可选）**在 MuJoCo 里看策略行走**。官方 `uv` 环境依赖 `jax[cuda12]`，要 Linux 或 WSL2 加 NVIDIA 显卡：
   ```bash
   cd Open_Duck_Playground
   uv run playground/open_duck_mini_v2/mujoco_infer.py -o ../Open_Duck_Mini/BEST_WALK_ONNX_2.onnx
   ```
   - 方向键：前后左右平移。
   - Q / E：转向。
   - H：切换到头部控制。

---

## 阶段 1：采购（1 小时）

完整清单和价格见 [[Ameba鸭子-可行性调研与详细设计#4.1 物料清单（BOM，第一阶段：不画 PCB）]]。最容易买错的 5 样：

1. **舵机 STS3215 买 7.4V 版**（型号后缀 C001，1:345），14 个。**不要买 12V 版**：策略的电机模型是按 7.4V 版辨识的。
2. **AMB82-mini 开发板**（不是 AMB82-Zero 或 HUB-8735）。买带排针的，或者自己焊。
3. **Waveshare Bus Servo Adapter (A)**，也叫"总线舵机驱动板"。买不带 ESP32 的那款。
4. **GY-521（MPU6050）**。不买 BNO055：Ameba 驱动不了它。
5. **18650 电芯要高倍率**（如 Molicel P30B），两节同批次。

> 其余五金件（M3 热熔螺母、螺丝、轴承）照官方 BOM 表买：Open_Duck_Mini README 里的 Google Sheet / 飞书中文版。

---

## 阶段 2：3D 打印（约 50 小时机时）

1. **先试打一只脚**：`foot_top` + `foot_side` + `foot_bottom_pla`。拿一个舵机试装，确认公差合适。
   - 社区反馈这批文件没有统一实物验证过，先试打能省很多返工。
2. 按下表打印原版零件。文件在 `Open_Duck_Mini/print/`，数量见 `docs/print_guide.md`，共 51 个零件。

   | 设置 | 值 |
   |---|---|
   | 材料 | PLA；受力件（`knee_to_ankle_*`、`*_roll_to_pitch`、`leg_spacer`）建议用 PETG |
   | 层高 | 0.2 mm |
   | 填充 | 15%（PLA）；`foot_bottom_tpu.stl` 用 **TPU，40%** |
   | 打印平台 | **≥220 × 220 mm**（`head.stl` 有 200 × 197 mm） |
   | 用料 | 约 1 kg PLA + 约 35 g TPU |

3. 打印本方案新增的零件。STL 已在 `$DUCK/cad/` 里；改尺寸的话运行 `python gen_parts.py --pipe-od 20`：
   - `amb82_tray.stl` × 1：主控托架。
   - `stand_foot_tee.stl` × 2 和 `stand_top_elbow.stl` × 2：调试支架接头。
4. **搭调试支架**（10 分钟）：
   - 20 mm PVC 管截成：立柱 2 × 500 mm、横梁 1 × 400 mm、底脚 2 × 300 mm。
   - 底脚横穿 T 接头，立柱竖插上去，顶部用弯头连横梁。
   - 横梁中间挂一根魔术贴绑带。

✅ **通过标准**：51 个原版零件 + 5 个新零件全部打完，试装没有明显干涉。

---

## 阶段 3：搭建 Arduino 环境并烧录固件（1 小时）

1. 安装 **Arduino IDE 2.x**。
2. 打开 `文件 → 首选项 → 附加开发板管理器网址`，填入：
   ```
   https://github.com/Ameba-AIoT/ameba-arduino-pro2/raw/main/Arduino_package/package_realtek_amebapro2_index.json
   ```
3. 打开 `工具 → 开发板 → 开发板管理器`，搜索 **Realtek Ameba Boards**，安装。
4. 选择 `工具 → 开发板 → AmebaPro2 ARM (32-bits) Boards → AMB82-MINI`。
5. 用 micro-USB 线连电脑。如果没出现串口，装 CH341 驱动（官方文档给的是 wch-ic.com 的 CH341SER）。
6. **生成策略权重头文件**（只需做一次，换策略时再做）：
   ```bash
   pip install onnx onnxruntime numpy
   python $DUCK/tools/export_policy.py --onnx Open_Duck_Mini/BEST_WALK_ONNX_2.onnx --out $DUCK/firmware/ameba_duck/
   ```
   会生成 `policy_weights.h`（约 4 MB 源码）和 `policy_selftest.h`。
7. 编辑 `duck_config.h`：
   - 先只填 Wi-Fi：`DUCK_WIFI_SSID` / `DUCK_WIFI_PASS`。2.4G 或 5G 都可以，手机热点也行。
   - 其他参数后面的阶段再填。
8. 用 Arduino IDE 打开 `$DUCK/firmware/ameba_duck/ameba_duck.ino`，然后点上传。
9. 让板子**进入下载模式**：
   - 按住 `UART_DOWNLOAD` 键；
   - 按一下再松开 `RESET`；
   - 松开 `UART_DOWNLOAD`。
10. 上传完成后按一次 `RESET`。打开串口监视器：115200，换行符选 "Both NL & CR"。

✅ **通过标准**：串口打印出：
```
=== Ameba Duck (Open Duck Mini v2 on AMB82-mini) ===
[policy] selftest max err = 0.0000010  OK
```
⚠ 第一次编译约 4 MB 的权重头文件会比较慢，大约几分钟，属正常现象。

> **如果编译报错：**
> - 先确认 `policy_weights.h` 在 `.ino` 同目录。
> - 再把报错第一行贴给我，或者贴到 [[log]]。
> - 本固件只在 PC 桩环境编译验证过，**还没有用 Ameba 官方工具链编译过**。第一次编译出问题的概率不低，报错一般都是小修。

---

## 阶段 4：逐个配置 14 个舵机（1.5 小时）

新舵机出厂 ID 都是 1，必须**一个一个**接上去配置。

1. 把 AMB82 接到转接板，接线见阶段 6 的表格，此时只接 D19、D18、GND。转接板 DC 输入接电池。
2. 转接板上**只插 1 个舵机**。
3. 在串口输入 `cfg 20`。
   - 这一步等价于官方的 `configure_motor.py --id 20`，会写入：位置模式、P=32、I=D=0、加速度 0、**最大加速度（寄存器 85）0**，然后改 ID 并回中到 2048。
   - ✅ 看到 `cfg OK: 1 -> 20, 已回中(2048)`。
4. **此时装舵盘**：照官方 `docs/configure_motors.md` 的照片尽量对正。不用追求完美，阶段 8 会标定。
5. 在舵机上贴标签写上 ID，然后拔下，换下一个。按下表依次配置：

   | 关节 | 左 ID | 右 ID |
   |---|---|---|
   | hip_yaw | 20 | 10 |
   | hip_roll | 21 | 11 |
   | hip_pitch | 22 | 12 |
   | knee | 23 | 13 |
   | ankle | 24 | 14 |
   | neck_pitch / head_pitch / head_yaw / head_roll | 30 / 31 / 32 / 33 | — |

6. 全部配完后，把 14 个舵机串在总线上，输入 `scan`。

✅ **通过标准**：`scan` 列出 `10 11 12 13 14 20 21 22 23 24 30 31 32 33`，共 14 个。

> **常见坑：**
> - `cfg: 没找到舵机`：
>   1. 转接板跳线帽要在 UART 输入档；
>   2. 交换 D19 和 D18 试一次；
>   3. 确认舵机供电是 7.4V，电池开关已打开。
> - 有教程说可以用飞特官方上位机 FD.exe 配置。注意**不要在 FD 里把位置设成 0**：舵机会从 4095 绕远路转过去，装好以后会堵转。

---

## 阶段 5：机械组装（6–10 小时）

**照官方 `docs/assembly_guide.md` 一步步装**，或者看 Tnkr 图文版（链接在 README）。和官方的不同只有下面 4 处：

1. **不装 Pi Zero 2W**：
   - 用 M2.5 螺丝把 `amb82_tray` 拧在头部原 Pi 的 4 个安装柱上。
   - AMB82-mini 焊在 50 × 70 洞洞板上（或插在载板上），再用 M3 自攻螺丝固定到托架。
2. **"Motor control board"位置装 Waveshare 转接板**，原版这里就是同类板子。
3. **IMU 换成 GY-521**：
   - 贴在原 BNO055 的位置，芯片平面朝上，排针朝向随意，后面会在软件里设轴向。
   - 用双面泡棉胶粘牢，**不能松动**。
4. **从头到身体要穿过脖子的线**：Serial 3 根（TX/RX/GND）、I²C 4 根、足底开关 3 根。线束留够脖子转动的余量。

> **组装要点（来自官方和社区）：**
> - 金属对金属的螺丝涂 Loctite 243，塑料件上的螺丝不要涂。
> - `hip_pitch` 舵机的朝向影响零位，按装配图照片的方向装。
> - 足底微动开关要压装到位，脚着地时必须能触发。

✅ **通过标准**：用手掰每个关节，转动顺畅，没有卡死；14 根舵机线都接好了。

---

## 阶段 6：接线与上电检查（2 小时）

| AMB82-mini | 接到 | 备注 |
|---|---|---|
| D19 (Serial2 TX) | 转接板 TX | 不通就和 RX 对调 |
| D18 (Serial2 RX) | 转接板 RX | |
| GND | 转接板 GND、GY-521 GND、开关公共端 | **全部共地** |
| D12 (SDA) | GY-521 SDA | |
| D13 (SCL) | GY-521 SCL | |
| — | GY-521 VCC ← UBEC 5V | 模块自带 LDO |
| D16 | 左脚开关 → GND | |
| D17 | 右脚开关 → GND | |
| D20 | 暂停按键 → GND（可选） | |
| micro-USB | ← UBEC 5V | 调试时也可以直接插电脑 USB |
| 电池 7.4V → 开关 → BMS | → 转接板 DC 输入 和 UBEC 输入 | 舵机电**不经过** AMB82 |

**上电检查**（按顺序做，每一步都量）：

1. 先**不插 AMB82**。打开电池，量转接板输入：应为 7.0–8.4V；量 UBEC 输出：应为 4.9–5.2V。
2. 只给 GY-521 上电，量 SDA 对 GND 的空闲电压，**必须 ≤3.6V**，否则会烧 AMB82。
3. 插上 AMB82，看串口：`[servo] 14/14`，`[imu] MPU6050 OK`。

✅ **通过标准**：上面 3 步全部达标。

---

## 阶段 7：板上自检（30 分钟）

鸭子用绑带吊在支架上，脚离地。在串口依次输入：

| 命令 | 期望 | 不对怎么办 |
|---|---|---|
| `selftest` | `policy max err` < 1e-5 | 重新导出权重头文件 |
| `check` | 14 行都有 pos、V≈7.4、T≈室温 | `无应答!` 说明那一路线或 ID 有问题 |
| `feet` | 用手按左脚 → `left=1` | 开关接线或 D16/D17 |
| `imu` | 见下面的轴向标定 | 改 `IMU_AXIS_MAP/SIGN` |
| `bench` | 推理 < 8000 µs；sync read 成功率 100% | 见故障排查 |

**IMU 轴向标定**（最重要的一步，轴向错了必摔）：

机身坐标系：**x 朝前（鸭嘴方向），y 朝左，z 朝上**。输入 `imu`，在 3 秒里依次做下面 4 个动作，对照期望值：

1. 静止直立：`acc ≈ (0, 0, +9.8)`。哪一轴是 ±9.8，z 就映射到那一轴；符号为负就把 sign 设成 −1。
2. 鸭嘴朝下低头：`acc_x` 变**负**。
3. 右侧朝下侧倾：`acc_y` 变**正**。
4. 从上往下看逆时针转（原地左转）：`gyro_z` 为**正**。

改 `duck_config.h` 里的两个数组：
```cpp
// body_axis[i] = sign[i] * chip_axis[map[i]]
static const uint8_t IMU_AXIS_MAP[3] = {0, 1, 2};   // 例: 芯片 y 轴朝前就写 {1, 0, 2}
static const int8_t  IMU_AXIS_SIGN[3] = {1, 1, 1};  // 方向反了写 -1
```
改完重新烧录，再跑一次 `imu`，直到 4 条全部符合。

✅ **通过标准**：表中 5 条命令全部达标。

---

## 阶段 8：关节零位标定（1 小时）

舵盘不可能装得完全对正，所以要测出每个关节的软偏移。等价于官方的 `find_soft_offsets.py`。

1. 鸭子吊在支架上，**腿悬空**。
2. 输入 `zero`：所有关节低刚度转到 0 位。这时腿是**伸直**的，零位定义对照 Onshape CAD。
3. 对每个关节 i（0–13，顺序同阶段 4 的表，左腿 0–4、头 5–8、右腿 9–13）：
   1. 输入 `offset i`：这个关节卸力。
   2. 用手把它摆到正确零位（对照 CAD / 装配图）。
   3. 输入 `offset i ok`：记录偏移，关节重新上力。
4. 全部做完后输入 `offsets`，会打印一行：
   ```cpp
   static float JOINT_OFFSETS[DUCK_NUM_JOINTS] = {0.0123f, ...};
   ```
   把它**整行粘贴**替换 `duck_config.h` 里的同名行，重新烧录。
5. 输入 `off`。

✅ **通过标准**：重新烧录后执行 `zero`，左右腿目视对称、笔直。

> 社区经验：`hip_pitch`（ID 12/22）如果没装水平，读数会接近 −1.57 rad，这种情况要拆下来重装舵盘，不要用 offset 硬补。

---

## 阶段 9：站立（15 分钟）

1. 仍然吊在支架上，输入 `stand`：先软后硬，转到默认站姿（膝盖微屈）。
2. 看左右腿是否对称，输入 `stat` 看电压和温度。
3. 放下绑带，让脚着地，手护在旁边。

✅ **通过标准**：落地后能自己稳稳站住 10 秒以上，腿不抖。

> **腿以约 5 Hz 抖动**：输入 `off`，再输入 `kp 20 8`，然后 `stand`。kp 可在 20–30 之间试，找到不抖的值后写回 `duck_config.h` 的 `KP_LEGS`。

---

## 阶段 10：行走与调参（1–3 小时）

1. **悬空试跑**：吊在支架上，输入 `walk`。
   - 腿应该做规律的踏步动作。
   - 输入 `stat` 看 `overruns=0`、`bus_miss` 不增长、`total` < 15000 µs。
2. **原地踏步**：落地，手护着，保持 `walk`。指令为 0 时应该原地踏步，不倒。
3. **遥控**：电脑和鸭子连同一个 Wi-Fi，运行：
   ```bash
   python $DUCK/tools/duck_teleop.py --ip <串口打印的 IP>
   ```
   - W/S 前后，A/D 平移，Q/E 转向，Shift 冲刺。
   - 1 = stand，2 = walk，空格 = 暂停，0 = 卸力。
   - 也可以在串口里直接输入 `c 0.1 0 0`，让它一直以 0.1 m/s 前进。
4. 摔倒（倾角 >55°）会自动卸力，绿灯快闪。输入 `off` 清除故障，扶正后 `stand` → `walk`。

✅ **通过标准**：能在平地上按指令前进、转向，走 1 分钟不摔。

### 调参与故障对照表

| 现象 | 最可能的原因 | 处理 |
|---|---|---|
| 一 `walk` 就往后仰倒 | 寄存器 85（最大加速度）不是 0，舵机比仿真慢 | 每个舵机重新 `cfg`，或单独写 85=0 |
| 一 `walk` 就往一侧倒，或乱踢 | IMU 轴向或符号错 | 重做阶段 7 的 IMU 标定 |
| 站立或行走时腿高频抖 | kp 太高 | `kp 20 8` |
| 走路一瘸一拐、左右不对称 | 零位偏移不准 | 重做阶段 8 |
| `stat` 里 `bus_miss` 一直涨 | 总线接触不良或线太长 | 查 5264 插头；舵机降到 500 kbps（每个舵机写寄存器 6=1，固件 `DUCK_SERVO_BAUD` 改成 500000） |
| `stat` 里 `overruns` 增长 | 一个周期超过 20 ms | 看 `infer`/`read` 哪项大；推理慢就考虑 INT8 量化（§5） |
| FAULT: battery low voltage | 电池 < 6.6V | 充电 |
| FAULT: servo over temperature | 连续走太久（STS3215 无热保护，实测可到 71 ℃） | 停 10 分钟降温 |
| 一走就 FAULT: fall detected | 也可能是 IMU 的 z 轴装反了 | 静止时跑 `imu`，确认 `acc_z ≈ +9.8` |

---

## 安全守则

1. **第一次跑任何新动作都吊在支架上做。**
2. 手指不要放进关节夹缝：STS3215 堵转扭矩约 19.5 kg·cm。
3. 锂电池要用带 BMS 的电池盒，充电时有人看着。
4. 出任何状况先输入 `off`，或直接关电池开关。

## 相关

[[Ameba鸭子-可行性调研与详细设计]] · [[Open Duck Mini]] · [[STS3215]] · [[AMB82-mini]] · [[Microduck]]
