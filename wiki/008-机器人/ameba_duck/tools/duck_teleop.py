#!/usr/bin/env python3
"""
duck_teleop.py — 电脑遥控 Ameba 鸭子 (Wi-Fi UDP)。只用 Python 自带 tkinter, 手柄可选(pygame)。

用法:
    python duck_teleop.py --ip 192.168.x.x          # IP 看固件开机串口打印的 [wifi] IP
键位(窗口获得焦点时):
    W/S 前进/后退   A/D 左/右平移   Q/E 左/右转   Shift 按住冲刺
    1 = stand   2 = walk   空格 = pause/resume   0 = off(卸力)
手柄(装了 pygame 且插着手柄时自动启用, 映射同官方 xbox_controller.py):
    左摇杆 = 平移, 右摇杆 X = 转向, A = 暂停/恢复, Start = walk, Back = off
协议: 每 50ms 发一行文本到 UDP 8888, 例如 "c 0.150 0.000 0.000"; 停发 0.5s 固件自动把速度清零。
"""
import argparse
import socket
import tkinter as tk

VX, VY, WZ = 0.15, 0.20, 1.0  # 与固件/训练范围一致


class Teleop:
    def __init__(self, ip, port):
        self.addr = (ip, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.keys = set()
        self.paused = False
        self.joy = None
        try:
            import pygame
            pygame.init()
            pygame.joystick.init()
            if pygame.joystick.get_count() > 0:
                self.pg = pygame
                self.joy = pygame.joystick.Joystick(0)
                self.joy.init()
        except Exception:
            self.joy = None

        self.root = tk.Tk()
        self.root.title(f"Ameba Duck -> {ip}:{port}")
        self.label = tk.Label(self.root, font=("Consolas", 14), justify="left",
                              text="点击窗口后用键盘控制\nW/S A/D Q/E, Shift 冲刺\n1 stand 2 walk 空格 pause 0 off")
        self.label.pack(padx=20, pady=10)
        self.status = tk.Label(self.root, font=("Consolas", 12), fg="#0a5")
        self.status.pack(padx=20, pady=10)
        for txt, cmd in (("stand", "stand"), ("walk", "walk"), ("pause/resume", None), ("OFF 卸力", "off")):
            tk.Button(self.root, text=txt, width=14,
                      command=(lambda c=cmd: self.send(c) if c else self.toggle_pause())).pack(pady=2)
        self.root.bind("<KeyPress>", self.on_down)
        self.root.bind("<KeyRelease>", self.on_up)
        self.sprint = False
        self.tick()

    def send(self, line):
        self.sock.sendto((line + "\n").encode(), self.addr)

    def toggle_pause(self):
        self.paused = not self.paused
        self.send("pause" if self.paused else "resume")

    def on_down(self, e):
        k = e.keysym.lower()
        if k in ("shift_l", "shift_r"):
            if not self.sprint:
                self.sprint = True
                self.send("sprint 1")
        elif k == "1":
            self.send("stand")
        elif k == "2":
            self.send("walk")
        elif k == "0":
            self.send("off")
        elif k == "space":
            self.toggle_pause()
        else:
            self.keys.add(k)

    def on_up(self, e):
        k = e.keysym.lower()
        if k in ("shift_l", "shift_r"):
            self.sprint = False
            self.send("sprint 0")
        self.keys.discard(k)

    def read_command(self):
        if self.joy is not None:
            self.pg.event.pump()
            dz = lambda v: 0.0 if abs(v) < 0.1 else v  # noqa: E731
            vx = -dz(self.joy.get_axis(1)) * VX
            vy = -dz(self.joy.get_axis(0)) * VY
            wz = -dz(self.joy.get_axis(2)) * WZ
            if self.joy.get_button(0):  # A
                self.toggle_pause()
                self.root.after(300)
            if self.joy.get_numbuttons() > 7 and self.joy.get_button(7):  # Start
                self.send("walk")
            if self.joy.get_numbuttons() > 6 and self.joy.get_button(6):  # Back
                self.send("off")
            if vx or vy or wz:
                return vx, vy, wz
        k = self.keys
        vx = (VX if ("w" in k or "up" in k) else 0) - (VX if ("s" in k or "down" in k) else 0)
        vy = (VY if "a" in k else 0) - (VY if "d" in k else 0)
        wz = (WZ if ("q" in k or "left" in k) else 0) - (WZ if ("e" in k or "right" in k) else 0)
        return vx, vy, wz

    def tick(self):
        vx, vy, wz = self.read_command()
        self.send(f"c {vx:.3f} {vy:.3f} {wz:.3f}")
        self.status.config(text=f"vx={vx:+.2f} m/s  vy={vy:+.2f} m/s  wz={wz:+.2f} rad/s"
                                + ("  [冲刺]" if self.sprint else "") + ("  [暂停]" if self.paused else ""))
        self.root.after(50, self.tick)  # 20 Hz, 与官方手柄线程频率一致

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--ip", required=True)
    ap.add_argument("--port", type=int, default=8888)
    a = ap.parse_args()
    Teleop(a.ip, a.port).run()
