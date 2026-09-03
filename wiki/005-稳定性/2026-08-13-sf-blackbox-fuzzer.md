# SurfaceFlinger 黑盒 Fuzzer 用例 实施计划

> **给自动化执行者:** 必用子技能:用 superpowers:subagent-driven-development(推荐)或 superpowers:executing-plans 按任务逐个实施本计划。步骤用复选框(`- [ ]`)语法跟踪。

**目标:** 新增一个不依赖 AOSP 源码的黑盒 SurfaceFlinger fuzzer 用例,通过 adb `service call SurfaceFlinger <code> <随机类型参数>` 向 Binder `onTransact` 打随机/畸形事务,验证 SF 不崩、无内存破坏墓碑、画面非黑、设备存活,并对任何崩溃保存可复现命令。

**架构:** 三层。①`common/Gpu/gfwk_stress_util.py` 加两个纯函数(`rand_service_args` 生成随机 typed 参数、`sf_fuzz_call` 构造并发射一次 `service call`)——纯逻辑可离线单测。②新用例 `TC_SF_FUZZER_001.py` 用固定 seed 的 `random.Random` 驱动 N 次注入,每批体检(SF pid / device_alive / 墓碑 / 画面),收集崩溃复现命令。③文档(README 目录行 + vault wiki 教学页)。复用优先:注入/体检全走已有 `gfwk.*`(pid/tombstones/tombstone_owner/has_double_free/service_ready/device_alive/screen_not_black/wait_render)。

**技术栈:** Python 3.12、pytest、gta 框架(`BootInfo`/`Duts`/`init_gpu_aw`)、adb over `--serial`、Android `service call`(Binder 事务注入)。

**规格来源:** 无独立规格文档;需求源自本会话对话(黑盒 SF fuzzer,方案 B)。判据与头注释风格对齐既有 `cases/MultiMedia/GFWK/SurfaceFlinger/Fault/TC_SF_FAULT_001.py`。

## 全局约束

- Python 3.12;pytest 用例函数名 **必须 == 文件名**(gta 收集规则),即 `def TC_SF_FUZZER_001(...)`。
- 运行须带 `--serial <设备号>`;用例内 `aw.adb_root(aw.dut.car0)` 提权。
- 设备访问只用 `aw._adb_output(cmd)` / `aw.adb_shell(aw.dut.car0, cmd)`;不新起 subprocess。
- **复用优先**:注入/体检不得新建本地函数,凡 `gfwk_stress_util.py` 已有的一律复用(见 [[gfwk-reuse-common-first]] 规则)。新增的可复用逻辑下沉到 `gfwk_stress_util.py`。
- 头注释用 `TC_SF_FAULT_001` 的结构(用例编号/标题/创建人员/优先级/前置/后置/测试步骤/预期结果/校准记录);**不写 背景/运行/说明 三节**(见 [[gfwk-docstring-no-bg-run-note]])。
- 随机性可复现:所有随机走单个 `random.Random(seed)`,seed 由 `SF_FUZZER_SEED` 覆盖,默认固定值,日志打印 seed。
- `service call` 字符串参数(`s16`)一律单引号包裹且不含空格/NUL,保证 shell 安全。
- 目标台架 A41AEC42(SG2086)。

---

### 任务 1: 纯函数 `rand_service_args` —— 随机 typed 参数生成器

**涉及文件:**
- 修改: `common/Gpu/gfwk_stress_util.py`(文件尾追加 "SF 黑盒 binder fuzzing" 段)
- 测试: `common/Gpu/test_sf_fuzz.py`(新建,离线单测,不连设备)

**接口:**
- 依赖上游: 标准库 `random.Random`。
- 对外产出:
  - `SF_FUZZ_ARG_TYPES: list[str]`(常量,值 `["i32", "i64", "f", "d", "s16", "null"]`)
  - `rand_service_arg(rng: random.Random) -> list[str]` —— 返回单个 `service call` 参数的 token 列表,如 `["i32", "-1"]`、`["null"]`、`["s16", "'AAAA'"]`。
  - `rand_service_args(rng: random.Random, n: int) -> list[str]` —— 返回 n 个参数拼平后的 token 列表。

- [ ] **步骤 1: 写失败单测**

在新建 `common/Gpu/test_sf_fuzz.py`:

