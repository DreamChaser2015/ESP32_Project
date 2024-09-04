set(srcs "main.c"
    "adc_button.c"
    "ws2812_led.c"
    "mpu6050_test.c"
    "sd_test_io.c"
    "sdcard.c"
    "lvgl_user.c"
    "lvgl_test.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
