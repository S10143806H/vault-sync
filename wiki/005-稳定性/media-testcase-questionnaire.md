# 媒体用例结果填报问卷

## 目标

统一收集台架与实车两轮媒体测试的用例执行结果。一次提交 = 一条用例，
避免单次提交承载全部用例导致的填写量过大与附件上传失败丢数据。

采集字段：测试次数、用例测试结果、测试问题、用例拍照结果。

## 用例来源

| 章节 | 模块 | 条数 |
|---|---|---:|
| 台架验证 | BPU基础用例 | 10 |
| 台架验证 | 显示图形栈基础用例 | 19 |
| 台架验证 | 音频测试用例 | 2 |
| 台架验证 | Camera台架用例 | 9 |
| 台架验证 | Video台架用例 | 2 |
| 实车验证 | Camera/BPU实车用例 | 10 |
| 实车验证 | Display/GPU/GFWK实车用例 | 5 |
| 实车验证 | Video实车用例 | 2 |
| 实车验证 | Audio实车用例 | 7 |
| | **合计** | **66** |

用例清单由脚本从 wiki 需求文档解析生成，非手工维护。

## 题目结构

共 19 题，分 3 页。

| 页 | 题号 | 内容 |
|---|---|---|
| 1 | Q1–Q4 | 验证人、车辆信息、测试日期、软件版本 |
| 2 | Q5–Q15 | 章节、模块、用例编号 |
| 3 | Q16–Q19 | 测试次数、测试结果、问题、拍照 |

## 显示逻辑

文本导入不携带逻辑，需在问卷星界面配置以下 12 条。

| 题号 | 题目 | 显示条件 |
|---|---|---|
| Q6 | 测试模块 | Q5=台架验证 → 仅前 5 项；Q5=实车验证 → 仅后 4 项 |
| Q7 | 用例编号（BPU基础用例） | Q6 =「BPU基础用例」 |
| Q8 | 用例编号（显示图形栈基础用例） | Q6 =「显示图形栈基础用例」 |
| Q9 | 用例编号（音频测试用例） | Q6 =「音频测试用例」 |
| Q10 | 用例编号（Camera台架用例） | Q6 =「Camera台架用例」 |
| Q11 | 用例编号（Video台架用例） | Q6 =「Video台架用例」 |
| Q12 | 用例编号（Camera/BPU实车用例） | Q6 =「Camera/BPU实车用例」 |
| Q13 | 用例编号（Display/GPU/GFWK实车用例） | Q6 =「Display/GPU/GFWK实车用例」 |
| Q14 | 用例编号（Video实车用例） | Q6 =「Video实车用例」 |
| Q15 | 用例编号（Audio实车用例） | Q6 =「Audio实车用例」 |
| Q18 | 测试问题 | Q17 ∈ 失败 / 阻塞 / 未测 |
| Q19 | 用例拍照结果 | Q17 ∈ 失败 / 阻塞 / 未测 |

## 导入流程

1. 问卷星 → 新建问卷 → 文本导入
2. 粘贴 `wjx_import.txt` 全文并导入
3. 修正题型：Q3 改为日期题，Q19 改为文件上传题（文本导入不支持这两种题型）
4. 按上表配置 12 条显示逻辑
5. 按上表设置分页

## 题目全文

