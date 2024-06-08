#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"

static void add_branch_cycles(struct InstructionContext* ctx) {
  ++ctx->cycles;
  if (pages_differ(ctx->pc, ctx->address)) {
    ++ctx->cycles;
  }
}

static void set_negative_flag(struct Cpu* cpu, uint8_t value) {
  set_flag(cpu, kFlagNegative, (value & 0x80) != 0);
}

static void set_zero_flag(struct Cpu* cpu, uint8_t value) {
  set_flag(cpu, kFlagZero, value == 0);
}

static void compare(struct Cpu* cpu, uint8_t a, uint8_t b) {
  uint8_t c = a - b;
  set_negative_flag(cpu, c);
  set_zero_flag(cpu, c);
  set_flag(cpu, kFlagCarry, a >= b);
}

static void instr_adc(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t a = cpu->a;
  uint8_t b = cpu->bus->read(cpu->bus->ctx, ctx->address);
  bool c = get_flag(cpu, kFlagCarry);

  uint16_t d = a + b + c;
  cpu->a = a + b + c;
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
  set_flag(cpu, kFlagCarry, d > 0xff);
  set_flag(cpu, kFlagOverflow,
           ((a ^ b) & 0x80) == 0 && ((a ^ cpu->a) & 0x80) != 0);
}

