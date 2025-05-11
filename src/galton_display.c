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


// Renderização e inicialização
#include "galton_config.h" // Inclui configurações e definições necessárias do projeto Galton Board

// Função principal de configuração do sistema
void setup() {
    stdio_init_all(); // Inicializa a comunicação padrão (ex: saída serial para debug)

    // -------- CONFIGURAÇÃO DO DISPLAY OLED --------
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa o barramento I2C com frequência de 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C); // Define o pino SDA como função I2C
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C); // Define o pino SCL como função I2C
    gpio_pull_up(SDA_PIN); // Habilita pull-up interno no SDA (necessário para I2C)
    gpio_pull_up(SCL_PIN); // Habilita pull-up interno no SCL
    ssd1306_init(); // Inicializa o display OLED SSD1306
    memset(oled_buffer, 0, SSD1306_BUFFER_SIZE); // Limpa o buffer de imagem do display (preenche com zeros)

    // -------- CONFIGURAÇÃO DOS BOTÕES --------
    gpio_init(BUTTON_A_PIN); // Inicializa o pino do botão A
    gpio_init(BUTTON_B_PIN); // Inicializa o pino do botão B
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN); // Define botão A como entrada
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN); // Define botão B como entrada
    gpio_pull_up(BUTTON_A_PIN); // Habilita resistor de pull-up interno para botão A
    gpio_pull_up(BUTTON_B_PIN); // Habilita resistor de pull-up interno para botão B
    last_button_time = get_absolute_time(); // Salva o tempo atual para controle de debounce

    // -------- GEOMETRIA DA GALTON BOARD --------
    int total_width = NUM_BINS * BIN_WIDTH; // Largura total das canaletas (bins)
    WALL_LEFT = (OLED_WIDTH - total_width) / 2 - WALL_OFFSET; // Calcula posição da parede esquerda
    WALL_RIGHT = WALL_LEFT + total_width + 2 * WALL_OFFSET;   // Calcula posição da parede direita
    CHUTE_LEFT = OLED_WIDTH / 2 - CHUTE_WIDTH / 2; // Canaleta central (entrada de bolas)
    CHUTE_RIGHT = OLED_WIDTH / 2 + CHUTE_WIDTH / 2;

    // -------- INICIALIZA POSIÇÃO DOS PINOS --------
    int idx = 0; // Índice global dos pinos
    for (int row = 0; row < PIN_ROWS; row++) { // Para cada linha de pinos
        int pins_in_row = row + 1; // Quantidade de pinos na linha (formato triangular)
        int start_x = OLED_WIDTH / 2 - (pins_in_row - 1) * PIN_SPACING_HORIZONTAL / 2; // Centraliza linha

        for (int col = 0; col < pins_in_row; col++) { // Para cada pino da linha
            pins[idx].x = start_x + col * PIN_SPACING_HORIZONTAL; // Calcula posição X do pino
            pins[idx].y = 15 + row * PIN_SPACING_VERTICAL;         // Calcula posição Y do pino
            idx++; // Avança índice
        }
    }

    // -------- INICIALIZA HISTOGRAMA E ALEATORIEDADE --------
    memset(histogram, 0, sizeof(uint16_t) * NUM_BINS); // Zera os valores do histograma
    total_particles = 0; // Zera contador de partículas
    last_particle_time = get_absolute_time(); // Tempo da última partícula lançada
    last_button_time = get_absolute_time();   // Tempo do último botão pressionado
    srand(to_us_since_boot(get_absolute_time())); // Semente para geração aleatória baseada no tempo atual
}

// Função que renderiza toda a tela OLED a cada quadro
void render_oled() {
    memset(oled_buffer, 0, SSD1306_BUFFER_SIZE); // Limpa buffer do display (preto)

    // --- DESENHA CANALETA CENTRAL ---
    for (int x = CHUTE_LEFT; x <= CHUTE_RIGHT; x++) {
        for (int y = 0; y < 5; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true); // Marca pixels da canaleta de entrada
        }
    }

    // --- DESENHA PAREDES LATERAIS ---
    for (int y = 0; y < OLED_HEIGHT; y++) {
        ssd1306_set_pixel(oled_buffer, WALL_LEFT, y, true);  // Parede esquerda
        ssd1306_set_pixel(oled_buffer, WALL_RIGHT, y, true); // Parede direita
    }

    // --- DESENHA DIVISÓRIAS DAS CANALETAS (BINS) ---
    for (int i = 0; i <= NUM_BINS; i++) {
        int x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH;
        for (int y = HISTOGRAM_BASE_Y - MAX_HISTOGRAM_HEIGHT; y < OLED_HEIGHT; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true); // Linha vertical da divisória
        }
    }

    // --- DESENHA OS PINOS ---
    for (int i = 0; i < (PIN_ROWS * (PIN_ROWS + 1) / 2); i++) {
        int radius = PIN_DIAMETER / 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) { // Verifica se está dentro do círculo
                    int px = pins[i].x + dx;
                    int py = pins[i].y + dy;
                    if (px >= 0 && px < OLED_WIDTH && py >= 0 && py < OLED_HEIGHT) {
                        ssd1306_set_pixel(oled_buffer, px, py, true); // Marca pixel do pino
                    }
                }
            }
        }
    }

    // --- DESENHA AS PARTÍCULAS (BOLAS) ---
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) { // Se a partícula está ativa
            int radius = BALL_DIAMETER / 2;
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    if (dx*dx + dy*dy <= radius*radius) {
                        int px = (int)particles[i].x + dx;
                        int py = (int)particles[i].y + dy;
                        if (px >= 0 && px < OLED_WIDTH && py >= 0 && py < OLED_HEIGHT) {
                            ssd1306_set_pixel(oled_buffer, px, py, true); // Marca pixel da partícula
                        }
                    }
                }
            }
        }
    }

    // --- DESENHA O HISTOGRAMA ---
    for (int i = 0; i < NUM_BINS; i++) {
        int bar_height = histogram[i]; // Altura da barra atual
        int start_x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH + 1;
        for (int h = 0; h < bar_height; h++) {
            for (int w = 1; w < BIN_WIDTH - 1; w++) {
                ssd1306_set_pixel(oled_buffer, start_x + w, HISTOGRAM_BASE_Y - h, true); // Preenche barra
            }
        }
    }

    // --- EXIBE INFORMAÇÕES NO TOPO DA TELA ---
    char info_str[16];
    snprintf(info_str, sizeof(info_str), "A:%d", BALLS_PER_DROP); // Quantidade de bolas por lançamento
    ssd1306_draw_string(oled_buffer, 2, 2, info_str);

    snprintf(info_str, sizeof(info_str), "T:%lu", total_particles); // Total de bolas lançadas
    ssd1306_draw_string(oled_buffer, 2, 12, info_str);

    snprintf(info_str, sizeof(info_str), "B:%.0f", BALANCE_BIAS); // Viés da simulação (desbalanceamento)
    ssd1306_draw_string(oled_buffer, OLED_WIDTH - 24, 2, info_str);

    // --- FINALIZA E EXIBE NA TELA ---
    calculate_render_area_buffer_length(&oled_area); // Calcula área útil do buffer
    render_on_display(oled_buffer, &oled_area);      // Envia buffer para o display
}

// Função principal
int main() {
    setup(); // Inicializa o sistema

    // Loop infinito de execução
    while (true) {
        update_particles(); // Atualiza a posição das partículas
        render_oled();      // Atualiza o display com nova renderização
        sleep_ms(TICK_DELAY_MS); // Espera um tempo para manter FPS controlado
    }

    return 0;
}
