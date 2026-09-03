# SurfaceFlinger AOSP Fuzzer(方案 A · 依赖源码)实施计划

> **给自动化执行者:** 必用子技能:用 superpowers:subagent-driven-development(推荐)或 superpowers:executing-plans 按任务逐个实施。步骤用复选框(`- [ ]`)跟踪。

**目标:** 编译 AOSP 自带的 5 个 SurfaceFlinger fuzzer(覆盖引导 + HWASAN),在 host 端与真机上跑,把崩溃复现/最小化成可上报的 finding;最后用 pytest 包一层做 CI 冒烟(时限内无 crash 即通过)。

**架构:** 四层。①**编译层** —— Soong `m` 出 fuzzer 二进制,HWASAN 插桩。②**执行层** —— host 端 libFuzzer 直接跑(快、无需真机),或 `adb sync data` 推真机 `/data/fuzz` 跑(测真实 vendor HAL 路径)。③**分诊层** —— 崩溃落 `crash-<sha1>`,用同一二进制复现 + `-minimize_crash` 最小化 + HWASAN 栈定位。④**CI 包装层(可选)** —— pytest 用例在真机上跑 `-max_total_time` 并断言 exit code 0 且无 crash 文件;其纯日志解析逻辑离线单测。

**技术栈:** AOSP Soong / `m`、libFuzzer、HWASAN(`SANITIZE_TARGET=hwaddress`)、adb、Python 3.12 + pytest。

**规格来源:** 需求源自本会话;fuzzer 目标以台架源码树 `Android.bp` 实测为准(5 个 `cc_fuzz`,componentid 155276)。

## 全局约束

- **编译只在放这棵树的 Linux 构建机上跑**;`Z:\...\aaos` 是 Windows 映射盘,仅供浏览,Soong/`m` 不在 Windows 下工作。
- 5 个 target 名固定(来自 `frameworks/native/services/surfaceflinger/fuzzer/Android.bp`):
  `surfaceflinger_fuzzer`、`surfaceflinger_displayhardware_fuzzer`、`surfaceflinger_scheduler_fuzzer`、`surfaceflinger_layer_fuzzer`、`surfaceflinger_frametracer_fuzzer`。
- host 产物路径 `$ANDROID_HOST_OUT/fuzz/x86_64/<target>/`;设备产物 `$OUT/data/fuzz/<arch>/<target>/`。
- 崩溃复现命令 = `<fuzzer 二进制> <crash 文件>`;最小化 = `<fuzzer> -minimize_crash=1 -runs=100000 <crash 文件>`。
- pytest 包装(任务 5)设备访问只用 `aw._adb_output` / `aw.adb_shell`;复用 `common/Gpu/gfwk_stress_util.py`(见 [[gfwk-reuse-common-first]])。
- 目标台架 A41AEC42(SG2086),arm64。

---

### 任务 1: 编译 5 个 fuzzer(Linux 构建机)

**涉及文件:** 无(命令操作);产物在 `$OUT` / `$ANDROID_HOST_OUT`。

- [ ] **步骤 1: 初始化构建环境 + 补 Android.bp**

在构建机的 aaos 树根:
```bash
source build/envsetup.sh
lunch guav100-userdebug     # product 名是 guav100(无下划线); echo $TARGET_PRODUCT 确认
```
预期: `echo $TARGET_PRODUCT` 非空,`$OUT` 指向 `out/target/product/<device>`。

必需改 `frameworks/native/services/surfaceflinger/fuzzer/Android.bp`,给
`surfaceflinger_fuzz_defaults` 的 `srcs` 补 `":libsurfaceflinger_multiplevsync_sources"`
(位于 `libsurfaceflinger_sources` 与 `libsurfaceflinger_mock_sources` 之间),否则编译失败。

- [ ] **步骤 2: 清 intermediates 后编译顶层 fuzzer(HWASAN)**

```bash
rm -rf out/soong/.intermediates/frameworks/native/services/surfaceflinger/fuzzer
SANITIZE_TARGET=hwaddress m surfaceflinger_fuzzer
```
> 改了 Android.bp srcs 后必须先 `rm -rf` 该 intermediates,否则 Soong 用旧依赖图,报错依旧。

预期: `#### build completed successfully ####`;产物存在:
```bash
ls -l $OUT/data/fuzz/arm64/surfaceflinger_fuzzer/surfaceflinger_fuzzer
ls    $ANDROID_HOST_OUT/fuzz/x86_64/surfaceflinger_fuzzer/ 2>/dev/null
```

- [ ] **步骤 3: 编译其余 4 个 fuzzer**

