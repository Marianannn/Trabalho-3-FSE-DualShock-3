#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h" // Nova biblioteca da v5

static const char *TAG = "JOYSTICK_V5";

// --- Configuração dos Pinos (Para D15 e D2) ---
// D15 e D2 ficam no ADC Unit 2
#define ADC_UNIT       ADC_UNIT_2

// Canais correspondentes aos pinos:
// D15 = ADC Channel 3 (no Unit 2)
#define CANAL_X        ADC_CHANNEL_3 // TROCAR FUTURAMENTE DE ACORDO COM O DIAGRAMA
// D2  = ADC Channel 2 (no Unit 2)
#define CANAL_Y        ADC_CHANNEL_2 // TROCAR FUTURAMENTE DE ACORDO COM O DIAGRAMA
// Valores para deadzone
#define CENTRO_TEORICO 2048 // Metade do valor maximo 4095
#define DEADZONE 520

const char* obter_estado(int valor, const char* nome_min, const char* nome_max) {
    if (valor <= (CENTRO_TEORICO - DEADZONE)) {
        return nome_min;
    } else if (valor >= (CENTRO_TEORICO + DEADZONE)) {
        return nome_max;
    } else {
        return "CENTRO";
    }
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, CANAL_X, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, CANAL_Y, &config));

    ESP_LOGI(TAG, "Iniciando leitura ADC OneShot (v5)...");

    int valor_x = 0, valor_y = 0;
    //int min_x = 2048, min_y = 2048;

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, CANAL_X, &valor_x));
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, CANAL_Y, &valor_y));

        const char* texto_x = obter_estado(valor_x, "CIMA", "BAIXO");
        const char* texto_y = obter_estado(valor_y, "DIREITA", "ESQUERDA");

        ESP_LOGI(TAG, "Estado: [X: %s (%d)] | [Y: %s (%d)]", 
                 texto_x, valor_x, 
                 texto_y, valor_y);

        vTaskDelay(pdMS_TO_TICKS(200));
    }

}
    // Para testar a variacao em estado parado
    /* while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, CANAL_X, &valor_x));
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, CANAL_Y, &valor_y));

        if (valor_x < min_x) min_x = valor_x;
        if (valor_y < min_y) min_y = valor_y;

        ESP_LOGI(TAG, "X (D15): %d | Y (D2): %d | | MIN X: %d | MIN Y: %d", valor_x, valor_y, min_x, min_y);

        vTaskDelay(pdMS_TO_TICKS(200));
    } */


