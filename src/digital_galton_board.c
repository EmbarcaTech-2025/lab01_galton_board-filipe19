//----------------------------------------------------------------------------------
/**
 * Digital Galton Board - Simulação da distribuição binomial
 * Embarcatech, April 2025
 * Autor: Filipe Alves de Sousa
 * 
 * Este programa simula uma placa de Galton digital, demonstrando:
 * - Distribuição binomial e Lei dos Grandes Números
 * - Sistema de coordenadas e temporização em ticks
 * - Geração de números aleatórios para tomada de decisão
 * - Interface com display OLED e controles
 */

//----------------------------------------------------------------------------------

/**
 * Placa de Galton Digital com Controles por Botão
 * 
 * Funcionalidades:
 * - Simulação física de bolas caindo através de pinos
 * - Controle por botões:
 *   - Botão A (GPIO 5): Controla número de bolas simultâneas (1-5)
 *   - Botão B (GPIO 6): Controla desbalanceamento (0-10)
 * - Exibição de:
 *   - Total de bolas lançadas
 *   - Número atual de bolas por liberação
 *   - Nível de desbalanceamento -->[5]=Balanceado; [1 - 4]=tendencia direita; [6-10]=tendencia esquerda.
 * - Histograma normalizado automaticamente
 * - Decisões aleatórias com viés ajustável nas colisões
 */
// ==============================================
// CONFIGURAÇÕES DE HARDWARE E CONSTANTES
// ==============================================

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
#define SDA_PIN 14                  // Pino I2C SDA
#define SCL_PIN 15                  // Pino I2C SCL
#define I2C_PORT i2c1               // Porta I2C utilizada
#define OLED_WIDTH 128              // Largura do display em pixels
#define OLED_HEIGHT 64              // Altura do display em pixels

/* Controle de partículas */
#define MAX_PARTICLES 15            // Número máximo de bolas na tela simultaneamente
#define PARTICLES_PER_SECOND 1      // Taxa base de liberação de bolas (por segundo)
#define BALL_DIAMETER 1             // Diâmetro visual das bolas em pixels
float GRAVITY = 0.2f;               // Aceleração gravitacional (ajusta velocidade de queda)
float BOUNCINESS = 0.3f;            // Coeficiente de elasticidade nas colisões

/* Configuração dos pinos da placa de Galton */
#define PIN_ROWS 5                  // Número de linhas de pinos
int PIN_DIAMETER = 3;               // Diâmetro visual dos pinos em pixels
int PIN_SPACING_HORIZONTAL = 9;     // Espaçamento horizontal entre pinos
int PIN_SPACING_VERTICAL = 7;       // Espaçamento vertical entre linhas de pinos

/* Configuração das canaletas e receptáculos */
#define NUM_BINS 6                  // Número de canaletas receptoras (deve ser ímpar para distribuição simétrica)
int CHUTE_WIDTH = 0;                // Largura inicial da canaleta de entrada 
int BIN_WIDTH = 9;                  // Largura de cada canaleta do histograma
int WALL_OFFSET = 0;                // Distância das paredes laterais até a primeira canaleta
int MAX_HISTOGRAM_HEIGHT = 13;      // Altura máxima do histograma em pixels

/* Controle de tempo e desempenho */
int TICK_DELAY_MS = 35;             // Intervalo entre atualizações da simulação (em milissegundos)

/* Configuração dos botões de controle */
#define BUTTON_A_PIN 5              // GPIO para botão A (controla número de bolas)
#define BUTTON_B_PIN 6              // GPIO para botão B (controla desbalanceamento)
#define DEBOUNCE_MS 200             // Tempo de debounce para evitar leituras múltiplas

// ==============================================
// ESTRUTURAS DE DADOS E VARIÁVEIS GLOBAIS
// ==============================================

