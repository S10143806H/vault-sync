---
title: "SIGSEGV（段错误 / signal 11）"
tags:
  - AAOS
  - 崩溃
  - 调试
  - Linux
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# SIGSEGV（段错误 / signal 11）

**一句话**：进程访问了**非法内存**（空指针、越界、已释放/未映射地址），内核发 `signal 11 (SIGSEGV)` 强制终止它，并（Android/Linux）生成 [[tombstone（墓碑 验尸报告）|tombstone]]/coredump。

## 为什么关心
它是**真崩溃**的硬证据——与"[[退出码101 vs SIGSEGV（composer_stub 崩溃判定）|优雅退出(exit 101)]]"截然不同。本项目里 **kill IVI HWC → A720 `composer_stub` SIGSEGV** 就是靠它坐实的跨SoC缺陷（[[BUG-kill-HWC-crashes-A720-composer_stub]]）。

## 常见诱因
| 诱因 | 例子 |
|---|---|
| 空指针解引用 | `*NULL` |
| **use-after-free（UAF）** | 持已释放的 [[Fence\|fence]] 还 `ppoll`（对应 fence UAF 缺陷） |
| 越界 / 栈溢出 | 数组越界、深递归 |
| 访问已 unmap 内存 | 库卸载后仍调用（如 `wl_proxy_get_version` @ libwayland-client） |

## 怎么读 tombstone
```
signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x...
backtrace:
  #00 pc ...  /system/lib64/libwayland-client.so (wl_proxy_get_version+..)
```
> `fault addr` = 非法地址；`backtrace` 顶帧 = 崩溃点。本项目顶帧常见 `wl_proxy_get_version`（IVI 合成器死后 A720 Wayland 客户端仍访问失效代理）。

## 关联
- 判定 → [[退出码101 vs SIGSEGV（composer_stub 崩溃判定）]]
- 现场 → [[tombstone（墓碑 验尸报告）]]｜[[RAMdump]]
- 案例 → [[BUG-kill-HWC-crashes-A720-composer_stub]]｜[[composer_stub]]
- 相关 → [[Fence]]（fence UAF）｜[[SIGKILL （kill -9）]]（主动 kill≠SIGSEGV）

## 📚 延伸阅读
- `man 7 signal`（信号语义）：https://man7.org/linux/man-pages/man7/signal.7.html
- Android debuggerd / tombstone：https://source.android.com/docs/core/tests/debug
