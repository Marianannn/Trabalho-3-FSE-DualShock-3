idf_component_register(
    SRCS "joystick.c" "joystick_task.c"
    INCLUDE_DIRS "."
    REQUIRES esp_adc log freertos)