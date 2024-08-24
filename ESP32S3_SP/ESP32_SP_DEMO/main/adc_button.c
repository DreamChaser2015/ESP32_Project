/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"

#define EXAMPLE_ADC_UNIT                    ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit)         #unit
#define EXAMPLE_ADC_UNIT_STR(unit)          _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN                   ADC_ATTEN_DB_12
#define EXAMPLE_ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH


#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type2.data)

#define READ_LEN                    256

static adc_channel_t channel[1] = {ADC_CHANNEL_0};

static TaskHandle_t s_task_handle;
static const char *TAG = "ADC BUTTON";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++) {
        adc_pattern.atten = ADC_ATTEN_DB_12;
        adc_pattern.channel = channel[i] & 0x7;
        adc_pattern.unit = ADC_UNIT_1;
        adc_pattern.bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
    }
    dig_cfg.adc_pattern = &adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

#define LOW_VALUE_1 600
#define HIGH_VALUE_1 700
#define LOW_VALUE_2 3400
#define HIGH_VALUE_2 3500
#define LOW_VALUE_3 1200
#define HIGH_VALUE_3 1400
#define LOW_VALUE_4 1900
#define HIGH_VALUE_4 2100
#define LOW_VALUE_5 2600
#define HIGH_VALUE_5 2800

static uint16_t button_adc_range[4][5][2] = {

    // 方向1
    {
        {LOW_VALUE_1, HIGH_VALUE_1},     // up
        {LOW_VALUE_2, HIGH_VALUE_2},     // down
        {LOW_VALUE_3, HIGH_VALUE_3},     // left
        {LOW_VALUE_4, HIGH_VALUE_4},     // right
        {LOW_VALUE_5, HIGH_VALUE_5},     // push
    },
    // 方向2
    {
        {LOW_VALUE_3, HIGH_VALUE_3},     // up
        {LOW_VALUE_4, HIGH_VALUE_4},     // down
        {LOW_VALUE_2, HIGH_VALUE_2},     // left
        {LOW_VALUE_1, HIGH_VALUE_1},     // right
        {LOW_VALUE_5, HIGH_VALUE_5},     // push
    },
    // 方向3
    {
        {LOW_VALUE_2, HIGH_VALUE_2},     // up
        {LOW_VALUE_1, HIGH_VALUE_1},     // down
        {LOW_VALUE_4, HIGH_VALUE_4},     // left
        {LOW_VALUE_3, HIGH_VALUE_3},     // right
        {LOW_VALUE_5, HIGH_VALUE_5},     // push
    },
    // 方向4
    {
        {LOW_VALUE_4, HIGH_VALUE_4},     // up
        {LOW_VALUE_3, HIGH_VALUE_3},     // down
        {LOW_VALUE_1, HIGH_VALUE_1},     // left
        {LOW_VALUE_2, HIGH_VALUE_2},     // right
        {LOW_VALUE_5, HIGH_VALUE_5},     // push
    },
};

const char * button_sting[] = {"up", "down", "left", "right", "push"};

void button_task(void * para)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[READ_LEN] = {0};
    memset(result, 0xcc, READ_LEN);
    static uint32_t sum = 0;
    static uint32_t data;
    uint8_t button_value = 5;    // 0 up, 1 down, 2 left, 3 right, 4 push, 5 none
    uint8_t pre_button_value = 5;

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    while (1) {

        /**
         * This is to show you the way to use the ADC continuous mode driver event callback.
         * This `ulTaskNotifyTake` will block when the data processing in the task is fast.
         * However in this example, the data processing (print) is slow, so you barely block here.
         *
         * Without using this event callback (to notify this task), you can still just call
         * `adc_continuous_read()` here in a loop, with/without a certain block timeout.
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        char unit[] = EXAMPLE_ADC_UNIT_STR(EXAMPLE_ADC_UNIT);

        while (1) {
    
            ret = adc_continuous_read(handle, result, READ_LEN, &ret_num, 0);
            
            if (ret == ESP_OK) {
                //ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                sum = 0;
                for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES) {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[i];
                    uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                    data = EXAMPLE_ADC_GET_DATA(p);
                    /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                    // if (chan_num < SOC_ADC_CHANNEL_NUM(EXAMPLE_ADC_UNIT)) {
                    //     ESP_LOGI(TAG, "Unit: %s, Channel: %"PRIu32", Value: %"PRIx32", %"PRIu32, unit, chan_num, data, data);
                    // } else {
                    //     ESP_LOGW(TAG, "Invalid data [%s_%"PRIu32"_%"PRIx32"]", unit, chan_num, data);
                    // }

                    sum += data;
                }

                data = sum / (ret_num / SOC_ADC_DIGI_RESULT_BYTES);
                
                //printf("adc value: %ld\n", data);

                button_value = 5;
                for (uint32_t i = 0; i < 5; i++)
                {
                    // 使用方向1
                    if ((data > button_adc_range[0][i][0]) && (data < button_adc_range[0][i][1]))
                    {
                        button_value = i;
                        break;
                    }
                }

                if (pre_button_value != button_value)
                {
                    #if 0
                    ESP_LOGI(TAG, "%s", button_sting[button_value]);
                    #else
                    if (button_value == 0)
                    {
                        ESP_LOGI(TAG, "UP");
                    }
                    else if (button_value == 1)
                    {
                        ESP_LOGI(TAG, "DOWN");
                    }
                    else if (button_value == 2)
                    {
                        ESP_LOGI(TAG, "LEFT");
                    }
                    else if (button_value == 3)
                    {
                        ESP_LOGI(TAG, "RIGHT");
                    }
                    else if (button_value == 4)
                    {
                        ESP_LOGI(TAG, "PUSH");
                    }
                    else if (button_value == 5)
                    {
                        ESP_LOGI(TAG, "RELEASE");
                    }
                    #endif
                }

                pre_button_value = button_value;

                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */

                vTaskDelay(10 / portTICK_PERIOD_MS);
            } else if (ret == ESP_ERR_TIMEOUT) {
                //We try to read `READ_LEN` until API returns timeout, which means there's no available data
                break;
            }
        }
    }

    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}