```python
# -*- coding: utf-8 -*-
"""离线单测：SF fuzzer 纯函数，不连设备。运行: pytest common/Gpu/test_sf_fuzz.py -v"""
import random
from common.Gpu import gfwk_stress_util as gfwk


def test_rand_service_arg_types_are_valid():
    rng = random.Random(0)
    seen = set()
    for _ in range(200):
        tok = gfwk.rand_service_arg(rng)
        assert isinstance(tok, list) and 1 <= len(tok) <= 2
        head = tok[0]
        assert head in gfwk.SF_FUZZ_ARG_TYPES
        if head == "null":
            assert len(tok) == 1
        else:
            assert len(tok) == 2 and isinstance(tok[1], str)
        if head == "s16":
            # 字符串必须单引号包裹且 shell 安全(无空格/NUL)
            assert tok[1].startswith("'") and tok[1].endswith("'")
            assert " " not in tok[1] and "\x00" not in tok[1]
        seen.add(head)
    # 200 次应覆盖到多种类型(至少 3 种)，证明确有随机性
    assert len(seen) >= 3


def test_rand_service_args_is_deterministic_by_seed():
    a = gfwk.rand_service_args(random.Random(42), 5)
    b = gfwk.rand_service_args(random.Random(42), 5)
    assert a == b            # 同 seed 完全可复现
    assert 5 <= len(a) <= 10  # n 个参数, 每个 1~2 token


def test_rand_service_args_length_scales_with_n():
    toks = gfwk.rand_service_args(random.Random(7), 0)
    assert toks == []
```

- [ ] **步骤 2: 跑单测确认失败**

运行: `pytest common/Gpu/test_sf_fuzz.py -v`
预期: 失败 —— `AttributeError: module ... has no attribute 'rand_service_arg'` / `SF_FUZZ_ARG_TYPES`。

- [ ] **步骤 3: 最小实现**

在 `common/Gpu/gfwk_stress_util.py` 末尾追加:

```python
# =============== SF 黑盒 binder fuzzing(TC_SF_FUZZER_001) ===============
# 思路: 不依赖 AOSP 源码, 用 `service call SurfaceFlinger <code> <typed args>` 向
# ISurfaceComposer 的 onTransact 直接打随机/畸形参数, 验证 Binder 层健壮性(应拒绝坏
# 参数返回错误, 而非崩溃)。参数类型对齐 Android `service` 命令支持的 typed args。
SF_FUZZ_ARG_TYPES = ["i32", "i64", "f", "d", "s16", "null"]

# 边界偏置的整数池(易触发溢出/负 size 分支)
_SF_FUZZ_INT_POOL = [0, 1, -1, 2**31 - 1, -(2**31), 2**63 - 1, -(2**63)]
_SF_FUZZ_FLOAT_POOL = ["0", "-1", "3.4e38", "1.2e-38", "nan", "inf", "-inf"]
# 字符串池: 已 shell 安全(无空格/NUL); 覆盖空串/超长/格式串/路径穿越
_SF_FUZZ_STR_POOL = ["''", "'%s%s%n'", "'../../../etc/hosts'",
                     "'" + "A" * 256 + "'", "'-1'", "'0x41414141'"]


def rand_service_arg(rng):
    """返回单个 `service call` 参数的 token 列表, 如 ['i32','-1'] / ['null']。"""
    t = rng.choice(SF_FUZZ_ARG_TYPES)
    if t == "null":
        return ["null"]
    if t in ("i32", "i64"):
        pool = _SF_FUZZ_INT_POOL + [rng.randint(-(2**31), 2**31 - 1)]
        return [t, str(rng.choice(pool))]
    if t in ("f", "d"):
        return [t, rng.choice(_SF_FUZZ_FLOAT_POOL)]
    # s16: 字符串已带单引号, shell 安全
    return [t, rng.choice(_SF_FUZZ_STR_POOL)]


def rand_service_args(rng, n):
    """返回 n 个随机参数拼平后的 token 列表。"""
    toks = []
    for _ in range(n):
        toks += rand_service_arg(rng)
    return toks
```

- [ ] **步骤 4: 跑单测确认通过**

运行: `pytest common/Gpu/test_sf_fuzz.py -v`
预期: 通过(3 passed)。

- [ ] **步骤 5: 提交**

```bash
git add common/Gpu/gfwk_stress_util.py common/Gpu/test_sf_fuzz.py
git commit -m "SF fuzzer: 加随机 typed 参数生成器 rand_service_args + 离线单测"
```

---

