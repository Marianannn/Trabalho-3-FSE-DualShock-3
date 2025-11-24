#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "motor.h"



// O duty (duty cycle) é a porcentagem de tempo que o sinal PWM fica em nível alto (1) dentro de um ciclo completo.
// ➡ Em outras palavras: é o quanto do tempo o sinal fica ligado.
// Duty é a quantidade de energia média entregue ao LED/motor

QueueHandle_t pwmQueue = NULL;
QueueHandle_t vibrationQueue = NULL;

#define PWM_MAX 255
#define PWM_MIN 0


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
    ledc_channel_config_t chanel_config_motor_2 = {
        .gpio_num = MOTOR_2,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE, 
        .timer_sel = LEDC_TIMER_1,
        .duty = 0, 
        .hpoint = 0
    };

    ledc_channel_config(&chanel_config_motor_2);
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
    ledc_timer_config_t timer_config_motor_2 = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num =  LEDC_TIMER_1,
        .freq_hz = 1000,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer_config_motor_2);

    
}

void setup_motores(){
    setup_timer_motores();
    setup_channel_motores();

    // criando filas de pwm e de tipos de vibração
    pwmQueue = xQueueCreate(10, sizeof(uint8_t));
    vibrationQueue = xQueueCreate(10, sizeof(uint8_t));

    if (!pwmQueue || !vibrationQueue)
        ESP_LOGE("FILAS", "Erro ao criar filas de motor");
}

// I/O MOTORES ====================================================================================

void ligar_motores(){
    setar_velocidade(255);
    printf("motor ligado!!!\n");
    vTaskDelay(170);
}

void desligar_motores(){
    setar_velocidade(0);
    printf("motor desligado!!!\n");
    vTaskDelay(170);
}

// Setar velocidade/direções

void setar_direcao(int dir){

}

void setar_velocidade(int vel){
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);

    //motor 1
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, vel);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    //motor 2
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, vel);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}



// tipos de vibrações, TEXTURAS ========================================================================================

void vibrar_curto(){
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 150, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0, 150, LEDC_FADE_NO_WAIT);
    vTaskDelay(170);

    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255, 150, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255, 150, LEDC_FADE_NO_WAIT);   
    vTaskDelay(170);
}

void vibrar_medio(){
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 150, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0, 150, LEDC_FADE_NO_WAIT);
    vTaskDelay(170);

    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255, 250, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255, 250, LEDC_FADE_NO_WAIT);
    vTaskDelay(270);
}

void vibrar_longo(){
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 150, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0, 150, LEDC_FADE_NO_WAIT);
    vTaskDelay(170);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255, 350, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255, 350, LEDC_FADE_NO_WAIT);
    vTaskDelay(370);
}

// uso de fades

void habilitar_fade() {
    ledc_fade_func_install(0);
}

void desabilitar_fade() {
    ledc_fade_func_uninstall();
}