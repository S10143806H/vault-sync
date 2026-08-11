---
title: "TC_GRAL_FAULT_001 — dma-heap OOM"
tags:
  - 稳定性
  - AAOS
  - 测试用例
  - GFWK
  - Gralloc
  - dma-buf
platform: "gua / guav100 (AAOS)"
优先级: P1
维度: 故障注入(D3)
状态: smoke 通过(2026-08-04, SG0286, 任务95539, 1轮不误报)
created: 2026-08-03
---

# TC_GRAL_FAULT_001 — dma-heap OOM

> 上级 [[GFWK 稳定性测试用例全量清单]] · 栈层 [[Gralloc]] / [[dma-buf heap]] · 方法 [[GFWK kill-恢复类测试]]
> 代码 `autocase/cases/MultiMedia/GPU/Gralloc/TC_GRAL_FAULT_001.py`（branch `qi.zhu`）

## 目标
把图形内存池 [[dma-buf heap]] 压到枯竭，验图形栈**优雅降级不崩**：分配失败应返回错误码而非崩溃/冻屏。这是全量清单 §3.2 的**第 1 号待补**（Gralloc 历史 gap 最多、`❤️❤️` 易出 bug）。

## 四阶段
| 阶段 | 动作 |
|---|---|
| ① 基线 | 记 [[Gralloc]] allocator pid、[[SurfaceFlinger]] pid（旁路，验不被 LMK 误杀）、墓碑集、跨 SoC 崩溃计数、`MemAvailable`、多屏 display id |
| ② 注入 | 把 dma-heap / 整机内存压到枯竭，持续 `GRAL_F001_OOM_SEC`（默认 8s） |
| ③ 恢复判定 | `RECOVER_SLA`（默认 20s）内 allocator + SF 两进程均存活 |
| ④ 三态断言 | 不掉线/不进 ramdump；关键进程未被 OOM-killer 杀且 SLA 内在；逐屏（含后排 VHAL 唤醒）非黑；无 double-free 墓碑；跨 SoC `composer_stub` 无新崩溃 |

## 注入通道（可插拔，均纯 adb，平台无 ioctl 工具时逐级降级）
1. `GRAL_OOM_CMD` 指定的设备内命令（最精确，如自带 `dmabuf_alloc`/`ion_test`）
2. 自动发现 `/data/local/tmp/{dmabuf_stress,dmabuf_alloc,ion_test}`
3. `stress-ng --vm`（整机内存压力 → LMK → 逼图形重分配）
4. 兜底 `am` 反复拉大 SurfaceView Activity churn gralloc 分配

> dma-heap 真分配需 `DMA_HEAP_IOCTL_ALLOC`（ioctl），纯 shell 不能直接调；故最精确路径是**设备内放一个 allocator 小工具**再用通道 1/2。无工具时通道 3/4 也能有效施压恢复路径。

## 关键 env
| env | 默认 | 说明 |
|---|---|---|
| `GRAL_F001_ROUNDS` | 10 | 轮次；长稳调大 |
| `GRAL_F001_OOM_SEC` | 8 | 每轮 OOM 施压秒数 |
| `GRAL_F001_SLA` | 20 | 进程恢复 SLA |
| `GRAL_OOM_CMD` | — | 覆盖注入命令（有设备内 allocator 时首选） |
| `GRALLOC_PROC` / `SF_PROC` | 自动发现 | 覆盖进程名 |

## 运行
```bash
GRAL_F001_ROUNDS=1 pytest cases/MultiMedia/GPU/Gralloc/TC_GRAL_FAULT_001.py --bench=<yaml> -v
```

## 预期挖出
- 大 buffer 分配失败时崩溃而非返回错误 → allocator 墓碑
- OOM 后冻屏/黑屏（旧帧留存、新帧分配不到）
- SF/allocator 被 OOM-killer 误杀（进程隔离失效）
- 跨 SoC 侧被内存压力拖崩

## 📚 延伸阅读
- Android Graphics / BufferQueue：https://source.android.com/docs/core/graphics
- Gralloc HAL：https://source.android.com/docs/core/graphics/arch-bq-gralloc
- Kernel DMA-BUF Heaps：https://docs.kernel.org/userspace-api/dma-buf-alloc-exchange.html
- Low Memory Killer（LMKD）：https://source.android.com/docs/core/perf/lmkd

## 关联
- 栈层概念 [[Gralloc]] · [[dma-buf heap]] · [[000-GFWK图形框架总览]]
- 映射方法 [[06-GFWK如何映射到测试用例]]
- 同层用例 `TC_GRAL_FAULT_002`（坏 dma-buf fd）· `TC_GRAL_LEAK_001`（百万次 alloc/free）
