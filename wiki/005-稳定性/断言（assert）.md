---
title: 断言（assert）
tags:
  - 稳定性
  - AAOS
  - 断言
  - 测试用例
  - assert
platform: "gua / guav100 (AAOS)"
created: 2026-07-28
---

断言 = 用例里的"质检卡点"：我断定某条件必须成立，不成立就判用例失败并打印原因。

assert not recover_fails, "SF 未在 5s 内恢复"
#      └─必须为真的条件      └─不成立时打印这句并 FAIL

类比：流水线质检员卡一道关——"这个尺寸必须 <5mm，否则打回"。一条用例 = 一串断言：全过=PASS，任一不成立=FAIL。
这次 3 条断言（恢复、画面、fatal）里前两条阈值定得不准，所以误报了 FAIL——不是 SF 有问题。