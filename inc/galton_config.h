// Configurações e estruturas de dados

#ifndef GALTON_CONFIG_H
#define GALTON_CONFIG_H
// Essas são diretivas do pré-processador para evitar inclusões múltiplas do mesmo arquivo

// Inclusão de bibliotecas necessárias
#include <stdio.h>      // Para funções de entrada/saída como printf
#include <string.h>     // Para funções de manipulação de strings e memória
#include <stdlib.h>     // Para funções padrão como malloc e rand
#include <math.h>       // Para funções matemáticas como sqrtf
#include "pico/stdlib.h" // SDK básico do Raspberry Pi Pico
#include "hardware/adc.h" // Para acesso ao conversor analógico-digital
#include "hardware/i2c.h" // Para comunicação I2C com o display
#include "hardware/gpio.h" // Para controle dos GPIOs
#include "hardware/pwm.h" // Para controle de PWM
#include "inc/ssd1306.h" // Biblioteca específica do display OLED

/* Configurações do display OLED */
#define SDA_PIN 14      // Pino GPIO para dados I2C (SDA)
#define SCL_PIN 15      // Pino GPIO para clock I2C (SCL)
#define I2C_PORT i2c1   // Porta I2C utilizada (a Pico tem duas: i2c0 e i2c1)
#define OLED_WIDTH 128  // Largura do display em pixels
#define OLED_HEIGHT 64  // Altura do display em pixels
#define SSD1306_BUFFER_SIZE (OLED_WIDTH * OLED_HEIGHT / 8) // Tamanho do buffer (1 bit por pixel)

/* Controle de partículas */
#define MAX_PARTICLES 15 // Número máximo de bolas na tela simultaneamente
#define PARTICLES_PER_SECOND 1 // Quantidade de bolas liberadas por segundo
#define BALL_DIAMETER 1  // Diâmetro visual das bolas em pixels
extern float GRAVITY;    // Aceleração gravitacional (será definido em .c)
extern float BOUNCINESS; // Coeficiente de elasticidade (será definido em .c)

/* Configuração dos pinos da placa de Galton */
#define PIN_ROWS 5       // Número de linhas de pinos
extern int PIN_DIAMETER; // Diâmetro visual dos pinos
extern int PIN_SPACING_HORIZONTAL; // Espaçamento horizontal entre pinos
extern int PIN_SPACING_VERTICAL;   // Espaçamento vertical entre linhas

/* Configuração das canaletas e receptáculos */
#define NUM_BINS 6       // Número de caixas coletoras (deve ser ímpar para simetria)
extern int CHUTE_WIDTH;  // Largura da canaleta inicial
extern int BIN_WIDTH;    // Largura de cada caixa coletora
extern int WALL_OFFSET;  // Distância das paredes laterais
extern int MAX_HISTOGRAM_HEIGHT; // Altura máxima do histograma

/* Controle de tempo e desempenho */
extern int TICK_DELAY_MS; // Intervalo entre atualizações da simulação

/* Configuração dos botões de controle */
#define BUTTON_A_PIN 5   // GPIO para botão A (controla número de bolas)
#define BUTTON_B_PIN 6   // GPIO para botão B (controla desbalanceamento)
#define DEBOUNCE_MS 200  // Tempo para evitar bouncing dos botões

/* Estrutura para representar uma partícula/bola */
typedef struct {
    float x, y;         // Posição atual (coordenadas)
    float vx, vy;       // Velocidade nos eixos x e y
    bool active;        // Indica se a partícula está ativa
    int bin_position;   // Índice da caixa onde caiu (-1 se ainda em movimento)
} Particle;

/* Estrutura para representar um pino da placa de Galton */
typedef struct {
    int x, y;           // Posição do pino (coordenadas)
} Pin;

/* Variáveis globais */
extern uint8_t oled_buffer[SSD1306_BUFFER_SIZE]; // Buffer para o display
extern struct render_area oled_area; // Área de renderização do display
extern Particle particles[MAX_PARTICLES]; // Array de partículas
extern Pin pins[PIN_ROWS * (PIN_ROWS + 1) / 2]; // Array de pinos (disposição triangular)
extern uint16_t histogram[NUM_BINS]; // Contagem de bolas por caixa
extern uint32_t total_particles;    // Contador total de bolas lançadas
extern absolute_time_t last_particle_time; // Último momento de liberação de bolas
extern absolute_time_t last_button_time;   // Último pressionamento de botão
extern float BALANCE_BIAS;         // Fator de desbalanceamento (0-10)
extern int BALLS_PER_DROP;         // Bolas liberadas por ciclo (1-5)
extern int WALL_LEFT, WALL_RIGHT;  // Posições das paredes laterais
extern int HISTOGRAM_BASE_Y;       // Posição vertical base do histograma
extern int CHUTE_LEFT, CHUTE_RIGHT; // Limites da canaleta inicial

/* Declarações de funções */
void setup_display();    // Configura o display OLED
void setup_buttons();    // Configura os botões
void calculate_geometry(); // Calcula posições dos elementos
void initialize_pins();  // Posiciona os pinos na tela
void init_particle(int index); // Inicializa uma partícula
void check_buttons();    // Verifica estado dos botões
bool random_decision_with_bias(); // Decisão aleatória com viés
void check_pin_collisions(int idx); // Verifica colisões com pinos
void normalize_histogram(); // Ajusta o histograma para caber na tela
void update_particles(); // Atualiza a simulação física
void render_oled();      // Renderiza tudo no display
void setup();           // Inicialização geral do sistema

#endif // Fim do header guard