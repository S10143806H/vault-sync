App的界面说明
https://www.bilibili.com/video/BV1CwGJ6dEF8/?spm_id_from=333.337.search-card.all.click&vd_source=82a58e3396f3f041ae0fb6816f2aa179

一种驾驭(Harness )大模型的方式，负责调度，其实是个壳，类似的还有[[OpenClaw]] 和 [[Hermes Agent]] 模型负责思考

## Instructions
Claude Cowork 提供三级管理
**全局指令 Global Instruction** 
- Settings/Cowork, 系统提示词，全局作用所有的cowork对话都会知道，这里不用写太细，只要童工背景信息就好
- 每次回答都尽可能认真思考
```
  模板
  我是一名xxx （工作岗位）
  我希望你每次会打钱

```

**项目指令 Projects Instruction** 
- 一个项目中有不同的文件夹, 可以边做边改慢慢迭代
- 
**文件夹指令 Folder  Instruction**
- 文件夹指令 `CLAUDE.md` 可以跟着文件夹拷贝转移



## Memory
请记住：xxx， 相关的记忆会存储在MEMORY.md 中



