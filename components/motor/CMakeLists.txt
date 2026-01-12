idf_component_register(
    SRCS "motor.c" "motor_task.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos
)
