#pragma once
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

#define CENTRO_TEORICO 2048
#define DEADZONE       530

typedef struct {
    adc_channel_t canal_x;
    adc_channel_t canal_y;
    gpio_num_t pino_botao;
} Joystick_t;

void joystick_init_adc(void);
Joystick_t setup_joystick_struct(adc_channel_t canal_x, adc_channel_t canal_y, gpio_num_t pino_botao);

int ler_joystick_x(Joystick_t joytick_n);
int ler_joystick_y(Joystick_t joytick_n);
int ler_joystick_botao(Joystick_t joytick_n);