### 任务 2: `sf_fuzz_call` —— 构造并发射一次 `service call`

**涉及文件:**
- 修改: `common/Gpu/gfwk_stress_util.py`(接任务 1 的段落继续追加)
- 测试: `common/Gpu/test_sf_fuzz.py`(追加用例)

**接口:**
- 依赖上游: 任务 1 的 `rand_service_args`。
- 对外产出:
  - `sf_fuzz_cmd(code: int, args: list[str]) -> str` —— 纯函数, 返回完整 shell 命令字符串, 如 `service call SurfaceFlinger 5 i32 -1 null`。
  - `sf_fuzz_call(aw, code: int, args: list[str]) -> tuple[str, str]` —— 发射一次, 返回 `(cmd, output)`;`output` 为 `aw._adb_output(cmd)` 原样文本。

- [ ] **步骤 1: 写失败单测**

在 `common/Gpu/test_sf_fuzz.py` 追加:

```python
class _FakeAw:
    """离线桩: 记录收到的命令, 回固定文本, 不连设备。"""
    def __init__(self):
        self.calls = []
    def _adb_output(self, cmd):
        self.calls.append(cmd)
        return "Result: Parcel(00000000 '....')"


def test_sf_fuzz_cmd_builds_well_formed_string():
    cmd = gfwk.sf_fuzz_cmd(5, ["i32", "-1", "null"])
    assert cmd == "service call SurfaceFlinger 5 i32 -1 null"


def test_sf_fuzz_cmd_no_args():
    assert gfwk.sf_fuzz_cmd(1, []) == "service call SurfaceFlinger 1"


def test_sf_fuzz_call_fires_and_returns_pair():
    aw = _FakeAw()
    cmd, out = gfwk.sf_fuzz_call(aw, 3, ["i64", "9223372036854775807"])
    assert cmd == "service call SurfaceFlinger 3 i64 9223372036854775807"
    assert aw.calls == [cmd]          # 确实发射了一次
    assert "Parcel" in out
```

- [ ] **步骤 2: 跑单测确认失败**

运行: `pytest common/Gpu/test_sf_fuzz.py -v`
预期: 失败 —— `AttributeError: ... has no attribute 'sf_fuzz_cmd'`。

- [ ] **步骤 3: 最小实现**

在 `common/Gpu/gfwk_stress_util.py` 的 SF fuzzing 段继续追加:

```python
def sf_fuzz_cmd(code, args):
    """构造一次 `service call SurfaceFlinger` 命令(纯函数, 便于离线测/复现)。"""
    tail = (" " + " ".join(args)) if args else ""
    return "service call SurfaceFlinger %d%s" % (code, tail)


def sf_fuzz_call(aw, code, args):
    """向 SurfaceFlinger 发射一次畸形 binder 事务, 返回 (cmd, 原始输出)。"""
    cmd = sf_fuzz_cmd(code, args)
    return cmd, aw._adb_output(cmd)
```

- [ ] **步骤 4: 跑单测确认通过**

运行: `pytest common/Gpu/test_sf_fuzz.py -v`
预期: 通过(6 passed)。

- [ ] **步骤 5: 提交**

```bash
git add common/Gpu/gfwk_stress_util.py common/Gpu/test_sf_fuzz.py
git commit -m "SF fuzzer: 加 sf_fuzz_cmd/sf_fuzz_call 命令构造与发射 + 桩测"
```

---

### 任务 3: 用例 `TC_SF_FUZZER_001` —— 设备上黑盒 fuzz + 体检

**涉及文件:**
- 新建: `cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py`
- 依赖(已存在, 复用): `common/Gpu/gfwk_stress_util.py` 的 `pid/tombstones/tombstone_owner/has_double_free/service_ready/device_alive/screen_not_black/wait_render/adb_server_reset` 及任务 1/2 新增函数。

**接口:**
- 依赖上游: 任务 1/2 的 `rand_service_args` / `sf_fuzz_call`;gta fixture `init_gpu_aw` / `info: BootInfo`。
- 对外产出: pytest 收集的用例函数 `TC_SF_FUZZER_001(sf_restore, info)`;无对外符号(叶子用例)。

- [ ] **步骤 1: 写用例文件**

创建 `cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py`:

