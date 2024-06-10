#include <stdbool.h>

#include "cpu.h"

void op_adc(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->a;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = get_flag(cpu, kFlagCarry);
  u16 d = a + b + c;

  set_flag(cpu, kFlagCarry, d > 0x00ff);
  set_flag(cpu, kFlagZero, (d & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagOverflow, ~(a ^ b) & (a ^ d) & 0x0080);
  set_flag(cpu, kFlagNegative, d & 0x0080);

  cpu->a = d & 0x00ff;
}

void op_and(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a &= implied ? cpu->a : read(cpu, addr);

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_asl(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  a <<= 1;

  set_flag(cpu, kFlagCarry, (a & 0xff00) != 0x0000);
  set_flag(cpu, kFlagZero, (a & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, a & 0x0080);

  if (implied) {
    cpu->a = a & 0x00ff;
  } else {
    write(cpu, addr, a & 0x00ff);
  }
}

void op_bcc(struct Cpu* cpu, u16 addr, bool implied) {
  if (!get_flag(cpu, kFlagCarry)) {
    cpu->pc += addr;
  }
}

void op_bcs(struct Cpu* cpu, u16 addr, bool implied) {
  if (get_flag(cpu, kFlagCarry)) {
    cpu->pc += addr;
  }
}

void op_beq(struct Cpu* cpu, u16 addr, bool implied) {
  if (get_flag(cpu, kFlagZero)) {
    cpu->pc += addr;
  }
}

void op_bit(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->a;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = a & b;

  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagOverflow, c & (1 << 6));
  set_flag(cpu, kFlagNegative, c & (1 << 7));
}

void op_bmi(struct Cpu* cpu, u16 addr, bool implied) {
  if (get_flag(cpu, kFlagNegative)) {
    cpu->pc += addr;
  }
}

void op_bne(struct Cpu* cpu, u16 addr, bool implied) {
  if (!get_flag(cpu, kFlagZero)) {
    cpu->pc += addr;
  }
}

void op_bpl(struct Cpu* cpu, u16 addr, bool implied) {
  if (!get_flag(cpu, kFlagNegative)) {
    cpu->pc += addr;
  }
}

void op_brk(struct Cpu* cpu, u16 addr, bool implied) {
  ++cpu->pc;  // TODO: Should this be increments twice?

  write(cpu, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00ff);
  write(cpu, 0x0100 + cpu->s--, cpu->pc & 0x00ff);

  set_flag(cpu, kFlagBreak, true);
  write(cpu, 0x0100 + cpu->s--, cpu->p);
  set_flag(cpu, kFlagBreak, false);
  set_flag(cpu, kFlagInterrupt, true);

  u16 lo = read(cpu, 0xfffe);
  u16 hi = read(cpu, 0xffff);
  cpu->pc = (hi << 8) | lo;
}

void op_bvc(struct Cpu* cpu, u16 addr, bool implied) {
  if (!get_flag(cpu, kFlagOverflow)) {
    cpu->pc += addr;
  }
}

void op_bvs(struct Cpu* cpu, u16 addr, bool implied) {
  if (get_flag(cpu, kFlagOverflow)) {
    cpu->pc += addr;
  }
}

void op_clc(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagCarry, false);
}

void op_cld(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagDecimal, false);
}

void op_cli(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagInterrupt, false);
}

void op_clv(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagOverflow, false);
}

void op_cmp(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->a;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = a - b;

  set_flag(cpu, kFlagCarry, a >= b);
  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, c & 0x0080);
}

void op_cpx(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->x;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = a - b;

  set_flag(cpu, kFlagCarry, a >= b);
  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, c & 0x0080);
}

void op_cpy(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->y;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = a - b;

  set_flag(cpu, kFlagCarry, a >= b);
  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, c & 0x0080);
}

