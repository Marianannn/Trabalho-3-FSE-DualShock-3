#pragma once


#define MOTOR_1 22 
#define MOTOR_2 23

typedef struct {
    int id;
    int velocidade;
    int direcao;
} ComandoMotor;

extern QueueHandle_t pwmQueue;
extern QueueHandle_t vibrationQueue;

void setup_channel_motores();
void setup_timer_motores();
void setup_motores();
void ligar_motores();
void desligar_motores();
void setar_direcao();
void setar_velocidade(int vel);
void vibrar_curto();
void vibrar_medio();
void vibrar_longo();
void habilitar_fade();
void desabilitar_fade();