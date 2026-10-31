---
title: Open Duck Mini
tags: [机器人, 开源硬件, 双足, 强化学习, 实体]
created: 2026-10-31
updated: 2026-10-31
source: https://github.com/apirrone/Open_Duck_Mini
---

# Open Duck Mini

Disney BDX 机器人的开源微缩版。作者是 Antoine Pirrone（Pollen Robotics），由 Hugging Face / Pollen 赞助。当前版本是 **v2**；截至 2026-10 没有 v3，只有社区改版。

## 关键事实（v2）

| 项 | 值 |
|---|---|
| 尺寸 | 腿伸直约 42 cm |
| BOM | 低于 $400（官方 README）；国内自购约 ¥2,500–3,000 |
| 自由度 | 14：每条腿 5 个（hip yaw/roll/pitch、knee、ankle），头颈 4 个 |
| 执行器 | 14 × [[STS3215]] 7.4V，单总线 1 Mbps |
| 主控 / IMU | Raspberry Pi Zero 2W / BNO055 |
| 供电 | 2 × 18650 串联（7.4V）+ BMS + USB-C 充电；5V UBEC 给 Pi 供电 |
| 结构 | 36 种共 51 个打印件（PLA 15%，鞋底 TPU 40%），Onshape CAD 公开 |
| 许可 | Apache-2.0。Runtime 仓库没有 LICENSE 文件（issue #45） |

## 软件栈

- **训练**：`Open_Duck_Playground`，基于 MuJoCo Playground + Brax PPO。
  - 用 Disney BDX 论文里的模仿奖励，参考步态由 `Open_Duck_reference_motion_generator` 生成。
  - 电机模型用 Rhoban **BAM** 辨识，参数来自 `feetech_sts3215_7_4V`。
- **部署**：`Open_Duck_Mini_Runtime`（v2 分支），Python + onnxruntime + rustypot，50 Hz。
- **策略**：MLP，101 → [512, 256, 128] SiLU → 14（tanh），参数 22 万。观测布局见 [[Ameba鸭子-可行性调研与详细设计#4.4 固件设计]]。

## 社区衍生

| 项目 | 内容 |
|---|---|
| `rkuo2000/AmebaPro_OpenDuck` | AMB82-mini 的 IK 步态移植（不是 RL），用 MPU6050 |
| `HeyPCB/quack-core` | Pi Zero 载板。KiCad，CERN-OHL-S，尚未打样 |
| `jyxddd/open-duck-mini-notes` | 中文调试笔记：寄存器 85、kp、I²C 问题 |
| `open-duck-mini-rdkx5` | 地平线 RDK-X5 移植 |

## 相关

[[Microduck]] · [[Ameba鸭子-可行性调研与详细设计]] · [[Ameba鸭子-操作手册]]
