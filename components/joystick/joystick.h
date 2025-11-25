#pragma once
#include "esp_adc/adc_oneshot.h"

#define CENTRO_TEORICO 2048
#define DEADZONE       530

void setup_joystick(adc_channel_t canal_x, adc_channel_t canal_y);

int ler_joystick_x(void);
int ler_joystick_y(void);