```python
# -*- coding: utf-8 -*-
# copyright: 2026, XG Tech.Inc. All rights reserved.
"""
===============================================================================
用例编号:   TC_SF_FUZZER_001
用例标题:   SurfaceFlinger Binder 事务黑盒 fuzz(畸形 service call 健壮性)
创建人员:   qi.zhu   创建时间: 2026.08.13   最后修改: 2026.08.13
优先级:     P1    维度: 边界条件/故障注入(D2/D3)
#REQUIRE:adb,car0
===============================================================================
前置条件:
    1. 台架刷机完成正常启动; 2. adb 可 root; 3. 运行须带 --serial <设备号>
后置条件:
    1. SurfaceFlinger 存活未重启, 画面正常
测试步骤:
    1. 记录 SF 基线 pid 与墓碑集合; 固定 seed 的 random.Random 驱动
    2. 循环 ITERS 次: 随机事务码 code∈[CODE_LO,CODE_HI] + 随机 typed 参数,
       service call SurfaceFlinger 注入一次畸形事务
    3. 每 BATCH 次体检: SF pid 未变 + device_alive; 变/失联即记崩溃并存复现命令
    4. 结束体检: 新增墓碑分类(SF 自身崩溃/double-free 硬失败, 其他进程告警);
       画面非黑; SF service ready
预期结果:
    Binder 拒绝畸形参数(返回错误), SF pid 全程不变, 无 SF 崩溃墓碑, 无 double-free,
    设备存活, 画面非黑
校准记录:
    1. 崩溃复现: 命中即记录触发的完整 `service call ...` 命令, 便于最小复现
    2. 事务码范围默认 [1,60](ISurfaceComposer FIRST_CALL_TRANSACTION 起), 可环境覆盖
===============================================================================
"""
import os
import random
import time
import pytest
from gta import BootInfo
from common.Gpu import gfwk_stress_util as gfwk

ITERS = int(os.environ.get("SF_FUZZER_ITERS", "2000"))     # 注入次数; CI 冒烟设 50
BATCH = int(os.environ.get("SF_FUZZER_BATCH", "50"))       # 每多少次体检一次
CODE_LO = int(os.environ.get("SF_FUZZER_CODE_LO", "1"))
CODE_HI = int(os.environ.get("SF_FUZZER_CODE_HI", "60"))
ARGN_MAX = int(os.environ.get("SF_FUZZER_ARGN_MAX", "6"))  # 每次最多几个参数
SEED = int(os.environ.get("SF_FUZZER_SEED", "20260813"))
SF = "surfaceflinger"


@pytest.fixture(scope="function")
def sf_restore(init_gpu_aw, info: BootInfo):
    """收尾: 结束后若 SF 未就绪, 全框架 stop/start 经 zygote 恢复干净 UI。"""
    aw = init_gpu_aw
    yield aw
    try:
        if gfwk.service_ready(aw, "SurfaceFlinger"):
            return
        info.logger.info("[teardown] SF 未就绪, 全框架 stop/start 恢复(经 zygote)")
        aw.adb_root(aw.dut.car0)
        aw.adb_shell(aw.dut.car0, "stop")
        time.sleep(2)
        aw.adb_shell(aw.dut.car0, "start")
        t0 = time.time()
        while time.time() - t0 < 60:
            if gfwk.service_ready(aw, "SurfaceFlinger"):
                break
            time.sleep(2)
    except Exception as e:
        info.logger.warning(f"[teardown] 恢复异常(不影响结论): {e}")


def _check_alive(aw, info, r, cmd, crashes):
    """一次体检: SF pid 未变 + 设备存活; 异常记 (轮次, 现象, 复现命令)。"""
    if not gfwk.device_alive(aw):
        crashes.append((r, "设备失联", cmd))
        info.logger.error(f"[FAIL] 第{r}次后设备失联, 触发命令: {cmd}")
        gfwk.adb_server_reset(aw.dut.car0)
        return
    now = gfwk.pid(aw, SF)
    if not now:
        crashes.append((r, "SF 消失", cmd))
        info.logger.error(f"[FAIL] 第{r}次后 SF 进程消失, 触发命令: {cmd}")


def TC_SF_FUZZER_001(sf_restore, info: BootInfo):
    aw = sf_restore
    aw.adb_root(aw.dut.car0)
    info.logger.info(
        f"=== TC_SF_FUZZER_001 ITERS={ITERS} code=[{CODE_LO},{CODE_HI}] seed={SEED} ===")
    rng = random.Random(SEED)
    pid0 = gfwk.pid(aw, SF)
    assert pid0, "起始未找到 surfaceflinger 进程"
    base_tombs = gfwk.tombstones(aw)
    info.logger.info(f"起始 SF pid={pid0}, 墓碑基线={len(base_tombs)} 个")

    crashes = []          # (轮次, 现象, 复现命令) —— pid 变/失联
    last_cmd = ""
    for r in range(1, ITERS + 1):
        code = rng.randint(CODE_LO, CODE_HI)
        args = gfwk.rand_service_args(rng, rng.randint(0, ARGN_MAX))
        last_cmd, _ = gfwk.sf_fuzz_call(aw, code, args)
        if r % BATCH == 0:
            _check_alive(aw, info, r, last_cmd, crashes)
            now = gfwk.pid(aw, SF)
            if now and now != pid0:
                crashes.append((r, f"SF 重启 pid {pid0}->{now}", last_cmd))
                info.logger.error(f"[FAIL] 第{r}次 SF 重启 {pid0}->{now}, 触发命令: {last_cmd}")
                pid0 = now   # 跟随新 pid, 避免后续批次重复误判
            info.logger.info(f"[进度] {r}/{ITERS} 完成, 累计崩溃={len(crashes)}")

    # 结束体检
    _check_alive(aw, info, ITERS, last_cmd, crashes)
    # 墓碑分类
    new_tombs = gfwk.tombstones(aw) - base_tombs
    sf_crash, dbl_free, other_crash = [], [], []
    for t in new_tombs:
        proc, sig = gfwk.tombstone_owner(aw, t)
        if gfwk.has_double_free(aw, t):
            dbl_free.append((t, proc, sig))
        if SF in proc.lower():
            sf_crash.append((t, proc, sig))
        else:
            other_crash.append((t, proc, sig))
    # 画面
    gfwk.wait_render(aw, info)
    screen_black = not gfwk.screen_not_black(aw, info, tag="sf_fuzzer")

    info.logger.info(
        f"汇总: 崩溃/重启={len(crashes)} SF墓碑={len(sf_crash)} "
        f"double-free={len(dbl_free)} 其他墓碑={len(other_crash)} 画面黑={screen_black}")
    if other_crash:
        info.logger.warning(f"[WARN] 非SF进程崩溃(疑似扩散,需上报): {other_crash}")

    assert not crashes, f"SF fuzz 触发崩溃/重启(附复现命令): {crashes}"
    assert not sf_crash, f"检测到 SF 自身崩溃墓碑: {sf_crash}"
    assert not dbl_free, f"检测到 double-free/UAF 墓碑(内存破坏): {dbl_free}"
    assert not screen_black, "fuzz 后主屏黑"
    assert gfwk.service_ready(aw, "SurfaceFlinger"), "fuzz 后 SurfaceFlinger 未就绪"
```

