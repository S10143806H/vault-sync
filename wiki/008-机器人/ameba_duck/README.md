---
title: ameba_duck 代码目录
tags: [机器人, 代码, AmebaPro2]
created: 2026-10-31
updated: 2026-10-31
---

# ameba_duck — Open Duck Mini v2 on AMB82-mini

文档入口：[[Ameba鸭子-操作手册]]（照着做）· [[Ameba鸭子-可行性调研与详细设计]]（为什么这么做）

```
ameba_duck/
├── firmware/ameba_duck/          Arduino 工程 (开发板选 AMB82-MINI)
│   ├── ameba_duck.ino            主程序: 50Hz 控制线程 + 命令行 + UDP + 标定向导 + 保护
│   ├── duck_config.h             ⚠ 每台鸭子要改: Wi-Fi / 零位偏移 / IMU 轴向 / kp
│   ├── sts_bus.h/.cpp            STS3215 总线驱动
│   ├── duck_policy.h/.cpp        MLP 推理 (纯 C++)
│   ├── imu_mpu6050.h/.cpp        MPU6050 驱动
│   └── policy_weights.h          ← 由 tools/export_policy.py 生成 (不入库)
├── tools/
│   ├── export_policy.py          ONNX -> C 头文件 (先与 onnxruntime 对拍)
│   ├── run_host_tests.sh         PC 一键自测
│   ├── test_policy_host.cpp      推理对拍
│   ├── test_sts_host.cpp         虚拟舵机总线单测
│   └── duck_teleop.py            Wi-Fi 遥控 (键盘/手柄)
└── cad/
    ├── gen_parts.py              新增打印件生成器 (manifold3d)
    ├── amb82_tray.stl            主控托架 (装在原 Pi Zero 位置)
    ├── stand_foot_tee.stl        调试支架底座接头 x2
    └── stand_top_elbow.stl       调试支架顶部弯头 x2
```

## 快速开始

```bash
pip install onnx onnxruntime numpy
bash tools/run_host_tests.sh /path/to/Open_Duck_Mini/BEST_WALK_ONNX_2.onnx   # 期望 ALL HOST TESTS PASSED
```

## 已验证 / 未验证

- ✅ C++ 推理与 onnxruntime 最大误差 9.9e-7；STS 报文与 rustypot 测试向量逐字节一致；虚拟总线 14 路读写与掉线容错通过。
- ✅ 固件在 g++ + Arduino 桩头文件下编译并能运行 `setup()`。
- ⏳ 尚未在 Ameba 官方工具链编译，尚未上实物。

## 许可与致谢

- 策略、观测定义、舵机参数源自 [Open_Duck_Mini / Runtime / Playground](https://github.com/apirrone)（Apache-2.0）。
- 协议细节对照 [pollen-robotics/rustypot](https://github.com/pollen-robotics/rustypot)。
- 半双工电路为 Feetech/Waveshare 参考电路，对照 [HeyPCB/quack-core](https://github.com/HeyPCB/quack-core) 的原理图核对。
