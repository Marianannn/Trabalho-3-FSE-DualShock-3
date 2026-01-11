
#include "buttons.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// Array de estados dos botões
button_state_t buttons[BTN_COUNT] = {0};

// Nomes dos botões para debug
static const char *button_names[BTN_COUNT] = {
    "CROSS", "CIRCLE", "SQUARE", "TRIANGLE",
    "L1", "R1", "L2", "R2",
    "SELECT", "START", "L3", "R3",
    "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT"};

// Pinos de seleção do MUX
static const gpio_num_t mux_select_pins[4] = {
    MUX_S0_PIN, MUX_S1_PIN, MUX_S2_PIN, MUX_S3_PIN};

/**
 * @brief Inicializa os GPIOs do multiplexador
 */
void buttons_init(void)
{
    // Configura pinos de seleção como saída
    for (int i = 0; i < 4; i++)
    {
        esp_rom_gpio_pad_select_gpio(mux_select_pins[i]);
        gpio_set_direction(mux_select_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(mux_select_pins[i], 0);
    }

    // Configura pino de saída do MUX como entrada com pull-up
    // (botões geralmente conectam ao GND quando pressionados)
    esp_rom_gpio_pad_select_gpio(MUX_Z_PIN);
    gpio_set_direction(MUX_Z_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(MUX_Z_PIN, GPIO_PULLUP_ONLY);

    // Inicializa array de estados
    for (int i = 0; i < BTN_COUNT; i++)
    {
        buttons[i].current_state = false;
        buttons[i].previous_state = false;
        buttons[i].last_change = 0;
        buttons[i].just_pressed = false;
        buttons[i].just_released = false;
    }

    printf("[BUTTONS] Inicializado - %d canais configurados\n", BTN_COUNT);
}

/**
 * @brief Seleciona um canal específico do multiplexador
 * @param channel Número do canal (0-15)
 */
static void mux_select_channel(uint8_t channel)
{
    // Extrai cada bit do número do canal e seta nos pinos
    gpio_set_level(MUX_S0_PIN, (channel >> 0) & 0x01);
    gpio_set_level(MUX_S1_PIN, (channel >> 1) & 0x01);
    gpio_set_level(MUX_S2_PIN, (channel >> 2) & 0x01);
    gpio_set_level(MUX_S3_PIN, (channel >> 3) & 0x01);

    // Pequeno delay para estabilização do MUX (importante!)
    esp_rom_delay_us(10);
}

/**
 * @brief Lê o estado de um canal específico
 * @param channel Número do canal (0-15)
 * @return true se botão pressionado (nível LOW), false caso contrário
 */
static bool mux_read_channel(uint8_t channel)
{
    mux_select_channel(channel);
    // Lógica invertida: botão pressionado = nível LOW (conectado ao GND)
    return (gpio_get_level(MUX_Z_PIN) == 0);
}

/**
 * @brief Varre todos os botões e atualiza estados com debounce
 */
void buttons_scan(void)
{
    uint32_t current_time = esp_timer_get_time() / 1000; // Converte para ms

    for (int i = 0; i < BTN_COUNT; i++)
    {
        // Lê estado raw do botão
        bool raw_state = mux_read_channel(i);

        // Salva estado anterior
        buttons[i].previous_state = buttons[i].current_state;

        // Reseta flags de transição
        buttons[i].just_pressed = false;
        buttons[i].just_released = false;

        // Aplica debounce
        if (raw_state != buttons[i].current_state)
        {
            // Estado mudou - verifica tempo de debounce
            if ((current_time - buttons[i].last_change) >= DEBOUNCE_TIME_MS)
            {
                buttons[i].current_state = raw_state;
                buttons[i].last_change = current_time;

                // Detecta transições
                if (raw_state && !buttons[i].previous_state)
                {
                    buttons[i].just_pressed = true;
                }
                else if (!raw_state && buttons[i].previous_state)
                {
                    buttons[i].just_released = true;
                }
            }
        }
    }
}

/**
 * @brief Verifica se um botão está pressionado
 */
bool button_is_pressed(button_id_t btn)
{
    if (btn >= BTN_COUNT)
        return false;
    return buttons[btn].current_state;
}

/**
 * @brief Verifica se um botão acabou de ser pressionado (borda de subida)
 */
bool button_just_pressed(button_id_t btn)
{
    if (btn >= BTN_COUNT)
        return false;
    return buttons[btn].just_pressed;
}

/**
 * @brief Verifica se um botão acabou de ser solto (borda de descida)
 */
bool button_just_released(button_id_t btn)
{
    if (btn >= BTN_COUNT)
        return false;
    return buttons[btn].just_released;
}

/**
 * @brief Retorna estados de todos os botões como bitmask
 * @return uint16_t onde cada bit representa um botão
 */
uint16_t buttons_get_all_states(void)
{
    uint16_t states = 0;
    for (int i = 0; i < BTN_COUNT; i++)
    {
        if (buttons[i].current_state)
        {
            states |= (1 << i);
        }
    }
    return states;
}

/**
 * @brief Retorna nome do botão (para debug)
 */
const char *button_get_name(button_id_t btn)
{
    if (btn >= BTN_COUNT)
        return "UNKNOWN";
    return button_names[btn];
}