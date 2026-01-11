#include "joystick_task.h"
#include "joystick.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "JOYSTICK_TASK";

Joystick_t joystick_direita;
Joystick_t joystick_esquerda;

const char* obter_estado(int valor, const char* nome_min, const char* nome_max) {
    if (valor <= (CENTRO_TEORICO - DEADZONE)) {
        return nome_min;
    } else if (valor >= (CENTRO_TEORICO + DEADZONE)) {
        return nome_max;
    } else {
        return "◉";
    }
}
void joystick_task(void *pvParameters) {
    int valor_dir_x, valor_dir_y, btn_dir;
    int valor_esq_x, valor_esq_y, btn_esq;

    while (1) {
        // --- Leitura Direita ---
        valor_dir_x = ler_joystick_x(joystick_direita);
        valor_dir_y = ler_joystick_y(joystick_direita);
        btn_dir   = ler_joystick_botao(joystick_direita);

        // --- Leitura Esquerda ---
        valor_esq_x = ler_joystick_x(joystick_esquerda);
        valor_esq_y = ler_joystick_y(joystick_esquerda);
        btn_esq   = ler_joystick_botao(joystick_esquerda);

        // --- Estados Direita ---
        const char* texto_dir_x = obter_estado(valor_dir_x, "↑", "↓");
        const char* texto_dir_y = obter_estado(valor_dir_y, "→", "←");
        const char* texto_dir_botao = (btn_dir == 0) ? "PRESSIONADO" : "SOLTO";

        // --- Estados Esquerda ---
        const char* texto_esq_x = obter_estado(valor_dir_x, "↑", "↓");
        const char* texto_esq_y = obter_estado(valor_dir_y, "→", "←");
        const char* texto_esq_botao = (btn_esq == 0) ? "PRESSIONADO" : "SOLTO";

        ESP_LOGI(TAG, "JOYSTICK DIREITA: [X: %s (%d)] | [Y: %s (%d)] | [Botao: %s ] || JOYSTICK ESQUERDA: [X: %s (%d)] | [Y: %s (%d)] | [Botao: %s ]", 
                 texto_dir_x, valor_dir_x, 
                 texto_dir_y, valor_dir_y,
                 texto_dir_botao,
                 texto_esq_x, valor_esq_x, 
                 texto_esq_y, valor_esq_y,
                 texto_esq_botao);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}