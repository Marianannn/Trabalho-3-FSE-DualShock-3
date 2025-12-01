# Trabalho-3-FSE-DualShock-3
<p align="justify"> &emsp; &emsp;
Este repositório é designado para o trabalho 3 da disciplina de Fundamento de Sistemas Embarcados da Universidade de Brasília. O objetivo do projeto desenvolvido pela equipe é recriar funcionalidades do controle DualShock 3 da empresa Sony.
</p>

## 👤 Integrantes

|  | Nome | Matricula |
|------|-----------|------|
|<img src="https://avatars.githubusercontent.com/u/86726332?v=4" width="60px;"/>| [Gabrielly Assunção](https://github.com/GabriellyAssuncao) | 200018442 |
|<img src="https://avatars.githubusercontent.com/u/101186218?v=4" width="60px;"/>| [Henrique Batalha](https://github.com/HeBatalha) | 211061850 |
|<img src="https://avatars.githubusercontent.com/u/38669960?v=4" width="60px;"/>| [Laís Ramos](https://github.com/laisramos123) | 170107574 |
|<img src="https://avatars.githubusercontent.com/u/99679547?v=4" width="60px;"/>| [Mariana Letícia](https://github.com/Marianannn) | 211062230|

</center>


## 📌 Resumo do Projeto

Reprodução das principais funcionalidades do controle **DualShock 3**, incluindo:

- Leitura de joysticks analógicos e botões digitais*
- Geração de vibração
...

## 🧱 Arquitetura do Projeto

Arquivos principais:

| Componente | Arquivo | Descrição |
|-----------|---------|-----------|
| **Firmware principal** | [`main/main.c`](main/main.c) | Inicialização de periféricos, criação das tasks |
| **Controle dos motores (PWM / rumble)** | [`components/motor/motor.c`](components/motor/motor.c) | Funções para vibração e velocidade |
| **Interface das tasks de motor** | [`components/motor/motor_task.h`](components/motor/motor_task.h) | Prototipação das tasks |
| **Mapeamento de pinos** | [`main/gpio_config.h`](main/gpio_config.h) <br> [`mapeamento_pinos.md`](mapeamento_pinos.md) | Definição dos GPIOs usados |
| **Build system** | [`CMakeLists.txt`](CMakeLists.txt) | Configuração do projeto |
| **Ambiente de desenvolvimento** | [`.devcontainer/Dockerfile`](.devcontainer/Dockerfile) | DevContainer para ambiente reprodutível |

---

## 🛠️ Requisitos de Software

- **ESP-IDF** (versão compatível indicada pelo ambiente)

## 🚀 Build, Flash e Monitor

No ambiente ESP-IDF:

```sh
idf.py build
idf.py -p PORT flash monitor  # PORT ex: /dev/ttyUSB0
