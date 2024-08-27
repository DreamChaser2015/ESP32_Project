set(srcs "pretty_effect.c"
    "main.c"
    "decode_image.c"
    "lcd.c"
    "adc_button.c"
    "ws2812_led.c"
    "mpu6050_test.c"
    "sd_test_io.c"
    "sdcard.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    EMBED_FILES image.jpg)
