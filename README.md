# High-Freq-Signal-Counter 高频脉冲计数器

This is a counter prototype for incoming TTL signal with frequency upto 20Mhz. 
It uses 8/12 bits IC counter (LS160) to receive signals, 
and Arduino UNO to record total counts and write to USB devices through USB control chip (CH376s) every 30s.
To use the prototype, run and upload `pulseCounter` or `pulseCounter12bits` for 8 bits and 12 bits counter respectively in Arduino IDE.
Need to include library `TimerOne` and `usbCh376s` in Arduino IDE. <br/><br/>
本原型机利用8/12位计数器(LS160)接收高速脉冲信号(约20Mhz)，当计数器数值溢出时向Arduino UNO开发板输出信号，
统计总脉冲数，并每30秒钟通过USB主接口模块(CH376s)向USB设备写数据文件。
原型机使用的程序文件为`pulseCounter`和`pulseCounter12bits`，前者需使用8位计数器，后者使用12位计数器。
同时还需要添加`TimerOne`和`usbCh376s`两个库文件。<br/><br/>

<img src=https://github.com/zhouyuq6/High-Fequency-Pulse-Counter/blob/master/picture%20of%20prototype.jpg width="500">
