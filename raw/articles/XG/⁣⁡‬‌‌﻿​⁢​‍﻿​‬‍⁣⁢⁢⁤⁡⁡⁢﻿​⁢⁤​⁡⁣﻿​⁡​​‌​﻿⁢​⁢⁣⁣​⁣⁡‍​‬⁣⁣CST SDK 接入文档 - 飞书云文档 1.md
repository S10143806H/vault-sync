---
title: "⁣⁡‬‌‌﻿​⁢​‍﻿​‬‍⁣⁢⁢⁤⁡⁡⁢﻿​⁢⁤​⁡⁣﻿​⁡​​‌​﻿⁢​⁢⁣⁣​⁣⁡‍​‬⁣⁣CST SDK 接入文档 - 飞书云文档"
source: "https://guatechltd.feishu.cn/wiki/YDQ4wgQAviqbggkM7qWc5nInnob"
author:
published:
created: 2026-09-16
description:
tags:
  - "clippings"
---
- [CST SDK 接入文档](#X2gEdT4cVoZWPgxvNvAcDIh7nZc)
- [一、概述](#doxcnvEsSbS2if9i9dCRYDnpide)
- [1.1 什么是 CST SDK](#doxcnVW1E6gEqKC749hzE7JNX0f)
- [1.2 三大核心模块](#doxcnQbF1zK7KdA3JXZDqo4dGUg)
- [1.3 系统架构](#doxcn65Cl5FCPA0fVZHgJueSRJ8)
- [二、快速接入](#doxcncniZqgiIvMOHyioI8HvRjh)
- [2.1 构建依赖配置](#doxcnDiGqnL7TebJmRh0f4HZRog)
- [2.1.1 C/C++ 项目（Android.bp）](#doxcn9GwwLqwUvUMQA4zjypwKEg)
- [2.1.2 Java 项目（Android.bp）](#doxcngQzc0A6ZU8LowOfRw6Qo9d)
- [2.1.3 Linux 平台（CMakeLists.txt）](#doxcnqJHYzJtlVcvbYujMaQ1r9d)
- [2.1.4 APK 项目（Gradle）](#doxcnpEUviGepAXxzydf98UY40A)
- [一、获取 SDK 依赖](#doxcnztm418PK86Z9npwLft7Xjd)
- [二、Gradle 配置](#doxcnSgiYngnpiKvDJnQap8BCyc)
- [三、AndroidManifest.xml 配置](#doxcnBKBFAixYCVWY4SAq7x2Kng)
- [四、签名配置](#doxcntq1NjJXox5VWD4zdTw2JHd)
- [五、预装到系统镜像](#doxcnyLg4R1VlsjnQqA5UXDERp8)
- [六、完整 Gradle 接入示例](#doxcnoGLoCOzCVJ7FYpSZgJgR1g)
- [2.2 头文件引入](#doxcnqy184aDFLUfXOaI5YgPI2g)
- [三、Debug 模块](#doxcnQ3536jcg61e7WpIwLIrbCf)
- [3.1 子系统 ID 定义](#doxcnz5db2U4taycoowKtyLHX9d)
- [3.2 日志上报 API](#doxcnJzNbQFuscZRge8TxtINURd)
- [3.2.1 文本日志上报](#doxcn2QSnxpgb9zsfN9Cn6c0eIe)
- [3.2.2 二进制日志上报](#doxcn8aCSerIyfaglp8VZ8eJXqe)
- [3.2.3 返回码](#doxcnjMT6QoAm7yCxYYLHqwrL2b)
- [3.3 命令注册与处理 API](#doxcnqpULCw6gxw1skGn4C5Ruxh)
- [3.3.1 命令处理器回调类型](#doxcnFBnUZz4BZ0Ze1YVWZcSXSe)
- [3.3.2 注册命令处理器](#doxcnS87wRkExPkkrnmUFruALGb)
- [3.3.3 异步命令响应上报](#doxcnms0cpg4dtdoR8duXZzgKof)
- [3.4 RawData 传输 API](#doxcnfrUoujCaKWi6JoaCNFtIye)
- [3.4.1 共享内存方式](#doxcnQm5988HBVRfqriIaWBOT8g)
- [3.4.2 文件方式](#doxcneZaL5Jlm9vtvhvjmwxi8PR)
- [3.5 Debug 模块完整示例](#doxcnL9wnBuJs335gv64vrRvyBg)
- [四、Diag 诊断模块](#doxcn59IRMuJWGTB7Gqh7xAomIg)
- [4.1 故障严重等级](#doxcnMteIjewv6AeZvwM2DpkEyh)
- [4.2 C++ API（推荐）](#doxcnxCue52RNtgqJSD2W0sx3Td)
- [4.2.1 故障上报与恢复](#doxcnU1VwD643lssjq3pzW1pEVS)
- [4.2.2 故障订阅](#doxcnj15dPyyoMvO1vkCYyMwmpZ)
- [4.2.3 故障查询](#doxcnbxmTwfyDfaVUkMGeEM5nah)
- [4.3 C API](#doxcnu8ap392K0ltVqHmBzRMkdh)
- [4.4 Java API](#doxcnB1478y2Jt2GmiX7n3kR2jb)
- [4.5 Diag OTA 升级模块](#doxcnJ7DvEf5nB5txlFq2pagmDc)
- [4.5.1 OTA 事件类型](#doxcnoVhG6amqPIBOPkW6ZTNwQh)
- [4.5.2 OTA 回调接口](#doxcnt8NVQ3abq0vKsQaos3JAAh)
- [4.5.3 OTA 注册与结果上报](#doxcn6M1YRUjxUHtopqQUZ3qM8g)
- [4.5.4 OTA 结果码](#doxcnVWEjtCqgVNuGzcOH6AdEZd)
- [五、Maint 维测模块](#doxcnepUgVm9huOkayD5mKF3pAw)
- [5.1 Event ID 约束](#doxcnzHXTJzr3cfEPdWlnSuBESb)
- [5.2 C++ API](#doxcn8ztGRfzaYqaRYHSXS3DI5e)
- [5.3 C API](#doxcn8eggeUd9wpqhrwSJm1l2Cd)

输入“/”快速插入内容

一、概述

1.1 什么是 CST SDK

CST SDK（Cornerstone SDK）是 Gua 车载 AAOS（Android Automotive OS）平台提供的统一客户端开发套件，封装了与底层诊断通信服务（GDC）、故障服务（GFS）和维护服务（Cornerstone）的交互逻辑，为上层应用提供简洁的 C/C++ 和 Java API。

1.2 三大核心模块

<table><tbody><tr><td rowspan="1" colspan="1"><p></p><p>模块</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>头文件 / 类</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>核心能力</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>底层服务</p><p></p></td></tr></tbody></table>

<table><colgroup><col width="100"> <col width="100"> <col width="250"> <col width="150"></colgroup><tbody><tr><td rowspan="1" colspan="1"><p></p><p>Debug</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>cst_debug.h</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>日志上报、命令注册与处理、RawData 传输</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>GDC (IGdcHalService)</p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p>Diag</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>cst_diag.h / cst_diag_ota.h</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>故障上报/恢复、故障订阅、故障查询、OTA 升级</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>GFS (IGfs / IDiagOtaManager)</p><p></p></td></tr><tr><td rowspan="1" colspan="1"><p></p><p>Maint</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>cst_maint.h</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>事件上报、点位上报</p><p></p></td><td rowspan="1" colspan="1"><p></p><p>Cornerstone (ICornerStoneService)</p><p></p></td></tr></tbody></table>

1.3 系统架构

评论（0）

跳转至首条评论

6,187 字

- 上传日志

- 联系客服

- 功能更新

- 帮助中心

- 快捷键

连续按下 Ctrl + A 以选中全文