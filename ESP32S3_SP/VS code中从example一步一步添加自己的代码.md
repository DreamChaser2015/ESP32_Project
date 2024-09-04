# VS code中从ESP32 ESP-IDF example一步一步添加自己的代码

硬件使用酷世的ESP32S3_SP V2 development板，目标是以ESP-IDF的demo工程一步一步实现 development板上所有硬件的驱动。

淘宝链接：[酷世DIY](https://item.taobao.com/item.htm?spm=a1z0d.6639537/202406.item.d667230365314.7aab74842aCL8O&id=667230365314&from=cart&pisk=fpPji0DcInxf_IzSjxQrF2_imxh_1o1F1FgT-Pd2Wjhxffa_JAdNnjks1unzgmzqMcG_5yHTbcz2o8qUJIPVnfRsifcOYM5FTq4msfeCPIWDmQ3sk1JtXfC1PT9GYM5F1tL-1isUQyvqG0gs2q3tBIQ5wVgK6qHxXai-52v9DlhOPzno7K3vHCnJy20HkviwRV6jmr_8LKPrbiG3k0OQr7gAOD1n2CQ3wqaSfrBM6CFSlxNwDdDiLxyT-vV4M1Oogyw-w4VNqIn_e2aZN5sDgccx-v2EFnQmuRUuBmVCfBiYi8NsaWIMTkZ42WlaMNJ7vAibeSHWWChrG8Z_CXKRrqyTrRn0OGOIDS47LAwcApqLzyPmE5sB2c2q-b3u_gAxf83O4SAEApnMCUMH14iFPa9MQTNbIXITrRSxH40jUa_WDRDxr4iFPa9MIx3ol275PneG.&sku_properties=-1:-14)

IDF版本：5.3.0

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

> NOTE: 和LED WS2812引脚(GPIO38)冲突

![Snipaste_2024-08-26_17-31-48](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202408261749368.png)

SDK配置：

![Snipaste_2024-08-26_18-13-49](https://gitee.com/DreamChaser2015/drawbed/raw/master/ESP32/202408261814820.png)

一直报`I BOD: Brownout detector was triggered`错误，更换电源芯片的电感，解决操作sdcard时，3.3v被拉低的问题。



## LVGL 9.1移植

- [ESP Component Registry (espressif.com)](https://components.espressif.com/)找到移植好的esp32 lvgl组件，然后在vscode终端输入：

  ```
  idf.py add-dependency "espressif/esp_lvgl_port^2.3.1"
  ```

  执行完指令之后，在main文件夹下的`idf_component.yml`会新增lvgl，编译时会自动拉取lvgl代码：

  ```
  dependencies:
    espressif/esp_lvgl_port: "^2.3.1"
    espressif/mpu6050: "^1.2.0"
    idf: ">=5.3"
  ```

- 编译时会自动拉取esp_lvgl_port仓库

- 将managed_components\espressif_esp_lvgl_port\test_apps\main\test.c拷贝到项目main文件夹下，并将其添加到main文件夹下的CMakeLists.txt中。

- 修改spi对应引脚

  ```
  /* LCD pins */
  #define EXAMPLE_LCD_GPIO_SCLK       (GPIO_NUM_21)
  #define EXAMPLE_LCD_GPIO_MOSI       (GPIO_NUM_47)
  #define EXAMPLE_LCD_GPIO_RST        (GPIO_NUM_NC)
  #define EXAMPLE_LCD_GPIO_DC         (GPIO_NUM_45)
  #define EXAMPLE_LCD_GPIO_CS         (GPIO_NUM_14)
  #define EXAMPLE_LCD_GPIO_BL         (GPIO_NUM_48)
  ```

  

- 重新编译

TODO: 

- 旋转屏幕时需要设置gap偏移才能正确显示
- 重力感应屏幕旋转不太稳定
- 按键旋转屏幕几次之后，屏幕不刷新了。具体反映为进度条不走了





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



## Wifi



## BLE





## LED WS2812

> NOTE: 用GPIO38 和sdcard引脚冲突



