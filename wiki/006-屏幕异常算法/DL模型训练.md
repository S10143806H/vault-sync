
现有件正好拼成标注生产线：

黑屏 skills.md 里的飞书问题单拉取流程（按关键词筛单→下载 mp4 附件→按 GUA 号建目录）扩展到五种异常关键词，定时同步到 `Y:\issue-sync\<类型>\`；

每个新样本自动过规则检测产出 evidence 拼图；你蓝圈复核（就像今天两轮）后，TP/FP 结论+bbox+时间区间写入统一的 `labels.jsonl`；同时 `regression_labels.json` 这类已复核基准直接并入。

**每张 evidence 拼图复核 ≈ 10-40 个带框标注帧**，照今天的节奏一两周就能攒够首批训练集（每类 300+ 区域）。到时候训练管线（帧级 MobileNetV3 判别器 + 规则粗筛级联）我直接在 Camera-Algorithm 里搭。

TODO： 建议下一步先把"问题单拉取 skill + labels.jsonl 汇聚脚本"做出来，让数据开始自动积累。