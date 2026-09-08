#include "cpu.h"
#include <string.h>

bool chip8_init(Chip8 *cpu) {
    void *result = memset(cpu, 0, sizeof(Chip8));
    if (result == nullptr) {
        return false;
    }

    cpu->pc = CHIP8_START_ADDRESS;
    cpu->draw_flag = true;

    return true;
}