/* Estrutura para representar uma partícula/bola */
typedef struct {
    float x, y;             // Posição atual (coordenadas)
    float vx, vy;           // Velocidade nos eixos x e y
    bool active;            // Flag indicando se a partícula está ativa
    int bin_position;       // Índice da canaleta onde a partícula terminou (-1 se ainda em movimento)
} Particle;

/* Estrutura para representar um pino da placa de Galton */
typedef struct {
    int x, y;               // Posição do pino (coordenadas)
} Pin;

// Buffer e área de renderização do display OLED
uint8_t oled_buffer[ssd1306_buffer_length];
struct render_area oled_area = {0, OLED_WIDTH - 1, 0, ssd1306_n_pages - 1};

// Arrays para partículas e pinos
Particle particles[MAX_PARTICLES];
Pin pins[PIN_ROWS * (PIN_ROWS + 1) / 2];  // Número triangular para disposição dos pinos

// Histograma e contadores
uint16_t histogram[NUM_BINS] = {0};       // Contagem de bolas por canaleta
uint32_t total_particles = 0;             // Contador total de bolas lançadas

// Variáveis de tempo
absolute_time_t last_particle_time;       // Último momento em que bolas foram liberadas
absolute_time_t last_button_time;         // Último momento em que botões foram pressionados

// Controles ajustáveis pelo usuário
float BALANCE_BIAS = 5.0f;               // Fator de desbalanceamento (0-10, padrão 5 = balanceado)
int BALLS_PER_DROP = 1;                  // Número de bolas liberadas por ciclo (1-5)

// Posições calculadas das estruturas
int WALL_LEFT, WALL_RIGHT;               // Posições das paredes laterais
int HISTOGRAM_BASE_Y = OLED_HEIGHT - 2;  // Posição vertical base do histograma
int CHUTE_LEFT, CHUTE_RIGHT;             // Limites da canaleta de entrada



// ==============================================
// FUNÇÕES DE INICIALIZAÇÃO E CONFIGURAÇÃO
// ==============================================

/**
 * Inicializa o display OLED
 * Configura a interface I2C e prepara o display para receber dados
 */
