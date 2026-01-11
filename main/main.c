#include <stdio.h>
#include <inttypes.h>          // Fornece tipos inteiros com tamanho fixo, como uint8_t, int32_t, uint64_t
#include "sdkconfig.h"         // arquivo de configurações do projeto
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
#include "buttons.h"  
#include "gpio_config.h"

#define BUTTON_SCAN_PERIOD_MS 10

#define LED 4

extern Joystick_t joystick_direita;
extern Joystick_t joystick_esquerda;

/**
 * @brief Task para varredura contínua dos botões
 */
void button_scan_task(void *pvParameters)
{
    printf("[TASK] Button scan iniciada\n");

    while (1)
    {
        buttons_scan();
        vTaskDelay(pdMS_TO_TICKS(BUTTON_SCAN_PERIOD_MS));
    }
}

/**
 * @brief Task de debug - imprime mudanças de estado
 */
void button_debug_task(void *pvParameters)
{
    printf("[TASK] Button debug iniciada\n");

    while (1)
    {
        // Verifica transições de cada botão
        for (int i = 0; i < BTN_COUNT; i++)
        {
            if (button_just_pressed(i))
            {
                printf("[BTN] %s PRESSIONADO\n", button_get_name(i));
            }
            if (button_just_released(i))
            {
                printf("[BTN] %s SOLTO\n", button_get_name(i));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief Task de exemplo - mostra bitmask a cada segundo
 */
void button_status_task(void *pvParameters)
{
    while (1)
    {
        uint16_t states = buttons_get_all_states();
        if (states != 0)
        {
            printf("[STATUS] Botões ativos: 0x%04X\n", states);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void){

    printf("\n========================================\n");
    printf("DualShock Controller - ESP32\n");
    printf("Teste de Botões via Multiplexador\n");
    printf("========================================\n\n");

    // Inicializa módulo de botões
    buttons_init();

    // Cria tasks
    xTaskCreate(button_scan_task, "btn_scan", 2048, NULL, 10, NULL);
    xTaskCreate(button_debug_task, "btn_debug", 2048, NULL, 5, NULL);
    xTaskCreate(button_status_task, "btn_status", 2048, NULL, 3, NULL);

    // MOTORES
    setup_motores();
    habilitar_fade();
    joystick_direita = setup_joystick_struct(JOYSTICK_DIR_X, JOYSTICK_DIR_Y, JOYSTICK_DIR_BTN);
    joystick_esquerda = setup_joystick_struct(JOYSTICK_ESQ_X, JOYSTICK_ESQ_Y, JOYSTICK_ESQ_BTN);

    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(bluetooth_motor_task, "bluetooth_motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(joystick_task, "joy_task", 4096, NULL, 5, NULL);

}