```text
1、验证人名字【填空题】

2、车辆信息（车型 / 车牌 / VIN 后六位）【填空题】

3、测试日期【填空题】    ※导入后在界面改成【日期题】

4、验证软件版本【填空题】

5、测试章节【单选题】
台架验证
实车验证

6、测试模块【单选题】
BPU基础用例
显示图形栈基础用例
音频测试用例
Camera台架用例
Video台架用例
Camera/BPU实车用例
Display/GPU/GFWK实车用例
Video实车用例
Audio实车用例

7、用例编号（BPU基础用例）【下拉框】
test_tast_resource_001 · push 测试资源
TC_AI_BPU_ADAS_Bsp_Func_015_test · BPU输出一致性比对测试
TC_AI_BPU_ADAS_Drv_Func_007 · 设置BPU工作频点
TC_AI_BPU_ADAS_Drv_Func_044 · BPU核上下电压测
TC_AI_BPU_ADAS_Perf_001 · BPU benchmark性能看护
TC_AI_BPU_IVI_HAL_Func_006_test · 代理方案测试套
TC_AI_BPU_ADAS_Drv_Func_018 · 模型推理过程中执行reboot 3OS
TC_AI_BPU_ADAS_Drv_Func_019 · 模型推理过程中执行reboot mainreboot
TC_AI_BPU_ADAS_Drv_Func_037 · BPU休眠唤醒测试
BPU-BASE-TEST-009 · bpu哨兵综合场景 TC_SENTINEL_XG1_Func_050

8、用例编号（显示图形栈基础用例）【下拉框】
GFWK-BASE-TEST-001 · 投屏GIPC通道断裂注错测试
GFWK-BASE-TEST-002 · 显示合成策略稳定压力测试
GFWK-BASE-TEST-003 · 图形栈异常注入-上层服务异常测试
GFWK-BASE-TEST-004 · 性能用例监控用例
GFWK-BASE-TEST-005 · 背光压力测试
GFWK-BASE-TEST-006 · 后排屏开合测试 (开关流/背光压力测试）
GFWK-BASE-TEST-007 · GWDT卡死检测
GFWK-BASE-TEST-007#2 · Weston 开关流压力测试
GFWK-BASE-TEST-007#3 · IVI 开关流压力测试
GPU-BASE-TEST-001 · IVI GLES渲染
GPU-BASE-TEST-002 · Cluster GLES渲染
GPU-BASE-TEST-003 · GPUA上下电测试
GPU-BASE-TEST-004 · GPUB上下电测试
GPU-BASE-TEST-005 · IVI ShaderCache功能测试
GPU-BASE-TEST-006 · Gralloc基础操作验证
DISPLAY-BASE-TEST-001 · DRM/KMS基础操作验证
DISPLAY-BASE-TEST-002 · 屏开关显示恢复
DISPLAY-BASE-TEST-003 · 休眠唤醒显示恢复
DISPLAY-BASE-TEST-004 · SERDES链路状态监控

9、用例编号（音频测试用例）【下拉框】
AUDIO-BASE-TEST-001 · 维测：音频数据dump（adsp）
AUDIO-BASE-TEST-002 · 音源&焦点测试&音量调节

10、用例编号（Camera台架用例）【下拉框】
TC_Camera_ASIC_Inject_001 · 多摄预览场景下IVI重启 adb reboot osreboot
TC_Camera_ASIC_Inject_002 · 多摄预览场景下ADAS重启
TC_Camera_ASIC_Inject_003 · 多摄预览场景下整机重启，cp0 reboot
TC_Camera_ASIC_Inject_004 · 多摄预览场景下整机下电
TC_Camera_ASIC_Inject_007 · 多摄预览场景下hypervisor重启
TC_Camera_ASIC_Inject_008 · 多摄预览场景下mainreboot，cp1 mainreboot
TC_Camera_FaultInject_014 · 多摄预览场景下KL15休眠唤醒
TC_Camera_FaultInject_017 · AVM预览场景下KL15休眠唤醒
TC_Camera_FaultInject_017#2 · 双滚轮重启

11、用例编号（Video台架用例）【下拉框】
Video-FUNC-TEST-001 · Video 基础功能测试: （ADAS VENC, VDEC, ADAS JENC, JDEC, IVI MMI Dec
Video-STAB-TEST-001 · Video 杀进程功能恢复测试 Video 重启功能恢复测试 Video STR/上下电功能恢复测试

12、用例编号（Camera/BPU实车用例）【下拉框】
Camera-TEST-001 · 系统启动，智驾SR渲染周边正常
Camera-TEST-002 · 系统启动，AVM出图正常
Camera-TEST-003 · 系统启动，DVR出图正常
Camera-TEST-004 · 稳定性测试1
Camera-TEST-005 · 稳定性测试2
Camera-TEST-006 · 极致节能测试
Camera-TEST-009-V27 · 哨兵模式
Camera-TEST-009-T29（未交付） · 哨兵模式（未交付）
Camera-TEST-010 · 实车智驾30分钟（CP1路线）
Camera-TEST-011-T29 · T29 人驾 - DMS基础功能点检

13、用例编号（Display/GPU/GFWK实车用例）【下拉框】
V27 · Display-TEST-001 · 各屏背光调节
V27 · Display-TEST-002 · 吸顶屏开合测试
V27 · Display-TEST-004 · 水印注入冻帧
T29 · Display-TEST-001 · 各屏背光调节
T29 · Display-TEST-002 · 吸顶、空调屏开关测试

14、用例编号（Video实车用例）【下拉框】
Video-TEST-001 · 实车 IVI 侧视频播放录制功能验证
Video-TEST-002 · 实车 ADAS 侧 Datamask Encoder 图像验证

15、用例编号（Audio实车用例）【下拉框】
Audio-TEST-001 · 转向灯、智驾提示音验证、挡位切换
Audio-TEST-002 · 语音交互验证
Audio-TEST-003 · 无麦K歌测试
Audio-TEST-004 · 蓝牙电话
Audio-TEST-005 · AVAS测试
Audio-TEST-006 · 车外喊话
Audio-TEST-007 · 头枕音区

16、本条用例实际执行的次数【填空题】

17、用例测试结果【单选题】
通过
失败
阻塞
未测

18、测试问题：现象与复现步骤【填空题】

19、用例拍照结果（建议不超过 3 张）【填空题】    ※导入后在界面改成【文件上传题】

```

## 脚本附件

| 文件 | 用途 |
|---|---|
| `gen_wjx.py` | 从用例数据源生成 `wjx_import.txt` 与本文档 |
| `sync_cases.py` | wiki 文档 → 用例清单表，按同步键幂等 upsert |
| `sync_rc_list.py` | wiki 文档第 3 章 → 实车用例清单表 |
| `sync_progress.py` | 测试记录 → 用例清单回写 + 进度汇总重建 |

用例增删后重跑 `gen_wjx.py`，并将新的 `wjx_import.txt` 重新导入问卷星。

## 验证方法

| 检查项 | 方法 |
|---|---|
| 选项条数 | 各模块下拉项数与「用例来源」表一致 |
| 编号唯一 | 同一模块内无重复用例编号 |
| 逻辑生效 | 测试结果选「通过」时，测试问题与拍照两题不出现 |
| 模块联动 | 切换测试模块后，仅对应模块的用例编号题出现 |

## 已知限制

- 用例选项在导入时固化，需求文档新增用例后必须重新生成并导入，问卷不会自动更新。
- 题号随用例增减保持不变（模块数固定），但逻辑条件需复核。
- 若问卷星不支持按上一题动态过滤本题选项，将 Q5 与 Q6 合并为单题九选一。
- 部分用例编号在需求文档中重复，脚本以 `#2` `#3` 后缀区分，非文档原文编号。
