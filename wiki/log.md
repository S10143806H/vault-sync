# Log — 只追加流水账

> 格式：`[YYYY-MM-DD] <ingest|query|lint> | <一句话>`。可 grep。新在下方追加。

[2026-08-11] lint | 建立 karpathy LLM-Wiki 结构：新增 raw/ wiki/，8 域 112 篇迁入 wiki/，生成 index.md / log.md / 根 CLAUDE.md(schema)。
[2026-08-11] lint | 补 composer_stub 缺失双链：HWC/SERDES链路/GFWK稳定性-阅读脉络/TC_CSOC_FAULT_003/TC_GFWK_STR_005/TC_GFWK_STRESS_006/BUG-STRESS006 共7页 prose/footer 裸术语→[[composer_stub]]（跳过代码/mermaid/路径）。
[2026-08-11] ingest | SERDES(GMSL)链路: 台架实测确认数据源(debugfs serdes_status/dmesg gmsl lock[0x8a]/link training 注入), 更新 [[SERDES链路]], 新增用例页 [[TC_SERDES_FAULT_002]]/[[TC_SERDES_STRESS_003]](均实测通过)。

[2026-09-15] query | SF: 补全 main thread/commit/composite/present 四词定义表+调用链于 SurfaceFlinger.md
[2026-09-15] ingest | Fence 页补充：举手信号直觉说明 + present fence 死屏案例(FENCE GAP display=100)，双链 [[SurfaceFlinger 主线程与三阶段]]

[2026-09-15] lint | 美化 SurfaceFlinger.md：callout 分节、修复断裂编号/悬空命令、关联导航表格化
[2026-09-15] ingest | 新增 [[MessageQueue]]（SF 主线程事件循环/基于 Looper）：唤醒链路+message语义(what INVALIDATE/REFRESH)+单线程模型+版本演进；双链 [[SurfaceFlinger]]/[[SurfaceFlinger 主线程与三阶段]]，登记 index(004-AAOS 51 篇/共115)。

[2026-09-16] ingest | 美化+补全 [[自生长本地AI知识库]]：据 Bilibili(Xuan_酱《Codex 联动 Obsidian 搭卡帕西同款知识库》)补 A/B/C/D 四层闭环+速查表+映射本 vault+延伸阅读；新增 [[karpathy]]/[[Obsidian]] 双链页，登记 index(007-速查 4 篇/未归类)。