void setup_display() {
    // Inicializa a interface I2C na velocidade padrão de 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    
    // Configura os pinos GPIO para função I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    
    // Habilita resistores de pull-up nos pinos I2C
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    
    // Inicializa o display OLED
    ssd1306_init();
    
    // Limpa o buffer do display
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

/**
 * Inicializa os botões de controle
 * Configura os pinos GPIO como entradas com pull-up
 */
void setup_buttons() {
    // Inicializa os pinos dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    
    // Configura os pinos como entrada
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    
    // Habilita resistores de pull-up internos
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    
    // Inicializa o tempo do último pressionamento
    last_button_time = get_absolute_time();
}

/**
 * Calcula as posições geométricas dos elementos na tela
 * Baseado nas constantes definidas no início do código
 */
void calculate_geometry() {
    // Calcula a largura total necessária para todas as canaletas
    int total_width = NUM_BINS * BIN_WIDTH;
    
    // Calcula as posições das paredes laterais, centralizando o conjunto
    WALL_LEFT = (OLED_WIDTH - total_width) / 2 - WALL_OFFSET;
    WALL_RIGHT = WALL_LEFT + total_width + 2 * WALL_OFFSET;
    
    // Calcula os limites da canaleta de entrada (centralizada)
    CHUTE_LEFT = OLED_WIDTH/2 - CHUTE_WIDTH/2;
    CHUTE_RIGHT = OLED_WIDTH/2 + CHUTE_WIDTH/2;
}

/**
 * Inicializa os pinos da placa de Galton em padrão triangular
 * A disposição segue o padrão triangular clássico da placa de Galton
 */
void initialize_pins() {
    int idx = 0;  // Índice para o array de pinos
    
    // Para cada linha de pinos
    for (int row = 0; row < PIN_ROWS; row++) {
        int pins_in_row = row + 1;  // Número de pinos nesta linha
        
        // Calcula a posição x inicial para centralizar os pinos
        int start_x = OLED_WIDTH/2 - (pins_in_row-1)*PIN_SPACING_HORIZONTAL/2;
        
        // Posiciona cada pino da linha
        for (int col = 0; col < pins_in_row; col++) {
            pins[idx].x = start_x + col * PIN_SPACING_HORIZONTAL;
            pins[idx].y = 15 + row * PIN_SPACING_VERTICAL;
            idx++;
        }
    }
}

// ==============================================
// FUNÇÕES DE CONTROLE E SIMULAÇÃO
// ==============================================

/**
 * Cria uma nova partícula/bola na canaleta superior
 * @param index Índice no array de partículas a ser inicializado
 */
void init_particle(int index) {
    // Inicializa a partícula com posição central e pequena variação aleatória
    particles[index] = (Particle){
        .x = OLED_WIDTH / 2 + (rand() % CHUTE_WIDTH - CHUTE_WIDTH/2),
        .y = 5,
        .vx = 0,
        .vy = 0,
        .active = true,
        .bin_position = -1
    };
    
    // Garante que a partícula comece dentro dos limites da canaleta
    if (particles[index].x < CHUTE_LEFT) particles[index].x = CHUTE_LEFT;
    if (particles[index].x > CHUTE_RIGHT) particles[index].x = CHUTE_RIGHT;
}

/**
 * Verifica o estado dos botões e atualiza as configurações
 * Implementa debounce para evitar leituras múltiplias
 */
void check_buttons() {
    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_button_time, now) / 1000;
    
    // Verifica o debounce - ignora pressionamentos muito próximos
    if (time_since_last < DEBOUNCE_MS) return;
    
    // Botão A (GPIO 5) - controla número de bolas por liberação (1-5)
    if (!gpio_get(BUTTON_A_PIN)) {
        BALLS_PER_DROP++;
        if (BALLS_PER_DROP > 5) BALLS_PER_DROP = 1;
        last_button_time = now;
    }
    
    // Botão B (GPIO 6) - controla desbalanceamento (0-10)
    if (!gpio_get(BUTTON_B_PIN)) {
        BALANCE_BIAS += 1.0f;
        if (BALANCE_BIAS > 10.0f) BALANCE_BIAS = 0.0f;
        last_button_time = now;
    }
}

/**
 * Função de decisão aleatória com viés ajustável
 * @return true para direita, false para esquerda
 * O viés é controlado pela variável BALANCE_BIAS (0-10)
 */
bool random_decision_with_bias() {
    // Gera número aleatório entre 0 e 99
    int rand_val = rand() % 100;
    
    // Calcula o threshold baseado no BALANCE_BIAS (0-10)
    // Mapeia 0 → 0%, 5 → 50%, 10 → 100%
    int threshold = (int)(BALANCE_BIAS * 10);
    
    // Limita o threshold entre 5% e 95% para evitar extremos absolutos
    if (threshold < 5) threshold = 5;
    if (threshold > 95) threshold = 95;
    
    return (rand_val < threshold);
}

/**
 * Verifica colisões entre uma partícula e os pinos
 * @param idx Índice da partícula a ser verificada
 */
void check_pin_collisions(int idx) {
    Particle *p = &particles[idx];
    
    // Verifica colisão com cada pino
    for (int i = 0; i < (PIN_ROWS * (PIN_ROWS + 1) / 2); i++) {
        float dx = p->x - pins[i].x;
        float dy = p->y - pins[i].y;
        float distance = sqrtf(dx*dx + dy*dy);
        
        // Se houve colisão (distância menor que a soma dos raios)
        if (distance < (PIN_DIAMETER + BALL_DIAMETER)/2) {
            // Toma decisão aleatória com viés
            if (random_decision_with_bias()) {
                p->vx = PIN_SPACING_HORIZONTAL * 0.06f;  // Direita
            } else {
                p->vx = -PIN_SPACING_HORIZONTAL * 0.06f; // Esquerda
            }
            // Aplica o coeficiente de elasticidade
            p->vy = -p->vy * BOUNCINESS;
            break;
        }
    }
}

