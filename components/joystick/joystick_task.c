#include "joystick_task.h"
#include "joystick.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "JOYSTICK_TASK";

const char* obter_estado(int valor, const char* nome_min, const char* nome_max) {
    if (valor <= (CENTRO_TEORICO - DEADZONE)) {
        return nome_min;
    } else if (valor >= (CENTRO_TEORICO + DEADZONE)) {
        return nome_max;
    } else {
        return "CENTRO";
    }
}
void joystick_task(void *pvParameters) {
    int valor_x, valor_y;

    while (1) {
        valor_x = ler_joystick_x();
        valor_y = ler_joystick_y();

        const char* texto_x = obter_estado(valor_x, "CIMA", "BAIXO");
        const char* texto_y = obter_estado(valor_y, "DIREITA", "ESQUERDA");

        ESP_LOGI(TAG, "Estado: [X: %s (%d)] | [Y: %s (%d)]", 
                 texto_x, valor_x, 
                 texto_y, valor_y);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}