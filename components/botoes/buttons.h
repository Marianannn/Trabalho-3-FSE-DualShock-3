// buttons.h - Header para o módulo de botões
#pragma once

#include <stdint.h>
#include <stdbool.h>

// Definições do Multiplexador
#define MUX_S0_PIN 12
#define MUX_S1_PIN 13
#define MUX_S2_PIN 14
#define MUX_S3_PIN 27
#define MUX_Z_PIN 26

// Número de canais do MUX (16 canais = 4 bits de seleção)
#define MUX_NUM_CHANNELS 16

// Tempo de debounce em ms
#define DEBOUNCE_TIME_MS 50

// Mapeamento dos botões do DualShock nos canais do MUX
typedef enum
{
    BTN_CROSS = 0,    // X
    BTN_CIRCLE = 1,   // O
    BTN_SQUARE = 2,   // Quadrado
    BTN_TRIANGLE = 3, // Triângulo
    BTN_L1 = 4,
    BTN_R1 = 5,
    BTN_L2 = 6,
    BTN_R2 = 7,
    BTN_SELECT = 8,
    BTN_START = 9,
    BTN_L3 = 10, // Clique joystick esquerdo
    BTN_R3 = 11, // Clique joystick direito
    BTN_DPAD_UP = 12,
    BTN_DPAD_DOWN = 13,
    BTN_DPAD_LEFT = 14,
    BTN_DPAD_RIGHT = 15,
    BTN_COUNT = 16
} button_id_t;

// Estrutura para armazenar estado de um botão
typedef struct
{
    bool current_state;   // Estado atual (true = pressionado)
    bool previous_state;  // Estado anterior
    uint32_t last_change; // Timestamp da última mudança (para debounce)
    bool just_pressed;    // Flag: acabou de ser pressionado
    bool just_released;   // Flag: acabou de ser solto
} button_state_t;

// Array global de estados dos botões
extern button_state_t buttons[BTN_COUNT];

// Funções públicas
void buttons_init(void);
void buttons_scan(void);
bool button_is_pressed(button_id_t btn);
bool button_just_pressed(button_id_t btn);
bool button_just_released(button_id_t btn);
uint16_t buttons_get_all_states(void);

// Função para obter nome do botão (debug)
const char *button_get_name(button_id_t btn);