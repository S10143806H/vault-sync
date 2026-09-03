# SurfaceFlinger AOSP Fuzzer 测试文档

## 目标

对 SurfaceFlinger(SF)执行覆盖引导的模糊测试,验证其在畸形/随机输入下的内存安全与健壮性。使用 AOSP 自带的 libFuzzer 目标,在 HWASAN 插桩下运行,捕获堆越界、释放后使用(UAF)、double-free 等内存破坏;对命中的崩溃产出可复现、可最小化的输入,供定位与上报。

覆盖三种运行形态:

- **Host 端** —— x86_64 二进制,无需真机,速度最快,用于日常回归。
- **目标设备端** —— arm64 二进制,经真实 vendor 组件路径,用于上板确认。
- **CI 冒烟** —— pytest 包装,时限内无崩溃即通过,接入 GTMP 自动化。

## 架构

fuzzer 二进制把 SF 源码直接链入,在单进程内构造真实 SurfaceFlinger 对象并调用其 API;HWComposer / RenderEngine / Gralloc HAL 全部替换为 Mock,因此无需真实显示或 GPU。

```
随机字节 (libFuzzer 变异)
   │
   ▼
FuzzedDataProvider ── 字节翻译为一串 SF 操作(display/layer/transaction/power/capture ...)
   │
   ▼
真 SurfaceFlinger 代码 (in-process)  ──  HWComposer / RenderEngine / Gralloc = Mock
   │
   ▼
HWASAN 插桩(每次内存访问检查)
   │
   ├── 无异常 → libFuzzer 依覆盖率(cov/ft)保留有价值输入进 corpus,继续变异
   └── 命中   → 落 crash-<sha1> + 打印 SUMMARY: <sanitizer 类型> + 栈
```

关键性质:白盒、in-process、覆盖引导、Mock HAL。相比经 Binder 的黑盒 `service call`,可深入 SF 内部解析、状态机与算术路径。

## 配置矩阵

### Fuzzer 目标(源自 `frameworks/native/services/surfaceflinger/fuzzer/Android.bp`)

| 目标 | 输入语义 | 主要覆盖 |
|---|---|---|
| `surfaceflinger_fuzzer` | 顶层 SF API 调用序列 | 事务 / 状态机整体 |
| `surfaceflinger_displayhardware_fuzzer` | HWComposer 命令 / display 配置 | 显示硬件抽象层解析 |
| `surfaceflinger_scheduler_fuzzer` | VSync 时间戳 / 刷新率序列 | 调度器算术与时序 |
| `surfaceflinger_layer_fuzzer` | Layer 属性 / 几何 / buffer | 图层状态计算 |
| `surfaceflinger_frametracer_fuzzer` | 帧时间线事件 | 帧追踪越界 / 泄漏 |

### 构建 / 运行参数

| 项 | 值 | 说明 |
|---|---|---|
| 插桩 | `SANITIZE_TARGET=hwaddress` | HWASAN;捕获内存破坏 |
| Host 产物 | `$ANDROID_HOST_OUT/fuzz/x86_64/<target>/` | 二进制 + seed corpus |
| 设备产物 | `$OUT/data/fuzz/arm64/<target>/` | `adb sync data` 同步至 `/data/fuzz/arm64/` |
| 时限 | `-max_total_time=<秒>` | 冒烟 120–300;深测数小时以上 |
| 统计 | `-print_final_stats=1` | 末尾打印 cov / exec/s |
| 崩溃落盘 | `-artifact_prefix=<dir>/` | 指定 crash 文件目录 |
| 最小化 | `-minimize_crash=1 -runs=100000` | 缩小复现输入 |

### 用例环境变量(CI 包装)

| 变量 | 默认 | 说明 |
|---|---|---|
| `SF_FUZZ_TARGET` | `surfaceflinger_fuzzer` | 跑哪个 target |
| `SF_FUZZ_TIME` | `120` | 时限秒数 |

## 推荐操作流程

> 编译只在放源码树的 Linux 构建机上执行;Windows 映射盘仅供浏览,Soong / `m` 不在 Windows 下工作。

### 1. 编译(Linux 构建机)

**① 补 `Android.bp` 的 srcs(必需)** —— stock 树 `surfaceflinger_fuzz_defaults` 的 `srcs` 缺 `libsurfaceflinger_multiplevsync_sources`,不补会编译失败。改为:

```
cc_defaults {
    name: "surfaceflinger_fuzz_defaults",
    ...
    srcs: [
        ":libsurfaceflinger_sources",
        ":libsurfaceflinger_multiplevsync_sources",   // ← 新增
        ":libsurfaceflinger_mock_sources",
    ],
    ...
}
```
文件:`frameworks/native/services/surfaceflinger/fuzzer/Android.bp`。

