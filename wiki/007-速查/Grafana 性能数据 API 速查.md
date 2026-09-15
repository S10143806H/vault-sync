---
title: "Grafana 性能数据 API 速查 (02-性能主看板)"
tags: [grafana, prometheus, 速查, 性能, API]
platform: "gua / guav100 (AAOS)"
source: "http://grafana-k8s.guasemi.com:32080/d/e0a81fbb-910c-4e4f-913e-ada645962550"
created: 2026-09-02
---
> 从 `02-性能主看板` 直接用 HTTP API 拉性能数据(FPS/CPU/GPU/内存)。**匿名可访问,无需 token**;数据源是 Prometheus,查询用 PromQL。多设备用 `deviceID` 标签过滤(序列号大写,如 `A41AEC42`)。

## Definition

Grafana 是一个开源的监控数据可视化平台——把 **时序数据(性能指标、日志)拉成图表看板**。

| Layer | Name       | Usage                                   |
| ----- | ---------- | --------------------------------------- |
| 采集/存储 | Prometheus | 存台架性能指标(FPS/CPU/GPU/内存)                 |
| 拿数据   | HTTP API   | Grafana 代理转发 PromQL 给 Prometheus,返 JSON |
| 展示    | Grafana    | 从 Prometheus 查数据,画成看板(你看的「02-性能主看板」)    |


## 基础参数
```bash
B=http://grafana-k8s.guasemi.com:32080
DS=a9f6465f-f806-47d1-9184-c0f053fb477f          # Prometheus 数据源 UID
P="$B/api/datasources/proxy/uid/$DS/api/v1"      # 经 Grafana 代理直连 Prometheus
```

- 看板 UID:`e0a81fbb-910c-4e4f-913e-ada645962550`
- Grafana v10.2.3;`/api/health` 返 200 即可达



## ② 瞬时查询(最近 ~5min)
```bash
curl -s --data-urlencode 'query=sys_gpu_usage{deviceID="A41AEC42"}' "$P/query" | jq '.data.result'
```

## ③ 范围查询(历史,最常用)
```bash
END=$(date +%s); START=$((END-7*86400))          # 近 7 天
curl -s --data-urlencode 'query=sys_gpu_usage{deviceID="A41AEC42"}' \
  --data-urlencode "start=$START" --data-urlencode "end=$END" \
  --data-urlencode "step=3600" "$P/query_range" | jq '.data.result'
```
- `step` = 采样间隔秒(3600=每小时一点);窗口越长 step 调大
- 返回 `.data.result[].values` = `[[时间戳, 值], ...]`

## ④ 常用性能指标(PromQL)
| 指标                                                                 | 含义              |
| ------------------------------------------------------------------ | --------------- |
| `fps`                                                              | 帧率              |
| `sys_cpu_usage` / `sys_gpu_usage`                                  | 中控 CPU/GPU 算力 % |
| `sys_memory_available`                                             | 中控可用内存          |
| `process_cpu_usage` / `process_gpu_usage` / `process_memory_usage` | 单进程占用           |
| `cluster_sys_cpu_usage` / `cluster_sys_gpu_usage`                  | **仪表域** CPU/GPU |
| `cluster_process_cpu_usage` / `cluster_process_memory_usage`       | 仪表域单进程          |

聚合示例:`avg(avg_over_time(sys_gpu_usage{deviceID="A41AEC42"}[5m]))`

### 4.1 Add to Kill SF 

在 `gfwk_stress_util.py 加个 perf_snapshot(device, metrics, window) helper → kill SF 后调一次,只记录进 log,跑几轮看数据靠不靠谱,再决定要不要升级成断言。

| 指标                     | Rules                                                              |
| ---------------------- | ------------------------------------------------------------------ |
| `fps`                  | upon killing whether will be resumed 【back to 80% within baseline】 |
| `sys_gpu_usage`        | 是否卡死(0)或飙满(100) 【回到正常区间】                                           |
| `sys_memory_available` | 逐轮下降：内存泄漏 【不持续走低】                                                  |
| cpu_xxx                | compare with baseline, daily task                                  |

### 4.2 Problem
1. Timing: Prometheus scrape delay, maybe read adb device data will be in real-time
2. DB retry
3. /query_range or /query

## ⑤ 发现有哪些设备 / 指标
```bash
curl -s "$P/label/deviceID/values" | jq '.data'          # 全部设备序列号
curl -s "$P/label/__name__/values" | jq '.data'          # 全部指标名
```
- 测试机示例:`A41AEC42`(V27)、`a000025a`(T29)——均有数据

## ⑥ 注意
- **DB 间歇抽风**:偶发 `sqlstore.max-retries-reached` → HTTP 500,脚本里重试 2~3 次即可,数据本身正常
- 瞬时 `/query` 只返最近 ~5min;要历史一律 `/query_range`
- `deviceID` 用**大写**序列号
