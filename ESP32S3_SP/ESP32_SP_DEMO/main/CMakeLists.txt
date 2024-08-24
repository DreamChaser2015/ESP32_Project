set(srcs "pretty_effect.c"
    "main.c"
    "decode_image.c"
    "lcd.c"
    "adc_button.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    EMBED_FILES image.jpg)
