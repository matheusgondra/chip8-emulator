#include <stdio.h>
#include "cpu.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
        return 1;
    }

    Chip8 cpu;
    if (!chip8_init(&cpu)) {
        fprintf(stderr, "Failed to initialize CHIP-8 CPU\n");
        return 1;
    }

    if (!chip8_load_rom(&cpu, argv[1])) {
        fprintf(stderr, "Failed to load ROM: %s\n", argv[1]);
        return 1;
    }

    // Main emulation loop (simplified)
    while (true) {
        chip8_cycle(&cpu);
        // Add delay and input handling here
    }

    return 0;
}