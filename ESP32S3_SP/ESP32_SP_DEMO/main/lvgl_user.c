#include "lvgl_user.h"
#include "freertos/idf_additions.h"
#include "lv_demos.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "string.h"

#define LCD_HOST    SPI2_HOST

#define PIN_NUM_MISO 39 //  development板上没有接，暂时用其他io代替
#define PIN_NUM_MOSI 47
#define PIN_NUM_CLK  21
#define PIN_NUM_CS   14

#define PIN_NUM_DC   45
#define PIN_NUM_RST  38 //  development板上没有接，暂时用其他io代替
#define PIN_NUM_BCKL 48

#define LCD_BK_LIGHT_ON_LEVEL   1 // 高电平点亮背光

#define LCD_H_RES               240
#define LCD_V_RES               240
#define LCD_BUF_LINES   60

#define DISPLAY_WIDTH 480
/*Static or global buffer(s). The second buffer is optional*/
#define BYTE_PER_PIXEL 2 //(LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
#define BUFF_SIZE (DISPLAY_WIDTH * 10 * BYTE_PER_PIXEL)
static uint8_t buf1[BUFF_SIZE] __attribute__((aligned(32)));
static uint8_t buf2[BUFF_SIZE] __attribute__((aligned(32)));

lv_display_t *my_disp;

spi_device_handle_t spi;

#include "../managed_components/lvgl__lvgl/src/drivers/display/st7789/lv_st7789.h"

/* Initialize LCD I/O bus, reset LCD */
static int32_t my_lcd_io_init(void)
{
    return 0;
}

/* Send command to the LCD controller */
static void my_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
    ESP_LOGW("lvgl_test", "%s", __FUNCTION__);

    // // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
    // spi_device_acquire_bus(spi, portMAX_DELAY);

    // static uint8_t test_cmd = 0x88;

    // esp_err_t ret;
    // static spi_transaction_t t;
    // //memset(&t, 0, sizeof(t));       //Zero out the transaction
    // ESP_LOGW("lvgl_test", "%d", __LINE__);
    // t.length = 8 * cmd_size;                   //Command is 8 bits
    // ESP_LOGW("lvgl_test", "%d", __LINE__);
    // t.tx_buffer = &test_cmd;             //The data is the cmd itself
    // ESP_LOGW("lvgl_test", "%d", __LINE__);
    // t.user = (void*)0;              //D/C needs to be set to 0
    // ESP_LOGW("lvgl_test", "%d", __LINE__);
    // t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer

    // ESP_LOGW("lvgl_test", "%d", __LINE__);
    // ret = spi_device_polling_transmit(spi, &t); //Transmit!
    // assert(ret == ESP_OK);

    // vTaskDelay(100 / portTICK_PERIOD_MS);

    // if (param_size)
    // {
    //     memset(&t, 0, sizeof(t));       //Zero out the transaction
    //     t.length = param_size * 8;             //Len is in bytes, transaction length is in bits.
    //     t.tx_buffer = param;             //Data
    //     t.user = (void*)1;              //D/C needs to be set to 1
    //     ret = spi_device_polling_transmit(spi, &t); //Transmit!
    // }

    // vTaskDelay(100 / portTICK_PERIOD_MS);

    // Release bus
    // spi_device_release_bus(spi);
}

/* Send pixel data to the LCD controller */
static void my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
    ESP_LOGW("lvgl_test", "%s", __FUNCTION__);

    // When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
    spi_device_acquire_bus(spi, portMAX_DELAY);

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8 * cmd_size;                   //Command is 8 bits
    t.tx_buffer = cmd;             //The data is the cmd itself
    t.user = (void*)0;              //D/C needs to be set to 0
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    ret = spi_device_polling_transmit(spi, &t); //Transmit!

    if (param_size)
    {
        memset(&t, 0, sizeof(t));       //Zero out the transaction
        t.length = param_size * 8;             //Len is in bytes, transaction length is in bits.
        t.tx_buffer = param;             //Data
        t.user = (void*)1;              //D/C needs to be set to 1
        ret = spi_device_polling_transmit(spi, &t); //Transmit!
    }

    // Release bus
    spi_device_release_bus(spi);
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void lvgl_heat_task(void * arg)
{
    while (1)
    {
        lv_tick_inc(1); // lvgl heart beat
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void lvgl_test_task(void * arg)
{
    esp_err_t ret;
    spi_bus_config_t buscfg = {
    .miso_io_num = PIN_NUM_MISO,
    .mosi_io_num = PIN_NUM_MOSI,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 16 * 240 * 2
};
spi_device_interface_config_t devcfg = {
#ifdef CONFIG_LCD_OVERCLOCK
    .clock_speed_hz = 26 * 1000 * 1000,     //Clock out at 26 MHz
#else
    .clock_speed_hz = 10 * 1000 * 1000,     //Clock out at 10 MHz
#endif
    .mode = 0,                              //SPI mode 0
    .spics_io_num = PIN_NUM_CS,             //CS pin
    .queue_size = 7,                        //We want to be able to queue 7 transactions at a time
    .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    xTaskCreate(lvgl_heat_task, "lvgl_heat_task", 128, NULL, 0, NULL);

    // ///Enable backlight
    // gpio_set_level(PIN_NUM_BCKL, LCD_BK_LIGHT_ON_LEVEL);

    // /* Initialize LVGL */
    // lv_init();

    // /* Initialize LCD bus I/O */
    // if (my_lcd_io_init() != 0)
    //         return;

    // /* Create the LVGL display object and the LCD display driver */
    // //my_disp = lv_lcd_generic_mipi_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, my_lcd_send_cmd, my_lcd_send_color);
    // my_disp = lv_st7789_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, my_lcd_send_cmd, my_lcd_send_color);

    // /* Set display orientation to landscape */
    // lv_display_set_rotation(my_disp, LV_DISPLAY_ROTATION_90);

    // /* Configure draw buffers, etc. */
    // #if 0
    // uint8_t * buf1 = NULL;
    // uint8_t * buf2 = NULL;

    // uint32_t buf_size = LCD_H_RES * LCD_V_RES / 10 * lv_color_format_get_size(lv_display_get_color_format(my_disp));

    // buf1 = lv_malloc(buf_size);
    // if(buf1 == NULL) {
    //     LV_LOG_ERROR("display draw buffer malloc failed");
    //     return;
    // }
    // lv_display_set_buffers(my_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // #else
    // lv_display_set_buffers(my_disp, buf1, buf2, BUFF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // #endif

    

    // //ui_init(my_disp);

    // //lv_demo_widgets();

    // static lv_style_t style_bg;
    // static lv_style_t style_indic;
    // lv_obj_t *bar = lv_bar_create(lv_screen_active());
    // lv_obj_remove_style_all(bar); /*To have a clean start*/
    // lv_obj_add_style(bar, &style_bg, 0);
    // lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    // lv_obj_set_size(bar, 200, 40);
    // // lv_obj_center(bar);
    // lv_obj_align(bar, LV_ALIGN_CENTER, 0, -100);
    // lv_bar_set_value(bar, 0, LV_ANIM_ON);

    static uint16_t x = 0;

    while(true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        /* Periodically call the lv_timer handler */


        // lv_timer_handler();

        // x++;
        // if (x > 500)
        // {
        //     x = 0;
        //     ESP_LOGI("lvgl_test", "xx");
        // }
        // lv_bar_set_value(bar, x / 5, LV_ANIM_ON);
    }
}














