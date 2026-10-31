#!/usr/bin/env python3
"""
gen_parts.py — 生成 Ameba 鸭子新增的 3D 打印件 (原版 Open Duck Mini v2 的 36 个 STL 不变, 照常打印)。

生成:
  amb82_tray.stl         主控托架: 底部 4 孔对齐原版 Pi Zero 2W 的 58x23mm 安装孔(M2.5), 直接装在原 Pi 位置;
                         上部 50x70mm 定位槽 + 4 个 M3 柱(孔距 43x63mm), 放洞洞板或自制载板 PCB
  stand_foot_tee.stl x2  悬挂调试支架: 底座 T 型接头 (横穿一根 PVC 管当脚, 竖插立柱)
  stand_top_elbow.stl x2 悬挂调试支架: 顶部弯头 (立柱 -> 横梁), 横梁中间挂绑带吊住鸭子身体

依赖: pip install manifold3d trimesh numpy
用法: python gen_parts.py [--pipe-od 20.0] [--out .]
所有尺寸单位 mm, 参数集中在 PARAMS, 打印前请用卡尺核对 PVC 管外径和 AMB82-mini 实物。
"""
import argparse
import os

import numpy as np
import trimesh
from manifold3d import Manifold

PARAMS = dict(
    # 原版 Pi Zero 2W 安装孔 (官方机械图: 板 65x30, 孔距 58x23, M2.5)
    pi_hole_dx=58.0, pi_hole_dy=23.0, pi_hole_d=2.8,
    # 载板 / 洞洞板
    pcb_w=50.0, pcb_l=70.0, pcb_hole_dx=43.0, pcb_hole_dy=63.0, pcb_screw_pilot=2.6,  # M3 自攻
    tray_t=2.5, tray_margin=3.0, pocket_depth=1.2, boss_h=5.0, boss_d=6.5,
    # PVC 管 (国标电工穿线管 dn20 外径 20mm; 用 dn16/dn25 时改这里)
    pipe_od=20.0, pipe_clear=0.4, wall=3.2, socket_depth=32.0,
)
SEG = 64


def cyl(h, d, center=False):
    return Manifold.cylinder(h, d / 2.0, d / 2.0, SEG, center)


def box(x, y, z, center=False):
    return Manifold.cube([x, y, z], center)


def to_trimesh(m: Manifold) -> trimesh.Trimesh:
    mesh = m.to_mesh()
    v = np.asarray(mesh.vert_properties)[:, :3]
    f = np.asarray(mesh.tri_verts)
    return trimesh.Trimesh(vertices=v, faces=f, process=True)


def amb82_tray(p):
    w = p["pcb_w"] + 2 * p["tray_margin"]
    l = p["pcb_l"] + 2 * p["tray_margin"]
    t = p["tray_t"]
    base = box(w, l, t + p["pocket_depth"], center=False).translate([-w / 2, -l / 2, 0])
    pocket = box(p["pcb_w"] + 0.6, p["pcb_l"] + 0.6, p["pocket_depth"] + 1, center=False).translate(
        [-(p["pcb_w"] + 0.6) / 2, -(p["pcb_l"] + 0.6) / 2, t])
    part = base - pocket
    # 载板支撑柱 + 自攻孔
    for sx in (-1, 1):
        for sy in (-1, 1):
            x, y = sx * p["pcb_hole_dx"] / 2, sy * p["pcb_hole_dy"] / 2
            boss = cyl(p["boss_h"], p["boss_d"]).translate([x, y, t])
            part = part + boss
            part = part - cyl(t + p["boss_h"] + 2, p["pcb_screw_pilot"]).translate([x, y, -1])
    # 底部 Pi Zero 安装孔 (旋转 90°, 让 58mm 孔距沿托架长边)
    for sx in (-1, 1):
        for sy in (-1, 1):
            x, y = sx * p["pi_hole_dy"] / 2, sy * p["pi_hole_dx"] / 2
            part = part - cyl(t + 5, p["pi_hole_d"]).translate([x, y, -1])
    # 走线/散热窗口 (避开柱子和 Pi 孔)
    part = part - box(20, 36, t + 5, center=False).translate([-10, -18, -1])
    return part


def foot_tee(p):
    d_in = p["pipe_od"] + p["pipe_clear"]
    d_out = d_in + 2 * p["wall"]
    length = 50.0
    # 横向套管 (沿 x), 底面削平便于打印和放稳
    horiz = cyl(length, d_out, center=True).rotate([0, 90, 0]).translate([0, 0, d_out / 2])
    flat_cut = box(length + 2, d_out + 2, 2, center=False).translate([-(length + 2) / 2, -(d_out + 2) / 2, -2 + 1.0])
    vert = cyl(p["socket_depth"] + d_out / 2, d_out).translate([0, 0, d_out / 2])
    part = (horiz + vert) - flat_cut
    part = part - cyl(length + 2, d_in, center=True).rotate([0, 90, 0]).translate([0, 0, d_out / 2])
    part = part - cyl(p["socket_depth"] + 1, d_in).translate([0, 0, d_out + 0.5])
    return part.translate([0, 0, -1.0])


def top_elbow(p):
    d_in = p["pipe_od"] + p["pipe_clear"]
    d_out = d_in + 2 * p["wall"]
    s = p["socket_depth"]
    hub = box(d_out, d_out, d_out, center=True)
    down = cyl(s, d_out).translate([0, 0, -s - d_out / 2 + 0.01])           # 向下插立柱
    side = cyl(s, d_out).rotate([0, 90, 0]).translate([d_out / 2 - 0.01, 0, 0])  # 向 +x 插横梁
    part = hub + down + side
    part = part - cyl(s + 1, d_in).translate([0, 0, -s - d_out / 2 - 0.5])
    part = part - cyl(s + 1, d_in).rotate([0, 90, 0]).translate([d_out / 2 - 0.5 + 0.01, 0, 0])
    # 打印时把 -z 立柱口朝下放置即可; 平移到 z>=0
    return part.translate([0, 0, s + d_out / 2])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.dirname(os.path.abspath(__file__)))
    ap.add_argument("--pipe-od", type=float, default=PARAMS["pipe_od"])
    args = ap.parse_args()
    p = dict(PARAMS, pipe_od=args.pipe_od)

    parts = {
        "amb82_tray.stl": amb82_tray(p),
        "stand_foot_tee.stl": foot_tee(p),
        "stand_top_elbow.stl": top_elbow(p),
    }
    for name, m in parts.items():
        tm = to_trimesh(m)
        path = os.path.join(args.out, name)
        tm.export(path)
        e = tm.bounds[1] - tm.bounds[0]
        print(f"{name:22s} {e[0]:6.1f} x {e[1]:6.1f} x {e[2]:6.1f} mm  "
              f"vol {tm.volume / 1000:5.1f} cm3  watertight={tm.is_watertight}")


if __name__ == "__main__":
    main()