/**
 * Normaliza o histograma para caber no espaço disponível
 * Mantém as proporções entre as barras
 */
void normalize_histogram() {
    // Encontra o valor máximo atual no histograma
    uint16_t max_val = 1;  // Evita divisão por zero
    
    for (int i = 0; i < NUM_BINS; i++) {
        if (histogram[i] > max_val) {
            max_val = histogram[i];
        }
    }
    
    // Se já está dentro do limite, não faz nada
    if (max_val <= MAX_HISTOGRAM_HEIGHT) return;
    
    // Redimensiona todos os valores proporcionalmente
    for (int i = 0; i < NUM_BINS; i++) {
        histogram[i] = (uint16_t)((float)histogram[i] / max_val * MAX_HISTOGRAM_HEIGHT);
    }
}

// ==============================================
// LÓGICA PRINCIPAL DE SIMULAÇÃO
// ==============================================

/**
 * Atualiza o estado de todas as partículas
 * Libera novas partículas conforme configuração
 * Processa física e colisões
 */
void update_particles() {
    // Primeiro verifica os botões
    check_buttons();
    
    // Verifica se é hora de liberar novas partículas
    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_particle_time, now) / 1000;
    
    // Libera novas partículas conforme a taxa configurada
    if (time_since_last > (1000 / PARTICLES_PER_SECOND)) {
        for (int i = 0; i < BALLS_PER_DROP; i++) {
            for (int j = 0; j < MAX_PARTICLES; j++) {
                if (!particles[j].active) {
                    init_particle(j);
                    total_particles++;
                    break;
                }
            }
        }
        last_particle_time = now;
    }

    // Atualiza o estado de cada partícula
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        // Aplica gravidade
        particles[i].vy += GRAVITY;
        
        // Atualiza posição
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        // Colisão com paredes laterais
        if (particles[i].x <= WALL_LEFT + BALL_DIAMETER/2) {
            particles[i].x = WALL_LEFT + BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }
        if (particles[i].x >= WALL_RIGHT - BALL_DIAMETER/2) {
            particles[i].x = WALL_RIGHT - BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }

        // Verifica colisões com pinos
        check_pin_collisions(i);

        // Verifica se chegou ao fundo
        if (particles[i].y >= HISTOGRAM_BASE_Y - BALL_DIAMETER) {
            particles[i].active = false;
            
            // Determina em qual canaleta a partícula caiu
            int bin = (particles[i].x - WALL_LEFT - WALL_OFFSET) / BIN_WIDTH;
            bin = bin < 0 ? 0 : (bin >= NUM_BINS ? NUM_BINS-1 : bin);
            
            // Atualiza o histograma
            particles[i].bin_position = bin;
            histogram[bin]++;
            
            // Normaliza periodicamente o histograma
            if (total_particles % 10 == 0) {
                normalize_histogram();
            }
        }
    }
}

// ==============================================
// FUNÇÕES DE RENDERIZAÇÃO
// ==============================================

/**
 * Renderiza toda a cena no display OLED
 * Inclui a placa de Galton, partículas, histograma e informações
 */
