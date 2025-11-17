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

#define MOTOR_1 2 // por enquanto está com esses numeros por eu estar testando através de um led
#define MOTOR_2 2

// O duty (duty cycle) é a porcentagem de tempo que o sinal PWM fica em nível alto (1) dentro de um ciclo completo.
// ➡ Em outras palavras: é o quanto do tempo o sinal fica ligado.
// Duty é a quantidade de energia média entregue ao LED/motor


void setup_channel_motores(){
    // ====================================MOTOR 1 ==================================================================
    ledc_channel_config_t chanel_config_motor_1 = {
        .gpio_num = MOTOR_1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE, // ✔ PWM comum / LED piscando:→ LEDC_INTR_DISABLE✔ Sequência automática de fades (ex.: LED respirando, animações, máquinas de estado):→ LEDC_INTR_FADE_END
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // aperentemente determina o numero de inicio do duty
        .hpoint = 0// É o ponto inicial do contador do PWM dentro do ciclo. Se você tem vários canais PWM na mesma frequência, pode usar diferentes hpoint para evitar ruído ou picos de corrente.
    };

    ledc_channel_config(&chanel_config_motor_1);

    // ====================================MOTOR 2 ==================================================================
    // ledc_channel_config_t chanel_config_motor_2 = {
    //     .gpio_num = MOTOR_2,
    //     .speed_mode = LEDC_LOW_SPEED_MODE,
    //     .channel = LEDC_CHANNEL_1,
    //     .intr_type = LEDC_INTR_DISABLE, 
    //     .timer_sel = LEDC_TIMER_1,
    //     .duty = 0, 
    //     .hpoint = 0
    // };

    // ledc_channel_config(&chanel_config_motor_2);
}

void setup_timer_motores(){
    /*LEDC_HIGH_SPEED_MODE	High-speed	Atualiza o duty instantaneamente, sem sincronização
    LEDC_LOW_SPEED_MODE	Low-speed	    Atualização sincronizada com o timer, mais estável*/

    // ====================================MOTOR 1 ==================================================================
    ledc_timer_config_t timer_config_motor_1 = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // define qual grupo de temporizadores/canais o hardware do PWM vai usar
        .timer_num =  LEDC_TIMER_0, // qual dos timers será utilizado durante a operação
        .freq_hz = 1000, // frequencia em heartz do timer
        .duty_resolution = LEDC_TIMER_8_BIT, // Ele ajusta quantos bits o contador interno do PWM vai ter 8 bits -> 255; 10 bits-> 1023 etc.
        .clk_cfg = LEDC_AUTO_CLK // diz ao ESP32 qual clock interno o periférico PWM (LEDC) vai usar — e, no caso do AUTO, ele escolhe automaticamente o melhor clock disponível.
    };

    ledc_timer_config(&timer_config_motor_1);
    
    // ====================================MOTOR 2 ==================================================================
    // ledc_timer_config_t timer_config_motor_2 = {
    //     .speed_mode = LEDC_LOW_SPEED_MODE,
    //     .timer_num =  LEDC_TIMER_1,
    //     .freq_hz = 1000,
    //     .duty_resolution = LEDC_TIMER_8_BIT,
    //     .clk_cfg = LEDC_AUTO_CLK
    // };

    // ledc_timer_config(&timer_config_motor_2);

    ledc_fade_func_install(0); // habilida o efeito de fading
}

void app_main(void){
    setup_timer_motores();
    setup_channel_motores();


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