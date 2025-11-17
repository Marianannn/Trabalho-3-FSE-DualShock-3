/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h> // Fornece tipos inteiros com tamanho fixo, como uint8_t, int32_t, uint64_t
#include "sdkconfig.h" // arquivo de configurações do projeto
#include "freertos/FreeRTOS.h" // Traz definições essenciais: filas, semáforos, prioridades, tempo de tick
#include "freertos/task.h" // Biblioteca específica para criação e gerenciamento de tasks (threads)
#include "esp_system.h" // Contém funções gerais do sistema
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "motor.h"





void app_main(void){
    ledc_fade_func_install(0); // habilida o efeito de fading
    setup_motores();

    


    while(true)
    {
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 1000, LEDC_FADE_WAIT_DONE);
        // ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0, 1000, LEDC_FADE_WAIT_DONE);
        printf("led desligado!!!\n");
  
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255, 2000, LEDC_FADE_WAIT_DONE);
        // ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255, 2000, LEDC_FADE_WAIT_DONE);
        printf("led ligado!!!\n");

    }
}