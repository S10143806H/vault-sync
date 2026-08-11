---
title: "SELinux（强制访问控制 / avc denied）"
tags:
  - AAOS
  - 安全
  - Linux
  - 权限
platform: "gua / guav100 (AAOS)"
created: 2026-08-03
---

# SELinux（强制访问控制 / avc denied）

**一句话**：Linux/Android 的**强制访问控制（MAC）**——每个进程有安全域（context/domain），每个资源有标签；只有策略（policy）显式允许的"域→资源→操作"才放行，否则 `avc: denied`。

## permissive vs enforcing（关键）
| 模式 | 行为 |
|---|---|
| **permissive** | 违规**只记录不拦**（日志 `... permissive=1`），程序照常跑 |
| **enforcing** | 违规**真拦**（操作失败，可能导致进程报错/崩溃） |

> 量产一般 enforcing；开发/调试常 permissive。**permissive 下的 denied = 潜在策略缺口**：现在无害，转 enforcing 会真拦。

## 怎么读 avc denied
```
avc: denied { use } for pid=9325 comm="binder:9325_3" path="anon_inode:sync_file"
  scontext=...:composer_stub_t  tcontext=...:weston_t  tclass=fd  permissive=1
```
| 字段 | 含义 |
|---|---|
| `{ use }` | 被拒的操作 |
| `scontext` | 源域（发起者）= `composer_stub_t` |
| `tcontext` | 目标域（资源属主）= `weston_t` |
| `tclass` | 资源类型 = `fd`（文件描述符） |
| `path` | 资源 = `anon_inode:sync_file`（一个 [[Fence\|fence]] fd） |

## 与本项目（实测发现）
cornerstone 里出现：**`composer_stub_t` 想 `use` 属于 `weston_t` 的 `sync_file` fd，denied 但 permissive=1（放行）**。
- 含义：跨 SoC [[Fence|fence]]（sync_file）在 `weston`↔`composer_stub` 间传递，SELinux 策略没显式允许
- 当前 permissive **无害**；若 Cluster 转 enforcing，**跨 SoC fence 传递会被拒** → 需补 `allow composer_stub_t weston_t:fd use;` 类策略。属**策略缺口**，记一笔（与 [[Fence]] 生命周期相关）

## 关联
- 资源 → [[Fence]]（sync_file fd）｜[[SHMEM]]｜[[composer_stub]]｜[[Weston]]
- 上级 → [[GFWK 双bots自动挖bug闭环]]（分析发现来源）

## 📚 延伸阅读
- Android SELinux：https://source.android.com/docs/security/features/selinux
- SELinux 概念（NSA/Redhat）：https://selinuxproject.org/page/Main_Page
