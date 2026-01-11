#include "joystick.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "JOYSTICK_DRIVER";

static adc_oneshot_unit_handle_t adc_handle = NULL;

void joystick_init_adc(void) {
    if (adc_handle != NULL) {
        return;
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1, // ADC1 para pinos 32, 33, 34, 35
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    
    ESP_LOGI(TAG, "Unidade ADC1 Inicializada com Sucesso!");
}

Joystick_t setup_joystick_struct(adc_channel_t canal_x, adc_channel_t canal_y, gpio_num_t pino_botao) {
    
    joystick_init_adc();

    // Cria a estrutura local
    Joystick_t new_joystick;
    new_joystick.canal_x = canal_x;
    new_joystick.canal_y = canal_y;
    new_joystick.pino_botao = pino_botao;

    // Configura os canais especificos deste joystick
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, new_joystick.canal_x, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, new_joystick.canal_y, &config));

    // 4. Configura o botao correspondte
    gpio_set_direction(new_joystick.pino_botao, GPIO_MODE_INPUT);
    gpio_set_pull_mode(new_joystick.pino_botao, GPIO_PULLUP_ONLY);
    
    ESP_LOGI(TAG, "Joystick Configurado: X(ch%d) Y(ch%d) Btn(%d)", canal_x, canal_y, pino_botao);

    return new_joystick;
}

int ler_joystick_x(Joystick_t joystick) {
    int valor = 0;
    // Usa a variavel guardada g_canal_x
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, joystick.canal_x, &valor));
    return valor;
}

int ler_joystick_y(Joystick_t joystick) {
    int valor = 0;
    // Usa a variável guardada g_canal_y
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, joystick.canal_y, &valor));
    return valor;
}

int ler_joystick_botao(Joystick_t joystick) {
    // Retorna 0 (Pressionado) ou 1 (Solto)
    return gpio_get_level(joystick.pino_botao);
}