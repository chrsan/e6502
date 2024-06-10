#include <stdbool.h>
#include <stdlib.h>

#include "cpu.h"

static void addr_mode_imm(struct Cpu* cpu, u16* addr) { *addr = cpu->pc++; }

static void addr_mode_zp(struct Cpu* cpu, u16* addr) {
  *addr = read(cpu, cpu->pc++) & 0x00ff;
}

static void addr_mode_zpx(struct Cpu* cpu, u16* addr) {
  *addr = (read(cpu, cpu->pc++) + cpu->x) & 0x00ff;
}

static void addr_mode_zpy(struct Cpu* cpu, u16* addr) {
  *addr = (read(cpu, cpu->pc++) + cpu->y) & 0x00ff;
}

static void addr_mode_rel(struct Cpu* cpu, u16* addr) {
  *addr = read(cpu, cpu->pc++);
  if (*addr & 0x80) {
    *addr |= 0xff00;
  }
}

static void addr_mode_abs(struct Cpu* cpu, u16* addr) {
  u16 lo = read(cpu, cpu->pc++);
  u16 hi = read(cpu, cpu->pc++);
  *addr = (hi << 8) | lo;
}

static void addr_mode_abx(struct Cpu* cpu, u16* addr) {
  u16 lo = read(cpu, cpu->pc++);
  u16 hi = read(cpu, cpu->pc++);
  *addr = ((hi << 8) | lo) + cpu->x;
}

static void addr_mode_aby(struct Cpu* cpu, u16* addr) {
  u16 lo = read(cpu, cpu->pc++);
  u16 hi = read(cpu, cpu->pc++);
  *addr = ((hi << 8) | lo) + cpu->y;
}

static void addr_mode_ind(struct Cpu* cpu, u16* addr) {
  u16 lo = read(cpu, cpu->pc++);
  u16 hi = read(cpu, cpu->pc++);
  u16 a = (hi << 8) | lo;
  if (lo == 0x00ff) {
    *addr = (read(cpu, a & 0xff00) << 8) | read(cpu, a);
  } else {
    *addr = (read(cpu, a + 1) << 8) | read(cpu, a);
  }
}

static void addr_mode_izx(struct Cpu* cpu, u16* addr) {
  u16 a = read(cpu, cpu->pc++);
  u16 lo = read(cpu, (a + cpu->x) & 0x00ff);
  u16 hi = read(cpu, (a + cpu->x + 1) & 0x00ff);
  *addr = (hi << 8) | lo;
}

static void addr_mode_izy(struct Cpu* cpu, u16* addr) {
  u16 a = read(cpu, cpu->pc++);
  u16 lo = read(cpu, a & 0x00ff);
  u16 hi = read(cpu, (a + 1) & 0x00ff);
  *addr = ((hi << 8) | lo) + cpu->y;
}

static void op_xxx(struct Cpu* cpu, u16 addr, bool implied) {}

