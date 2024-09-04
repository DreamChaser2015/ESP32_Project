/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "adc_button.h"
#include "ws2812_led.h"
#include "sdcard.h"
#include "lvgl_user.h"

extern void mpu6050_test(void * para);
extern void test_lvgl(void *arg);

void app_main(void)
{
    sdcard_test();

    // 按键检测任务
    xTaskCreate(button_task, "button_task", 4000, NULL, 0, NULL);

    // LED任务 (和SD卡io冲突)
    //xTaskCreate(led_task, "led_task", 4000, NULL, 0, NULL);

    // ui任务
    xTaskCreate(test_lvgl, "lvgl_task", 4000, NULL, 0, NULL);

    // mpu6050
    xTaskCreate(mpu6050_test, "mpu6050_task", 4000, NULL, 0, NULL);
}
