# OpenAI Codex 使用

> 上海辅论教育科技有限公司 · 5月7日修改
> 来源：飞书云文档 https://kwz55xptfhg.feishu.cn/wiki/CizCwUjCliYbqskDwkGcl1MHn9c

## AI 速览

本文讨论了 OpenAI 推出的 AI 编程工具 Codex 的全维度使用指南，涵盖产品特性、安装配置、操作方法、生态扩展、竞品对比、团队协作、成本管控等内容，面向开发者、架构师等相关人群提供落地指引。关键要点包括：

1. **核心特性与形态**：Codex 基于 Rust 开发、Apache 2.0 开源、支持跨平台本地运行，分为 CLI、IDE 插件、桌面版 App 三类形态，支持 MCP 扩展、多智能体协同，核心优势是高自定义性、运行速度快。
2. **订阅与成本规则**：分为 Free（0 美元/月，有限额度）、Plus（20 美元/月，含 5 美元 Codex 额度）、Pro（200 美元/月，含 50 美元 Codex 额度）等档位，日常开发推荐用 o4-mini 模型，可降低 90% 成本。
3. **核心操作能力**：提供 suggest/auto-edit/full-auto 三种操作模式，支持自定义斜杠命令、AGENTS.md 项目规则配置、MCP 外部服务集成，可实现工作流自动化。
4. **竞品选型参考**：和 Claude Code、Cursor、Copilot 等工具对比，Codex 适合追求精细控制的极客/架构师，也可搭配其他工具组合使用，比如 Codex CLI + Cursor 是常用的高效组合。
5. **安全与团队方案**：付费用户的代码不会被用于训练，支持配置权限、敏感文件防护；团队可共享 Skills 规范、集成 CI/CD 实现自动化代码审查，降低协作成本。

👥 **适用人群**：开发者、架构师、团队负责人、编程爱好者
🛤️ **学习路径**：安装配置 → 基础使用 → 高级功能 → 团队协作 → 安全合规 → 实战案例

---

## 第一章：Codex 简介

### 1.1 什么是 Codex？

Codex 是 OpenAI 推出的 AI 编程助手，将 GPT 级别的推理能力与本地代码执行能力结合，让开发者用自然语言即可读取、修改、执行代码。

#### 核心特点

| 特性 | 说明 |
| --- | --- |
| 🏠 本地运行 | 直接在电脑上运行，代码不离开本机 |
| 🔓 开源免费 | Apache 2.0 许可，Rust 构建 |
| 🌍 跨平台 | macOS / Linux / Windows |
| 🔌 MCP 支持 | 连接 Model Context Protocol 扩展能力 |
| ⚡ 多种模式 | suggest / auto-edit / full-auto |
| 🧠 多智能体 | 并行运行多个 Agent 协同工作 |

#### Codex 的三种形态

**① Codex CLI（终端版）**

```bash
codex "帮我重构这个函数"
```

| 项目 | 说明 |
| --- | --- |
| 特点 | 轻量、快速、高度可配置 |
| 适用 | 终端用户、CI/CD 集成 |
| 技术 | Rust 编写，性能极佳 |

**② Codex IDE 插件**

集成于 IDE 中使用。

**③ Codex App（桌面版）**

图形界面，支持多智能体与自动化（详见第九章）。

#### Codex vs Claude Code 对比

| 维度 | 🟦 OpenAI Codex | 🟪 Anthropic Claude Code |
| --- | --- | --- |
| 底层技术 | Rust（速度极快） | Node.js（生态兼容好） |
| 开源 | ✅ Apache 2.0 | ❌ 闭源 |
| 核心哲学 | 精细控制、自定义 | 深度推理、自主思考 |
| 自定义扩展 | ⭐⭐⭐ 自定义 Slash 命令 | ⭐ CLAUDE.md |
| 记忆文件 | AGENTS.md | CLAUDE.md |
| 适合人群 | 极客 / 架构师 | 业务开发者 |

> 💡 **提示**：两者并不冲突，很多开发者同时使用 Codex CLI + Claude Code，各取所长。

---

## 第二章：安装与配置

### 系统要求

| 要求 | 最低 | 推荐 |
| --- | --- | --- |
| 操作系统 | macOS 12+ / Ubuntu 20.04+ / Win 11 | 最新 LTS 版本 |
| Node.js | v22+ | v22 LTS |
| Git | 2.23+ | 最新版 |
| 内存 | 4 GB | 8 GB+ |

### Codex CLI 安装

**npm 全局安装（推荐）：**

```bash
npm install -g @openai/codex
```

**Homebrew Cask：**

```bash
brew install --cask codex
```

**二进制文件：** 前往 GitHub Releases 下载：

| 平台 | 文件名 |
| --- | --- |
| macOS (Apple Silicon) | codex-aarch64-apple-darwin.tar.gz |
| macOS (Intel) | codex-x86_64-apple-darwin.tar.gz |
| Linux (x86_64) | codex-x86_64-unknown-linux-musl.tar.gz |
| Linux (arm64) | codex-aarch64-unknown-linux-musl.tar.gz |

**验证安装：**

```bash
codex --version
```

### Codex App 桌面端安装

**macOS 安装**

官网下载（推荐）：
1. 访问 https://chatgpt.com/codex
2. 点击 "Download for Mac" 下载 `.dmg` 文件

Homebrew Cask：

```bash
brew install --cask openai-codex
```

**Windows 安装**

官网下载（推荐）：
1. 访问 https://chatgpt.com/codex
2. 点击 "Download for Windows" 下载 `.exe` 安装包
3. 双击运行安装程序，按向导完成安装

winget 命令安装：

```bash
winget install OpenAI.Codex
```

Microsoft Store：
1. 打开 Microsoft Store
2. 搜索 "OpenAI Codex"
3. 点击安装

### 认证配置

**方式一：ChatGPT 账号登录（推荐）**

运行 `codex` 后选择 Sign in with ChatGPT：

| 计划 | Codex 支持 | 额度 |
| --- | --- | --- |
| ChatGPT Free | ✅ | 有限 |
| ChatGPT Plus | ✅ | $5 / 30天 |
| ChatGPT Pro | ✅ | $50 / 30天 |
| ChatGPT Team | ✅ | 按成员分配 |

