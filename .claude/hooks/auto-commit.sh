#!/usr/bin/env bash
# Auto-commit vault .md changes after Claude edits (Linux side). Wired via .claude/settings.local.json.
cd /home/gua/vault_import2/AI赋能 || exit 0

changes=$(git status --porcelain)
[ -z "$changes" ] && exit 0
# 仅当变更含 .md
echo "$changes" | grep -q '\.md' || exit 0

git add -A
git commit -m "auto: update notes (linux)" >/dev/null 2>&1

# push 前先 rebase 拉取远程（含 Windows 端提交）
if ! git pull --rebase origin master >/dev/null 2>&1; then
  git rebase --abort >/dev/null 2>&1
  echo "CONFLICT: 远程有冲突改动，已暂停自动 push，请手动 git pull --rebase 解决"
  exit 0
fi
git push origin master >/dev/null 2>&1
exit 0
