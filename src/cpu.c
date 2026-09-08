#include "cpu.h"
#include <stdio.h>
#include <string.h>

bool chip8_init(Chip8 *cpu) {
    if (cpu == nullptr) {
        return false;
    }

    memset(cpu, 0, sizeof(Chip8));

    cpu->pc = CHIP8_START_ADDRESS;
    cpu->draw_flag = true;

    return true;
}

static long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long rom_size = ftell(file);
    rewind(file);

    return rom_size;
}

bool chip8_load_rom(Chip8 *cpu, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == nullptr) {
        fprintf(stderr, "Failed to load ROM %s\n", filename);
        return false;
    }
    
    long rom_size = get_file_size(file);

    fread(&cpu->memory[cpu->pc], sizeof(uint8_t), rom_size, file);

    fclose(file);
    return true;
}