**② 清 fuzzer intermediates 后编译(product 名用 `guav100`)**:

```bash
source build/envsetup.sh
lunch guav100-userdebug
rm -rf out/soong/.intermediates/frameworks/native/services/surfaceflinger/fuzzer
SANITIZE_TARGET=hwaddress m surfaceflinger_fuzzer
```

> 改了 `Android.bp` 的 srcs 后必须先 `rm -rf` 该 intermediates,否则 Soong 缓存旧依赖图,报错依旧。
> 其余 4 个 target(`displayhardware/scheduler/layer/frametracer`)同一命令追加 target 名一起编。

产物校验:
```bash
ls $OUT/data/fuzz/arm64/surfaceflinger_fuzzer/surfaceflinger_fuzzer
```

### 2. Host 端冒烟

```bash
cd "$(mktemp -d)"
$ANDROID_HOST_OUT/fuzz/x86_64/surfaceflinger_fuzzer/surfaceflinger_fuzzer \
  -max_total_time=120 -print_final_stats=1 2>&1 | tee sf_fuzzer_host.log
```

### 3. 目标设备端

```bash
adb -s <设备号> root && adb -s <设备号> remount
adb -s <设备号> sync data
adb -s <设备号> shell " \
  cd /data/local/tmp && \
  /data/fuzz/arm64/surfaceflinger_fuzzer/surfaceflinger_fuzzer \
    /data/fuzz/arm64/surfaceflinger_fuzzer/corpus \
    -max_total_time=300 -print_final_stats=1 -artifact_prefix=/data/local/tmp/" \
  2>&1 | tee sf_fuzzer_device.log
```

### 4. 崩溃分诊(命中时)

```bash
# 复现
<fuzzer 二进制> ./crash-<sha1>
# 最小化
<fuzzer 二进制> -minimize_crash=1 -runs=100000 ./crash-<sha1>
```

记录:target、SUMMARY 类型、首帧栈、复现命令、最小化输入路径,按流程提缺陷。

### 5. CI 冒烟(pytest)

```bash
SF_FUZZ_TIME=60 pytest \
  cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py \
  --serial <设备号> -v
```

## 脚本附件

| 文件 | 用途 |
|---|---|
| `cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py` | 真机时限跑 + 断言无 crash 的 pytest 用例 |
| `common/Gpu/gfwk_stress_util.py` → `parse_libfuzzer_result()` | 解析 libFuzzer 输出(crashed / reason / execs / cov) |
| `common/Gpu/test_sf_fuzz_ci.py` | 上述解析函数的离线单测 |

> 实施细节见计划文档 `docs/superpowers/plans/2026-08-13-sf-aosp-fuzzer.md`。

## 验证方法

| 检查 | 通过判据 |
|---|---|
| fuzzer 真正运行 | 末尾输出含 `cov: <N>`;`cov` / `ft` 随时间增长 |
| 无崩溃 | 退出码 0;运行目录无 `crash-* / oom-* / timeout-*` |
| 无内存破坏 | 输出无 `SUMMARY: HWAddressSanitizer:` |
| CI 冒烟 | 用例 PASS,日志 `crashed=False cov=<N>` |

## 故障定位

| 观察 | 判断 | 操作 |
|---|---|---|
| `m` 报缺 `libXXX` | target 依赖未编到 | 单独 `m <target>` 复现,补依赖或记录跳过该 target |
| 无 `cov` 输出、秒退 | 二进制未跑起来 / 参数错 | 检查 corpus 路径与 `-max_total_time`;确认非 HWASAN 加载失败 |
| `crash-<sha1>` 落盘 | 命中内存 bug | 转分诊:复现 + 最小化 + 读 SUMMARY 栈 |
| 复现不稳定 | 疑似并发 / 时序相关 | 标注 flaky,多次复现取交集,记录环境 |
| 设备端 `sync data` 后找不到二进制 | 未 remount / 未编 arm64 | `adb remount`;确认 `$OUT/data/fuzz/arm64/<target>` 存在后重同步 |
| pytest 直接 skip | 未 `adb sync data` | 先在构建机编译并同步,再跑用例 |

## 说明与限制

- Mock HAL 覆盖 SF 自身逻辑,不覆盖真实驱动 / vendor HAL 内部缺陷;后者需设备端跑 + 真实合成负载另行验证。
- 覆盖引导依赖 seed corpus 质量;corpus 越贴近真实事务,深层路径命中越快。
- `frametracer` 为本源码树变体(非上游 `frontend`),目标集合以本树 `Android.bp` 为准。
- 黑盒健壮性方案(不依赖源码,经 `service call` 注入)见 `docs/superpowers/plans/2026-08-13-sf-blackbox-fuzzer.md`,作为拿不到源码时的补充手段。