struct Instruction instructions[256] = {
    {.name = "BRK", .op_impl = op_brk, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_zp},
    {.name = "ASL", .op_impl = op_asl, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "PHP", .op_impl = op_php, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_imm},
    {.name = "ASL", .op_impl = op_asl, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_abs},
    {.name = "ASL", .op_impl = op_asl, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BPL", .op_impl = op_bpl, .addr_mode = addr_mode_rel},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_zpx},
    {.name = "ASL", .op_impl = op_asl, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CLC", .op_impl = op_clc, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_aby},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ORA", .op_impl = op_ora, .addr_mode = addr_mode_abx},
    {.name = "ASL", .op_impl = op_asl, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "JSR", .op_impl = op_jsr, .addr_mode = addr_mode_abs},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BIT", .op_impl = op_bit, .addr_mode = addr_mode_zp},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_zp},
    {.name = "ROL", .op_impl = op_rol, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "PLP", .op_impl = op_plp, .addr_mode = NULL},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_imm},
    {.name = "ROL", .op_impl = op_rol, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BIT", .op_impl = op_bit, .addr_mode = addr_mode_abs},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_abs},
    {.name = "ROL", .op_impl = op_rol, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BMI", .op_impl = op_bmi, .addr_mode = addr_mode_rel},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_zpx},
    {.name = "ROL", .op_impl = op_rol, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "SEC", .op_impl = op_sec, .addr_mode = NULL},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_aby},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "AND", .op_impl = op_and, .addr_mode = addr_mode_abx},
    {.name = "ROL", .op_impl = op_rol, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "RTI", .op_impl = op_rti, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_zp},
    {.name = "LSR", .op_impl = op_lsr, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "PHA", .op_impl = op_pha, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_imm},
    {.name = "LSR", .op_impl = op_lsr, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "JMP", .op_impl = op_jmp, .addr_mode = addr_mode_abs},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_abs},
    {.name = "LSR", .op_impl = op_lsr, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BVC", .op_impl = op_bvc, .addr_mode = addr_mode_rel},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_zpx},
    {.name = "LSR", .op_impl = op_lsr, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CLI", .op_impl = op_cli, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_aby},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "EOR", .op_impl = op_eor, .addr_mode = addr_mode_abx},
    {.name = "LSR", .op_impl = op_lsr, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "RTS", .op_impl = op_rts, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_zp},
    {.name = "ROR", .op_impl = op_ror, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "PLA", .op_impl = op_pla, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_imm},
    {.name = "ROR", .op_impl = op_ror, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "JMP", .op_impl = op_jmp, .addr_mode = addr_mode_ind},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_abs},
    {.name = "ROR", .op_impl = op_ror, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BVS", .op_impl = op_bvs, .addr_mode = addr_mode_rel},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_zpx},
    {.name = "ROR", .op_impl = op_ror, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "SEI", .op_impl = op_sei, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_aby},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "ADC", .op_impl = op_adc, .addr_mode = addr_mode_abx},
    {.name = "ROR", .op_impl = op_ror, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "STY", .op_impl = op_sty, .addr_mode = addr_mode_zp},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_zp},
    {.name = "STX", .op_impl = op_stx, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "DEY", .op_impl = op_dey, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "TXA", .op_impl = op_txa, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "STY", .op_impl = op_sty, .addr_mode = addr_mode_abs},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_abs},
    {.name = "STX", .op_impl = op_stx, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BCC", .op_impl = op_bcc, .addr_mode = addr_mode_rel},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "STY", .op_impl = op_sty, .addr_mode = addr_mode_zpx},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_zpx},
    {.name = "STX", .op_impl = op_stx, .addr_mode = addr_mode_zpy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "TYA", .op_impl = op_tya, .addr_mode = NULL},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_aby},
    {.name = "TXS", .op_impl = op_txs, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "STA", .op_impl = op_sta, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "LDY", .op_impl = op_ldy, .addr_mode = addr_mode_imm},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_izx},
    {.name = "LDX", .op_impl = op_ldx, .addr_mode = addr_mode_imm},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "LDY", .op_impl = op_ldy, .addr_mode = addr_mode_zp},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_zp},
    {.name = "LDX", .op_impl = op_ldx, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "TAY", .op_impl = op_tay, .addr_mode = NULL},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_imm},
    {.name = "TAX", .op_impl = op_tax, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "LDY", .op_impl = op_ldy, .addr_mode = addr_mode_abs},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_abs},
    {.name = "LDX", .op_impl = op_ldx, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BCS", .op_impl = op_bcs, .addr_mode = addr_mode_rel},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "LDY", .op_impl = op_ldy, .addr_mode = addr_mode_zpx},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_zpx},
    {.name = "LDX", .op_impl = op_ldx, .addr_mode = addr_mode_zpy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CLV", .op_impl = op_clv, .addr_mode = NULL},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_aby},
    {.name = "TSX", .op_impl = op_tsx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "LDY", .op_impl = op_ldy, .addr_mode = addr_mode_abx},
    {.name = "LDA", .op_impl = op_lda, .addr_mode = addr_mode_abx},
    {.name = "LDX", .op_impl = op_ldx, .addr_mode = addr_mode_aby},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CPY", .op_impl = op_cpy, .addr_mode = addr_mode_imm},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CPY", .op_impl = op_cpy, .addr_mode = addr_mode_zp},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_zp},
    {.name = "DEC", .op_impl = op_dec, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "INY", .op_impl = op_iny, .addr_mode = NULL},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_imm},
    {.name = "DEX", .op_impl = op_dex, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CPY", .op_impl = op_cpy, .addr_mode = addr_mode_abs},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_abs},
    {.name = "DEC", .op_impl = op_dec, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BNE", .op_impl = op_bne, .addr_mode = addr_mode_rel},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_zpx},
    {.name = "DEC", .op_impl = op_dec, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CLD", .op_impl = op_cld, .addr_mode = NULL},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_aby},
    {.name = "NOP", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "CMP", .op_impl = op_cmp, .addr_mode = addr_mode_abx},
    {.name = "DEC", .op_impl = op_dec, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CPX", .op_impl = op_cpx, .addr_mode = addr_mode_imm},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_izx},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "CPX", .op_impl = op_cpx, .addr_mode = addr_mode_zp},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_zp},
    {.name = "INC", .op_impl = op_inc, .addr_mode = addr_mode_zp},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "INX", .op_impl = op_inx, .addr_mode = NULL},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_imm},
    {.name = "NOP", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_sbc, .addr_mode = NULL},
    {.name = "CPX", .op_impl = op_cpx, .addr_mode = addr_mode_abs},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_abs},
    {.name = "INC", .op_impl = op_inc, .addr_mode = addr_mode_abs},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "BEQ", .op_impl = op_beq, .addr_mode = addr_mode_rel},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_izy},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_zpx},
    {.name = "INC", .op_impl = op_inc, .addr_mode = addr_mode_zpx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "SED", .op_impl = op_sed, .addr_mode = NULL},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_aby},
    {.name = "NOP", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
    {.name = "???", .op_impl = op_nop, .addr_mode = NULL},
    {.name = "SBC", .op_impl = op_sbc, .addr_mode = addr_mode_abx},
    {.name = "INC", .op_impl = op_inc, .addr_mode = addr_mode_abx},
    {.name = "???", .op_impl = op_xxx, .addr_mode = NULL},
};
