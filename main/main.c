#include <stdio.h>
#include <inttypes.h> // Fornece tipos inteiros com tamanho fixo, como uint8_t, int32_t, uint64_t
#include "sdkconfig.h" // arquivo de configurações do projeto
#include "freertos/FreeRTOS.h" // Traz definições essenciais: filas, semáforos, prioridades, tempo de tick
#include "freertos/task.h" // Biblioteca específica para criação e gerenciamento de tasks (threads)
#include "esp_system.h" // Contém funções gerais do sistema
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "motor.h"
#include "motor_task.h"
#include "joystick.h" 
#include "joystick_task.h" 
#include "gpio_config.h"

#define LED 4

extern Joystick_t joystick_direita;
extern Joystick_t joystick_esquerda;

void app_main(void){

    // MOTORES
    setup_motores();
    habilitar_fade();
    joystick_direita = setup_joystick_struct(JOYSTICK_DIR_X, JOYSTICK_DIR_Y, JOYSTICK_DIR_BTN);
    joystick_esquerda = setup_joystick_struct(JOYSTICK_ESQ_X, JOYSTICK_ESQ_Y, JOYSTICK_ESQ_BTN);

    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(bluetooth_motor_task, "bluetooth_motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(joystick_task, "joy_task", 4096, NULL, 5, NULL);

}