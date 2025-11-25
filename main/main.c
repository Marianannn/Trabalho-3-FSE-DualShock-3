/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>          // Fornece tipos inteiros com tamanho fixo, como uint8_t, int32_t, uint64_t
#include "sdkconfig.h"         // arquivo de configurações do projeto
#include "freertos/FreeRTOS.h" // Traz definições essenciais: filas, semáforos, prioridades, tempo de tick
#include "freertos/task.h"     // Biblioteca específica para criação e gerenciamento de tasks (threads)
#include "esp_system.h"        // Contém funções gerais do sistema
#include "buttons.h"           // Inclui o módulo de botões definido anteriormente

#define BUTTON_SCAN_PERIOD_MS 10

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

void app_main(void)
{
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

    printf("[MAIN] Tasks criadas - sistema rodando\n\n");
}
