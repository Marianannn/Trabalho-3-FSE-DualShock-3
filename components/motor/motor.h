#pragma once

#define MOTOR_1 22 // por enquanto está com esses numeros por eu estar testando através de um led
#define MOTOR_2 23

void setup_channel_motores();
void setup_timer_motores();
void setup_motores();
void ligar_motores();
void desligar_motores();
void vibrar_curto();
void vibrar_medio();
void vibrar_longo();