```bash
SANITIZE_TARGET=hwaddress m \
  surfaceflinger_displayhardware_fuzzer \
  surfaceflinger_scheduler_fuzzer \
  surfaceflinger_layer_fuzzer \
  surfaceflinger_frametracer_fuzzer
```
预期: 全部 build success;逐个 `ls $OUT/data/fuzz/arm64/<target>/<target>` 存在。
若某 target 缺依赖报错: 记录 target 名与首条 error,单独 `m <target>` 复现,不阻塞其它 target。

---

### 任务 2: host 端冒烟跑(快,无需真机)

**涉及文件:** 无。

- [ ] **步骤 1: 顶层 fuzzer 短跑**

```bash
cd $(mktemp -d)      # 崩溃文件落当前目录, 用临时目录隔离
$ANDROID_HOST_OUT/fuzz/x86_64/surfaceflinger_fuzzer/surfaceflinger_fuzzer \
  -max_total_time=120 -print_final_stats=1 2>&1 | tee sf_fuzzer_host.log
```
预期: 末尾 `DONE ... cov: <N> ft: <M> exec/s: <K>`;退出码 0;当前目录**无** `crash-*` 文件。
`cov`/`ft` 随时间增长 = 覆盖引导生效。

- [ ] **步骤 2: 其余 4 个各短跑 60s**

对 `displayhardware/scheduler/layer/frametracer` 各跑:
```bash
$ANDROID_HOST_OUT/fuzz/x86_64/<target>/<target> -max_total_time=60 -print_final_stats=1 \
  2>&1 | tee <target>_host.log
```
预期: 各自退出码 0、无 `crash-*`。
若命中 crash: 保留 `crash-<sha1>` 与日志 → 转任务 4 分诊(这是真 finding)。

---

### 任务 3: 真机跑(测真实 vendor HAL 路径)

**涉及文件:** 无。

- [ ] **步骤 1: 推送 fuzzer + corpus 到设备**

```bash
adb -s <设备号> root && adb -s <设备号> remount
adb -s <设备号> sync data          # 同步 $OUT/data → /data, 含 /data/fuzz/arm64/*
adb -s <设备号> shell ls /data/fuzz/arm64/surfaceflinger_fuzzer/
```
预期: 设备上列出二进制 + `corpus/` + `seed_corpus`(若有)。

- [ ] **步骤 2: 真机跑顶层 fuzzer 时限跑**

```bash
adb -s <设备号> shell " \
  cd /data/local/tmp && \
  /data/fuzz/arm64/surfaceflinger_fuzzer/surfaceflinger_fuzzer \
    /data/fuzz/arm64/surfaceflinger_fuzzer/corpus \
    -max_total_time=300 -print_final_stats=1 -artifact_prefix=/data/local/tmp/" \
  2>&1 | tee sf_fuzzer_device.log
```
预期: `DONE ... cov/ft` 增长,退出码 0,`/data/local/tmp/` 无 `crash-*`。
崩溃则 crash 文件落 `/data/local/tmp/crash-<sha1>` → 任务 4。

---

### 任务 4: 崩溃分诊(命中崩溃时)

**涉及文件:** finding 记录(自定,建议 `docs/findings/sf-fuzz-<sha1>.md`)。

- [ ] **步骤 1: 复现**

```bash
# host:
$ANDROID_HOST_OUT/fuzz/x86_64/<target>/<target> ./crash-<sha1>
# 或真机:
adb shell /data/fuzz/arm64/<target>/<target> /data/local/tmp/crash-<sha1>
```
预期: 稳定重现同一 HWASAN/ASAN 报告(`==ERROR:`、`SUMMARY:`、栈)。不稳定 = 疑似并发/时序,单独标注。

- [ ] **步骤 2: 最小化**

```bash
<fuzzer 二进制> -minimize_crash=1 -runs=100000 ./crash-<sha1>
```
预期: 产出更小的 `minimized-from-<sha1>`,复现同一栈。

- [ ] **步骤 3: 落 finding**

记录: target、HWASAN 报告首帧栈、`SUMMARY` 类型(heap-buffer-overflow / use-after-free / …)、复现命令、最小化输入路径。按团队流程提 Meego 缺陷。

---

### 任务 5(可选): pytest CI 包装 —— 真机时限跑 + 断言无 crash

**涉及文件:**
- 修改: `common/Gpu/gfwk_stress_util.py`(加纯函数 `parse_libfuzzer_result`)
- 测试: `common/Gpu/test_sf_fuzz_ci.py`(离线单测)
- 新建: `cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py`(真机 CI 冒烟)

**接口:**
- 产出: `parse_libfuzzer_result(stdout: str, exit_code: int, crash_files: list[str]) -> dict` —— 返回 `{"crashed": bool, "reason": str, "execs": int|None, "cov": int|None}`。

