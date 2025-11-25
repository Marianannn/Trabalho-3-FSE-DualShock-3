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

#define LED 4

//===============================TEMPORARIO=======================================
#define JOYSTICK_X_CHANNEL  ADC_CHANNEL_3  // Corresponde ao GPIO 15
#define JOYSTICK_Y_CHANNEL  ADC_CHANNEL_2  // Corresponde ao GPIO 2
//===============================TEMPORARIO=======================================

void app_main(void){

    // MOTORES
    setup_motores();
    habilitar_fade();
    setup_joystick(JOYSTICK_X_CHANNEL, JOYSTICK_Y_CHANNEL);

    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(bluetooth_motor_task, "bluetooth_motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(joystick_task, "joy_task", 4096, NULL, 5, NULL);

}