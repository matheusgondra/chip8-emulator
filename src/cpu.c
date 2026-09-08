#include "cpu.h"
#include <stdint.h>
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

static uint16_t fetch_opcode(const Chip8 *cpu) {
    uint16_t opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    return opcode;
}

static OpcodeType resolve_opcode_type(uint16_t raw) {
    uint8_t category = (raw & 0xF000) >> 12;
    uint8_t n        = raw & 0x000F;
    uint8_t nn       = raw & 0x00FF;

    switch (category) {
        case 0x0:
            if (nn == 0xE0) {
                return OP_CLS_00E0;
            }

            if (nn == 0xEE) {
                return OP_RET_00EE;
            }
            
            return OP_SYS_0NNN;

        case 0x1: return OP_JP_1NNN;
        case 0x2: return OP_CALL_2NNN;
        case 0x3: return OP_SE_3XNN;
        case 0x4: return OP_SNE_4XNN;
        case 0x5: return (n == 0x0) ? OP_SE_5XY0 : OP_INVALID;
        case 0x6: return OP_LD_6XNN;
        case 0x7: return OP_ADD_7XNN;

        case 0x8:
            switch (n) {
                case 0x0: return OP_LD_8XY0;
                case 0x1: return OP_OR_8XY1;
                case 0x2: return OP_AND_8XY2;
                case 0x3: return OP_XOR_8XY3;
                case 0x4: return OP_ADD_8XY4;
                case 0x5: return OP_SUB_8XY5;
                case 0x6: return OP_SHR_8XY6;
                case 0x7: return OP_SUBN_8XY7;
                case 0xE: return OP_SHL_8XYE;
                default:  return OP_INVALID;
            }

        case 0x9: return (n == 0x0) ? OP_SNE_9XY0 : OP_INVALID;
        case 0xA: return OP_LD_I_ANNN;
        case 0xB: return OP_JP_V0_BNNN;
        case 0xC: return OP_RND_CXNN;
        case 0xD: return OP_DRW_DXYN;

        case 0xE:
            if (nn == 0x9E) {
                return OP_SKP_EX9E;
            }

            if (nn == 0xA1) {
                return OP_SKNP_EXA1;
            }

            return OP_INVALID;

        case 0xF:
            switch (nn) {
                case 0x07: return OP_LD_VX_DT_FX07;
                case 0x0A: return OP_WAIT_KEY_FX0A;
                case 0x15: return OP_SET_DT_FX15;
                case 0x18: return OP_SET_ST_FX18;
                case 0x1E: return OP_ADD_I_FX1E;
                case 0x29: return OP_LD_FONT_FX29;
                case 0x33: return OP_BCD_FX33;
                case 0x55: return OP_DUMP_REGS_FX55;
                case 0x65: return OP_LOAD_REGS_FX65;
                default:   return OP_INVALID;
            }

        default:
            return OP_INVALID;
    }
}

Opcode decode_opcode(uint16_t raw_opcode) {
    return (Opcode) {
        .raw = raw_opcode,
        .category = (raw_opcode & 0xF000) >> 12,
        .x = (raw_opcode & 0x0F00) >> 8,
        .y = (raw_opcode & 0x00F0) >> 4,
        .n = raw_opcode & 0x000F,
        .nn = raw_opcode & 0x00FF,
        .nnn = raw_opcode & 0x0FFF,
        .type = resolve_opcode_type(raw_opcode)
    };
}

void chip8_cycle(Chip8 *cpu) {
    uint16_t raw_opcode = fetch_opcode(cpu);
    cpu->pc += 2;

    Opcode opcode = decode_opcode(raw_opcode);
    switch (opcode.type) {
        case OP_INVALID:
            fprintf(stderr, "Invalid opcode: 0x%04X\n", raw_opcode);
            break;
        default:
            fprintf(stdout, "Opcode unknown: 0x%04X\n", raw_opcode);
            break;
    }
}
