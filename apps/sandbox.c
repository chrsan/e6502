#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e6502.h"

static uint8_t* read_program_file(const char* path, long* size) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    return NULL;
  }

  if (fseek(fp, 0, SEEK_END) < 0) {
    goto err;
  }

  *size = ftell(fp);
  if (size < 0) {
    goto err;
  }

  if (fseek(fp, 0, SEEK_SET) < 0) {
    goto err;
  }

  uint8_t* data = malloc(*size * sizeof(uint8_t));
  if (!data) {
    goto err;
  }

  bool ok = fread(data, sizeof(uint8_t), *size, fp) == *size;
  fclose(fp);

  if (ok) {
    return data;
  }

  free(data);
  return NULL;

err:
  fclose(fp);
  return NULL;
}

static uint8_t bus_read(void* ctx, uint16_t address) {
  if (address < 0x100) {
    return 0;
  }

  uint8_t* ram = ctx;
  return ram[address];
}

static void bus_write(void* ctx, uint16_t address, uint8_t value) {
  uint8_t* ram = ctx;
  if (address >= 0x100) {
    ram[address] = value;
  }
}

static const char* bit_table[16] = {
    "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111",
};

static void print_instruction(uint16_t pc, uint16_t address,
                              const struct Instruction* instr,
                              const struct Cpu* cpu) {
  char b1[3] = {' ', ' ', '\0'};
  if (instr->num_bytes >= 2) {
    snprintf(b1, 3, "%02x", instr->bytes[1]);
  }

  char b2[3] = {' ', ' ', '\0'};
  if (instr->num_bytes == 3) {
    snprintf(b2, 3, "%02x", instr->bytes[2]);
  }

  printf("%04x  %02x %s %s  %s  A:%02x X:%02x Y:%02x S:%02x P:%s%s @ %04x\n",
         pc, instr->bytes[0], b1, b2, instr->opcode, cpu->a, cpu->x, cpu->y,
         cpu->s, bit_table[cpu->p >> 4], bit_table[cpu->p & 0x0f], address);
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    return 2;
  }

  long program_data_size;
  uint8_t* program_data = read_program_file(argv[1], &program_data_size);
  if (!program_data) {
    return 2;
  }

  if ((0xfff0 - program_data_size) < 0x200) {
    free(program_data);
    return 2;
  }

  uint8_t* ram = calloc(0x10000, sizeof(uint8_t));
  if (!ram) {
    free(program_data);
    return 1;
  }

  ram[0xfffc] = 0x00;
  ram[0xfffd] = 0x02;

  memcpy(ram + 0x200, program_data, program_data_size);
  free(program_data);

  struct Bus bus = {
      .ctx = ram,
      .read = bus_read,
      .write = bus_write,
  };

  struct Cpu cpu;
  if (cpu_init(&cpu, &bus)) {
    puts("CPU has been initialized");
  }

  uint16_t pc = 0x200;
  uint16_t address;
  struct Instruction instr;
  for (;;) {
    fetch_instruction(&bus, pc, &instr);
    bool brk = cpu_step(&cpu, &address, NULL) == 0x00;
    print_instruction(pc, address, &instr, &cpu);
    if (brk) {
      break;
    }

    pc = cpu.pc;
  }

  printf("RES: %02x\n", ram[0x400]);

  free(ram);
  return 0;
}