- [ ] **步骤 2: 静态编译检查**

运行: `python -m py_compile cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py`
预期: 无输出(退出码 0)。

- [ ] **步骤 3: 冒烟跑(台架, 50 次)**

运行: `SF_FUZZER_ITERS=50 pytest cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py --serial <设备号> -v`
预期: 通过 —— 日志见 `汇总: 崩溃/重启=0 ...`;SF pid 全程不变;画面非黑。
若通过:注入向量有效且 SF 健壮。
若失败且 `crashes` 非空:记录断言里打印的 `service call ...` 复现命令 —— 这是真 finding,手动复跑该命令确认。

- [ ] **步骤 4: 提交**

```bash
git add cases/MultiMedia/GFWK/SurfaceFlinger/Fuzzer/TC_SF_FUZZER_001.py
git commit -m "SF fuzzer: 新增 TC_SF_FUZZER_001 Binder 事务黑盒 fuzz 用例(实测通过)"
```

---

### 任务 4: 文档 —— README 目录行 + vault 教学页

**涉及文件:**
- 修改: `cases/MultiMedia/GFWK/README.md`(目录表加 `SurfaceFlinger/Fuzzer/` 行)
- 新建: `/home/gua/vault_import2/AI赋能/wiki/005-稳定性/TC_SF_FUZZER_001.md`
- 修改: `/home/gua/vault_import2/AI赋能/wiki/index.md`(005 计数 +1, 增条目)
- 修改: `/home/gua/vault_import2/AI赋能/wiki/log.md`(追加一行)

**接口:**
- 依赖上游: 任务 3 的用例路径/判据。
- 对外产出: 无代码符号。

- [ ] **步骤 1: README 加目录行**

