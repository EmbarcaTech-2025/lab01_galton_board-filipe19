//Configurações e estruturas de dados

#ifndef GALTON_CONFIG_H
#define GALTON_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"

/* Configurações do display OLED */
#define SDA_PIN 14
#define SCL_PIN 15
#define I2C_PORT i2c1
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define SSD1306_BUFFER_SIZE (OLED_WIDTH * OLED_HEIGHT / 8)

/* Controle de partículas */
#define MAX_PARTICLES 15
#define PARTICLES_PER_SECOND 1
#define BALL_DIAMETER 1
extern float GRAVITY;
extern float BOUNCINESS;

/* Configuração dos pinos da placa de Galton */
#define PIN_ROWS 5
extern int PIN_DIAMETER;
extern int PIN_SPACING_HORIZONTAL;
extern int PIN_SPACING_VERTICAL;

/* Configuração das canaletas e receptáculos */
#define NUM_BINS 6
extern int CHUTE_WIDTH;
extern int BIN_WIDTH;
extern int WALL_OFFSET;
extern int MAX_HISTOGRAM_HEIGHT;

/* Controle de tempo e desempenho */
extern int TICK_DELAY_MS;

/* Configuração dos botões de controle */
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define DEBOUNCE_MS 200

/* Estrutura para representar uma partícula/bola */
typedef struct {
    float x, y;
    float vx, vy;
    bool active;
    int bin_position;
} Particle;

/* Estrutura para representar um pino da placa de Galton */
typedef struct {
    int x, y;
} Pin;

/* Variáveis globais */
extern uint8_t oled_buffer[SSD1306_BUFFER_SIZE];
extern struct render_area oled_area;
extern Particle particles[MAX_PARTICLES];
extern Pin pins[PIN_ROWS * (PIN_ROWS + 1) / 2];
extern uint16_t histogram[NUM_BINS];
extern uint32_t total_particles;
extern absolute_time_t last_particle_time;
extern absolute_time_t last_button_time;
extern float BALANCE_BIAS;
extern int BALLS_PER_DROP;
extern int WALL_LEFT, WALL_RIGHT;
extern int HISTOGRAM_BASE_Y;
extern int CHUTE_LEFT, CHUTE_RIGHT;

/* Declarações de funções */
void setup_display();
void setup_buttons();
void calculate_geometry();
void initialize_pins();
void init_particle(int index);
void check_buttons();
bool random_decision_with_bias();
void check_pin_collisions(int idx);
void normalize_histogram();
void update_particles();
void render_oled();
void setup();

#endif