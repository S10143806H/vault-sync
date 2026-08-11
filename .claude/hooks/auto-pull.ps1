# Auto-pull vault when remote (GitHub) has new commits. Run periodically (every ~2 min).
$ErrorActionPreference = 'SilentlyContinue'
$env:GIT_TERMINAL_PROMPT = '0'
$vault = 'C:\Users\XGTech_ZHU\iCloudDrive\iCloud~md~obsidian\AI赋能'
Set-Location $vault

git fetch -q origin master
$local  = (git rev-parse '@' 2>$null)
$remote = (git rev-parse '@{u}' 2>$null)
if (-not $remote -or $local -eq $remote) { exit 0 }   # 无更新

# 远程有新提交：rebase 拉取，自动暂存本地未提交改动再回放
git pull --rebase --autostash origin master 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
  git rebase --abort 2>$null | Out-Null
  Write-Host "CONFLICT: 自动拉取遇冲突，已回滚，请手动 git pull --rebase 解决"
}
exit 0
