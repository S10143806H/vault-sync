# Auto-commit vault .md changes after Claude edits. Called by PostToolUse hook.
$ErrorActionPreference = 'SilentlyContinue'
$vault = 'C:\Users\XGTech_ZHU\iCloudDrive\iCloud~md~obsidian\AI赋能'
Set-Location $vault

# 有无变更
$changes = git status --porcelain
if (-not $changes) { exit 0 }

# 变更里是否包含 .md
$md = $changes | Where-Object { $_ -match '\.md\s*$' -or $_ -match '\.md"' }
if (-not $md) { exit 0 }

git add -A
git commit -m "auto: update notes (win)" | Out-Null

# push 前先拉取远程（含 Linux 端提交），rebase 合并
git pull --rebase origin master 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
  # rebase 冲突：放弃 rebase，保留本地，等人工/Claude 解决，不强推
  git rebase --abort 2>$null | Out-Null
  Write-Host "CONFLICT: 远程有冲突改动，已暂停自动 push，请手动 git pull --rebase 解决"
  exit 0
}
git push origin master 2>$null | Out-Null
exit 0