**方式二：API Key 配置**

```bash
# — Linux / macOS —
# 永久
echo 'export OPENAI_API_KEY="sk-your-api-key-here"' >> ~/.bashrc
source ~/.bashrc
```

```powershell
# — Windows PowerShell —
# 永久
[Environment]::SetEnvironmentVariable("OPENAI_API_KEY", "sk-your-api-key-here", "User")
```

### 配置文件路径

```text
# Windows
C:\Users\[用户名]\.codex\
├── config.toml    ← 主配置文件
├── auth.json      ← 认证信息
└── AGENTS.md      ← 全局指导文件
```

### 基础配置示例

```toml
# ~/.codex/config.toml

# 模型配置
model = "o4-mini"
model_provider = "openai"
model_reasoning_effort = "high"

# 操作模式
approval_mode = "auto-edit"

# 隐私保护
disable_response_storage = true

# MCP 服务器
[mcp_servers]
github = { command = "npx", args = ["-y", "@modelcontextprotocol/server-github"] }
```

---

## 第三章：OpenAI 订阅指南

### 订阅计划全景

| 计划 | 月费 | Codex 额度 | 适合人群 |
| --- | --- | --- | --- |
| Free | $0 | 基础体验 | 试用者 |
| Plus | $20/月 | $5 / 30天 | 个人开发者 |
| Pro | $200/月 | $50 / 30天 | 重度用户 |
| Team | $25/人/月 | 按成员分配 | 小团队 |
| Enterprise | 定制 | 自定义 | 企业 |

### 各计划详细权益

**🆓 Free 免费版**
- ✅ 可使用 Codex CLI 和 App
- ✅ 访问 GPT-4o mini
- ⚠️ 严重限速，高峰期需排队

**⭐ Plus 订阅（最推荐入门）**
- ✅ 无限 GPT-4o 访问
- ✅ o3 / o4-mini 推理模型
- ✅ Codex 专用额度 $5 / 月
- ✅ DALL·E 图片生成
- ✅ GPTs 商店访问
- ✅ 优先响应，无需排队

**🚀 Pro 订阅（重度用户首选）**
- ✅ Plus 的全部权益
- ✅ 无限 o3-pro 推理模型
- ✅ Codex 专用额度 $50 / 月
- ✅ 更高的速率限制
- ✅ 早期体验新功能

#### API 模式费用估算

| 模型 | 输入 / 1M tokens | 输出 / 1M tokens | 适合任务 |
| --- | --- | --- | --- |
| o4-mini | $1.10 | $4.40 | 日常开发（推荐） |
| gpt-4o | $2.50 | $10.00 | 中等复杂度 |
| o3 | $10.00 | $40.00 | 复杂推理 |

### 省钱技巧

| 技巧 | 说明 | 节省幅度 |
| --- | --- | --- |
| 🎯 用 o4-mini 做简单任务 | 便宜 90% | ⭐⭐⭐ |
| 📝 精简提示词 | 减少上下文 Token | ⭐⭐ |
| 🔄 /compact 定期压缩 | 避免上下文过长 | ⭐⭐ |
| 💳 Plus 订阅 | 获 $5 免费额度 | ⭐⭐⭐ |
| 📊 设 API 花费上限 | platform.openai.com 设置 | 防止超支 |

### 中国大陆用户支付方案

**方案一：虚拟信用卡（推荐）**

| 服务 | 注册门槛 | 手续费 | 推荐度 |
| --- | --- | --- | --- |
| WildCard | 低，支付宝认证 | ~2% | ⭐⭐⭐ |
| Dupay | 低 | ~3% | ⭐⭐⭐ |

**方案二：API 预付费（无需订阅）**
1. 访问 https://platform.openai.com
2. 注册账号并充值 API Credits
3. 使用 API Key 模式运行 Codex（按量计费）

---

## 第四章：基础使用

### 启动方式

| 方式 | 命令 | 说明 |
| --- | --- | --- |
| 交互模式 | `codex` | 进入对话界面 |
| 直接提问 | `codex "解释项目结构"` | 单次执行 |
| 指定模型 | `codex -m o4-mini "简单任务"` | 临时切换模型 |

### 三种操作模式

| 模式 | 说明 | 自主程度 | 适用场景 |
| --- | --- | --- | --- |
| suggest | 只建议，不执行 | 🔒 最安全 | 阅读 / 分析代码 |
| auto-edit | 自动编辑，需确认 | ⚖️ 平衡 | 修改代码（默认） |
| full-auto | 自动执行一切 | ⚡ 最快 | 批量重构（谨慎） |

指定模式：

```bash
codex -a suggest "重构这个函数"
codex -a auto-edit "添加错误处理"
codex -a full-auto "运行测试并修复错误"
```

### 基础交互示例

代码分析：

```bash
codex "分析这个函数的时间复杂度"
codex "找出代码中的潜在 bug"
codex "这个模块的依赖关系是什么？"
```

代码生成：

```bash
codex "为这个类写一个单元测试"
codex "生成 TypeScript 类型定义"
codex "根据注释生成实现代码"
```

代码修改：

```bash
codex "将这个函数改为异步实现"
codex "添加输入参数验证"
codex "重构为更小的函数"
```

### 命令行参数速查

```text
codex [options] [prompt]

  -m, --model <model>        指定模型
  -a, --approval-mode <mode> 设置操作模式
  -q, --quiet                安静模式
  --profile <name>           使用配置文件
  --help                     帮助
  --version                  版本号
```

---

## 第五章：斜杠命令大全

### 会话与流程控制

| 命令 | 功能 | 典型场景 |
| --- | --- | --- |
| /new | 新建会话 | 上一个任务结束 |
| /undo | 撤销上一步 | Codex 改错代码 |
| /exit | 退出程序 | 下班 / 切换项目 |
| /logout | 登出账号 | 切换 OpenAI 账号 |
| /resume | 恢复对话 | 继续之前的工作 |
| /fork | 分叉线程 | 工作出现分支 |
| /compact | 压缩历史 | 对话过长变慢 |

### 配置与权限

