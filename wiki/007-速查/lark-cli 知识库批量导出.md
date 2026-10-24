---
title: "lark-cli 知识库批量导出"
tags: [速查, lark-cli, 飞书, 知识库, wiki, 导出]
created: 2026-10-24
updated: 2026-10-24
source: "lark-cli v1.0.98 --help 及内置 skill（lark-wiki / lark-drive references/lark-drive-export.md）"
---

# lark-cli 知识库批量导出

> lark-cli **没有**"一键导出整个知识库"的命令。做法 = `wiki +node-list` 递归遍历节点树 + 逐个 `drive +export`。
> 相关：[[常用linux 指令速查]]（lark-cli 授权 bot 示例）· [[claudecode 飞书]]

## 0. 准备

```bash
npm install -g @larksuite/cli          # 装 lark-cli
lark-cli config init                   # 填 app-id / app-secret
lark-cli auth login --domain wiki,drive,docs   # 浏览器授权（user 身份）
lark-cli wiki +space-list --as user --format table   # 拿 SPACE_ID
```

## 1. 单篇导出

```bash
lark-cli drive +export --url "https://xxx.feishu.cn/wiki/<NODE_TOKEN>" \
  --file-extension markdown --output-dir ./exports --as user
```

| 底层 obj_type | 可选 `--file-extension` |
|---|---|
| docx（新版文档） | `markdown` / `docx` / `pdf` |
| doc（旧版文档） | `docx` / `pdf` |
| sheet | `xlsx` / `csv`（csv 需 `--sub-id`） |
| bitable | `xlsx` / `csv` / `base` |
| slides | `pptx` / `pdf` |
| file（上传的附件） | 不走 export，用 `drive +download --wiki-token <NODE_TOKEN>` |

## 2. 批量导出脚本（递归，保留目录层级）

用法：`./lark-wiki-export.sh <SPACE_ID> [PARENT_NODE_TOKEN] [OUT_DIR]`，依赖 `jq`。只导出某个子目录时传 `PARENT_NODE_TOKEN`。

```bash
#!/usr/bin/env bash
# 递归导出飞书知识库（空间或子树）到本地，保留目录层级
# 用法: ./lark-wiki-export.sh <SPACE_ID> [PARENT_NODE_TOKEN] [OUT_DIR]
# 依赖: lark-cli（已 auth login）、jq
set -uo pipefail
SPACE="$1"; ROOT="${2:-}"; OUT="${3:-./wiki-export}"
AS="--as user"
FAIL_LOG="$OUT/_failed.tsv"; mkdir -p "$OUT"; : > "$FAIL_LOG"

safe() { printf '%s' "$1" | tr '/\\:*?"<>|' '_________' | cut -c1-120; }

export_one() {  # $1=node_token $2=obj_type $3=title $4=dir
  local tok="$1" type="$2" name; name="$(safe "$3")"
  local ext
  case "$type" in
    docx)          ext=markdown ;;
    doc)           ext=docx ;;
    sheet|bitable) ext=xlsx ;;
    slides)        ext=pptx ;;
    file)
      lark-cli drive +download --wiki-token "$tok" --output "$4/" $AS --overwrite </dev/null >/dev/null \
        || printf '%s\t%s\t%s\n' "$tok" "$type" "$3" >> "$FAIL_LOG"
      return ;;
    *) echo "skip [$type] $3"; return ;;   # mindnote 等不支持导出
  esac
  lark-cli drive +export --token "$tok" --doc-type wiki --file-extension "$ext" \
    --file-name "$name" --output-dir "$4" --overwrite $AS </dev/null >/dev/null \
    || printf '%s\t%s\t%s\n' "$tok" "$type" "$3" >> "$FAIL_LOG"
  sleep 1   # 防限频 99991400
}

walk() {  # $1=parent_node_token(空=根) $2=本地目录
  local parent="$1" dir="$2"; mkdir -p "$dir"
  local args=(wiki +node-list --space-id "$SPACE" --page-all --page-limit 0 $AS)
  [ -n "$parent" ] && args+=(--parent-node-token "$parent")
  lark-cli "${args[@]}" </dev/null | jq -r '.data.nodes[] | [.node_token,.obj_type,.has_child,.title] | @tsv' |
  while IFS=$'\t' read -r tok type child title; do
    echo "→ $dir/$title [$type]"
    export_one "$tok" "$type" "$title" "$dir"
    [ "$child" = "true" ] && walk "$tok" "$dir/$(safe "$title")"
  done
}

walk "$ROOT" "$OUT"
echo "完成。失败清单: $FAIL_LOG（$(wc -l < "$FAIL_LOG") 条）"
```

## 3. 坑

- **限频**：`99991400` / `9499 too many requests` → 等 ≥1 分钟后重跑；脚本里每篇 `sleep 1`。
- **导出超时不等于失败**：返回 `ticket` + `timed_out=true` 时，用 `drive +task_result --scenario export` 续查，再 `drive +export-download`。
- **`1069914`**：把 wiki node token 当成 docx token 用了 → 裸 token 必须带 `--doc-type wiki`（脚本已带）。
- **`1069902`**：无导出权限（分享设置/密级/租户策略），需 owner 授权，重试无用。
- **while 循环里调 lark-cli 要 `</dev/null`**，否则子进程会吞掉管道里的节点列表。
- `wiki +node-list` 默认只取 1 页，必须 `--page-all --page-limit 0`。
- mindnote 等类型不支持导出，脚本会 skip。
