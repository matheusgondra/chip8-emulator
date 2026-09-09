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
    bool display[32][64];
    bool draw_flag;
} Chip8;

typedef enum {
    // System and Flow
    OP_SYS_0NNN,      // 0NNN - Native machine call (ignored/NOP)
    OP_CLS_00E0,      // 00E0 - Clear screen
    OP_RET_00EE,      // 00EE - Return from subroutine
    OP_JP_1NNN,       // 1NNN - Unconditional jump
    OP_CALL_2NNN,     // 2NNN - Subroutine call

    // Comparisons / Conditionals
    OP_SE_3XNN,       // 3XNN - Skip next instruction if Vx == NN
    OP_SNE_4XNN,      // 4XNN - Skip next instruction if Vx != NN
    OP_SE_5XY0,       // 5XY0 - Skip next instruction if Vx == Vy
    OP_SNE_9XY0,      // 9XY0 - Skip next instruction if Vx != Vy

    // Assignment and Basic Arithmetic
    OP_LD_6XNN,       // 6XNN - Vx = NN
    OP_ADD_7XNN,      // 7XNN - Vx += NN

    // Logical and Mathematical Operations (8 family)
    OP_LD_8XY0,       // 8XY0 - Vx = Vy
    OP_OR_8XY1,       // 8XY1 - Vx |= Vy
    OP_AND_8XY2,      // 8XY2 - Vx &= Vy
    OP_XOR_8XY3,      // 8XY3 - Vx ^= Vy
    OP_ADD_8XY4,      // 8XY4 - Vx += Vy (VF = carry)
    OP_SUB_8XY5,      // 8XY5 - Vx -= Vy (VF = NOT borrow)
    OP_SHR_8XY6,      // 8XY6 - Vx >>= 1
    OP_SUBN_8XY7,     // 8XY7 - Vx = Vy - Vx (VF = NOT borrow)
    OP_SHL_8XYE,      // 8XYE - Vx <<= 1

    // Memory, Special Jumps, and Graphics
    OP_LD_I_ANNN,     // ANNN - I = NNN
    OP_JP_V0_BNNN,    // BNNN - Jump to NNN + V0
    OP_RND_CXNN,      // CXNN - Vx = rand() & NN
    OP_DRW_DXYN,      // DXYN - Draw sprite of N rows

    // Keyboard
    OP_SKP_EX9E,      // EX9E - Skip if key Vx is pressed
    OP_SKNP_EXA1,     // EXA1 - Skip if key Vx is NOT pressed

    // Timers and Miscellaneous (F family)
    OP_LD_VX_DT_FX07, // FX07 - Vx = delay_timer
    OP_WAIT_KEY_FX0A, // FX0A - Wait for a key press and save it in Vx
    OP_SET_DT_FX15,   // FX15 - delay_timer = Vx
    OP_SET_ST_FX18,   // FX18 - sound_timer = Vx
    OP_ADD_I_FX1E,    // FX1E - I += Vx
    OP_LD_FONT_FX29,  // FX29 - I = font sprite for the digit in Vx
    OP_BCD_FX33,      // FX33 - Store BCD of Vx in I, I+1, I+2
    OP_DUMP_REGS_FX55,// FX55 - Store V0..Vx in RAM starting at I
    OP_LOAD_REGS_FX65,// FX65 - Load RAM starting at I into V0..Vx

    OP_INVALID        // Opcode not recognized
} OpcodeType;

typedef struct _opcode {
    uint16_t raw;
    uint8_t category;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t nn;
    uint16_t nnn;
    OpcodeType type;
} Opcode;

bool chip8_init(Chip8 *cpu);
bool chip8_load_rom(Chip8 *cpu, const char *filename);
void chip8_cycle(Chip8 *cpu);
void chip8_next_instruction(Chip8 *cpu);

Opcode decode_opcode(uint16_t raw_opcode);