| 命令 | 功能 | 典型场景 |
| --- | --- | --- |
| /approvals | 设置权限模式 | 控制自动化程度 |
| /model | 切换模型 | 简单任务用 mini 省钱 |
| /status | 查看状态 | Token 使用量 |
| /mcp | 管理 MCP 工具 | 调试外部连接 |

### 权限模式详解

```text
/approvals 可选模式：

1  Auto（默认推荐）
   • 读取文件 → 自动执行
   • 修改文件 / 运行命令 → 按 Enter 确认

2  Read Only（只读）
   • 只能读取，不能修改
   • 适合审查敏感代码

3  Full Access（全自动）
   • 拥有完全终端控制权
   • ⚠️ 慎用！适合大规模重构
```

### 上下文与记忆

| 命令 | 功能 | 典型场景 |
| --- | --- | --- |
| /mention | 提及文件，加入上下文 | Codex 找不到文件时 |
| /diff | 查看变更（Git Diff） | 提交前检查 |
| /review | 代码审查 | 让 AI 充当 Reviewer |

### 动作与高级功能

| 命令 | 功能 | 典型场景 |
| --- | --- | --- |
| /plan | Plan 模式，先规划后执行 | 复杂任务 |
| /agent | 多代理切换 | 多智能体协作 |
| /skills | 浏览技能 | 探索功能 |
| /theme | 语法高亮主题 | 个性化 |
| /apps | ChatGPT apps | 扩展能力 |

### 自定义斜杠命令 🔥 —— 杀手级功能

Codex 允许你编写自己的斜杠命令，只需创建 Markdown 文件！

创建方法：

```bash
mkdir -p ~/.codex/prompts
# 文件名 = 命令名
```

示例：安全审计命令

```markdown
# Security Audit

请扫描当前代码库，查找以下漏洞：
1. SQL 注入
2. 硬编码的密钥
3. XSS 漏洞
4. 不安全的依赖

生成 Markdown 报告，包含：漏洞位置、严重程度、修复建议
```

使用：

```bash
codex
> /security-audit    # 一键执行整套安全审计！
```

更多示例：

`~/.codex/prompts/daily-report.md`：

```markdown
# Daily Report
分析今天的代码提交，生成日报：
1. 列出所有提交  2. 按模块分类  3. 统计变更量  4. 标注重要改动
```

`~/.codex/prompts/fix-bug.md`：

```markdown
# Fix Bug Workflow
1. 运行测试找出失败用例  2. 分析原因  3. 修复  4. 重测  5. 提交
```

---

## 第六章：AGENTS.md 配置指南

### AGENTS.md 的作用

自动加载进上下文，定义 Codex 在仓库中的工作方式：

| 作用 | 示例 |
| --- | --- |
| 仓库结构 | 关键目录和文件说明 |
| 运行方式 | 构建 / 测试 / lint 命令 |
| 工程规范 | 代码风格、命名约定 |
| PR 期望 | 标题格式、审查要求 |
| 约束 | 禁止模式、敏感文件 |
| 验证 | 完成标准、自测步骤 |

### 基础模板

```markdown
# AGENTS.md

## 项目概述
Next.js 14 项目, App Router + Tailwind CSS。

## 目录结构
- /src/app        → 页面路由
- /src/components → React 组件
- /src/lib        → 工具函数
- /src/types      → TypeScript 类型

## 开发命令
- npm run dev    → 启动开发服务器
- npm run build  → 构建生产版本
- npm run test   → 运行测试
- npm run lint   → 代码检查

## 代码规范
- TypeScript, 禁止 any
- 函数式组件 + Hooks
- Tailwind, 不写内联 CSS
- 文件命名 kebab-case

## 测试要求
- 新功能必须写测试
- 覆盖率 > 80%
- Vitest + Testing Library

## PR 规范
- 标题: type(scope): description
- 必须通过 CI
- 至少一个 Reviewer

## 禁止事项
- ❌ 直接修改 node_modules
- ❌ 提交 .env 文件
- ❌ 跳过测试提交
```

### 多层级配置

```text
项目根目录/AGENTS.md    ← 项目（团队规范）
项目子目录/AGENTS.md    ← 模块（局部规则）

优先级: 子目录 > 项目根 > 全局
```

### 保持简洁

> 💡 一份简短准确的 AGENTS.md 比充斥模糊规则的长文件更有用。

建议：
1. 先从基础规则入手
2. 发现重复错误后才添加新规则
3. 让 Codex 自己更新

```text
# 当 Codex 犯了两次同样的错误
> 请回顾这次错误，并更新 AGENTS.md 以防止再次发生
```

---

## 第七章：MCP 服务器集成

### 什么是 MCP？

**MCP（Model Context Protocol）** 是开放协议，让 AI 模型连接外部工具和服务。

| 传统 | MCP |
| --- | --- |
| 手动复制粘贴 | 自动获取实时数据 |
| 记忆 API 用法 | 直接调用工具 |
| 信息可能过时 | 始终最新 |

### 配置方法

**命令行**

```bash
codex mcp add openaiDeveloperDocs --url https://developers.openai.com/mcp
codex mcp list           # 查看
codex mcp remove <name>  # 移除
```

**配置文件（~/.codex/config.toml）**

```toml
[mcp_servers]
github     = { command = "npx", args = ["-y", "@modelcontextprotocol/server-github"] }
filesystem = { command = "npx", args = ["-y", "@modelcontextprotocol/server-filesystem"] }
postgres   = { command = "npx", args = ["-y", "@modelcontextprotocol/server-postgres"] }
slack      = { command = "npx", args = ["-y", "@modelcontextprotocol/server-slack"] }
```

### 常用 MCP 服务器

| 服务器 | 功能 | 典型用途 |
| --- | --- | --- |
| 🔗 GitHub | Issue / PR / 分支管理 | 代码审查自动化 |
| 🐘 PostgreSQL | SQL 查询 / 结构查看 | 数据库管理 |
| 📁 Filesystem | 文件读写 | 跨项目文件操作 |

---

## 第八章：Skills 技能系统

### 设计原则

```text
一个技能 = 一项工作 + 明确输入/输出 + 清晰触发词
```

检查清单：
- [ ] 2-3 个具体用例
- [ ] 输入格式
- [ ] 输出格式
- [ ] 使用场景说明
- [ ] 触发短语

