
## 
### LCD 
LCD Liquid Crystal Display
- 液晶是一种材料，液晶这种材料具有一种特点：可以在电信号的驱动下液晶分子进行旋转，旋转时会影响透光性，
[[Backlight]]
- Passive devices such as LCD might requires backlight to support 
- Backlight is the white light shining from the backside of a LCD panel
- Electronic signal drives the liquid molecules partially pass through the light thus shows different color
### LED/OLED
**LED**：主要用在户外大屏幕
**OLED**：有机发光二极管又称为有机电激光显示(Organic Light-Emitting Diode，OLED)，OLED显示技术具有自发光的特性，采用非常薄的有机材料涂层和玻璃基板，可以做得更轻更薄，可视角度更大，并且能够显著节省电能。
### CRT
**CRT**：阴极摄像管显示器。 以前的那种大屁股电视机就是CRT显示，它曾是应用最广泛的显示器之一，不过现在基本没有在使用这种技术了。

---
## Communication Protocal
### MIPI
cross-SoC communucation,
- applied DDIC for MIPI-Soc communciaiton protocal
#### Flowchart
TODO: attache a data flowchart here, how data across SoC displays on a target display device
- 
-  --(MIPI)-- data-buffer -- GRAM
- 目前手机屏幕和SOC间多使用MIPI接口来传输屏幕数据，其实物如下图所示，图中的条状芯片就是负责更新显示屏的显示内容的芯片DDIC, 它一边通过mipi协议和SOC通信，一边把获取到的显示数据写入到显示存储器GRAM内， 屏幕(Panel)通过不停扫描GRAM来不停更新液晶显示点的颜色，实现画面的更新。

### DP??

## GRAM

**buffer queue**
Performaance 
- CPU%
- GPU%
- **Memory -- what is  memory and how it is being used and why it matters ?**

---

## Axis
屏幕坐标系

显示屏幕采用如下图所示的二维坐标系，以屏幕左上角为原点，X方向向右，Y轴方向向下，屏幕上的显示单元（像素）以行列式整齐排列，如下图所示，如下图示中以六边形块来代表一个像素点，如无例外说明，本文中所有图示都将以该六边形块来代表屏幕上的一个像素点。

  
  
