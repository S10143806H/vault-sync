source build/envsetup.sh && lunch <你的目标>
SOONG_GEN_COMPDB=1 m surfaceflinger_fuzzer

生成后,看 SurfaceFlinger.cpp 到底用什么命令编、有没有带上宏:

python3 -c "import json; \
[print(e['command']) for e in \
 json.load(open('out/soong/development/ide/compdb/compile_commands.json')) \
 if e['file'].endswith('SurfaceFlinger.cpp')]" | tr ' ' '\n' | grep -E "clang|SF_FUZZ_NO_MULTIDISPLAY|SurfaceFlinger.cpp"



``` bash

cd ~/code/ws/workspace/aaos

# 1. 清被污染的变量
rm -rf out/soong out/bazel


# 2. 正确初始化
source build/envsetup.sh
lunch guav100-userdebug


# 3. 确认(必须显示 guav100,不能带 lunch/空格)
echo "$TARGET_PRODUCT"
# 必须显示 userdebug
echo "$TARGET_BUILD_VARIANT"


# 4. 生成 compile DB + 编译 fuzzer(注意 flinger)
SOONG_GEN_COMPDB=1 m surfaceflinger_fuzzer

rm -rf out/soong/soong_injection out/bazel
lunch guav100-userdebug
m <你的目标>
```