### 创建技能

文件结构：

```text
~/.agents/skills/
└── my-skill/
    └── SKILL.md    ← 核心文件
```

SKILL.md 模板：

```markdown
---
name: 日志分类器
description: 自动分析日志内容并分类
triggers:
  - "帮我分析日志"
  - "log analysis"
inputs:
  - 日志文件路径或文本
outputs:
  - 分类报告 + 异常统计
---

# 日志分类器

## 功能说明
自动识别 ERROR / WARNING / INFO / DEBUG

## 输出格式
统计摘要 + 异常详情 + 修复建议
```

### 技能管理

```text
> /skills          # 浏览技能
> $skill-installer # 安装技能
> $skill-creator   # 创建技能

# 位置
~/.agents/skills/          # 个人
项目根目录/.agents/skills/ # 团队共享
```

---

## 第九章：Codex App 桌面版

### 核心功能

**① 多智能体并行**

```text
┌─────────────────────────┐
│      Codex App 指挥中心     │
├─────────────────────────┤
│ 🧑‍💻 Agent 1 → 前端开发       │
│ 🧑‍💻 Agent 2 → 后端 API      │
│ 🧑‍💻 Agent 3 → 测试验证       │
│ 🧑‍💻 Agent 4 → 文档编写       │
└─────────────────────────┘
每个 Agent 在独立工作树中运行，互不干扰
```

**② Skills 技能沉淀**

将团队规范打包复用：前端规范 / API 规范 / 数据库规范 / 安全清单 / 部署流程

**③ Automations 自动化**

| 任务 | 频率 |
| --- | --- |
| 代码提交汇总 | 每天下班前 |
| Bug 扫描 | 每次 PR |
| 发布说明 | 发布时 |
| CI 故障检查 | 每天早上 |

### Codex App vs CLI

| 功能 | CLI | App |
| --- | --- | --- |
| 界面 | 终端 | 图形 |
| 多智能体 | ❌ | ✅ |
| 自动化 | ❌ | ✅ |
| 技能管理 | 基础 | 完整 |
| 多项目 | 手动 | 可视化 |
| 适合 | 个人 | 团队 |

### 桌面操控能力（2026年4月新功能）

| 功能 | 说明 |
| --- | --- |
| 🖱️ 光标控制 | 独立光标，实时查看屏幕 |
| 🖥️ 应用操控 | 点击元素、输入文字 |
| 🧠 智能记忆 | 保存偏好和工作流 |
| ⏱️ 长期调度 | 跨天/跨周推进项目 |

使用场景：

```text
> 请打开我的电商后台
> 点击"商品管理"页面
> 截图检查当前界面
> 测试添加商品功能
```

---

## 第十章：API 高级用法

### 流式输出（Streaming）

流式输出让响应实时逐步返回，无需等待完整生成。

**Python 示例：**

```python
from openai import OpenAI

client = OpenAI()

stream = client.chat.completions.create(
    model="gpt-4o",
    messages=[
        {"role": "system", "content": "你是一个代码审查助手。"},
        {"role": "user", "content": "审查这段代码的安全性问题: ..."}
    ],
    stream=True
)

for chunk in stream:
    if chunk.choices[0].delta.content:
        print(chunk.choices[0].delta.content, end="", flush=True)
```

**Node.js 示例：**

```javascript
const stream = await client.chat.completions.create({
  model: "gpt-4o",
  messages: [{ role: "user", content: "解释这段代码的作用" }],
  stream: true,
});

for await (const chunk of stream) {
  process.stdout.write(chunk.choices[0]?.delta?.content || "");
}
```

### 函数调用（Function Calling）

函数调用让模型结构化地请求执行操作，是 MCP 的底层机制。

定义函数：

```python
tools = [
    {
        "type": "function",
        "function": {
            "name": "get_weather",
            "description": "获取指定城市的天气",
            "parameters": {
                "type": "object",
                "properties": {
                    "city": {"type": "string", "description": "城市名称"},
                    "unit": {"type": "string", "enum": ["celsius", "fahrenheit"]}
                },
                "required": ["city"]
            }
        }
    }
]
```

处理函数调用：

```python
response = client.chat.completions.create(
    model="gpt-4o",
    messages=[{"role": "user", "content": "北京今天天气怎么样？"}],
    tools=tools,
    tool_choice="auto"
)

message = response.choices[0].message
if message.tool_calls:
    for tool_call in message.tool_calls:
        function_name = tool_call.function.name
        function_args = json.loads(tool_call.function.arguments)
        if function_name == "get_weather":
            result = get_weather(**function_args)
        messages.append(message)
        messages.append({
            "role": "tool",
            "tool_call_id": tool_call.id,
            "content": str(result)
        })
```

### 批量任务处理

批量代码审查：

```python
def batch_review(directory, pattern="*.py"):
    """批量审查目录下的所有 Python 文件"""
    import glob
    files = glob.glob(os.path.join(directory, "**", pattern), recursive=True)
    results = []
    for file_path in files:
        with open(file_path, 'r', encoding='utf-8') as f:
            code = f.read()
        response = client.chat.completions.create(
            model="o4-mini",
            messages=[
                {"role": "system", "content": "你是代码审查专家。简洁指出问题。"},
                {"role": "user", "content": f"审查以下代码，列出问题: \n```\n{code}\n```"}
            ]
        )
        results.append({"file": file_path, "review": response.choices[0].message.content})
    return results
```

并行批量（更高效）：

```python
import asyncio
from openai import AsyncOpenAI

client = AsyncOpenAI()

async def review_file(file_path):
    with open(file_path, 'r') as f:
        code = f.read()
    response = await client.chat.completions.create(
        model="o4-mini",
        messages=[{"role": "user", "content": f"审查代码: \n{code[:4000]}"}]
    )
    return {"file": file_path, "review": response.choices[0].message.content}

async def batch_parallel(files, concurrency=5):
    semaphore = asyncio.Semaphore(concurrency)
    async def limited_review(fp):
        async with semaphore:
            return await review_file(fp)
    return await asyncio.gather(*[limited_review(f) for f in files])

results = asyncio.run(batch_parallel(file_list))
```

