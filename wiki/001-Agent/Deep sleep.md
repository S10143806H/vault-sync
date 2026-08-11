安全岛“深度睡觉”

进入： `stability_aw.serial.checked_cmd_output(dut_serial.cp0, "sleep_mode deep", ...)` 
    （向 CP0 串口发 `sleep_mode deep`）
- 确认已睡： `stability_aw.serial.wait_domains_suspended(dut_serial, domains=("cp0", "cp1"), ...)`  
    （交互式探活，确认 shell 不再响应；旧的关键字扫描已注释）