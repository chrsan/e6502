#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum InterruptType {
  kInterruptTypeNone,
  kInterruptTypeNmi,
  kInterruptTypeIrq,
};

struct Bus {
  void* ctx;

  uint8_t (*read)(void* ctx, uint16_t address);
  void (*write)(void* ctx, uint16_t address, uint8_t value);
};

struct Instruction {
  uint8_t num_bytes;
  uint8_t bytes[3];
  const char* opcode;
};

void fetch_instruction(const struct Bus* bus, uint16_t address,
                       struct Instruction* instr);

struct Cpu {
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t s;
  uint8_t p;
  uint16_t pc;

  enum InterruptType interrupt;

  const struct Bus* bus;
};

bool cpu_init(struct Cpu* cpu, const struct Bus* bus);

void cpu_reset(struct Cpu* cpu);

uint8_t cpu_step(struct Cpu* cpu, uint16_t* address, uint8_t* cycles);

void cpu_trigger_nmi(struct Cpu* cpu);

void cpu_trigger_irq(struct Cpu* cpu);

#ifdef __cplusplus
}
#endif
