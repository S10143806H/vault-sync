# 参考资料
https://skillhub.cn/





**怎么选**：
- 目标系统有 HTTP API / CLI → 写 Skill 就够了（你的 gtmp-skill 就是证明）；
- 需要长连接、复杂状态、或想给不会写 curl 的场景用 → 才上 MCP。

## 把你的 Agent 打包成 Skill

Skill 就是一个目录，**核心只有一个 `SKILL.md`**：

```
my-agent-skill/
├── SKILL.md              # 必须：入口说明书
├── references/           # 可选：详细文档，按需懒加载
│   ├── api-xxx.md
│   └── rules.md
└── scripts/              # 可选：现成脚本，Claude 直接执行
    └── analyze.py
```

### 步骤

**1. 把 Agent 的 system prompt 变成 `SKILL.md`**

```markdown
---
name: my-agent-skill
description: "一句话说清楚：干什么 + 什么时候触发（写上触发关键词，这行决定 Claude 会不会自动想起用它）"
---

# 角色定义（原 agent 的 system prompt）
# 工作流程（收到 X → 做 Y → 输出 Z）
# API/命令模板（curl 示例，让 Claude 照抄）
# 禁止事项（如 gtmp-skill 的"禁止 DELETE"）
```

> **`description` 是灵魂**——Claude 靠它决定何时自动触发。写清楚场景和关键词，如"当用户提到 GTMP、刷机、测试任务时使用"。

**2. 大文档拆到 `references/`**，`SKILL.md` 里写映射表（"懒加载策略"），避免每次全量占上下文。

**3. Agent 依赖的代码逻辑放 `scripts/`**，SKILL.md 里写清楚怎么调用。

### 分享给同事（3 种方式，按省事程度）

| 方式             | 做法                                                                                                        |
| -------------- | --------------------------------------------------------------------------------------------------------- |
| **压缩包/Git**    | 目录发过去，同事解压到 `~/.claude/skills/` 即可，重启 Claude Code 生效                                                      |
| **公司 npm（推荐）** | 你们已有现成模式：发布到 `jf.guasemi.com` 私有 npm，同事一条命令安装：`npx --registry=... @xg/xxx-skill install`（和 gtmp-skill 一样） |
| **Plugin 市场**  | 打包成 Claude Code plugin 发到团队 marketplace，支持版本管理和批量分发                                                       |

> 💡 本机还装了 `skill-creator` 和 `lark-skill-maker` 两个造轮子工具——你说一句"帮我把 XXX agent 做成 skill"，我可以直接用 `skill-creator` 生成完整目录结构给你。要试试吗？把你的 agent 定义（prompt 或 .md 文件）发我即可。


---

### 原理
当一件工作重复3次以上，我们就要考虑把它做成一个skill，可以在agent中使用skill-creator制作skill

```
skill-folder-name # 文件夹的名字，
- SKILL.md # ☝️**必填 核心文件 大写（大小写敏感）**
- scripts/ # 可执行脚本 .sh .py .js 
- references/ # 参考文档 API手册 示例代码
- assets/ # 模板文件，图片，配置等静态资源
```


#### SKILL.md
name: skill-creator
description: feature of this skill

---
### Steps
1. 将 CC 调为 plan模式
2. 帮我制作一个skill，名称为 “ABC”，放置在 “..\\.claude\skills”下，按下列步骤操作：
	1. 第一步
	2. 第二步
	  ....
3. 如果有不清楚的地方就和我充分讨论清楚，讨论清楚后再制作。只要有不满意的地方就和Claude Code利用对话调整，直到满意后开始执行
4. 迭代完成后可以使用 /ABC 使用这个skill

如何把工作skill化
1. 从结果反推：按照这个文件标准来，我每次提供你 xxx 时 一起讨论 然后输出和上面类似的内容，制作一个 skill。例如：从飞书问题单把问题提取到飞书表格
2.


#### issue-sync
```
- 将 CC 调为 plan模式
- 帮我制作一个skill，名称为 “issue-sync”，放置在 “C:\Users\XGTech_ZHU\.claude\skills”下，按下列步骤操作：
	1. 按照[https://guatechltd.feishu.cn/sheets/Rywss1k5rhcHpbtV2Vpc70WBnIg?sheet=0dedc1](https://guatechltd.feishu.cn/sheets/Rywss1k5rhcHpbtV2Vpc70WBnIg?sheet=0dedc1) 花屏页面中的格式，从飞书问题单 https://project.feishu.cn/guav100r001/issue/homepage 中找出问题缺陷含有关键字 黑屏 花屏 闪屏 白屏 冻帧 的问题，分别填入飞书表格
	2. 表头中包含： Feature Tree，Issue ID，Title，根因，复现方法，问题证据留存，出现位置，自动化辨别方法，修复方案 ，是否包含视频附件
	3. 如果问题的附件中包含了复现问题的视频，按照\Pictures 中的分类方法，下载到 Y:\issue-sync 中留作调试相机算法的数据源
	4. 每天0点定时审核问题单，更新表格，用高亮表示
- 如果有不清楚的地方就和我充分讨论清楚，讨论清楚后再制作。只要有不满意的地方就和Claude Code利用对话调整，直到满意后开始执行
```

#### cam-algo
```
- 将 CC 调为 plan模式
- 帮我制作一个skill，名称为 “issue-sync”，放置在 “C:\Users\XGTech_ZHU\.claude\skills”下，按下列步骤操作：
	1. 按照[https://guatechltd.feishu.cn/sheets/Rywss1k5rhcHpbtV2Vpc70WBnIg?sheet=0dedc1](https://guatechltd.feishu.cn/sheets/Rywss1k5rhcHpbtV2Vpc70WBnIg?sheet=0dedc1) 花屏页面中的格式，从飞书问题单 https://project.feishu.cn/guav100r001/issue/homepage 中找出问题缺陷含有关键字 黑屏 花屏 闪屏 白屏 冻帧 的问题，分别填入飞书表格
	2. 表头中包含： Feature Tree，Issue ID，Title，根因，复现方法，问题证据留存，出现位置，自动化辨别方法，修复方案 ，是否包含视频附件
	3. 如果问题的附件中包含了复现问题的视频，按照\Pictures 中的分类方法，下载到 Y:\issue-sync 中留作调试相机算法的数据源
	4. 每天0点定时审核问题单，更新表格，用高亮表示
- 如果有不清楚的地方就和我充分讨论清楚，讨论清楚后再制作。只要有不满意的地方就和Claude Code利用对话调整，直到满意后开始执行
```

