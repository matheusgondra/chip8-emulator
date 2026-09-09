#include "cpu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void chip8_next_instruction(Chip8 *cpu) {
    cpu->pc += 2;
}

void chip8_previous_instruction(Chip8 *cpu) {
    cpu->pc -= 2;
}

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

static void set_colision_flag(Chip8 *cpu, bool is_collision) {
    cpu->V[0xF] = is_collision ? 1 : 0;
}

void chip8_cycle(Chip8 *cpu) {
    uint16_t raw_opcode = fetch_opcode(cpu);
    chip8_next_instruction(cpu);

    Opcode opcode = decode_opcode(raw_opcode);
    switch (opcode.type) {
        case OP_SYS_0NNN:
            break;
        case OP_CLS_00E0:
            memset(cpu->display, 0, sizeof(cpu->display));
            cpu->draw_flag = true;
            break;
        case OP_RET_00EE:
            cpu->sp--;
            cpu->pc = cpu->stack[cpu->sp];
            break;
        case OP_JP_1NNN:
            cpu->pc = opcode.nnn;
            break;
        case OP_CALL_2NNN:
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->sp++;
            cpu->pc = opcode.nnn;
            break;
        case OP_SE_3XNN:
            if (cpu->V[opcode.x] == opcode.nn) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_SNE_4XNN:
            if (cpu->V[opcode.x] != opcode.nn) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_SE_5XY0:
            if (cpu->V[opcode.x] == cpu->V[opcode.y]) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_LD_6XNN:
            cpu->V[opcode.x] = opcode.nn;
            break;
        case OP_ADD_7XNN:
            cpu->V[opcode.x] += opcode.nn;
            break;
        case OP_LD_8XY0:
            cpu->V[opcode.x] = cpu->V[opcode.y];
            break;
        case OP_OR_8XY1:
            cpu->V[opcode.x] |= cpu->V[opcode.y];
            break;
        case OP_AND_8XY2:
            cpu->V[opcode.x] &= cpu->V[opcode.y];
            break;
        case OP_XOR_8XY3:
            cpu->V[opcode.x] ^= cpu->V[opcode.y];
            break;
        case OP_ADD_8XY4:
            cpu->V[opcode.x] += cpu->V[opcode.y];
            break;
        case OP_SUB_8XY5:
            cpu->V[opcode.x] -= cpu->V[opcode.y];
            break;
        case OP_SHR_8XY6:
            cpu->V[opcode.x] >>= 1;
            break;
        case OP_SUBN_8XY7:
            cpu->V[opcode.x] = cpu->V[opcode.y] - cpu->V[opcode.x];
            break;
        case OP_SHL_8XYE:
            cpu->V[opcode.x] <<= 1;
            break;
        case OP_SNE_9XY0:
            if (cpu->V[opcode.x] != cpu->V[opcode.y]) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_LD_I_ANNN:
            cpu->I = opcode.nnn;
            break;
        case OP_JP_V0_BNNN:
            cpu->pc = opcode.nnn + cpu->V[0];
            break;
        case OP_RND_CXNN:
            cpu->V[opcode.x] = (rand() % 256) & opcode.nn;
            break;
        case OP_DRW_DXYN: {
            uint8_t start_x = cpu->V[opcode.x] % 64;
            uint8_t start_y = cpu->V[opcode.y] % 32;

            set_colision_flag(cpu, false);

            for (int row = 0; row < opcode.n; row++) {
                bool is_row_out_of_bounds = (start_y + row >= 32);
                if (is_row_out_of_bounds) {
                    break;
                }

                uint8_t sprite_byte = cpu->memory[cpu->I + row];

                for (uint8_t col = 0; col < 8; col++) {
                    bool is_col_out_of_bounds = (start_x + col >= 64);
                    if (is_col_out_of_bounds) {
                        break;
                    }

                    uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
                    if (sprite_pixel) {
                        if (cpu->display[start_y + row][start_x + col]) {
                            set_colision_flag(cpu, true);
                        }

                        cpu->display[start_y + row][start_x + col] ^= 1;
                    }
                }
            }

            cpu->draw_flag = true;
            break;
        }            
        case OP_SKP_EX9E:
            if (cpu->keyboard[cpu->V[opcode.x]]) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_SKNP_EXA1:
            if (!cpu->keyboard[cpu->V[opcode.x]]) {
                chip8_next_instruction(cpu);
            }

            break;
        case OP_LD_VX_DT_FX07:
            cpu->V[opcode.x] = cpu->delay_timer;
            break;
        case OP_WAIT_KEY_FX0A:
            bool key_pressed = false;

            for (int i = 0; i < 8; i++) {
                if (cpu->keyboard[i]) {
                    cpu->V[opcode.x] = i;
                    key_pressed = true;
                    break;
                }
            }

            if (!key_pressed) {
                chip8_previous_instruction(cpu);
            }

            break;
        case OP_SET_DT_FX15:
            cpu->delay_timer = cpu->V[opcode.x];
            break;
        case OP_SET_ST_FX18:
            cpu->sound_timer = cpu->V[opcode.x];
            break; 
        case OP_ADD_I_FX1E:
            cpu->I += cpu->V[opcode.x];
            break;
        case OP_LD_FONT_FX29:
            cpu->I = FONTSET_START_ADDRESS + (cpu->V[opcode.x] * 5);
            break;
        case OP_BCD_FX33: {
            uint8_t value = cpu->V[opcode.x];
            
            cpu->memory[cpu->I] = value / 100;
            cpu->memory[cpu->I + 1] = (value / 10) % 10;
            cpu->memory[cpu->I + 2] = value % 10;
            
            break;
        }
        case OP_DUMP_REGS_FX55: 
            for (int i = 0; i <= opcode.x; i++) {
                cpu->memory[cpu->I + i] = cpu->V[i];
            }
            break;
        case OP_LOAD_REGS_FX65:
            for (int i = 0; i <= opcode.x; i++) {
                cpu->V[i] = cpu->memory[cpu->I + i];
            }
            break;
        case OP_INVALID:
            fprintf(stderr, "Invalid opcode: 0x%04X\n", raw_opcode);
            break;
        default:
            fprintf(stdout, "Opcode unknown: 0x%04X\n", raw_opcode);
            break;
    }
}