void op_dec(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  --a;

  write(cpu, addr, a & 0x00ff);
  set_flag(cpu, kFlagZero, (a & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, a & 0x0080);
}

void op_dex(struct Cpu* cpu, u16 addr, bool implied) {
  --cpu->x;

  set_flag(cpu, kFlagZero, cpu->x == 0x00);
  set_flag(cpu, kFlagNegative, cpu->x & 0x80);
}

void op_dey(struct Cpu* cpu, u16 addr, bool implied) {
  --cpu->y;

  set_flag(cpu, kFlagZero, cpu->y == 0x00);
  set_flag(cpu, kFlagNegative, cpu->y & 0x80);
}

void op_eor(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a ^= implied ? cpu->a : read(cpu, addr);

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_inc(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  a += 1;

  write(cpu, addr, a & 0x00ff);
  set_flag(cpu, kFlagZero, (a & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, a & 0x0080);
}

void op_inx(struct Cpu* cpu, u16 addr, bool implied) {
  ++cpu->x;

  set_flag(cpu, kFlagZero, cpu->x == 0x00);
  set_flag(cpu, kFlagNegative, cpu->x & 0x80);
}

void op_iny(struct Cpu* cpu, u16 addr, bool implied) {
  ++cpu->y;

  set_flag(cpu, kFlagZero, cpu->y == 0x00);
  set_flag(cpu, kFlagNegative, cpu->y & 0x80);
}

void op_jmp(struct Cpu* cpu, u16 addr, bool implied) { cpu->pc = addr; }

void op_jsr(struct Cpu* cpu, u16 addr, bool implied) {
  u16 pc = cpu->pc - 1;

  write(cpu, 0x0100 + cpu->s--, (pc >> 8) & 0x00ff);
  write(cpu, 0x0100 + cpu->s--, pc & 0x00ff);

  cpu->pc = addr;
}

void op_lda(struct Cpu* cpu, u16 addr, bool implied) {
  if (!implied) {
    cpu->a = read(cpu, addr);
  }

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_ldx(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->x = implied ? cpu->a : read(cpu, addr);

  set_flag(cpu, kFlagZero, cpu->x == 0x00);
  set_flag(cpu, kFlagNegative, cpu->x & 0x80);
}

void op_ldy(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->y = implied ? cpu->a : read(cpu, addr);

  set_flag(cpu, kFlagZero, cpu->y == 0x00);
  set_flag(cpu, kFlagNegative, cpu->y & 0x80);
}

void op_lsr(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  u16 b = a >> 1;

  set_flag(cpu, kFlagCarry, a & 0x0001);
  set_flag(cpu, kFlagZero, (b & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, b & 0x0080);

  if (implied) {
    cpu->a = b & 0x00ff;
  } else {
    write(cpu, addr, b & 0x00ff);
  }
}

void op_nop(struct Cpu* cpu, u16 addr, bool implied) {}

void op_ora(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a |= implied ? cpu->a : read(cpu, addr);

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_pha(struct Cpu* cpu, u16 addr, bool implied) {
  write(cpu, 0x0100 + cpu->s--, cpu->a);
}

void op_php(struct Cpu* cpu, u16 addr, bool implied) {
  write(cpu, 0x0100 + cpu->s--, cpu->p | kFlagBreak | KFlagUnused);
  set_flag(cpu, kFlagBreak, false);
  set_flag(cpu, KFlagUnused, false);
}

void op_pla(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a = read(cpu, 0x0100 + ++cpu->s);

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_plp(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->p = read(cpu, 0x0100 + ++cpu->s);

  set_flag(cpu, KFlagUnused, true);
}

void op_rol(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  u16 b = get_flag(cpu, kFlagCarry);
  u16 c = (a << 1) | b;

  set_flag(cpu, kFlagCarry, c & 0xff00);
  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, c & 0x0080);

  if (implied) {
    cpu->a = c & 0x00ff;
  } else {
    write(cpu, addr, c & 0x00ff);
  }
}

void op_ror(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = implied ? cpu->a : read(cpu, addr);
  u16 b = get_flag(cpu, kFlagCarry);
  u16 c = (b << 7) | (a >> 1);

  set_flag(cpu, kFlagCarry, a & 0x0001);
  set_flag(cpu, kFlagZero, (c & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagNegative, c & 0x0080);

  if (implied) {
    cpu->a = c & 0x00ff;
  } else {
    write(cpu, addr, c & 0x00ff);
  }
}

void op_rti(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->p = read(cpu, 0x0100 + ++cpu->s);

  set_flag(cpu, kFlagBreak, false);
  set_flag(cpu, KFlagUnused, false);

  u16 lo = read(cpu, 0x0100 + ++cpu->s);
  u16 hi = read(cpu, 0x0100 + ++cpu->s);

  cpu->pc = (hi << 8) | lo;
}

void op_rts(struct Cpu* cpu, u16 addr, bool implied) {
  u16 lo = read(cpu, 0x0100 + ++cpu->s);
  u16 hi = read(cpu, 0x0100 + ++cpu->s);

  cpu->pc = ((hi << 8) | lo) + 1;
}

void op_sbc(struct Cpu* cpu, u16 addr, bool implied) {
  u16 a = cpu->a;
  u16 b = implied ? cpu->a : read(cpu, addr);
  u16 c = b ^ 0x00ff;
  u16 d = get_flag(cpu, kFlagCarry);
  u16 e = a + c + d;

  set_flag(cpu, kFlagCarry, e & 0xff00);
  set_flag(cpu, kFlagZero, (e & 0x00ff) == 0x0000);
  set_flag(cpu, kFlagOverflow, (e ^ a) & (e & c) & 0x0080);
  set_flag(cpu, kFlagNegative, e & 0x0080);

  cpu->a = e & 0x00ff;
}

void op_sec(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagCarry, true);
}

void op_sed(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagDecimal, true);
}

void op_sei(struct Cpu* cpu, u16 addr, bool implied) {
  set_flag(cpu, kFlagInterrupt, true);
}

void op_sta(struct Cpu* cpu, u16 addr, bool implied) {
  write(cpu, addr, cpu->a);
}

void op_stx(struct Cpu* cpu, u16 addr, bool implied) {
  write(cpu, addr, cpu->x);
}

void op_sty(struct Cpu* cpu, u16 addr, bool implied) {
  write(cpu, addr, cpu->y);
}

void op_tax(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->x = cpu->a;

  set_flag(cpu, kFlagZero, cpu->x == 0x00);
  set_flag(cpu, kFlagNegative, cpu->x & 0x80);
}

void op_tay(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->y = cpu->a;

  set_flag(cpu, kFlagZero, cpu->y == 0x00);
  set_flag(cpu, kFlagNegative, cpu->y & 0x80);
}

void op_tsx(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->x = cpu->s;

  set_flag(cpu, kFlagZero, cpu->x == 0x00);
  set_flag(cpu, kFlagNegative, cpu->x & 0x80);
}

void op_txa(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a = cpu->x;

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}

void op_txs(struct Cpu* cpu, u16 addr, bool implied) { cpu->s = cpu->x; }

void op_tya(struct Cpu* cpu, u16 addr, bool implied) {
  cpu->a = cpu->y;

  set_flag(cpu, kFlagZero, cpu->a == 0x00);
  set_flag(cpu, kFlagNegative, cpu->a & 0x80);
}
