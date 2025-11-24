#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "motor.h"
#include "motor_task.h"


void motor_task(void *params){
    uint8_t pwm_value = 0;          // Último comando PWM recebido
    uint8_t vib_cmd = 0;            // Comando de vibração (1,2,3)

    ESP_LOGI("TASK MOTOR", "Task de motor iniciada!");

    while (1)
    {
        // ================================================================
        // 1. PRIORIDADE MÁXIMA : VIBRAÇÕES
        // ================================================================
        if (xQueueReceive(vibrationQueue, &vib_cmd, 0)) 
        {
            ESP_LOGI("TASK MOTOR VIBRAÇÃO", "Comando de vibração recebido: %d", vib_cmd);

            switch (vib_cmd)
            {
                case 1: vibrar_curto(); break;
                case 2: vibrar_medio(); break;
                case 3: vibrar_longo(); break;
                default:
                    ESP_LOGW("TASK MOTOR VIBRAÇÃO", "Vibração desconhecida: %d", vib_cmd);
            }

            setar_velocidade(0);
        }

        // ================================================================
        // 2. PWM CONTÍNUO (SEGUNDA prioridade)
        // ================================================================
        if (xQueueReceive(pwmQueue, &pwm_value, 0)) 
        {
            if (pwm_value > 255) pwm_value = 255;

            setar_velocidade(pwm_value);

            ESP_LOGI("TASK MOTOR PWM", "Comando de PWM recebido; Atualizando PWM: %d", pwm_value);
        }

        // Evita 100% uso CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// função teste para receber dados do bluetooth futuramente
void bluetooth_motor_task(void *params){
    uint8_t op = 0;
    uint8_t v = 0;
    uint8_t pwm = 100;

    while(1){

        switch(op) {
            case 0x00: // PWM
                xQueueSend(pwmQueue, &pwm, 0);
                break;
                
                case 0x01: // vibração curta
                uint8_t v = 1;
                xQueueSend(vibrationQueue, &v, 0);
                break;

                case 0x02: // vibração média
                v = 2;
                xQueueSend(vibrationQueue, &v, 0);
                break;
                
                case 0x03: // vibração longa
                v = 3;
                xQueueSend(vibrationQueue, &v, 0);
                break;
        }

        op = (op + 1) % 4;

        vTaskDelay(pdMS_TO_TICKS(1000));
        
            
    }
}

