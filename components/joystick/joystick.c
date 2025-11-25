#include "joystick.h"
#include "esp_log.h"

static const char *TAG = "JOYSTICK_DRIVER";

static adc_oneshot_unit_handle_t adc_handle;

static adc_channel_t g_canal_x;
static adc_channel_t g_canal_y;

void setup_joystick(adc_channel_t canal_x, adc_channel_t canal_y) {
    
    g_canal_x = canal_x;
    g_canal_y = canal_y;

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