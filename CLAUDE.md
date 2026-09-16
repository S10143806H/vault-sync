# AI赋能 Wiki — Schema / 维护约定

> 本目录是一个 **LLM-Wiki**（karpathy 模式）：一个持续累积、逐步精炼的知识库。
> 人负责选源与提问；LLM 负责写页、维护交叉引用、记流水账。参考：<https://gist.github.com/karpathy/442a6bf555914893e9891c11519de94f>

## 三层结构

| 层 | 路径 | 谁写 | 说明 |
|---|---|---|---|
| **原始源** | `raw/` | 人 | 文章/PDF/图/日志导出等**不可变**源。LLM 只读不改，单一事实来源。 |
| **Wiki** | `wiki/` | LLM | 概念页、实体页、综述、用例笔记。人读，LLM 维护。 |
| **Schema** | `CLAUDE.md`（本文件） | 人+LLM 共同演进 | 结构与约定。 |

## wiki/ 内部

- `wiki/index.md` — **全站目录**，按域分类。每页一行 = `[[页名]]` + **一句话摘要**（可选元数据：日期/源数）。新增页必须登记。
- `wiki/log.md` — **只追加**流水账（ingest / query / lint）。行首统一 `[YYYY-MM-DD]`，故可 `grep "^\[" wiki/log.md | tail -5` 取最近记录。
- 按域编号子目录（现状，保留）：
  - `000-GenerativeAI` 生成式 AI 基础
  - `001-Agent` Agent / 编码助手
  - `002-MCP` MCP 工具
  - `003-Skills` 技能
  - `004-AAOS` AAOS 图形栈 / 跨SoC（显示知识库）
  - `005-稳定性` 稳定性测试（用例·故障·BUG）
  - `006-屏幕异常算法` 屏幕异常检测
  - `007-速查` cheatsheet

## 命名约定

- 页文件名 = 主题名（Obsidian `[[链接]]` 按文件名解析，**跨目录不断链**）。
- 交叉引用一律用 `[[页名]]`；不确定目标页是否存在也先写 `[[]]`，标记待补。
- BUG 页：`BUG-<组件>-<现象>.md`；用例页：`TC_<模块>_<类型>_<编号>.md`。
- log 行格式：`[YYYY-MM-DD] <ingest|query|lint> | <一句话>`。
- **Frontmatter**：每页顶部带 YAML frontmatter，供 Obsidian Dataview 检索。基础字段：`title`、`tags`、`created`/`updated`（YYYY-MM-DD）、`source`（来源，实体/概念页可选）。
- **图片本地化**：源图存 `raw/assets/`，wiki 用 `![[图名]]` 引用。LLM 读带图页**两步走**——先读正文文本，再按需单独查看引用的图片。

## 工作流

**Ingest（摄入）**：新源放 `raw/` → LLM 读 → 抽要点 → **整合进已有页**（而非只建索引）→ 更新相关页的 `[[链接]]` → 登记 `index.md` → 追加 `log.md`。矛盾要显式标注。

**Query（查询）**：先搜相关页 → 综合作答并给出处 → 好的综述回填成新 wiki 页。输出形态按需选择：markdown 页 / 对比表格 / Marp 幻灯片 / matplotlib 图 / canvas。

**Lint（体检）**：定期查 孤儿页 / 过期声明 / 矛盾 / 缺失交叉引用 / **数据缺口**（可 web 搜索补全）；主动**建议下一步该读的源与该问的问题**；结果记 `log.md`。

## 约束

- `raw/` 永不被 LLM 修改。
- 每次动 wiki 都要：① 更 `index.md`（若增删页）② 追加 `log.md`。
- 新页优先**整合进已有页**，避免碎片化。
