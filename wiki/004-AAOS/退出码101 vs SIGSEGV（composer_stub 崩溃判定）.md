---
title: "退出码101 vs SIGSEGV（composer_stub 崩溃判定）"
tags:
  - AAOS
  - 稳定性
  - 跨SoC
  - 崩溃
  - GFWK
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# 退出码101 vs SIGSEGV（composer_stub 崩溃判定）

**一句话**：判 A720 `composer_stub` 是**健康自恢复**还是**真崩溃**——两者日志完全不同，别混为一谈（analyze-bot 误报的根源）。

## 两种情形对照

| | 优雅退出（健康，非缺陷） | 真崩溃（缺陷） |
|---|---|---|
| 日志特征 | `composer_stub exit with code: 101`<br/>`init: Service 'composer_stub' exited with status 0`<br/>`init: Sending signal 9 to service ...`（前有 `GipcHalChannelTransport::abortReceive`） | `signal 11 ([[SIGSEGV]])` / [[tombstone（墓碑 验尸报告）\|tombstone]] / `wl_proxy_get_version` |
| 触发 | kill IVI `cluster-service` / 断 [[GIPC]]（数据面通道没了） | kill IVI **HWC**（合成器死拖崩 A720 Wayland 客户端） |
| 恢复 | init/**GWDT 看门狗**立即重启（`max_restart=3`，未耗尽=健康） | 进程崩溃、产 tombstone |
| 结论 | **非缺陷**（KB **G5**） | **产品缺陷/跨SoC耦合**（KB **G1**，见 [[BUG-kill-HWC-crashes-A720-composer_stub]]） |

## exit code 101 是什么
- **应用自定义退出码**（非标准 POSIX），composer_stub 检测到 GIPC 接收被中止（`abortReceive`，即 IVI 侧通道断）时**主动退出**，交给 init/GWDT 重启。
- 属**设计内的自恢复**：断链→干净退出→拉起新实例重连。20 次 kill = 20 次 101 退出 + 20 次重启，全程健康。

## GWDT（Gua WatchDog）
- `gwdt: AIDL register app=composer_stub timeout_ms=10000 max_restart=3 ...`
- 看门狗监控心跳；进程异常退出会重启，**`max_restart` 次内**属正常；**耗尽放弃**才是问题。

## 检测要点（给测试/分析用）
- HWC_FAULT_004 计数器只数 `composer_stub.*(segfault|fatal signal|SIGSEGV|trap)` → **不会**误报 101 退出 ✅
- analyze-bot `monitor_rules` 的 `signal [0-9]+` 会**误命中** init 的 `Sending signal 9` → 靠 KB **G5** 排除项让 GLM 正确归类为"健康自恢复"

## 关联
- 崩溃信号 → [[SIGSEGV]]｜主动 kill → [[SIGKILL （kill -9）]]
- 通道 → [[GIPC]]｜[[SHMEM]]｜合成器 → [[composer_stub]]｜[[Weston]]
- 缺陷 → [[BUG-kill-HWC-crashes-A720-composer_stub]]｜闭环 → [[GFWK 双bots自动挖bug闭环]]

## 📚 延伸阅读
- Android init service（oneshot/restart/信号）：https://android.googlesource.com/platform/system/core/+/master/init/README.md
- 进程退出码约定：https://tldp.org/LDP/abs/html/exitcodes.html
