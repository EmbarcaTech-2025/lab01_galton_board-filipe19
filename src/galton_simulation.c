// Lógica da simulação

#include "galton_config.h"  // Inclusão do cabeçalho com definições e constantes do projeto

/************ Variáveis globais de configuração ************/
float GRAVITY = 0.2f;                   // Aceleração gravitacional das partículas
float BOUNCINESS = 0.3f;               // Coeficiente de restituição (quanto a bola quica após colisão)
int PIN_DIAMETER = 3;                  // Diâmetro dos pinos na simulação
int PIN_SPACING_HORIZONTAL = 9;        // Espaçamento horizontal entre pinos
int PIN_SPACING_VERTICAL = 7;          // Espaçamento vertical entre pinos
int CHUTE_WIDTH = 0;                   // Largura do funil inicial por onde as bolas caem
int BIN_WIDTH = 9;                     // Largura de cada bin do histograma
int WALL_OFFSET = 0;                   // Compensação horizontal das paredes
int MAX_HISTOGRAM_HEIGHT = 13;         // Altura máxima (normalizada) das barras do histograma
int TICK_DELAY_MS = 35;                // Delay entre atualizações da simulação (em ms)
float BALANCE_BIAS = 5.0f;             // Tendência de desvio ao colidir com pinos (0 a 10)
int BALLS_PER_DROP = 1;                // Número de bolas lançadas a cada vez

/************ Estruturas e buffers ************/
uint8_t oled_buffer[SSD1306_BUFFER_SIZE];  // Buffer de imagem do display OLED
struct render_area oled_area = {0, OLED_WIDTH - 1, 0, ssd1306_n_pages - 1}; // Área de renderização do OLED

Particle particles[MAX_PARTICLES];         // Vetor de partículas (bolas)
Pin pins[PIN_ROWS * (PIN_ROWS + 1) / 2];   // Vetor de pinos (dispostos em pirâmide)
uint16_t histogram[NUM_BINS] = {0};        // Vetor com a contagem de partículas em cada bin
uint32_t total_particles = 0;              // Contador total de partículas lançadas

absolute_time_t last_particle_time;        // Tempo da última partícula lançada
absolute_time_t last_button_time;          // Tempo da última leitura dos botões (para debounce)

/************ Limites e controle de posição ************/
int WALL_LEFT, WALL_RIGHT;                 // Coordenadas das paredes esquerda e direita
int HISTOGRAM_BASE_Y = OLED_HEIGHT - 2;   // Posição vertical da base do histograma
int CHUTE_LEFT, CHUTE_RIGHT;              // Limites do funil inicial de lançamento

/************ Inicialização de uma partícula ************/
void init_particle(int index) {
    // Inicializa a partícula no centro com leve variação aleatória dentro do funil
    particles[index] = (Particle){
        .x = OLED_WIDTH / 2 + (rand() % CHUTE_WIDTH - CHUTE_WIDTH/2),
        .y = 5,        // Posição inicial no topo
        .vx = 0,       // Velocidade horizontal
        .vy = 0,       // Velocidade vertical
        .active = true,
        .bin_position = -1
    };

    // Garante que a partícula fique dentro dos limites do funil
    if (particles[index].x < CHUTE_LEFT) particles[index].x = CHUTE_LEFT;
    if (particles[index].x > CHUTE_RIGHT) particles[index].x = CHUTE_RIGHT;
}

/************ Leitura e ação dos botões A e B ************/
void check_buttons() {
    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_button_time, now) / 1000;

    if (time_since_last < DEBOUNCE_MS) return; // Ignora leitura se ainda dentro do tempo de debounce

    // Botão A: altera quantidade de bolas lançadas por ciclo (1 a 5)
    if (!gpio_get(BUTTON_A_PIN)) {
        BALLS_PER_DROP++;
        if (BALLS_PER_DROP > 5) BALLS_PER_DROP = 1;
        last_button_time = now;
    }

    // Botão B: altera viés de balanceamento (tendência para a direita)
    if (!gpio_get(BUTTON_B_PIN)) {
        BALANCE_BIAS += 1.0f;
        if (BALANCE_BIAS > 10.0f) BALANCE_BIAS = 0.0f;
        last_button_time = now;
    }
}

/************ Sorteio com viés ************/
bool random_decision_with_bias() {
    int rand_val = rand() % 100;
    int threshold = (int)(BALANCE_BIAS * 10);

    // Limita o viés para manter valores válidos
    if (threshold < 5) threshold = 5;
    if (threshold > 95) threshold = 95;

    return (rand_val < threshold);  // Retorna verdadeiro se dentro do limiar
}

/************ Checagem de colisão com os pinos ************/
void check_pin_collisions(int idx) {
    Particle *p = &particles[idx];

    for (int i = 0; i < (PIN_ROWS * (PIN_ROWS + 1) / 2); i++) {
        float dx = p->x - pins[i].x;
        float dy = p->y - pins[i].y;
        float distance = sqrtf(dx*dx + dy*dy);

        // Se houver colisão com um pino
        if (distance < (PIN_DIAMETER + BALL_DIAMETER)/2) {
            if (random_decision_with_bias()) {
                p->vx = PIN_SPACING_HORIZONTAL * 0.06f;   // Vai para a direita
            } else {
                p->vx = -PIN_SPACING_HORIZONTAL * 0.06f;  // Vai para a esquerda
            }
            p->vy = -p->vy * BOUNCINESS;  // Rebote vertical com perda de energia
            break;
        }
    }
}

/************ Normaliza histograma para caber no display ************/
void normalize_histogram() {
    uint16_t max_val = 1;

    // Encontra o maior valor atual
    for (int i = 0; i < NUM_BINS; i++) {
        if (histogram[i] > max_val) {
            max_val = histogram[i];
        }
    }

    // Se já está normalizado, não faz nada
    if (max_val <= MAX_HISTOGRAM_HEIGHT) return;

    // Normaliza cada bin proporcionalmente
    for (int i = 0; i < NUM_BINS; i++) {
        histogram[i] = (uint16_t)((float)histogram[i] / max_val * MAX_HISTOGRAM_HEIGHT);
    }
}

/************ Atualiza movimento das partículas ************/
void update_particles() {
    check_buttons();  // Verifica botões antes de atualizar partículas

    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_particle_time, now) / 1000;

    // Lança novas partículas se passou o tempo mínimo
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

    // Atualiza estado de cada partícula ativa
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        // Física básica: atualiza posição e velocidade
        particles[i].vy += GRAVITY;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        // Colisão com parede esquerda
        if (particles[i].x <= WALL_LEFT + BALL_DIAMETER/2) {
            particles[i].x = WALL_LEFT + BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }

        // Colisão com parede direita
        if (particles[i].x >= WALL_RIGHT - BALL_DIAMETER/2) {
            particles[i].x = WALL_RIGHT - BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }

        // Verifica colisão com pinos
        check_pin_collisions(i);

        // Se chegou na base, desativa e conta no histograma
        if (particles[i].y >= HISTOGRAM_BASE_Y - BALL_DIAMETER) {
            particles[i].active = false;

            int bin = (particles[i].x - WALL_LEFT - WALL_OFFSET) / BIN_WIDTH;
            bin = bin < 0 ? 0 : (bin >= NUM_BINS ? NUM_BINS-1 : bin);

            particles[i].bin_position = bin;
            histogram[bin]++;

            // Normaliza histograma a cada 10 partículas lançadas
            if (total_particles % 10 == 0) {
                normalize_histogram();
            }
        }
    }
}
