
### MCP
**定义**：**MCP**（Model Context Protocol）是 AI 工具的"标准插座"协议——好比给 Codex、Claude Code 等 AI Agent  装上 USB 连接外部工具的接口，任何符合协议的外部工具都能即插即用，能以统一协议调用 Playwright 外接插件工具。

#### playwright
[[Playwright]]  真正操作浏览器的自动化引擎，可以控制浏览器

> 那是不是可以用这个工具来读取飞书问题单呢？



## 🧩 Gerrit MCP 是什么



**Gerrit MCP** 就是把你们公司的 **Gerrit 代码评审系统**接到 AI（Cursor / Claude Code）上的一个"转接头"。它本质是一个本地 Python 小程序，AI 通过它调用 Gerrit 的 HTTP API。装好后你可以直接用自然语言：

> "帮我审查这个 Gerrit 提交 xxx，把 Critical 问题以评论提交上去"  
> "给 change 12345 打 Code-Review -1"

AI 就会自动拉取 diff、分析代码、写评论、打分——**不用手动打开 Gerrit 网页逐行看**。

## ✅ 已完成的安装步骤

按文档"手动安装"方式（在线 Smithery 安装是 Cursor 专用，手动方式更通用）：

1. **克隆仓库** → `D:\Dev_Doc\Automation\gerrit-code-review-mcp`
2. **创建虚拟环境** `.venv` 并安装依赖（`mcp 1.28.1`、`requests`、`python-dotenv`）
3. **复制 `.env.template` → `.env`**（待填写）

> 冒烟测试（import server.py）被自动权限模式拦截了——刚克隆的外部代码需要你确认后才能执行，不影响安装本身。