### 结构化输出

```python
response = client.chat.completions.create(
    model="gpt-4o",
    messages=[{"role": "user", "content": "分析这个函数的复杂度"}],
    response_format={
        "type": "json_schema",
        "json_schema": {
            "name": "complexity_analysis",
            "schema": {
                "type": "object",
                "properties": {
                    "time_complexity": {"type": "string"},
                    "space_complexity": {"type": "string"},
                    "issues": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "type": {"type": "string"},
                                "severity": {"type": "string", "enum": ["low", "medium", "high"]},
                                "description": {"type": "string"}
                            }
                        }
                    }
                }
            }
        }
    }
)
analysis = json.loads(response.choices[0].message.content)
```

### Embedding 与语义搜索

```python
    code_embeddings = []
    for fp in code_files:
        emb = client.embeddings.create(
            model="text-embedding-3-small",
            input=open(fp).read()[:8000]
        ).data[0].embedding
        code_embeddings.append((fp, emb))

    import numpy as np
    scores = []
    for fp, emb in code_embeddings:
        similarity = np.dot(query_embedding, emb) / (
            np.linalg.norm(query_embedding) * np.linalg.norm(emb)
        )
        scores.append((fp, similarity))

    scores.sort(key=lambda x: x[1], reverse=True)
    return scores[:top_k]
```

---

## 第十一章：竞品深度横评

### 六大竞品一览

| 维度 | 🟦 Codex | 🟪 Claude Code | 🟢 Cursor | 🟡 Copilot | 🔵 Windsurf | 🟠 Aider | ⬛ Devin |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 类型 | CLI + App | CLI | IDE | IDE 插件 | IDE | CLI | AI 员工 |
| 开源 | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ |
| 底层技术 | Rust | Node.js | Electron | 云端 | Electron | Python | 云端 |
| 多智能体 | ✅ App | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ |
| MCP | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 自定义命令 | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ |
| 代码执行 | ✅ 本地 | ✅ 本地 | ✅ 本地 | ❌ | ✅ 本地 | ✅ 本地 | ✅ 沙箱 |
| 适合人群 | 极客 | 业务开发 | 日常开发 | 初学者 | 全栈 | 极客 | 企业 |

### 逐一分析

**🟦 OpenAI Codex**
- 优势：开源，完全自定义；Rust 构建，速度极快；MCP 生态扩展能力强；自定义斜杠命令 = 工作流自动化；多智能体并行（App）
- 劣势：推理深度不如 Claude Code；文档尚在完善；社区生态较新
- 适合：追求控制力和效率的架构师/极客

**🟪 Anthropic Claude Code**
- 优势：自主思考能力强；超长上下文（200K tokens）；代码理解力出色
- 劣势：闭源，无法自定义底层；不支持 MCP；价格较高
- 适合：需要深度推理的复杂项目

**🟢 Cursor**
- 优势：IDE 体验最好；多模型切换（GPT/Claude/Gemini）；代码补全体验流畅；上下文感知精准
- 劣势：闭源，订阅制；无 CLI 模式；不支持多智能体
- 适合：日常开发，IDE 党首选

**🟡 GitHub Copilot**
- 优势：生态最广（VS Code 内置）；企业方案成熟
- 劣势：功能最基础；无法执行代码；不支持自定义工作流
- 适合：初学者、轻量使用

**🔵 Windsurf（Codeium）**
- 优势：Cascade 流式推理；多智能体协作；免费版功能丰富
- 适合：追求性价比的全栈开发者

**🟠 Aider**
- 优势：开源，Python 编写；支持多模型；Git 集成最好；完全免费
- 适合：终端极客、Git 工作流重度用户

**⬛ Devin（Cognition）**
- 优势：完全自主，接近人类员工
- 劣势：极贵（$500/月）；完全云端，代码离开本机；不透明，难以审查
- 适合：预算充足的企业团队

### 选型决策树

```text
你的首要需求是什么?
│
├─ 🎯 精细控制 & 自定义 → Codex CLI
├─ 🧠 深度推理 & 复杂逻辑 → Claude Code
├─ 🖥️ IDE 体验 & 日常开发 → Cursor
├─ 💰 最低成本入门 → Copilot
├─ ⚡ 性价比 & 多模型 → Windsurf / Aider
└─ 📋 全自动 AI 员工 → Devin
```

### 组合使用策略

| 策略 | 工具组合 | 场景 |
| --- | --- | --- |
| 🥇 黄金组合 | Codex CLI + Cursor | Codex 做架构/重构，Cursor 做日常编辑 |
| 🧠 深度思考 | Claude Code + Codex | Claude 做推理，Codex 做执行 |
| 💰 穷人方案 | Aider + Copilot Free | Aider 做复杂任务，Copilot 做补全 |
| 🏢 企业方案 | Codex App + Copilot Business | App 做项目管理，Copilot 做团队补全 |

---

## 第十二章：安全与隐私

### 数据处理流程

```text
你的代码
     ↓
本地 Codex (CLI/App)
     ↓ 加密传输
OpenAI API 服务器
     ↓ 生成响应
本地 Codex
     ↓
返回结果（仅本地）

⚠️ 代码会在传输过程中到达 OpenAI 服务器
⚠️ 但不会被用于训练（付费用户）
✅ 传输过程使用 TLS 加密
```

### 关键数据策略

| 计划 | 数据用于训练 | 数据保留 | 审计日志 |
| --- | --- | --- | --- |
| Free | 可能 | 30天 | ❌ |
| Plus/Pro | ❌ | 30天 | ❌ |
| Team | ❌ | 可配置 | ✅ |
| Enterprise | ❌ | 可配置 | ✅ |

> 💡 **最安全的模式**：使用 API Key + `disable_response_storage = true`，代码不会被存储。

### 沙箱与隔离机制

隔离层级：

```text
Level 1: 网络隔离 → Codex 不主动发送数据到外部
Level 2: 文件系统隔离 → suggest 模式只读，auto-edit 需确认
Level 3: 命令执行隔离 → 危险命令需确认，full-auto 有执行白名单
Level 4: Agent 隔离 (App) → 每个 Agent 独立工作树，互不干扰
```

