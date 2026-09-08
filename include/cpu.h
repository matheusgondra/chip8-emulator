#pragma once

#include <stdint.h>

constexpr uint16_t CHIP8_START_ADDRESS = 0x200;

typedef struct _chip8 {
    uint8_t memory[4 * 1024];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool keyboard[16];
    bool diaplay[32][64];
    bool draw_flag;
} Chip8;

bool chip8_init(Chip8 *cpu);
bool chip8_load_rom(Chip8 *cpu, const char *filename);
void chip8_cycle(Chip8 *cpu);
