#pragma once

#include <stdbool.h>

#include "e6502.h"

enum Flag {
  kFlagCarry = (1 << 0),
  kFlagZero = (1 << 1),
  kFlagInterrupt = (1 << 2),
  kFlagDecimal = (1 << 3),
  kFlagBreak = (1 << 4),
  KFlagUnused = (1 << 5),
  kFlagOverflow = (1 << 6),
  kFlagNegative = (1 << 7),
};

bool get_flag(const struct Cpu* cpu, enum Flag flag);

void set_flag(struct Cpu* cpu, enum Flag flag, bool value);

u8 read(const struct Cpu* cpu, u16 addr);

void write(const struct Cpu* cpu, u16 addr, u8 data);

struct Instruction {
  const char* name;
  void (*op_impl)(struct Cpu* cpu, u16 addr, bool implied);
  void (*addr_mode)(struct Cpu* cpu, u16* addr);
};

extern struct Instruction instructions[256];

void op_adc(struct Cpu* cpu, u16 addr, bool implied);

void op_and(struct Cpu* cpu, u16 addr, bool implied);

void op_asl(struct Cpu* cpu, u16 addr, bool implied);

void op_bcc(struct Cpu* cpu, u16 addr, bool implied);

void op_bcs(struct Cpu* cpu, u16 addr, bool implied);

void op_beq(struct Cpu* cpu, u16 addr, bool implied);

void op_bit(struct Cpu* cpu, u16 addr, bool implied);

void op_bmi(struct Cpu* cpu, u16 addr, bool implied);

void op_bne(struct Cpu* cpu, u16 addr, bool implied);

void op_bpl(struct Cpu* cpu, u16 addr, bool implied);

void op_brk(struct Cpu* cpu, u16 addr, bool implied);

void op_bvc(struct Cpu* cpu, u16 addr, bool implied);

void op_bvs(struct Cpu* cpu, u16 addr, bool implied);

void op_clc(struct Cpu* cpu, u16 addr, bool implied);

void op_cld(struct Cpu* cpu, u16 addr, bool implied);

void op_cli(struct Cpu* cpu, u16 addr, bool implied);

void op_clv(struct Cpu* cpu, u16 addr, bool implied);

void op_cmp(struct Cpu* cpu, u16 addr, bool implied);

void op_cpx(struct Cpu* cpu, u16 addr, bool implied);

void op_cpy(struct Cpu* cpu, u16 addr, bool implied);

void op_dec(struct Cpu* cpu, u16 addr, bool implied);

void op_dex(struct Cpu* cpu, u16 addr, bool implied);

void op_dey(struct Cpu* cpu, u16 addr, bool implied);

void op_eor(struct Cpu* cpu, u16 addr, bool implied);

void op_inc(struct Cpu* cpu, u16 addr, bool implied);

void op_inx(struct Cpu* cpu, u16 addr, bool implied);

void op_iny(struct Cpu* cpu, u16 addr, bool implied);

void op_jmp(struct Cpu* cpu, u16 addr, bool implied);

void op_jsr(struct Cpu* cpu, u16 addr, bool implied);

void op_lda(struct Cpu* cpu, u16 addr, bool implied);

void op_ldx(struct Cpu* cpu, u16 addr, bool implied);

void op_ldy(struct Cpu* cpu, u16 addr, bool implied);

void op_lsr(struct Cpu* cpu, u16 addr, bool implied);

void op_nop(struct Cpu* cpu, u16 addr, bool implied);

void op_ora(struct Cpu* cpu, u16 addr, bool implied);

void op_pha(struct Cpu* cpu, u16 addr, bool implied);

void op_php(struct Cpu* cpu, u16 addr, bool implied);

void op_pla(struct Cpu* cpu, u16 addr, bool implied);

void op_plp(struct Cpu* cpu, u16 addr, bool implied);

void op_rol(struct Cpu* cpu, u16 addr, bool implied);

void op_ror(struct Cpu* cpu, u16 addr, bool implied);

void op_rti(struct Cpu* cpu, u16 addr, bool implied);

void op_rts(struct Cpu* cpu, u16 addr, bool implied);

void op_sbc(struct Cpu* cpu, u16 addr, bool implied);

void op_sec(struct Cpu* cpu, u16 addr, bool implied);

void op_sed(struct Cpu* cpu, u16 addr, bool implied);

void op_sei(struct Cpu* cpu, u16 addr, bool implied);

void op_sta(struct Cpu* cpu, u16 addr, bool implied);

void op_stx(struct Cpu* cpu, u16 addr, bool implied);

void op_sty(struct Cpu* cpu, u16 addr, bool implied);

void op_tax(struct Cpu* cpu, u16 addr, bool implied);

void op_tay(struct Cpu* cpu, u16 addr, bool implied);

void op_tsx(struct Cpu* cpu, u16 addr, bool implied);

void op_txa(struct Cpu* cpu, u16 addr, bool implied);

void op_txs(struct Cpu* cpu, u16 addr, bool implied);

void op_tya(struct Cpu* cpu, u16 addr, bool implied);