### 安全配置示例

```toml
# ~/.codex/config.toml — 安全配置

disable_response_storage = true
approval_mode = "auto-edit"

# 限制可执行的命令
[allowed_commands]
run_test = ["npm", "test"]
run_lint = ["npm", "run", "lint"]
run_build = ["npm", "run", "build"]

# 禁止的文件路径
[restricted_paths]
secrets = ".env*"
keys = "*key*"
certs = "*cert*"
```

### 敏感文件保护

AGENTS.md 中添加禁止规则：

```markdown
## 禁止事项
- ❌ 不要读取 .env 文件
- ❌ 不要修改包含密钥的文件
- ❌ 不要执行包含 secrets 的命令
- ❌ 不要提交任何包含 API Key 的代码
```

### 企业合规审计

```python
# audit.py — 企业安全审计脚本

def generate_security_report():
    """生成安全审计报告"""
    import os
    from pathlib import Path

    sensitive_patterns = ["api_key", "secret", "password", "token", ".env"]
    violations = []

    for root, dirs, files in os.walk("."):
        # 跳过 node_modules 和 .git
        dirs[:] = [d for d in dirs if d not in ["node_modules", ".git", "dist"]]
        for file in files:
            if file.endswith((".py", ".js", ".ts", ".json")):
                filepath = Path(root) / file
                try:
                    content = filepath.read_text(encoding="utf-8", errors="ignore")
                    for pattern in sensitive_patterns:
                        if pattern.lower() in content.lower():
                            violations.append({
                                "file": str(filepath),
                                "pattern": pattern,
                                "raw": content[max(0, content.lower().find(pattern.lower())-20):]
                            })
                except Exception:
                    pass
    return violations
```

---

## 第十三章：团队协作方案

### 多角色权限配置

```toml
# ~/.codex/config.toml — 按角色配置

# Junior Developer (只读 + 小修改)
[junior]
approval_mode = "suggest"
allowed_dirs = ["src/components", "src/pages"]

# Senior Developer (自动编辑)
[senior]
approval_mode = "auto-edit"
allowed_dirs = ["src/**"]

# Tech Lead (全权限)
[techlead]
approval_mode = "full-auto"
allowed_dirs = ["**"]
```

### 团队共享 Skills

```text
团队技能目录：
团队 Git 仓库/.codex/skills/
├── pr-review.md        # PR 审查清单
├── test-generator.md   # 测试生成规范
├── api-doc.md          # API 文档规范
├── security-check.md   # 安全检查
└── deploy.md           # 部署流程
```

团队技能示例 `pr-review.md`：

```markdown
# PR Review — 团队标准审查

## 审查清单

### 代码质量
- [ ] 命名规范
- [ ] 函数长度 < 50 行
- [ ] 无重复代码
- [ ] 类型定义完整

### 测试覆盖
- [ ] 新功能有测试
- [ ] 边界条件有测试
- [ ] 测试通过率 100%

### 安全性
- [ ] 无硬编码密钥
- [ ] 输入验证完整
- [ ] SQL 参数化

### 文档
- [ ] 公共 API 有文档
- [ ] 复杂逻辑有注释
```

### CI/CD 集成

```yaml
# .github/workflows/codex-review.yml
name: AI Code Review

on:
  pull_request:
    branches: [main, develop]

jobs:
  ai-review:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Setup Codex
        run: npm install -g @openai/codex

      - name: Run AI Review
        env:
          OPENAI_API_KEY: ${{ secrets.OPENAI_API_KEY }}
        run: |
          codex --model o4-mini --no-input "
          审查本次 PR 的代码变更，重点关注：
          1. 代码质量和风格
          2. 潜在的 Bug 和安全问题
          3. 测试覆盖
          4. 文档完整性
          "
```

### 代码库迁移流程

```text
# 团队从旧工具迁移到 Codex
1  Tech Lead 安装并配置 Codex
2  创建团队 AGENTS.md 并提交到 Git
3  创建团队 Skills 库并同步
4  新成员 clone 后自动继承配置
5  逐步淘汰旧工具
```

---

## 第十四章：技术栈实战

### React 技术栈

AGENTS.md 模板：

```markdown
# AGENTS.md — React 项目

## 技术栈
- React 18 + TypeScript
- Vite (构建)
- TanStack Query (数据获取)
- React Router v6 (路由)
- Zustand (状态管理)
- Tailwind CSS (样式)
- React Hook Form + Zod (表单)
- Vitest + Testing Library (测试)

## 代码规范
- TypeScript，禁用 any
- 函数式组件 + Hooks
- Tailwind，不写内联 CSS
- 文件命名 kebab-case

## 常用命令
- npm run dev       → 启动开发
- npm run build     → 构建
- npm run test      → 测试
- npm run lint      → ESLint
```

高效提示词示例：

```bash
# 生成页面
codex "创建用户列表页面，使用 DataTable 组件，支持搜索和分页"

# 生成 API Hook
codex "为 /api/users 端点创建 useUsers Hook，使用 TanStack Query，包含类型定义"

# 调试
codex "这个组件在 SSR 时报 'window is not defined'，修复它"

# 测试
codex "为 UserList 组件编写测试，包含：加载状态、空数据、搜索过滤、分页切换"
```

### Python 技术栈

AGENTS.md 模板：

```markdown
# AGENTS.md — Python 项目

## 技术栈
- Python 3.12
- FastAPI (Web)
- SQLAlchemy 2.0 (ORM)
- Pydantic v2 (验证)
- pytest (测试)
- Ruff (Lint + Format)

## 代码规范
- 类型注解必须完整
- 使用 async/await
- Pydantic Model 做输入输出验证
- 依赖注入用 Depends()

## 常用命令
- uvicorn app.main:app --reload  → 开发
- pytest                          → 测试
- ruff check .                    → Lint
```

高效提示词：

```bash
# 生成 CRUD
codex "为 User 模型创建完整的 CRUD API，包含 Pydantic schema、路由、服务层"

# 数据库迁移
codex "创建 Alembic 迁移脚本，添加 User 表的 email_verified 字段"

# 测试
codex "为用户注册 API 编写 pytest 测试，覆盖: 正常注册、重复邮箱、密码强度验证"
```

