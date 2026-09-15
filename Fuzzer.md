🎉 成功!guard 修好了 —— fuzzer 真跑起来了,不再 abort。

#2  INITED cov: 3757 ft: 3751 corp: 1/1b
#2  DONE   cov: 3757 ...
Done 2 runs in 0 second(s)
EXIT=0

- EXIT=0 = 过了那个 __throw_bad_cast 静态初始化崩溃
- cov: 3757 = 真实覆盖到 3757 个代码分支 = fuzzer 确实在压 SurfaceFlinger 逻辑
- 那几条 GMOCK WARNING 无害(HAL 被 mock,预期行为)

真正 fuzz(限时 60 秒)

adb -s A41AEC42 shell 'cd /data/local/tmp/sf_fuzz && \
  LD_LIBRARY_PATH=./lib ./surfaceflinger_fuzzer/surfaceflinger_fuzzer \
  -max_total_time=60 -artifact_prefix=./ 2>/dev/null | tail -5'

一次跑全部 5 个 target

adb -s A41AEC42 shell 'cd /data/local/tmp/sf_fuzz && \
  for t in surfaceflinger_fuzzer surfaceflinger_layer_fuzzer \
           surfaceflinger_scheduler_fuzzer surfaceflinger_displayhardware_fuzzer \
           surfaceflinger_frametracer_fuzzer; do \
    echo "==== $t ====" ; \
    LD_LIBRARY_PATH=./lib ./$t/$t -runs=1 2>/dev/null | tail -2 ; \
    echo "EXIT=$?" ; \
  done'
每个都出 INITED cov: + EXIT=0 = 5 个全通。

崩了怎么办

libFuzzer 命中 crash 会在当前目录留 crash-<hash> 文件,并打印 SUMMARY: ...。把那文件拷回构建机复现:
adb -s A41AEC42 pull /data/local/tmp/sf_fuzz/crash-xxxx .

---

下一步二选一:
1. 先跑 60 秒真 fuzz(上面第一条),看有没有 crash;
2. 直接把这套跑法接回 TC_SF_FUZZER_001.py(它现在假设 /data/fuzz/arm64 + 无 LD_LIBRARY_PATH,得改成你这个 /data/local/tmp/sf_fuzz + LD_LIBRARY_PATH=./lib 布局)。

要我改 TC_SF_FUZZER_001.py 对接这个真实布局吗?