- [ ] **步骤 1: 写失败单测**

`common/Gpu/test_sf_fuzz_ci.py`:
```python
# -*- coding: utf-8 -*-
"""离线单测: libFuzzer 结果解析。运行: pytest common/Gpu/test_sf_fuzz_ci.py -v"""
from common.Gpu import gfwk_stress_util as gfwk

CLEAN = "#131072 DONE   cov: 4521 ft: 11033 corp: 512/40Kb exec/s: 900\nDone 131072 runs"
CRASH = ("==12345==ERROR: HWAddressSanitizer: heap-use-after-free\n"
         "SUMMARY: HWAddressSanitizer: heap-use-after-free sf.cpp:42\n")


def test_clean_run_not_crashed():
    r = gfwk.parse_libfuzzer_result(CLEAN, 0, [])
    assert r["crashed"] is False
    assert r["cov"] == 4521 and r["execs"] == 131072


def test_nonzero_exit_is_crash():
    r = gfwk.parse_libfuzzer_result(CRASH, 1, ["crash-abc"])
    assert r["crashed"] is True
    assert "heap-use-after-free" in r["reason"]


def test_crash_file_alone_is_crash():
    r = gfwk.parse_libfuzzer_result(CLEAN, 0, ["crash-def"])
    assert r["crashed"] is True
    assert "crash 文件" in r["reason"]
```

- [ ] **步骤 2: 跑单测确认失败**

运行: `pytest common/Gpu/test_sf_fuzz_ci.py -v`
预期: 失败 —— `AttributeError: ... 'parse_libfuzzer_result'`。

- [ ] **步骤 3: 最小实现**

在 `common/Gpu/gfwk_stress_util.py` 末尾追加:
```python
import re as _re


def parse_libfuzzer_result(stdout, exit_code, crash_files):
    """解析 libFuzzer 一次跑的结果(纯函数, 便于离线测)。
    crashed 判据: 非零退出 / 出现 crash 文件 / stdout 含 sanitizer ERROR。"""
    m_cov = _re.search(r"cov:\s*(\d+)", stdout or "")
    m_run = _re.search(r"#(\d+)\s+DONE", stdout or "")
    san = _re.search(r"SUMMARY:\s*\w*Sanitizer:\s*(.+)", stdout or "")
    reason = ""
    crashed = False
    if crash_files:
        crashed = True
        reason = "发现 crash 文件: %s" % ", ".join(crash_files)
    if san:
        crashed = True
        reason = (reason + "; " if reason else "") + san.group(1).strip()
    if exit_code != 0:
        crashed = True
        reason = reason or ("非零退出码 %d" % exit_code)
    return {
        "crashed": crashed,
        "reason": reason,
        "execs": int(m_run.group(1)) if m_run else None,
        "cov": int(m_cov.group(1)) if m_cov else None,
    }
```

- [ ] **步骤 4: 跑单测确认通过**

运行: `pytest common/Gpu/test_sf_fuzz_ci.py -v`
预期: 通过(3 passed)。

- [ ] **步骤 5: 写真机 CI 用例**

