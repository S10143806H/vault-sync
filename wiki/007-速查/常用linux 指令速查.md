![[Pasted image 20260729130815.png]]
# check bot services
```bash
(py312) gua@gua-SG0286:~/data/apps$ ps aux | grep -iE 'gtmp-analyze|feishu-gmtp'
gua      1032227  0.0  0.0 192280 10872 pts/3    S+   Jul28   0:00 journalctl --user -u GTMP-analyze-bot.service -f
gua      1535786  0.4  1.1 425132 190236 ?       Ssl  10:56   0:02 /home/gua/data/apps/GTMP-analyze-bot/.venv/bin/python /home/gua/data/apps/GTMP-analyze-bot/bot.py
gua      1541474  0.0  0.0   9300  2352 pts/8    S+   11:06   0:00 grep --color=auto -iE gtmp-analyze|feishu-gmtp
```

### Bot Services
``` bash
# check status
systemctl --user status GTMP-analyze-bot feishu-gtmp-bot

# check realtime reconnection log
journalctl --user -u feishu-gtmp-bot -f

# open_id/FEISHU_APP_ID
GTMP-analyze-bot 
ou_425d7822098a03488887f7a16aa2cc8c
cli_aab24322bc3a9bcd

feishu-gtmp-bot 
ou_05e5fd949562dd0500fe174ea8ebd751
cli_aacde73e2b789bd7

lark-cli drive +member-add \
  --token "https://guatechltd.feishu.cn/wiki/M2hqwIujpiOO3qk5klJcNvLznyb?sheet=GvJHOI" \
  --member-type appid --member-id cli_aacde73e2b789bd7 \
  --perm edit --as user --yes
```
