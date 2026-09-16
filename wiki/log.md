# Log — 只追加流水账

> 格式：`[YYYY-MM-DD] <ingest|query|lint> | <一句话>`。可 grep。新在下方追加。

[2026-08-11] lint | 建立 karpathy LLM-Wiki 结构：新增 raw/ wiki/，8 域 112 篇迁入 wiki/，生成 index.md / log.md / 根 CLAUDE.md(schema)。
[2026-08-11] lint | 补 composer_stub 缺失双链：HWC/SERDES链路/GFWK稳定性-阅读脉络/TC_CSOC_FAULT_003/TC_GFWK_STR_005/TC_GFWK_STRESS_006/BUG-STRESS006 共7页 prose/footer 裸术语→[[composer_stub]]（跳过代码/mermaid/路径）。
[2026-08-11] ingest | SERDES(GMSL)链路: 台架实测确认数据源(debugfs serdes_status/dmesg gmsl lock[0x8a]/link training 注入), 更新 [[SERDES链路]], 新增用例页 [[TC_SERDES_FAULT_002]]/[[TC_SERDES_STRESS_003]](均实测通过)。

[2026-09-15] query | SF: 补全 main thread/commit/composite/present 四词定义表+调用链于 SurfaceFlinger.md
[2026-09-15] ingest | Fence 页补充：举手信号直觉说明 + present fence 死屏案例(FENCE GAP display=100)，双链 [[SurfaceFlinger 主线程与三阶段]]

[2026-09-15] lint | 美化 SurfaceFlinger.md：callout 分节、修复断裂编号/悬空命令、关联导航表格化
[2026-09-15] ingest | 新增 [[MessageQueue]]（SF 主线程事件循环/基于 Looper）：唤醒链路+message语义(what INVALIDATE/REFRESH)+单线程模型+版本演进；双链 [[SurfaceFlinger]]/[[SurfaceFlinger 主线程与三阶段]]，登记 index(004-AAOS 51 篇/共115)。

[2026-09-16] ingest | 美化+补全 [[自生长LLM Wiki 方法论]]：据 Bilibili(Xuan_酱《Codex 联动 Obsidian 搭卡帕西同款知识库》)补 A/B/C/D 四层闭环+速查表+映射本 vault+延伸阅读；新增 [[karpathy]]/[[Obsidian]] 双链页，登记 index(007-速查 4 篇/未归类)。
[2026-09-16] lint | 对照 raw/articles/llm-wiki.md 体检 CLAUDE.md(schema)：补 index 一句话摘要、log grep 用法、frontmatter 字段约定、图片本地化(raw/assets 两步读图)、Query 输出形态、Lint 数据缺口+建议下一步；log 前缀保留 [YYYY-MM-DD] 不迁 ## 以护历史。
[2026-09-16] ingest | CST SDK 接入文档(v1.0,34页): 新增 [[CST SDK]](hub,004)/[[GDC]](004,Debug/跨SoC通信)/[[GFS]](005,Diag/故障4方向分发+OTA)；增补 [[CST - CornerStone]] 维护服务全貌(双进程/事件标记/云同步/USB导出/Maint客户端)；双链 [[GIPC]]/[[SHMEM]]/[[安全核]]/[[仪表]]/[[tombstone]]/[[RAMdump]]/故障用例；登记 index(004:53/005:47/共118)。
