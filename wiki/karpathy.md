---
title: karpathy
aliases: [卡帕西, Andrej Karpathy, LLM-Wiki]
type: 概念
tags: [知识管理, LLM-Wiki, AI]
updated: 2026-09-16
---

# karpathy（Andrej Karpathy）/ LLM-Wiki 范式

**Andrej Karpathy**：前 OpenAI 创始成员、前 Tesla AI 总监，深度学习与 LLM 应用领域的知名布道者。

在知识管理语境下，其提出的 **LLM-Wiki** 范式是本 vault（`AI赋能`）的方法论源头：

> 一个**持续累积、逐步精炼**的本地知识库——
> **人**负责选源与提问；**LLM**负责写页、维护交叉引用、记流水账。

## 核心特征

- **持续累积**：原始源不断投喂，永不丢弃（对应 `raw/`）。
- **逐步精炼**：LLM 定期把杂乱原料整合进已有页，而非碎片化堆积（对应 `wiki/`）。
- **人机分工**：人给方向（选源 + 提问），LLM 做苦力（结构化 + 交叉引用 + 流水账）。

## 与本 vault 的关系

本知识库的三层结构（`raw/` → `wiki/` → `CLAUDE.md` schema）即该范式的落地。
参见 [[自生长LLM Wiki 方法论]] 的 A/B/C/D 四层闭环。

## 📚 延伸阅读

- [Karpathy LLM-Wiki gist](https://gist.github.com/karpathy/442a6bf555914893e9891c11519de94f) — 范式原始说明：LLM 维护的持续精炼知识库，人选源提问、机器维护页与链接。

---

参考：[[自生长LLM Wiki 方法论]] · [[Obsidian]]
