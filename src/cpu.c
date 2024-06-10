#include "cpu.h"

#include <stdbool.h>

bool get_flag(const struct Cpu* cpu, enum Flag flag) {
  return (cpu->p & flag) != 0;
}

void set_flag(struct Cpu* cpu, enum Flag flag, bool value) {
  if (value) {
    cpu->p |= flag;
  } else {
    cpu->p &= ~flag;
  }
}

u8 read(const struct Cpu* cpu, u16 addr) {
  return cpu->bus->read(cpu->bus->ctx, addr);
}

void write(const struct Cpu* cpu, u16 addr, u8 data) {
  cpu->bus->write(cpu->bus->ctx, addr, data);
}

bool cpu_init(struct Cpu* cpu, const struct Bus* bus) {
  if (!cpu || !bus) {
    return false;
  }

  cpu->bus = bus;
  cpu->interrupt = kInterruptTypeNone;

  cpu_reset(cpu);

  return true;
}

void cpu_reset(struct Cpu* cpu) {
  cpu->a = 0;
  cpu->x = 0;
  cpu->y = 0;
  cpu->s = 0xfd;
  cpu->p = 0x24;

  u16 lo = read(cpu, 0xfffc);
  u16 hi = read(cpu, 0xfffd);
  cpu->pc = (hi << 8) | lo;
}

u8 cpu_step(struct Cpu* cpu) {
  if (!cpu) {
    return 0;
  }

  u8 opcode = read(cpu, cpu->pc++);
  set_flag(cpu, KFlagUnused, 1);

  struct Instruction* instr = instructions + opcode;
  bool implied = !instr->addr_mode;

  u16 addr = 0;
  if (!implied) {
    instr->addr_mode(cpu, &addr);
  }

  instr->op_impl(cpu, addr, implied);
  return opcode;
}