### Go 技术栈

AGENTS.md 模板：

```markdown
# AGENTS.md — Go 项目

## 技术栈
- Go 1.22
- Gin / Echo (HTTP)
- GORM (ORM)
- pgx (PostgreSQL driver)

## 代码规范
- 遵循 Effective Go
- 错误处理必须显式
- 接口定义在消费方
- 表驱动测试

## 常用命令
- go run ./cmd/server    → 启动
- go test ./...          → 测试
- go vet ./...           → 静态检查
- golangci-lint run      → Lint
```

### Rust 技术栈

AGENTS.md 模板：

```markdown
# AGENTS.md — Rust 项目

## 技术栈
- Rust 1.78+
- Tokio (异步运行时)
- Axum (Web)
- SQLx (数据库)
- Serde (序列化)

## 代码规范
- Clippy 零警告
- 所有 public API 必须有文档注释
- 错误使用 thiserror 定义
- 异步函数使用 async fn

## 常用命令
- cargo run     → 运行
- cargo test    → 测试
- cargo clippy  → Lint
- cargo fmt     → 格式化
```

---

## 第十五章：成本分析与管控

### 成本结构全景

直接成本：

| 成本项 | 月费 | 说明 |
| --- | --- | --- |
| ChatGPT Plus | $20 | 含 $5 Codex 额度 |
| ChatGPT Pro | $200 | 含 $50 Codex 额度 |
| API 额度 | 按量 | o3 ~$20-200/月（重度） |

### 用量追踪

```python
# track_usage.py — 每日用量追踪
from datetime import datetime, timedelta
from openai import OpenAI

client = OpenAI()

def get_daily_usage():
    """获取 API 使用量"""
    usage = client.billing.usage(
        start_date=(datetime.now() - timedelta(days=1)).strftime("%Y-%m-%d"),
        end_date=datetime.now().strftime("%Y-%m-%d")
    )
    return usage

PRICING = {
    "o4-mini": {"input": 1.10, "output": 4.40},
    "gpt-4o": {"input": 2.50, "output": 10.00},
    "o3": {"input": 10.00, "output": 40.00},
}

def estimate_cost(model, input_tokens, output_tokens):
    """估算单次调用成本"""
    price = PRICING.get(model, PRICING["o4-mini"])
    cost = (
        input_tokens / 1_000_000 * price["input"] +
        output_tokens / 1_000_000 * price["output"]
    )
    return cost

def monthly_projection(model, daily_tasks, avg_input=500, avg_output=800):
    """月度成本预测"""
    daily_cost = estimate_cost(model, avg_input, avg_output) * daily_tasks
    monthly_cost = daily_cost * 30
    return {
        "model": model,
        "daily_tasks": daily_tasks,
        "monthly_cost": f"${monthly_cost:.2f}",
    }
```

### 预算告警

```python
# budget_alert.py — 每日检查 API 用量并发送告警
import os
import requests
from openai import OpenAI
from datetime import datetime

SLACK_WEBHOOK = os.environ.get("SLACK_WEBHOOK_URL")
MONTHLY_BUDGET = 50  # USD

def check_budget():
    client = OpenAI()
    usage = client.billing.usage(
        start_date=datetime.now().replace(day=1).strftime("%Y-%m-%d"),
        end_date=datetime.now().strftime("%Y-%m-%d")
    )
    total = usage.total_usage / 100
    percentage = (total / MONTHLY_BUDGET) * 100
    if percentage >= 80:
        emoji = "🔴" if percentage >= 100 else "🟡"
        requests.post(SLACK_WEBHOOK, json={
            "text": f"{emoji} API 用量已达 {percentage:.0f}%（${total:.2f} / ${MONTHLY_BUDGET}）"
        })

if __name__ == "__main__":
    check_budget()
```

### ROI 计算

| 指标 | 无 AI | 有 Codex | 提升 |
| --- | --- | --- | --- |
| 功能开发时间 | 8h | 4h | 50% ⬆️ |
| 代码审查时间 | 2h | 0.5h | 75% ⬆️ |
| Bug 修复时间 | 4h | 2h | 50% ⬆️ |
| 月度成本 | $0 | $20-50 | - |
| 月度节省 | - | ~40h | - |

ROI 公式：

```text
月度 ROI = (节省工时 × 时薪 - AI 工具费) / AI 工具费 × 100%

示例（时薪 $50）:
ROI = (40h × $50 - $25) / $25 × 100% = 7900%
即每投入 $1，回报 $80
```

---

## 第十六章：最佳实践

### 提示词四要素

```text
优秀提示 = 目标 + 上下文 + 约束 + 完成条件
```

示例：

```text
🎯 目标: 优化用户登录接口性能

📍 上下文:
  • 文件: src/api/auth.ts
  • 问题: 每次登录 3 秒
  • 相关: src/utils/cache.ts

🔒 约束:
  • 接口不变
  • 不破坏安全性
  • 使用 Redis 缓存

✅ 完成条件:
  • 响应 < 500ms
  • 通过所有测试
```

### 推理级别选择

| 级别 | 场景 | 建议 |
| --- | --- | --- |
| 🟢 低 | 快速、范围清晰 | 日常开发 |
| 🔴 极高 | 需主动思考的长任务 | 全新系统设计 |

> ⚠️ 最强推理 ≠ 最佳结果，匹配任务复杂度即可

### 规划优先

复杂任务务必先 `/plan`：

```text
> /plan
# Codex 会: 收集上下文 → 提问 → 制定计划 → 等确认后执行
```

### 避坑指南

| ❌ 错误 | ✅ 正确 |
| --- | --- |
| 规则塞进提示里 | 移到 AGENTS.md |
| 不告诉如何跑测试 | 明确构建/测试命令 |
| 复杂任务不规划 | 先 /plan |
| 一上来给全权限 | 逐步放宽 |
| 多步不验证 | 每步自动测试 |
| 对话太长不压缩 | 定期 /compact |
| 一次提多个需求 | 一次一个清晰需求 |

### 进阶路线

