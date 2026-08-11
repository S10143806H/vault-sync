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
$stamp = git log -1 --format=%cd --date=format:'%Y-%m-%d %H:%M' 2>$null
git commit -m "auto: update notes" | Out-Null
git push origin master 2>$null | Out-Null
exit 0
