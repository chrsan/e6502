#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "e6502.h"

bool pages_differ(uint16_t page_a, uint16_t page_b);

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

void push8(struct Cpu* cpu, uint8_t value);

uint8_t pull8(struct Cpu* cpu);

void push16(struct Cpu* cpu, uint16_t value);

uint16_t pull16(struct Cpu* cpu);

uint16_t read16(const struct Cpu* cpu, uint16_t address, bool emulate_bug);

enum AddressMode {
  kAddressModeAbsolute,
  kAddressModeAbsoluteX,
  kAddressModeAbsoluteY,
  kAddressModeAccumulator,
  kAddressModeImmediate,
  kAddressModeImplied,
  kAddressModeIndexedIndirect,
  kAddressModeIndirect,
  kAddressModeIndirectIndexed,
  kAddressModeRelative,
  kAddressModeZeroPage,
  kAddressModeZeroPageX,
  kAddressModeZeroPageY,
};

struct InstructionContext {
  uint16_t pc;

  uint16_t address;
  enum AddressMode address_mode;
  uint8_t cycles;
};

bool resolve_address(const struct Cpu* cpu, uint8_t opcode,
                     struct InstructionContext* ctx);

typedef void (*Instruction)(struct Cpu* cpu, struct InstructionContext* ctx);

extern Instruction instructions[256];
