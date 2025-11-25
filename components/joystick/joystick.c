#include "joystick.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "JOYSTICK_DRIVER";

static adc_oneshot_unit_handle_t adc_handle;

static adc_channel_t g_canal_x;
static adc_channel_t g_canal_y;
static gpio_num_t g_pino_botao;

void setup_joystick(adc_channel_t canal_x, adc_channel_t canal_y, gpio_num_t pino_botao) {
    
    g_canal_x = canal_x;
    g_canal_y = canal_y;
    g_pino_botao = pino_botao;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2, // Atenção: Se mudar pino, confirmar se ainda e Unit 2
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, g_canal_x, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, g_canal_y, &config));

    gpio_set_direction(g_pino_botao, GPIO_MODE_INPUT);

    gpio_set_pull_mode(g_pino_botao, GPIO_PULLUP_ONLY);
    
    ESP_LOGI(TAG, "Joystick Configurado nos canais %d e %d", g_canal_x, g_canal_y);
}

int ler_joystick_x(void) {
    int valor = 0;
    // Usa a variavel guardada g_canal_x
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, g_canal_x, &valor));
    return valor;
}

int ler_joystick_y(void) {
    int valor = 0;
    // Usa a variável guardada g_canal_y
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, g_canal_y, &valor));
    return valor;
}

int ler_joystick_botao(void) {
    // Retorna 0 (Pressionado) ou 1 (Solto)
    return gpio_get_level(g_pino_botao);
}