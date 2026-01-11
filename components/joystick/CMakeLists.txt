idf_component_register(
    SRCS "joystick.c" "joystick_task.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_adc log freertos)