#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

// --- UART SETUP ---
void uart_init() {
    UCSR0A = (1 << U2X0);
    UBRR0H = 0; UBRR0L = 16;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_puts(const char* s) {
    while (*s) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *s++;
    }
}

// --- DATA TYPER ---
struct MeasureResult {
    uint32_t freq;
    uint16_t edges;
};

struct PieceProfile {
    char name[12];
    uint32_t freq;
    uint32_t tol;
    uint16_t minEdges;
    uint16_t maxEdges;
};

// --- PROFILER ---
struct PieceProfile profiles[] = {
    // Namn,      Mål-Hz,   Tol+/-,  MinE,  MaxE
    {"Tom",       241500,   1500,    28,    38},
    {"Bonde",     257000,   4000,    11,    18},
    {"Kung",      274000,   6000,     7,    14},
    {"Dam",       234000,   6000,    30,    55}
};

#define NUM_PROFILES 4

// --- MÄTFUNKTION ---
struct MeasureResult get_single_freq() {
    // Burst: 5 cykler anpassad för ca 240kHz
    DDRB |= (1 << DDB3);
    for(uint8_t i = 0; i < 5; i++) {
        PORTB |= (1 << PORTB3);  _delay_us(1.7);
        PORTB &= ~(1 << PORTB3); _delay_us(1.7);
    }
    DDRB &= ~(1 << DDB3);

    uint16_t edges = 0;
    uint16_t t_first = 0, t_last = 0;
    TCNT1 = 0;
    TCCR1B = (1 << CS10); 

    for(uint32_t timeout = 0; timeout < 20000; timeout++) {
        if (ACSR & (1 << ACO)) {
            if (edges == 0) t_first = TCNT1;
            t_last = TCNT1;
            edges++;
            // Vänta på att signalen går låg så vi inte räknar samma edge två gånger
            while((ACSR & (1 << ACO)) && (timeout < 20000)) timeout++;
        }
    }
    TCCR1B = 0;

    struct MeasureResult res;
    res.edges = edges;
    if (edges > 3) {
        res.freq = (uint32_t)(edges - 1) * 16000000UL / (t_last - t_first);
        } else {
        res.freq = 0;
    }
    return res;
}

// --- IDENTIFIERING ---
const char* identify_piece(uint32_t f, uint16_t e) {
    for (uint8_t i = 0; i < NUM_PROFILES; i++) {
        if (f >= (profiles[i].freq - profiles[i].tol) &&
        f <= (profiles[i].freq + profiles[i].tol)) {
            
            if (e >= profiles[i].minEdges && e <= profiles[i].maxEdges) {
                return profiles[i].name;
            }
        }
    }
    return "Okand";
}

// --- HUVUDPROGRAM ---
int main(void) {
    uart_init();
    ACSR = (1 << ACBG); 
    char buffer[100];
    
    char last_detected[12] = "";
    uint8_t stable_count = 0;

    while (1) {
        uint32_t freq_sum = 0;
        uint32_t edge_sum = 0;
        uint8_t valid_samples = 0;

        for(uint8_t i = 0; i < 10; i++) {
            struct MeasureResult m = get_single_freq();
            if (m.freq > 0) {
                freq_sum += m.freq;
                edge_sum += m.edges;
                valid_samples++;
            }
            _delay_ms(5);
        }

        if (valid_samples > 0) {
            uint32_t avg_f = freq_sum / valid_samples;
            uint16_t avg_e = edge_sum / valid_samples;
            
            const char* current_name = identify_piece(avg_f, avg_e);

            // Debounce-logik
            if (strcmp(current_name, last_detected) == 0) {
                stable_count++;
                } else {
                stable_count = 0;
                strcpy(last_detected, current_name);
            }

            if (stable_count >= 3) {
                snprintf(buffer, sizeof(buffer), "IDENTIFIERAD: %s (F:%lu Hz, E:%u)\r\n",
                current_name, avg_f, avg_e);
                uart_puts(buffer);
            }
            } else {
            uart_puts("Soker...\r\n");
        }

        _delay_ms(100);
    }
}
