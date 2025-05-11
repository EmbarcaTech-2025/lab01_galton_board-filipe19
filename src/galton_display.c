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


#include "galton_config.h"

void setup_display() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    ssd1306_init();
    memset(oled_buffer, 0, SSD1306_BUFFER_SIZE);
}

void setup_buttons() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    last_button_time = get_absolute_time();
}

void calculate_geometry() {
    int total_width = NUM_BINS * BIN_WIDTH;
    WALL_LEFT = (OLED_WIDTH - total_width) / 2 - WALL_OFFSET;
    WALL_RIGHT = WALL_LEFT + total_width + 2 * WALL_OFFSET;
    CHUTE_LEFT = OLED_WIDTH/2 - CHUTE_WIDTH/2;
    CHUTE_RIGHT = OLED_WIDTH/2 + CHUTE_WIDTH/2;
}

void initialize_pins() {
    int idx = 0;
    
    for (int row = 0; row < PIN_ROWS; row++) {
        int pins_in_row = row + 1;
        int start_x = OLED_WIDTH/2 - (pins_in_row-1)*PIN_SPACING_HORIZONTAL/2;
        
        for (int col = 0; col < pins_in_row; col++) {
            pins[idx].x = start_x + col * PIN_SPACING_HORIZONTAL;
            pins[idx].y = 15 + row * PIN_SPACING_VERTICAL;
            idx++;
        }
    }
}

void render_oled() {
    memset(oled_buffer, 0, SSD1306_BUFFER_SIZE);

    // Canaleta inicial
    for (int x = CHUTE_LEFT; x <= CHUTE_RIGHT; x++) {
        for (int y = 0; y < 5; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true);
        }
    }

    // Paredes laterais
    for (int y = 0; y < OLED_HEIGHT; y++) {
        ssd1306_set_pixel(oled_buffer, WALL_LEFT, y, true);
        ssd1306_set_pixel(oled_buffer, WALL_RIGHT, y, true);
    }

    // Divisórias das canaletas
    for (int i = 0; i <= NUM_BINS; i++) {
        int x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH;
        for (int y = HISTOGRAM_BASE_Y - MAX_HISTOGRAM_HEIGHT; y < OLED_HEIGHT; y++) {
            ssd1306_set_pixel(oled_buffer, x, y, true);
        }
    }

    // Pinagem
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

    // Partículas
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

    // Histograma
    for (int i = 0; i < NUM_BINS; i++) {
        int bar_height = histogram[i];
        int start_x = WALL_LEFT + WALL_OFFSET + i * BIN_WIDTH + 1;
        
        for (int h = 0; h < bar_height; h++) {
            for (int w = 1; w < BIN_WIDTH - 1; w++) {
                ssd1306_set_pixel(oled_buffer, start_x + w, HISTOGRAM_BASE_Y - h, true);
            }
        }
    }

    // Informações
    char info_str[16];
    snprintf(info_str, sizeof(info_str), "A:%d", BALLS_PER_DROP);
    ssd1306_draw_string(oled_buffer, 2, 2, info_str);
    
    snprintf(info_str, sizeof(info_str), "T:%lu", total_particles);
    ssd1306_draw_string(oled_buffer, 2, 12, info_str);
    
    snprintf(info_str, sizeof(info_str), "B:%.0f", BALANCE_BIAS);
    ssd1306_draw_string(oled_buffer, OLED_WIDTH - 24, 2, info_str);

    calculate_render_area_buffer_length(&oled_area);
    render_on_display(oled_buffer, &oled_area);
}

void setup() {
    stdio_init_all();
    setup_display();
    setup_buttons();
    calculate_geometry();
    initialize_pins();
    memset(histogram, 0, sizeof(uint16_t) * NUM_BINS);
    total_particles = 0;
    last_particle_time = get_absolute_time();
    last_button_time = get_absolute_time();
    srand(to_us_since_boot(get_absolute_time()));
}

int main() {
    setup();
    
    while (true) {
        update_particles();
        render_oled();
        sleep_ms(TICK_DELAY_MS);
    }

    return 0;
}