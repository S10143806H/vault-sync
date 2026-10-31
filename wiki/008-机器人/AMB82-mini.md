---
title: AMB82-mini（Realtek Ameba Pro2 / RTL8735B）
tags: [机器人, MCU, AmebaPro2, Arduino, 实体]
created: 2026-10-31
updated: 2026-10-31
source:
  - https://github.com/Ameba-AIoT/ameba-arduino-pro2 (dev 2026-10-30, variants/ameba_amb82-mini)
  - https://github.com/Ameba-AIoT/ameba-arduino-doc
---

# AMB82-mini（Ameba Pro2）

## 规格（已核对 SDK 源码）

| 项 | 值 |
|---|---|
| CPU | Arm Cortex-M33（Armv8-M）@500 MHz，单精度 FPU + DSP + TrustZone |
| 内存 | 片上 SRAM 512 KB（Arduino 可用约 106.5 + 222 KB）+ **封装内 DDR 128 MB**（NPU 占 36 MB） |
| Flash | 16 MB |
| AI | NPU 0.4 TOPS，只支持 Realtek Acuity 转换的固定类型模型（YOLO / 人脸 / 分类等）；另有 TFLite Micro 2.1-alpha |
| 无线 | 802.11 a/b/g/n 双频 Wi-Fi，BLE 5.1。**没有 BLE HID 主机**，连不了 Xbox 手柄 |
| 多媒体 | ISP、H.264/H.265、音频 codec、摄像头 FPC、SD 卡 |
| RTOS | FreeRTOS：1 kHz tick，11 级优先级，**堆 120 KB**；建线程用 `os_thread_create_arduino()` |
| 供电 / 电平 | micro-USB 5V（板上有两个 micro-USB）；IO 为 **3.3V，不耐 5V** |
| 外形 | 约 37.8 × 61.9 mm（Fritzing 图），两排各 15 针，没有安装孔 |

## 引脚速查（`variant.h`）

| 功能 | 引脚 |
|---|---|
| Serial（USB 日志） | D25 / D26 |
| **Serial1** | TX D21 (PA_2) / RX D22 (PA_3)。时钟源可能是 4 MHz（未核实），不建议跑 1 Mbps |
| **Serial2** | **TX D19 (PD_15) / RX D18 (PD_16)**，UART2 时钟 50 MHz，适合跑 1 Mbps |
| Serial3 | TX D15 / RX D14，与 SPI0 冲突 |
| **Wire** | **SDA D12 (PE_4) / SCL D13 (PE_3)** |
| Wire1 | SDA D9 / SCL D10 |
| ADC | A0 = D11，A1 = D10，A2 = D9，A4 = D28，A5 = D27，A6 = D21，A7 = D22；量程 0–3.3V |
| PWM | D1–D8、D23 |
| 板载 | LED_B = D23，LED_G = D24，PUSH_BTN = D29 |
| 尽量别用 | D27 / D28（SWD）、D25 / D26（日志口） |

## 坑（读 SDK 源码发现）

1. **`SerialX.flush()` 是空函数**（源码只有 `// TODO`）。所以半双工总线不能靠方向脚切换收发，要用自动换向电路。
2. **UART 接收环形缓冲只有 128 B**，每收一个字节进一次中断。大批量应答要分批读。
3. **`.text/.rodata` 链接在 DDR**：大常量数组（比如神经网络权重）可以直接放 `static const`；但 **malloc 走 120 KB 的 FreeRTOS 堆**，大块分配会失败。
4. **BNO055 不能用**：I²C clock stretching 问题，社区已确认。改用 MPU6050 或 BMI270，SDK 自带这两种的库。
5. **烧录**：按住 `UART_DOWNLOAD` → 按一下 `RESET` → 松开 `UART_DOWNLOAD`。开发板管理器 URL 见 [[Ameba鸭子-操作手册]]。

## 相关

[[Ameba鸭子-可行性调研与详细设计]] · [[STS3215]]
