#!/usr/bin/env bash
# 主机端(PC/Linux/WSL/macOS)自测: 导出策略头文件 -> 编译 C++ 推理 -> 与 onnxruntime 对拍 -> 舵机协议报文单测
# 用法: bash tools/run_host_tests.sh /path/to/BEST_WALK_ONNX_2.onnx
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
FW="$HERE/../firmware/ameba_duck"
ONNX="${1:?用法: run_host_tests.sh <BEST_WALK_ONNX_2.onnx>}"
PY="${PYTHON:-python3}"
BUILD="${BUILD_DIR:-$(mktemp -d)}"

echo "[1/3] 导出 policy_weights.h / policy_selftest.h"
"$PY" "$HERE/export_policy.py" --onnx "$ONNX" --out "$FW"

echo "[2/3] 编译并运行策略推理对拍"
g++ -O2 -std=c++17 -I"$FW" "$FW/duck_policy.cpp" "$HERE/test_policy_host.cpp" -o "$BUILD/test_policy"
"$BUILD/test_policy"

echo "[3/3] 舵机协议报文单测"
g++ -O2 -std=c++17 -DSTS_HOST_TEST -I"$FW" -I"$HERE" "$FW/sts_bus.cpp" "$HERE/test_sts_host.cpp" -o "$BUILD/test_sts"
"$BUILD/test_sts"
echo "ALL HOST TESTS PASSED"
