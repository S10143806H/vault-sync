---
title: Microduck
tags: [机器人, 双足, 强化学习, NVIDIA, HuggingFace, 实体]
created: 2026-10-31
updated: 2026-10-31
source:
  - https://github.com/pollen-robotics/microduck
  - https://github.com/pollen-robotics/microduck_rl
  - https://blogs.nvidia.com/blog/gtc-berlin-2026-keynote/
  - https://techcrunch.com/2026/10/23/hottest-toy-for-christmas-nvidia-ceo-plays-with-adorable-microduck-robots-20000-sell/
---

# Microduck

2026 年爆火的"英伟达鸭子"指的就是它。

- **研发**：Pollen Robotics / Hugging Face 研发，Seeed Studio 代工。
- **NVIDIA 的角色**：提供仿真训练工具（Isaac Lab 3.0 + Newton）并在台上推广。2026-10-21 GTC Berlin keynote 上，Jensen Huang 用 4 只 Microduck 收尾。
- **背景**：2026-09-02 NVIDIA 签约收购 Hugging Face，预计 2027 上半年交割。

## 关键事实

| 项 | 值 |
|---|---|
| 价格 / 销量 | $399 预售，2026-08-28 开售；2026-10-22 预订量超 2 万。新订单交期据报道推到 2027-04 之后 |
| 尺寸 | 约 25 cm，<800 g（MJCF 质量合计 0.737 kg） |
| 执行器 | 15 × **Dynamixel XL330**（Protocol 2.0，1 Mbps）。14 个由策略控制，第 15 个是嘴 |
| 主控 | Radxa Zero 3W（RK3566）。策略在 CPU 上用 ONNX Runtime 推理，NPU 只用来做鸭子检测 |
| IMU | 机身 LSM6DSV16X（挂在舵机总线上，ID 200），头部 BMI088 |
| 策略 | MLP [512, 256, 128] ELU，61 → 14，50 Hz；用 mjlab（MuJoCo Warp）+ PPO 训练。Isaac Lab 有 NVIDIA 的移植版 |
| 开源 | 软件 Apache-2.0；STL 网格 **CC BY-NC-SA 4.0**（禁止商用）；**CAD 和主板不开源**。官方媒体包要求不要称其为"开源硬件" |

## 对复刻的影响

- **硬件**：不能合法地照搬做商用。本 vault 的方案改用 [[Open Duck Mini]] v2 作基线。
- **价格**：Microduck 发布后，国内 XL330 报价 ¥180–500，Radxa Zero 3W 从 ¥135 涨到 ¥700 左右。STS3215 未受影响。
- **社区复刻**：`fanhao375/microduck-replica` 用飞特 HD-1910 + Radxa Zero 3W，2026-10-24 已实机行走。

## 相关

[[Open Duck Mini]] · [[Ameba鸭子-可行性调研与详细设计]]