创建 `cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py`:
```python
# -*- coding: utf-8 -*-
# copyright: 2026, XG Tech.Inc. All rights reserved.
"""
===============================================================================
用例编号:   TC_SF_FUZZER_001
用例标题:   SurfaceFlinger AOSP libFuzzer 真机时限冒烟(无 crash)
创建人员:   qi.zhu   创建时间: 2026.08.13   最后修改: 2026.08.13
优先级:     P1    维度: 故障注入/覆盖引导 fuzz(D3)
#REQUIRE:adb,car0
===============================================================================
前置条件:
    1. 已 `SANITIZE_TARGET=hwaddress m surfaceflinger_fuzzer` 并 `adb sync data`
    2. adb 可 root; 运行须带 --serial <设备号>
后置条件:
    1. fuzzer 正常退出, 无 crash 文件残留
测试步骤:
    1. 确认 /data/fuzz/arm64/<target>/ 存在
    2. 跑 <target> -max_total_time=SF_FUZZ_TIME, artifact 落 /data/local/tmp
    3. parse_libfuzzer_result 判 crashed; 收集 crash-* 文件路径
预期结果:
    退出码 0, 无 sanitizer ERROR, 无 crash-* 文件, cov 有值(fuzzer 真跑起来)
校准记录:
    1. 崩溃即保留 crash 文件路径 + SUMMARY 行, 供 host 复现/最小化
===============================================================================
"""
import os
import pytest
from gta import BootInfo
from common.Gpu import gfwk_stress_util as gfwk

TARGET = os.environ.get("SF_FUZZ_TARGET", "surfaceflinger_fuzzer")
FUZZ_TIME = int(os.environ.get("SF_FUZZ_TIME", "120"))
FDIR = f"/data/fuzz/arm64/{TARGET}"
ART = "/data/local/tmp"


@pytest.fixture(scope="function")
def sf_aw(init_gpu_aw):
    aw = init_gpu_aw
    aw.adb_root(aw.dut.car0)
    yield aw


def TC_SF_FUZZER_001(sf_aw, info: BootInfo):
    aw = sf_aw
    listing = aw._adb_output(f"ls {FDIR}/{TARGET} 2>/dev/null").strip()
    if not listing:
        pytest.skip(f"{FDIR}/{TARGET} 不存在: 先在构建机 "
                    f"`SANITIZE_TARGET=hwaddress m {TARGET}` 再 `adb sync data`")
    aw._adb_output(f"rm -f {ART}/crash-* {ART}/oom-* {ART}/timeout-* 2>/dev/null")
    info.logger.info(f"=== TC_SF_FUZZER_001 {TARGET} -max_total_time={FUZZ_TIME}s ===")
    cmd = (f"cd {ART} && {FDIR}/{TARGET} {FDIR}/corpus "
           f"-max_total_time={FUZZ_TIME} -print_final_stats=1 "
           f"-artifact_prefix={ART}/ ; echo EXIT=$?")
    out = aw._adb_output(cmd)
    ec_line = [l for l in out.splitlines() if l.startswith("EXIT=")]
    exit_code = int(ec_line[-1].split("=")[1]) if ec_line else 1
    crash_files = [l for l in
                   aw._adb_output(f"ls {ART}/crash-* {ART}/oom-* {ART}/timeout-* "
                                  f"2>/dev/null").splitlines() if l.strip()]
    r = gfwk.parse_libfuzzer_result(out, exit_code, crash_files)
    info.logger.info(f"结果: crashed={r['crashed']} cov={r['cov']} "
                     f"execs={r['execs']} reason={r['reason']}")
    assert r["cov"] is not None, f"fuzzer 未真正跑起来(无 cov 输出):\n{out[-500:]}"
    assert not r["crashed"], (f"SF fuzz 命中崩溃: {r['reason']}; "
                              f"crash 文件={crash_files}(host 复现: {TARGET} <crash>)")
```

- [ ] **步骤 6: 静态检查 + 冒烟跑**

```bash
python -m py_compile cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py
SF_FUZZ_TIME=60 pytest cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py --serial <设备号> -v
```
预期: 通过,日志 `crashed=False cov=<N>`。未 `adb sync data` 时自动 skip(附提示)。

- [ ] **步骤 7: 提交**

```bash
git add common/Gpu/gfwk_stress_util.py common/Gpu/test_sf_fuzz_ci.py \
        cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py
git commit -m "SF fuzzer(方案A): AOSP libFuzzer 真机 CI 包装 + 结果解析纯函数与单测"
```

---

### 任务 6: 文档

- [ ] **步骤 1: README 目录行**

`cases/MultiMedia/GFWK/README.md` 加行:
```
| `SurfaceFlinger/Fuzzer/` | TC_SF_FUZZER_001 | AOSP libFuzzer 真机 CI 冒烟(5 target: sf/displayhardware/scheduler/layer/frametracer) |
```

- [ ] **步骤 2: vault 教学页**

创建 `/home/gua/vault_import2/AI赋能/wiki/005-稳定性/TC_SF_FUZZER_001.md`,用 `TC_SERDES_FAULT_002.md` 结构。核心: ①一句话(覆盖引导 in-process fuzz)②原理(mock HAL 跑真 SF 代码)③5 target 分工表 ④编译/跑/分诊三步 ⑤崩溃判据表 ⑥与黑盒方案 B 对照。更新 `index.md`(005 +1)、`log.md`。

- [ ] **步骤 3: 提交(仓库侧)**

```bash
git add cases/MultiMedia/GFWK/README.md
git commit -m "SF fuzzer(方案A): README 目录加 Fuzzer 行"
```

---

## 自查

- **5 target 覆盖:** 任务 1 全编译,任务 2/3 全跑,target 名与台架 `Android.bp` 一致(sf/displayhardware/scheduler/layer/frametracer)。✓
- **占位符:** 无 TBD;命令与代码均具体。✓
- **类型一致:** `parse_libfuzzer_result(stdout, exit_code, crash_files) -> dict{crashed,reason,execs,cov}` 在任务 5 单测与用例中签名一致。✓
- **平台约束:** 明确编译在 Linux 构建机、Windows Z 盘仅浏览。✓
- **fallback:** 黑盒方案 B 见 `docs/superpowers/plans/2026-08-13-sf-blackbox-fuzzer.md`(拿不到源码/不想编译时用)。
