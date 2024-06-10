#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t u8;
typedef uint16_t u16;

enum InterruptType {
  kInterruptTypeNone,
  kInterruptTypeNmi,
  kInterruptTypeIrq,
};

struct Bus {
  void* ctx;

  uint8_t (*read)(void* ctx, u16 addr);
  void (*write)(void* ctx, u16 addr, u8 data);
};

struct Cpu {
  u8 a;
  u8 x;
  u8 y;
  u8 s;
  u8 p;
  u16 pc;

  const struct Bus* bus;
  enum InterruptType interrupt;
};

bool cpu_init(struct Cpu* cpu, const struct Bus* bus);

void cpu_reset(struct Cpu* cpu);

u8 cpu_step(struct Cpu* cpu);

// TODO: Interrupts

#ifdef __cplusplus
}
#endif