打开 `cases/MultiMedia/GFWK/README.md`,在目录表 SurfaceFlinger 相关行区域加一行(列与既有表头对齐,参照 `Serdes/` 行格式):

```
| `SurfaceFlinger/Fuzzer/` | TC_SF_FUZZER_001 | Binder 事务黑盒 fuzz(畸形 service call 健壮性) |
```

- [ ] **步骤 2: 建 vault 教学页**

创建 `/home/gua/vault_import2/AI赋能/wiki/005-稳定性/TC_SF_FUZZER_001.md`,用既有 `TC_SERDES_FAULT_002.md` 的教学结构(frontmatter + ①一句话 ②原理 ③流程图 mermaid ④关键点 ⑤复现指令 ⑥易出 bug 表 + 关联 + 延伸阅读)。核心内容:

- ①一句话:不依赖 AOSP 源码,用 `service call SurfaceFlinger` 向 Binder onTransact 打随机/畸形参数,验证不崩。
- ②原理:黑盒 vs AOSP libFuzzer(`surfaceflinger_fuzzer`) 对比 —— 无覆盖率引导,但零源码、直接上真机测 Binder 健壮性。
- ③流程图:seed→循环(随机 code+typed 参数→service call)→每批体检 pid/alive→结束墓碑/画面。
- ④关键点:崩溃即存完整复现命令;double-free 墓碑单独升级硬失败。
- ⑤复现指令:贴步骤 3 的 pytest 命令 + 单条 `service call SurfaceFlinger <code> i32 -1 ...`。
- ⑥易出 bug 表:`bad parcel 崩 SF`/`负 size 分配`/`整数溢出`/`格式串` 四行,各给判据。
- 关联:[[SurfaceFlinger]] · [[TC_SF_FAULT_001]] · [[TC_SF_BOUND_002]] · [[断言（assert）]]。

- [ ] **步骤 3: 更新 index.md 与 log.md**

- `index.md`:005 段标题计数 `(46)`→`(47)`,总篇数 `114`→`115`,在 SF 条目区插入 `- [[TC_SF_FUZZER_001]] — TC_SF_FUZZER_001 — SF Binder 事务黑盒 fuzz`。
- `log.md`:追加 `- 2026-08-13 新增 [[TC_SF_FUZZER_001]] SF Binder 黑盒 fuzz 用例(教学页)`。

- [ ] **步骤 4: 提交(仓库侧)**

```bash
git add cases/MultiMedia/GFWK/README.md
git commit -m "SF fuzzer: README 目录加 SurfaceFlinger/Fuzzer 行"
```

(vault 为独立目录,非本仓库;按你既有 vault 提交流程单独 commit/push。)

---

## 自查

**1. 规格覆盖(对照对话需求):**
- "黑盒 pytest fuzzer / 不依赖源码" → 任务 1-3 全程 `service call`,零源码。✓
- "洪泛畸形事务" → 任务 3 随机 code + typed 参数循环。✓
- "断言 SF 存活 + 无墓碑" → 任务 3 pid 未变 + 墓碑分类 + double-free。✓
- "复用 common" → 注入/体检全 `gfwk.*`,新逻辑下沉 common。✓
- "头注释不写 背景/运行/说明" → 任务 3 头注释无此三节。✓
- 用户四问(codebase/what/expected/duration)已在对话正文答复;用例可运行时长由 `SF_FUZZER_ITERS` 控制(冒烟 50≈秒级, 默认 2000≈数分钟)。✓

**2. 占位符扫描:** 无 TBD/TODO;所有步骤含真实代码或精确命令。README/vault 文本步骤给出确切插入行与结构参照,非"类似上文"。✓

**3. 类型一致性:** `rand_service_args(rng, n)` / `sf_fuzz_cmd(code, args)` / `sf_fuzz_call(aw, code, args)` 在任务 1→2→3 签名一致;用例复用的 `gfwk.pid/tombstones/tombstone_owner/has_double_free/service_ready/device_alive/screen_not_black/wait_render/adb_server_reset` 均在 `gfwk_stress_util.py`(505 行版本)确认存在。✓

**风险提示(执行时留意):** `service call` 的事务码语义随平台/版本变化,`[1,60]` 是黑盒猜测范围;无覆盖率引导,命中深层解析路径的概率低于真 libFuzzer。这是方案 B 的固有局限(健壮性 fuzz,非覆盖 fuzz),已在对话中说明。