```text
Level 1 → 生成代码（复制粘贴）
Level 2 → 分析修改（理解上下文）
Level 3 → 规划验证（计划 + 测试 + 审查）
Level 4 → 自动化流（Skills + Automation）
Level 5 → 团队协作（AGENTS.md + 团队规范）
Level 6 → 自定义生态（MCP + Skills + API 集成）
```

---

## 第十七章：实战案例

### 案例一：遗留代码重构

场景：重构 3 年前的 PHP 遗留系统

```text
codex

# 步骤 1: 理解现有代码
> 分析 src/ 目录下的 PHP 文件结构，理解业务逻辑
> 重点关注: 用户认证、订单处理、数据模型

# 步骤 2: 创建 AGENTS.md
> /init
> 为 PHP 重构项目创建 AGENTS.md，包含:
>   - 技术栈目标: Python FastAPI
>   - 代码规范
>   - 重构原则

# 步骤 3: 分模块迁移
> 先迁移用户认证模块到 Python
> 为迁移后的代码编写测试
> 验证功能等价性

# 步骤 4: 审查与优化
> /review
> 优化性能瓶颈

# 步骤 5: 文档与部署
> 生成 API 文档
> 编写部署脚本
```

### 案例二：自动化代码审查流水线

```yaml
# .github/workflows/daily-review.yml
name: Daily AI Code Review

on:
  schedule:
    - cron: '0 9 * * *'  # 每天早上 9 点

jobs:
  review:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install Codex
        run: npm install -g @openai/codex

      - name: Run AI Review
        env:
          OPENAI_API_KEY: ${{ secrets.OPENAI_API_KEY }}
        run: |
          codex --model o4-mini --no-input "
          审查昨天以来的代码变更，生成简洁的审查报告:
          1. 代码质量和风格
          2. 潜在的 Bug 和安全问题
          3. 改进建议
          "

      - name: Create GitHub Issue
        uses: actions/github-script@v7
        with:
          script: |
            github.rest.issues.create({
              owner: context.repo.owner,
              repo: context.repo.repo,
              title: '📋 AI 代码审查报告 - ' + new Date().toLocaleDateString(),
              body: '请查看自动生成的代码审查报告'
            })
```

### 案例三：TDD 开发工作流

```text
codex

# Test-Driven Development with Codex

# 步骤 1: 编写测试
> 为 UserService.get_user(id) 编写测试用例，覆盖:
>   - 正常获取
>   - 用户不存在
>   - 数据库连接失败

# 步骤 2: 运行测试（应该失败）
> pytest tests/test_user.py -v

# 步骤 3: 让 Codex 实现通过测试的代码
> 根据以上测试用例，实现 UserService.get_user 方法
>   确保通过所有测试用例

# 步骤 4: 运行测试（应该通过）
> pytest tests/test_user.py -v

# 步骤 5: 重构优化
> 重构代码，确保可读性和性能
> 确保所有测试仍然通过
```

### 案例四：全栈开发

```text
codex

# 电商后台全栈开发

# 后端
cd backend
> 创建商品管理 REST API
> 包含 CRUD、搜索、分页
> 为所有接口编写集成测试
> 生成 OpenAPI 文档

# 前端
cd frontend
> 创建商品管理页面
> 使用 React + Ant Design
> 对接后端 API
> 添加表单验证和错误处理

# 数据库
> 设计商品表结构
> 创建 Alembic 迁移
> 编写种子数据
> 配置 CI/CD
> 编写部署文档
```

### 案例五：DevOps 自动化

```text
codex

# 场景: 自动化发布流程

> 创建自动化部署脚本，包含:
> 1. 代码检查 (lint + test)
> 2. Docker 镜像构建
> 3. 推送到镜像仓库
> 4. 滚动更新到 Kubernetes
> 5. 健康检查
> 6. 发送部署通知到 Slack

> 创建回滚脚本，能在 30 秒内回滚到上一版本

> 创建运维监控 Dashboard，展示:
> - 服务健康状态
> - 错误率
> - 响应时间
> - 资源使用率
```

---

## 附录

### A. 命令速查卡

| 命令 | 功能 | 示例 |
| --- | --- | --- |
| codex | 启动交互 | `codex` |
| codex "..." | 单次执行 | `codex "解释代码"` |
| /new | 新会话 | /new |
| /init | 初始化项目 | /init |
| /diff | 查看变更 | /diff |
| /review | 代码审查 | /review |
| /plan | 规划任务 | /plan |
| /undo | 撤销修改 | /undo |
| /compact | 压缩历史 | /compact |
| /model | 切换模型 | /model |
| /status | 查看状态 | /status |
| /skills | 浏览技能 | /skills |
| /mcp | MCP 管理 | /mcp |

### B. 配置文件速查

| 配置项 | 说明 | 默认值 |
| --- | --- | --- |
| model | 默认模型 | o4-mini |
| approval_mode | 操作模式 | auto-edit |
| allow | 允许的命令 | [] |
| deny | 禁止的命令 | [] |

### C. 模型速查

| 模型 | 适用场景 | 性价比 |
| --- | --- | --- |
| o4-mini | 日常开发 | ⭐⭐⭐⭐⭐ |
| gpt-4o | 中等复杂度 | ⭐⭐⭐⭐ |
| o3 | 复杂推理 | ⭐⭐⭐ |

### D. 环境变量速查

| 变量 | 说明 |
| --- | --- |
| OPENAI_API_KEY | API 密钥 |
| OPENAI_BASE_URL | API 地址（代理时设置） |
| HTTP_PROXY | HTTP 代理 |
| HTTPS_PROXY | HTTPS 代理 |

### E. 常见错误代码

| 错误 | 原因 | 解决 |
| --- | --- | --- |
| AUTH_FAILED | API Key 无效 | 检查 OPENAI_API_KEY |
| RATE_LIMIT | 请求过于频繁 | 降低频率或升级计划 |
| CONTEXT_OVERFLOW | 上下文超限 | 使用 /compact |
| PERMISSION_DENIED | 权限不足 | 检查 config.toml |

### F. 资源链接

| 资源 | 链接 |
| --- | --- |
| 官方文档 | https://docs.anthropic.com/codex |
| GitHub | https://github.com/openai/codex |
| MCP Server | https://github.com/modelcontextprotocol |
| OpenAI API | https://platform.openai.com |
