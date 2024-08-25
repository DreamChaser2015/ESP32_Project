# VS code中从ESP32 ESP-IDF example一步一步添加自己的代码

硬件使用酷世的ESP32S3_SP V2 development板，目标是以ESP-IDF的demo工程一步一步实现 development板上所有硬件的驱动。

![](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241507770.png)

## 基于ESP-IDF LCD模板新建工程

USB连接`内置USB`，调试时会用到。连接好后设备管理器会显示两个usb设备：

![](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202408241111302.png)

![new project 1](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241459925.png)



![](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241459541.png)

![](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241500673.png)

新建工程时可能会卡住，如下图所示，一直转圈圈。点一下红色框住的地方。弹出通知，点`Yes`即可。

![new project 4](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241500100.png)

![new project 5](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241500066.png)

编译之后没有报错，烧录到板子之后监控串口打印信息，有如下错误：

![mosi not valid](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202404241500388.png)

这个时因为没有配置好GPIO导致的，按照板子配置IO：

```c
//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// Please update the following configuration according to your HardWare spec /////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LCD_HOST    SPI2_HOST

#define PIN_NUM_MISO 39 //  development板上没有接，暂时用其他io代替
#define PIN_NUM_MOSI 47
#define PIN_NUM_CLK  21
#define PIN_NUM_CS   14

#define PIN_NUM_DC   45
#define PIN_NUM_RST  38 //  development板上没有接，暂时用其他io代替
#define PIN_NUM_BCKL 48

#define LCD_BK_LIGHT_ON_LEVEL   1 // 高电平点亮背光
```

重新编译之后烧录，运行没有问题，LCD上可以显示ESP32 logo。但是显示不全，而且是颠倒的。

- 将main.c, pretty_effect.c, decode_image.c, decode_image.h中的320改为240，然后将image.jpg换为240x240的图片。
- 解决翻转的问题



## ADC按键

- pdf参考手册上的adc参考电压和网页编程指南的参考电压不一样，实测下来以网页编程指南为准，范围在 1000 mV 到 1200 mV 之间。
- 由于参考电压较低，需要配置ADC衰减`adc_pattern.atten = ADC_ATTEN_DB_12;`



## SD卡





## LVGL移植

[ESP Component Registry (espressif.com)](https://components.espressif.com/)找到需要的lvgl组件，然后在vscode终端输入：

```
idf.py add-dependency "lvgl/lvgl^9.1.0"
```



## MPU6050驱动移植

```
idf.py add-dependency "espressif/mpu6050^1.2.0"
```

出错分析：Brownout detector was triggered

```
I (204) app_init: ELF file SHA256:  f8247ce45...
I (209) app_init: ESP-IDF:          v5.3
I (214) efuse_init: Min chip rev:     v0.0
I (219) efuse_init: Max chip rev:     v0.99 
I (223) efuse_init: Chip rev:         v0.2
I (228) heap_init: Initializing. RAM available for dynamic allocation:
I (236) heap_init: At 3FC96BC8 len 00052B48 (330 KiB): RAM
I (242) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (248) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (254) heap_init: At 600FE100 len 00001EE8 (7 KiB): RTCRAM
I BOD: Brownout detector was triggered


ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x3 (RTC_SW_SYS_RST),boot:0x2a (SPI_FAST_FLASH_BOOT)
Saved PC:0x40375707
```

电源供电不足，换电源。



## Wifi + BLE





## LED WS2812