void render_oled() {
    // Limpa o buffer
    memset(oled_buffer, 0, sizeof(oled_buffer));

    // Desenha a canaleta inicial (de onde as bolas saem)
    for (int x = CHUTE_LEFT; x <= CHUTE_RIGHT; x++) {
        for (int y = 0; y < 5; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true);
        }
    }

    // Desenha as paredes laterais
    for (int y = 0; y < OLED_HEIGHT; y++) {
        ssd1306_set_pixel(oled_buffer, WALL_LEFT, y, true);
        ssd1306_set_pixel(oled_buffer, WALL_RIGHT, y, true);
    }

    // Desenha as divisórias das canaletas
    for (int i = 0; i <= NUM_BINS; i++) {
        int x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH;
        for (int y = HISTOGRAM_BASE_Y - MAX_HISTOGRAM_HEIGHT; y < OLED_HEIGHT; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true);
        }
    }

    // Desenha todos os pinos
    for (int i = 0; i < (PIN_ROWS * (PIN_ROWS + 1) / 2); i++) {
        int radius = PIN_DIAMETER / 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    int px = pins[i].x + dx;
                    int py = pins[i].y + dy;
                    if (px >= 0 && px < OLED_WIDTH && py >= 0 && py < OLED_HEIGHT) {
                        ssd1306_set_pixel(oled_buffer, px, py, true);
                    }
                }
            }
        }
    }

    // Desenha todas as partículas ativas
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            int radius = BALL_DIAMETER / 2;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    if (dx*dx + dy*dy <= radius*radius) {
                        int px = (int)particles[i].x + dx;
                        int py = (int)particles[i].y + dy;
                        if (px >= 0 && px < OLED_WIDTH && py >= 0 && py < OLED_HEIGHT) {
                            ssd1306_set_pixel(oled_buffer, px, py, true);
                        }
                    }
                }
            }
        }
    }

    // Desenha o histograma (barras de distribuição)
    for (int i = 0; i < NUM_BINS; i++) {
        int bar_height = histogram[i];
        int start_x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH + 1;
        
        for (int h = 0; h < bar_height; h++) {
            for (int w = 1; w < BIN_WIDTH - 1; w++) {
                ssd1306_set_pixel(oled_buffer, start_x + w, HISTOGRAM_BASE_Y - h, true);
            }
        }
    }

    // Desenha as informações de controle e status
    char info_str[16];
    
    // Número de bolas por liberação (A:X) no canto superior esquerdo
    snprintf(info_str, sizeof(info_str), "A:%d", BALLS_PER_DROP);
    ssd1306_draw_string(oled_buffer, 2, 2, info_str);
    
    // Total de bolas lançadas abaixo da informação A
    snprintf(info_str, sizeof(info_str), "T:%lu", total_particles);
    ssd1306_draw_string(oled_buffer, 2, 12, info_str);
    
    // Nível de desbalanceamento (B:Y) no canto superior direito
    snprintf(info_str, sizeof(info_str), "B:%.0f", BALANCE_BIAS);
    ssd1306_draw_string(oled_buffer, OLED_WIDTH - 24, 2, info_str);

    // Atualiza o display físico
    calculate_render_area_buffer_length(&oled_area);
    render_on_display(oled_buffer, &oled_area);
}

// ==============================================
// FUNÇÃO PRINCIPAL
// ==============================================

/**
 * Configuração inicial do sistema
 */
void setup() {
    stdio_init_all();               // Inicializa a comunicação serial (para debug)
    setup_display();                // Configura o display OLED
    setup_buttons();                // Configura os botões de controle
    calculate_geometry();           // Calcula posições dos elementos
    initialize_pins();              // Posiciona os pinos da placa de Galton
    
    // Inicializa o histograma e contadores
    memset(histogram, 0, sizeof(histogram));
    total_particles = 0;
    
    // Inicializa os tempos de controle
    last_particle_time = get_absolute_time();
    last_button_time = get_absolute_time();
    
    // Inicializa o gerador de números aleatórios
    srand(to_us_since_boot(get_absolute_time()));
}

/**
 * Loop principal do programa
 */
int main() {
    setup();
    
    // Loop infinito da simulação
    while (true) {
        update_particles();     // Atualiza a física e partículas
        render_oled();          // Desenha tudo no display
        sleep_ms(TICK_DELAY_MS); // Espera o tempo configurado
    }

    return 0;
}