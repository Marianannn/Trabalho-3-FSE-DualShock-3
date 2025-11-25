#pragma once
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

#define CENTRO_TEORICO 2048
#define DEADZONE       530

void setup_joystick(adc_channel_t canal_x, adc_channel_t canal_y, gpio_num_t pino_botao);

int ler_joystick_x(void);
int ler_joystick_y(void);
int ler_joystick_botao(void);