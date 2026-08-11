#!/usr/bin/env bash
# Auto-pull vault when remote (GitHub) has new commits. Run via cron every ~2 min.
cd /home/gua/vault_import2/AI赋能 || exit 0
export GIT_TERMINAL_PROMPT=0

git fetch -q origin master
local=$(git rev-parse @ 2>/dev/null)
remote=$(git rev-parse @{u} 2>/dev/null)
[ -z "$remote" ] && exit 0
[ "$local" = "$remote" ] && exit 0   # 无更新

# 远程有新提交：rebase 拉取，自动暂存本地未提交改动再回放
if ! git pull --rebase --autostash origin master >/dev/null 2>&1; then
  git rebase --abort >/dev/null 2>&1
  echo "CONFLICT: 自动拉取遇冲突，已回滚，请手动 git pull --rebase 解决"
fi
exit 0
