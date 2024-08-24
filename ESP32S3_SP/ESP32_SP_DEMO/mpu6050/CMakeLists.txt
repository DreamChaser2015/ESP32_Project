set(srcs "mpu6050.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
