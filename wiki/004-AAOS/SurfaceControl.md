---
title: SurfaceControl
tags:
  - AAOS
  - SurfaceFlinger
  - SurfaceControl
  - 图层
  - 多屏
platform: gua / guav100 (AAOS)
created: 2026-08-06
---

# SurfaceControl

## 一句话
[[SurfaceFlinger]] 图层树上**一个节点（一层 Surface）的句柄**——持有它就能绕过 App 的 View/Window 体系，**直接对某一层设层级(z)、位置、缩放、alpha、可见性、挂哪块屏**，改动打包进一个 `Transaction` 原子提交给 SF。

## 为什么需要它
普通 App 画东西走 `View → ViewRootImpl → Surface`，图层由 [[WMS]]/[[SurfaceFlinger]] 托管：**挂哪块屏、z-order、位置全由系统定**，App 说了不算。`SurfaceControl` 是系统/特权侧的**低层 API**，把「造一层、摆哪、多高、多透、上哪块屏」的控制权交到调用者手里。系统用它实现：
- **SystemUI/WindowManager** 本身对每个窗口的图层编排
- **多屏投送 / 画中画 / 屏幕镜像**（把一层送到别的 display）
- **测试/压测工具**（如 `MultiDisplayJavaDemo`）人造极端图层负载压 SF/HWC

## 核心概念

| 概念 | 说明 |
|---|---|
| **SurfaceControl** | 一层的句柄；`SurfaceControl.Builder().setName().setBufferSize().build()` 造出 |
| **Transaction** | 一批图层改动的原子提交：`new SurfaceControl.Transaction().setLayer(sc,z).setPosition(sc,x,y).setAlpha(sc,a).setVisibility(sc,true).apply()` |
| **reparent / setDisplay** | 把层挂到某父层或某块 display（跨屏搬运的底层） |
| **buffer 来源** | 层的像素从哪来：EGL(GPU 渲染)/CPU 填 / 真实 `BufferQueue`。填好 buffer 再 `setBuffer` |
| **生命周期** | 句柄释放 / 客户端进程死亡 → SF 回收该层。**没显式 release 又不死进程 → 层泄漏** |

## 与 SurfaceFlinger / Window 的关系
```
App View 树 ──ViewRootImpl──▶ Surface ─┐
                                        ├─▶ [[SurfaceFlinger]] 图层树 ─▶ 合成一帧 ─▶ [[HWC]] ─▶ 上屏
特权侧 SurfaceControl ─Transaction──────┘
```
- 普通路径：图层归 [[WMS]] 管，App 只填自己 Surface 的内容
- SurfaceControl 路径：**直接在 SF 图层树上挂节点**，自定 z/位置/屏——这是 `MultiDisplayJavaDemo` 的能力来源

## 类比
SF 是一块**多层玻璃叠画的灯箱**，每块玻璃 = 一层。普通 App 只能在物业([[WMS]])分给它那格玻璃上画；`SurfaceControl` 是**物业钥匙**：能自己往灯箱插玻璃、抽玻璃、调位置/透明度、甚至插到隔壁灯箱（跨屏）。`Transaction` = 把这一批操作**攒好一次性推进灯箱**（原子生效，避免中间态闪烁）。

## 最小示例（Java，特权/系统侧）
```java
SurfaceControl sc = new SurfaceControl.Builder()
        .setName("test-layer")
        .setBufferSize(400, 300)
        .build();
// ... 用 EGL/Canvas 往 sc 的 buffer 填色 ...
new SurfaceControl.Transaction()
        .setLayer(sc, 10)            // z-order
        .setPosition(sc, 100, 100)   // 屏内坐标
        .setAlpha(sc, 0.5f)          // 半透明 → 触发 SF alpha blending
        .setVisibility(sc, true)
        .apply();                    // 原子提交给 SurfaceFlinger
```
> `MultiDisplayJavaDemo` 的 `add/move/mirror/wander/rotate/scale` 命令，内部就是不同的 `SurfaceControl` + `Transaction` 组合。

## 常见误区
1. **`SurfaceControl` = Surface** ✗ ——`Surface` 是往里画像素的**生产端**；`SurfaceControl` 是控制这层**属性/在树上位置**的句柄，两者配套但不同。
2. **改一个属性就 `apply()` 一次** ——多个改动应攒进**同一个 Transaction 一次 apply**，否则出中间态、掉帧、且更压 SF。
3. **不 release 也不死进程** → 层不回收 → **图层泄漏**，正是稳定性要挖的（`clear` 清不掉的 mirror 层就靠 `force-stop` 让进程死来兜底回收）。

## 稳定性关联
- 驱动它做压测：[[TC_GFWK_STRESS_006]]（多屏图层随机增删，工具 `MultiDisplayJavaDemo.apk`）
- 合成引擎：[[SurfaceFlinger]] · 上屏：[[HWC]] · 跨屏投送走 [[跨SoC]] 通路
- 上级地图：[[000-GFWK图形框架总览]]

## 📚 延伸阅读
- SurfaceControl API：https://developer.android.com/reference/android/view/SurfaceControl
- SurfaceControl.Transaction：https://developer.android.com/reference/android/view/SurfaceControl.Transaction
- 图形架构（Surface/SurfaceFlinger）：https://source.android.com/docs/core/graphics/arch-sf-hwc
