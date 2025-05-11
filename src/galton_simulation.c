// Lógica da simulação

#include "galton_config.h"

/* Implementação das variáveis globais */
float GRAVITY = 0.2f;
float BOUNCINESS = 0.3f;
int PIN_DIAMETER = 3;
int PIN_SPACING_HORIZONTAL = 9;
int PIN_SPACING_VERTICAL = 7;
int CHUTE_WIDTH = 0;
int BIN_WIDTH = 9;
int WALL_OFFSET = 0;
int MAX_HISTOGRAM_HEIGHT = 13;
int TICK_DELAY_MS = 35;
float BALANCE_BIAS = 5.0f;
int BALLS_PER_DROP = 1;

uint8_t oled_buffer[SSD1306_BUFFER_SIZE];
struct render_area oled_area = {0, OLED_WIDTH - 1, 0, ssd1306_n_pages - 1};
Particle particles[MAX_PARTICLES];
Pin pins[PIN_ROWS * (PIN_ROWS + 1) / 2];
uint16_t histogram[NUM_BINS] = {0};
uint32_t total_particles = 0;
absolute_time_t last_particle_time;
absolute_time_t last_button_time;
int WALL_LEFT, WALL_RIGHT;
int HISTOGRAM_BASE_Y = OLED_HEIGHT - 2;
int CHUTE_LEFT, CHUTE_RIGHT;


/* Funções de simulação */
void init_particle(int index) {
    particles[index] = (Particle){
        .x = OLED_WIDTH / 2 + (rand() % CHUTE_WIDTH - CHUTE_WIDTH/2),
        .y = 5,
        .vx = 0,
        .vy = 0,
        .active = true,
        .bin_position = -1
    };
    
    if (particles[index].x < CHUTE_LEFT) particles[index].x = CHUTE_LEFT;
    if (particles[index].x > CHUTE_RIGHT) particles[index].x = CHUTE_RIGHT;
}

void check_buttons() {
    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_button_time, now) / 1000;
    
    if (time_since_last < DEBOUNCE_MS) return;
    
    if (!gpio_get(BUTTON_A_PIN)) {
        BALLS_PER_DROP++;
        if (BALLS_PER_DROP > 5) BALLS_PER_DROP = 1;
        last_button_time = now;
    }
    
    if (!gpio_get(BUTTON_B_PIN)) {
        BALANCE_BIAS += 1.0f;
        if (BALANCE_BIAS > 10.0f) BALANCE_BIAS = 0.0f;
        last_button_time = now;
    }
}

bool random_decision_with_bias() {
    int rand_val = rand() % 100;
    int threshold = (int)(BALANCE_BIAS * 10);
    
    if (threshold < 5) threshold = 5;
    if (threshold > 95) threshold = 95;
    
    return (rand_val < threshold);
}

void check_pin_collisions(int idx) {
    Particle *p = &particles[idx];
    
    for (int i = 0; i < (PIN_ROWS * (PIN_ROWS + 1) / 2); i++) {
        float dx = p->x - pins[i].x;
        float dy = p->y - pins[i].y;
        float distance = sqrtf(dx*dx + dy*dy);
        
        if (distance < (PIN_DIAMETER + BALL_DIAMETER)/2) {
            if (random_decision_with_bias()) {
                p->vx = PIN_SPACING_HORIZONTAL * 0.06f;
            } else {
                p->vx = -PIN_SPACING_HORIZONTAL * 0.06f;
            }
            p->vy = -p->vy * BOUNCINESS;
            break;
        }
    }
}

void normalize_histogram() {
    uint16_t max_val = 1;
    
    for (int i = 0; i < NUM_BINS; i++) {
        if (histogram[i] > max_val) {
            max_val = histogram[i];
        }
    }
    
    if (max_val <= MAX_HISTOGRAM_HEIGHT) return;
    
    for (int i = 0; i < NUM_BINS; i++) {
        histogram[i] = (uint16_t)((float)histogram[i] / max_val * MAX_HISTOGRAM_HEIGHT);
    }
}

void update_particles() {
    check_buttons();
    
    absolute_time_t now = get_absolute_time();
    int64_t time_since_last = absolute_time_diff_us(last_particle_time, now) / 1000;
    
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

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        particles[i].vy += GRAVITY;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        if (particles[i].x <= WALL_LEFT + BALL_DIAMETER/2) {
            particles[i].x = WALL_LEFT + BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }
        if (particles[i].x >= WALL_RIGHT - BALL_DIAMETER/2) {
            particles[i].x = WALL_RIGHT - BALL_DIAMETER/2;
            particles[i].vx = -particles[i].vx * BOUNCINESS;
        }

        check_pin_collisions(i);

        if (particles[i].y >= HISTOGRAM_BASE_Y - BALL_DIAMETER) {
            particles[i].active = false;
            
            int bin = (particles[i].x - WALL_LEFT - WALL_OFFSET) / BIN_WIDTH;
            bin = bin < 0 ? 0 : (bin >= NUM_BINS ? NUM_BINS-1 : bin);
            
            particles[i].bin_position = bin;
            histogram[bin]++;
            
            if (total_particles % 10 == 0) {
                normalize_histogram();
            }
        }
    }
}