static void instr_and(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a &= cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_asl(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (ctx->address_mode == kAddressModeAccumulator) {
    set_flag(cpu, kFlagCarry, ((cpu->a >> 7) & 1) != 0);

    cpu->a <<= 1;
    set_negative_flag(cpu, cpu->a);
    set_zero_flag(cpu, cpu->a);
  } else {
    uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
    set_flag(cpu, kFlagCarry, ((value >> 7) & 1) != 0);

    value <<= 1;
    cpu->bus->write(cpu->bus->ctx, ctx->address, value);
    set_negative_flag(cpu, value);
    set_zero_flag(cpu, value);
  }
}

static void instr_bcc(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (!get_flag(cpu, kFlagCarry)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_bcs(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (get_flag(cpu, kFlagCarry)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_beq(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (get_flag(cpu, kFlagZero)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_bit(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, value);
  set_zero_flag(cpu, value & cpu->a);
  set_flag(cpu, kFlagOverflow, ((value >> 6) & 1) != 0);
}

static void instr_bmi(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (get_flag(cpu, kFlagNegative)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_bne(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (!get_flag(cpu, kFlagZero)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_bpl(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (!get_flag(cpu, kFlagNegative)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_brk(struct Cpu* cpu, struct InstructionContext* ctx) {
  push16(cpu, cpu->pc);
  push8(cpu, cpu->p | 0x10);
  set_flag(cpu, kFlagInterrupt, true);
  cpu->pc = read16(cpu, 0xfffe, false);
}

static void instr_bvc(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (!get_flag(cpu, kFlagOverflow)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_bvs(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (get_flag(cpu, kFlagOverflow)) {
    cpu->pc = ctx->address;
    add_branch_cycles(ctx);
  }
}

static void instr_clc(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagCarry, false);
}

static void instr_cld(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagDecimal, false);
}

static void instr_cli(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagInterrupt, false);
}

static void instr_clv(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagOverflow, false);
}

static void instr_cmp(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
  compare(cpu, cpu->a, value);
}

static void instr_cpx(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
  compare(cpu, cpu->x, value);
}

static void instr_cpy(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
  compare(cpu, cpu->y, value);
}

static void instr_dec(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address) - 1;
  cpu->bus->write(cpu->bus->ctx, ctx->address, value);
  set_negative_flag(cpu, value);
  set_zero_flag(cpu, value);
}

static void instr_dex(struct Cpu* cpu, struct InstructionContext* ctx) {
  --cpu->x;
  set_negative_flag(cpu, cpu->x);
  set_zero_flag(cpu, cpu->x);
}

static void instr_dey(struct Cpu* cpu, struct InstructionContext* ctx) {
  --cpu->y;
  set_negative_flag(cpu, cpu->y);
  set_zero_flag(cpu, cpu->y);
}

static void instr_eor(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a ^= cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_inc(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address) + 1;
  cpu->bus->write(cpu->bus->ctx, ctx->address, value);
  set_negative_flag(cpu, value);
  set_zero_flag(cpu, value);
}

static void instr_inx(struct Cpu* cpu, struct InstructionContext* ctx) {
  ++cpu->x;
  set_negative_flag(cpu, cpu->x);
  set_zero_flag(cpu, cpu->x);
}

static void instr_iny(struct Cpu* cpu, struct InstructionContext* ctx) {
  ++cpu->y;
  set_negative_flag(cpu, cpu->y);
  set_zero_flag(cpu, cpu->y);
}

static void instr_jmp(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->pc = ctx->address;
}

static void instr_jsr(struct Cpu* cpu, struct InstructionContext* ctx) {
  push16(cpu, cpu->pc - 1);
  cpu->pc = ctx->address;
}

static void instr_lda(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a = cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_ldx(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->x = cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->x);
  set_zero_flag(cpu, cpu->x);
}

static void instr_ldy(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->y = cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->y);
  set_zero_flag(cpu, cpu->y);
}

static void instr_lsr(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (ctx->address_mode == kAddressModeAccumulator) {
    set_flag(cpu, kFlagCarry, (cpu->a & 1) != 0);

    cpu->a >>= 1;
    set_negative_flag(cpu, cpu->a);
    set_zero_flag(cpu, cpu->a);
  } else {
    uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
    set_flag(cpu, kFlagCarry, (value & 1) != 0);

    value >>= 1;
    cpu->bus->write(cpu->bus->ctx, ctx->address, value);
    set_negative_flag(cpu, value);
    set_zero_flag(cpu, value);
  }
}

static void instr_nop(struct Cpu* cpu, struct InstructionContext* ctx) {}

static void instr_ora(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a |= cpu->bus->read(cpu->bus->ctx, ctx->address);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_pha(struct Cpu* cpu, struct InstructionContext* ctx) {
  push8(cpu, cpu->a);
}

static void instr_php(struct Cpu* cpu, struct InstructionContext* ctx) {
  push8(cpu, cpu->p | 0x10);
}

static void instr_pla(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a = pull8(cpu);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_plp(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->p = (pull8(cpu) & 0xef) | 0x20;
}

static void instr_rol(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (ctx->address_mode == kAddressModeAccumulator) {
    bool c = get_flag(cpu, kFlagCarry);
    set_flag(cpu, kFlagCarry, ((cpu->a >> 7) & 1) != 0);

    cpu->a = (cpu->a << 7) | c;
    set_negative_flag(cpu, cpu->a);
    set_zero_flag(cpu, cpu->a);
  } else {
    bool c = get_flag(cpu, kFlagCarry);
    uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
    set_flag(cpu, kFlagCarry, ((value >> 7) & 1) != 0);

    value = (value << 1) | c;
    cpu->bus->write(cpu->bus->ctx, ctx->address, value);
    set_negative_flag(cpu, value);
    set_zero_flag(cpu, value);
  }
}

static void instr_ror(struct Cpu* cpu, struct InstructionContext* ctx) {
  if (ctx->address_mode == kAddressModeAccumulator) {
    bool c = get_flag(cpu, kFlagCarry);
    set_flag(cpu, kFlagCarry, (cpu->a & 1) != 0);

    cpu->a = (cpu->a >> 1) | (c << 7);
    set_negative_flag(cpu, cpu->a);
    set_zero_flag(cpu, cpu->a);
  } else {
    bool c = get_flag(cpu, kFlagCarry);
    uint8_t value = cpu->bus->read(cpu->bus->ctx, ctx->address);
    set_flag(cpu, kFlagCarry, (value & 1) != 0);

    value = (value >> 1) | (c << 7);
    cpu->bus->write(cpu->bus->ctx, ctx->address, value);
    set_negative_flag(cpu, value);
    set_zero_flag(cpu, value);
  }
}

static void instr_rti(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->p = (pull8(cpu) & 0xef) | 0x20;
  cpu->pc = pull16(cpu);
}

static void instr_rts(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->pc = pull16(cpu) + 1;
}

static void instr_sbc(struct Cpu* cpu, struct InstructionContext* ctx) {
  uint8_t a = cpu->a;
  uint8_t b = cpu->bus->read(cpu->bus->ctx, ctx->address);
  bool c = get_flag(cpu, kFlagCarry);

  int16_t d = a - b - (1 - c);
  cpu->a = a - b - (1 - c);
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
  set_flag(cpu, kFlagCarry, d >= 0);
  set_flag(cpu, kFlagOverflow,
           ((a ^ b) & 0x80) == 0 && ((a ^ cpu->a) & 0x80) != 0);
}

static void instr_sec(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagCarry, true);
}

static void instr_sed(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagDecimal, true);
}

static void instr_sei(struct Cpu* cpu, struct InstructionContext* ctx) {
  set_flag(cpu, kFlagInterrupt, true);
}

static void instr_sta(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->bus->write(cpu->bus->ctx, ctx->address, cpu->a);
}

static void instr_stx(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->bus->write(cpu->bus->ctx, ctx->address, cpu->x);
}

static void instr_sty(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->bus->write(cpu->bus->ctx, ctx->address, cpu->y);
}

static void instr_tax(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->x = cpu->a;
  set_negative_flag(cpu, cpu->x);
  set_zero_flag(cpu, cpu->x);
}

static void instr_tay(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->y = cpu->a;
  set_negative_flag(cpu, cpu->y);
  set_zero_flag(cpu, cpu->y);
}

static void instr_tsx(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->x = cpu->s;
  set_negative_flag(cpu, cpu->x);
  set_zero_flag(cpu, cpu->x);
}

static void instr_txa(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a = cpu->x;
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

static void instr_txs(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->s = cpu->x;
}

static void instr_tya(struct Cpu* cpu, struct InstructionContext* ctx) {
  cpu->a = cpu->x;
  set_negative_flag(cpu, cpu->a);
  set_zero_flag(cpu, cpu->a);
}

Instruction instructions[256] = {
    instr_brk, instr_ora, instr_nop, instr_nop, instr_nop, instr_ora, instr_asl,
    instr_nop, instr_php, instr_ora, instr_asl, instr_nop, instr_nop, instr_ora,
    instr_asl, instr_nop, instr_bpl, instr_ora, instr_nop, instr_nop, instr_nop,
    instr_ora, instr_asl, instr_nop, instr_clc, instr_ora, instr_nop, instr_nop,
    instr_nop, instr_ora, instr_asl, instr_nop, instr_jsr, instr_and, instr_nop,
    instr_nop, instr_bit, instr_and, instr_rol, instr_nop, instr_plp, instr_and,
    instr_rol, instr_nop, instr_bit, instr_and, instr_rol, instr_nop, instr_bmi,
    instr_and, instr_nop, instr_nop, instr_nop, instr_and, instr_rol, instr_nop,
    instr_sec, instr_and, instr_nop, instr_nop, instr_nop, instr_and, instr_rol,
    instr_nop, instr_rti, instr_eor, instr_nop, instr_nop, instr_nop, instr_eor,
    instr_lsr, instr_nop, instr_pha, instr_eor, instr_lsr, instr_nop, instr_jmp,
    instr_eor, instr_lsr, instr_nop, instr_bvc, instr_eor, instr_nop, instr_nop,
    instr_nop, instr_eor, instr_lsr, instr_nop, instr_cli, instr_eor, instr_nop,
    instr_nop, instr_nop, instr_eor, instr_lsr, instr_nop, instr_rts, instr_adc,
    instr_nop, instr_nop, instr_nop, instr_adc, instr_ror, instr_nop, instr_pla,
    instr_adc, instr_ror, instr_nop, instr_jmp, instr_adc, instr_ror, instr_nop,
    instr_bvs, instr_adc, instr_nop, instr_nop, instr_nop, instr_adc, instr_ror,
    instr_nop, instr_sei, instr_adc, instr_nop, instr_nop, instr_nop, instr_adc,
    instr_ror, instr_nop, instr_nop, instr_sta, instr_nop, instr_nop, instr_sty,
    instr_sta, instr_stx, instr_nop, instr_dey, instr_nop, instr_txa, instr_nop,
    instr_sty, instr_sta, instr_stx, instr_nop, instr_bcc, instr_sta, instr_nop,
    instr_nop, instr_sty, instr_sta, instr_stx, instr_nop, instr_tya, instr_sta,
    instr_txs, instr_nop, instr_nop, instr_sta, instr_nop, instr_nop, instr_ldy,
    instr_lda, instr_ldx, instr_nop, instr_ldy, instr_lda, instr_ldx, instr_nop,
    instr_tay, instr_lda, instr_tax, instr_nop, instr_ldy, instr_lda, instr_ldx,
    instr_nop, instr_bcs, instr_lda, instr_nop, instr_nop, instr_ldy, instr_lda,
    instr_ldx, instr_nop, instr_clv, instr_lda, instr_tsx, instr_nop, instr_ldy,
    instr_lda, instr_ldx, instr_nop, instr_cpy, instr_cmp, instr_nop, instr_nop,
    instr_cpy, instr_cmp, instr_dec, instr_nop, instr_iny, instr_cmp, instr_dex,
    instr_nop, instr_cpy, instr_cmp, instr_dec, instr_nop, instr_bne, instr_cmp,
    instr_nop, instr_nop, instr_nop, instr_cmp, instr_dec, instr_nop, instr_cld,
    instr_cmp, instr_nop, instr_nop, instr_nop, instr_cmp, instr_dec, instr_nop,
    instr_cpx, instr_sbc, instr_nop, instr_nop, instr_cpx, instr_sbc, instr_inc,
    instr_nop, instr_inx, instr_sbc, instr_nop, instr_sbc, instr_cpx, instr_sbc,
    instr_inc, instr_nop, instr_beq, instr_sbc, instr_nop, instr_nop, instr_nop,
    instr_sbc, instr_inc, instr_nop, instr_sed, instr_sbc, instr_nop, instr_nop,
    instr_nop, instr_sbc, instr_inc, instr_nop,
};
