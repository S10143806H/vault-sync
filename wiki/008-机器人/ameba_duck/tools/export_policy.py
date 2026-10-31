#!/usr/bin/env python3
"""
export_policy.py — 把 Open Duck Mini v2 的 ONNX 行走策略导出成 Ameba Pro2 可编译的 C 头文件。

输入:  BEST_WALK_ONNX_2.onnx  (obs[1,101] -> continuous_actions[1,14])
输出:  policy_weights.h      (归一化参数 + 4 层 MLP 权重, float32, 行主序 [in][out])
       policy_selftest.h     (1 组固定输入 + onnxruntime 期望输出, 固件开机自检用)

网络结构(已从 ONNX 图核实):
    x = (obs - mean) * inv_std
    h = silu(x @ W0 + b0)   101 -> 512
    h = silu(h @ W1 + b1)   512 -> 256
    h = silu(h @ W2 + b2)   256 -> 128
    y = h @ W3 + b3         128 -> 28
    action = tanh(y[:14])   (后 14 维是 PPO 的 std 分支, 推理时丢弃)

用法:
    pip install onnx onnxruntime numpy
    python export_policy.py --onnx BEST_WALK_ONNX_2.onnx --out ../firmware/ameba_duck/
"""
import argparse
import os

import numpy as np
import onnx
from onnx import numpy_helper

EXPECTED_OPS = [
    "Sub", "Mul",
    "Gemm", "Sigmoid", "Mul",
    "Gemm", "Sigmoid", "Mul",
    "Gemm", "Sigmoid", "Mul",
    "Gemm", "Split", "Tanh",
]


def load_params(path):
    model = onnx.load(path)
    ops = [n.op_type for n in model.graph.node]
    if ops != EXPECTED_OPS:
        raise SystemExit(
            f"ONNX 图结构与预期不符, 请人工检查:\n  实际: {ops}\n  预期: {EXPECTED_OPS}"
        )
    inits = {i.name: numpy_helper.to_array(i) for i in model.graph.initializer}
    nodes = model.graph.node

    mean = inits[nodes[0].input[1]].astype(np.float32)
    inv_std = inits[nodes[1].input[1]].astype(np.float32)

    layers = []
    for n in nodes:
        if n.op_type == "Gemm":
            for a in n.attribute:
                if a.name in ("transA", "transB") and a.i != 0:
                    raise SystemExit(f"Gemm {a.name}={a.i}, 本脚本只支持 0")
            w = inits[n.input[1]].astype(np.float32)  # [in, out]
            b = inits[n.input[2]].astype(np.float32)  # [out]
            layers.append((w, b))
    return mean, inv_std, layers


def ref_forward(obs, mean, inv_std, layers):
    x = (obs - mean) * inv_std
    for i, (w, b) in enumerate(layers):
        x = x @ w + b
        if i < len(layers) - 1:
            with np.errstate(over="ignore"):
                x = x / (1.0 + np.exp(-x))  # SiLU (exp 溢出为 inf 时结果正确趋于 0)
    n_act = x.shape[-1] // 2
    return np.tanh(x[..., :n_act])


def c_array(name, arr, per_line=8):
    flat = arr.astype(np.float32).ravel()
    lines = []
    for i in range(0, len(flat), per_line):
        chunk = ", ".join(f"{v:.9e}f" for v in flat[i:i + per_line])
        lines.append("    " + chunk + ",")
    return f"static const float {name}[{len(flat)}] = {{\n" + "\n".join(lines) + "\n};\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--onnx", required=True)
    ap.add_argument("--out", default=".")
    args = ap.parse_args()

    mean, inv_std, layers = load_params(args.onnx)
    obs_dim = mean.shape[0]
    dims = [obs_dim] + [w.shape[1] for w, _ in layers]
    act_dim = dims[-1] // 2
    n_params = sum(w.size + b.size for w, b in layers) + 2 * obs_dim
    print(f"layers: {dims}, act_dim={act_dim}, params={n_params} ({n_params*4/1024:.0f} KiB fp32)")

    # 与 onnxruntime 对拍, 确保解析正确
    import onnxruntime as ort
    sess = ort.InferenceSession(args.onnx, providers=["CPUExecutionProvider"])
    rng = np.random.default_rng(0)
    test_obs = rng.normal(0, 1, size=(64, obs_dim)).astype(np.float32)
    ort_out = np.concatenate(
        [sess.run(None, {sess.get_inputs()[0].name: test_obs[i:i + 1]})[0] for i in range(64)]
    )
    np_out = ref_forward(test_obs, mean, inv_std, layers)
    err = np.abs(ort_out - np_out).max()
    print(f"numpy vs onnxruntime max abs err = {err:.2e}")
    if err > 1e-4:
        raise SystemExit("参数解析结果与 onnxruntime 不一致, 停止导出")

    os.makedirs(args.out, exist_ok=True)
    hdr = os.path.join(args.out, "policy_weights.h")
    with open(hdr, "w") as f:
        f.write("// 自动生成, 勿手改。来源: " + os.path.basename(args.onnx) + "\n")
        f.write("#pragma once\n\n")
        f.write(f"#define POLICY_OBS_DIM {obs_dim}\n")
        f.write(f"#define POLICY_ACT_DIM {act_dim}\n")
        f.write(f"#define POLICY_NUM_LAYERS {len(layers)}\n")
        f.write(f"#define POLICY_MAX_DIM {max(dims)}\n")
        f.write("static const int POLICY_DIMS[POLICY_NUM_LAYERS + 1] = {"
                + ", ".join(str(d) for d in dims) + "};\n\n")
        f.write(c_array("POLICY_OBS_MEAN", mean))
        f.write(c_array("POLICY_OBS_INV_STD", inv_std))
        for i, (w, b) in enumerate(layers):
            f.write(c_array(f"POLICY_W{i}", w))  # 行主序 [in][out]
            f.write(c_array(f"POLICY_B{i}", b))
        f.write("static const float* const POLICY_W[POLICY_NUM_LAYERS] = {"
                + ", ".join(f"POLICY_W{i}" for i in range(len(layers))) + "};\n")
        f.write("static const float* const POLICY_B[POLICY_NUM_LAYERS] = {"
                + ", ".join(f"POLICY_B{i}" for i in range(len(layers))) + "};\n")
    print("wrote", hdr)

    n_st = 8
    st = os.path.join(args.out, "policy_selftest.h")
    with open(st, "w") as f:
        f.write("// 自动生成: 开机自检/主机单测用的输入与 onnxruntime 期望输出\n#pragma once\n\n")
        f.write(f"#define SELFTEST_N {n_st}\n")
        f.write(c_array("SELFTEST_OBS", test_obs[:n_st]))  # [N][OBS_DIM]
        f.write(c_array("SELFTEST_ACT", ort_out[:n_st]))   # [N][ACT_DIM]
    print("wrote", st)


if __name__ == "__main__":
    main()
