# 0001 — First session: 指标三部件模型

**Date**: 2026-09-03
**Mission tie-in**: 把性能数据接进 kill SF 用例。

## 教了什么
- 指标 = **数据源 → 度量 → 阈值判定** 三部件(Lesson 01)
- 用学员自己的 `screen_not_black` 做拆解锚点
- 判据指标 vs 监控指标 的区别 → 解释了"性能数据先做软监控"的道理

## 已知起点(ZPD)
- 学员是 autocase 用例作者,已实现 `image_bright_ratio` 判据(有真实工程直觉)
- 缺的是"指标"的抽象词汇 → 用三部件模型给已有实践命名
- 会写 Python、adb、PromQL;不缺语法,缺的是概念框架

## 下一课候选
1. **动手写 `perf_snapshot(device, metrics, window)`** helper —— 接一个真实监控指标(首选)
2. gauge vs counter,以及各自阈值写法
3. 怎么给性能指标定基线/阈值(需先积累几轮数据)

## 备注
- 教学工作区随本次请求从 `autocase/docs/` 迁至 Obsidian vault(`005-稳定性/teach-metrics/`)
