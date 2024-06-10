#include "e6502.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static void* map_program_file(int fd, size_t* size) {
  void* addr = MAP_FAILED;

  struct stat st;
  if (fstat(fd, &st)) {
    goto err;
  }

  if (!S_ISREG(st.st_mode)) {
    goto err;
  }

  *size = st.st_size;
  if (*size != 0) {
    addr = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
  }

err:
  close(fd);
  return addr == MAP_FAILED ? NULL : addr;
}

struct BusImpl {
  u8* ram;

  u8 io_byte;
  bool io_full;
};

static u8 bus_read(void* ctx, u16 address) {
  struct BusImpl* bus = ctx;
  if (address >= 0x0100) {
    return bus->ram[address - 0x0100];
  }

  if (address == 0x0000) {
    return bus->io_full;
  }

  if (address == 0x0001) {
    return bus->io_byte;
  }

  return 0;
}

#define RAM_OFFSET 0x0100

static void bus_write(void* ctx, u16 address, u8 data) {
  struct BusImpl* bus = ctx;
  if (address >= RAM_OFFSET) {
    bus->ram[address - RAM_OFFSET] = data;
  }

  if (address == 0x0001) {
    bus->io_byte = data;
    bus->io_full = true;
  }
}

// https://csdb.dk/forums/?roomid=11&topicid=162839

struct InstructionInfo {
  const char* name;
  u8 len;
};

static struct InstructionInfo instruction_info[256] = {
    {.name = "BRK", .len = 0}, {.name = "ORA", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "SLO", .len = 1},
    {.name = "NOP", .len = 0}, {.name = "ORA", .len = 1},
    {.name = "ASL", .len = 1}, {.name = "SLO", .len = 1},
    {.name = "PHP", .len = 0}, {.name = "ORA", .len = 1},
    {.name = "ASL", .len = 0}, {.name = "ANC", .len = 1},
    {.name = "NOP", .len = 2}, {.name = "ORA", .len = 2},
    {.name = "ASL", .len = 2}, {.name = "SLO", .len = 2},
    {.name = "BPL", .len = 1}, {.name = "ORA", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "SLO", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "ORA", .len = 1},
    {.name = "ASL", .len = 1}, {.name = "SLO", .len = 1},
    {.name = "CLC", .len = 0}, {.name = "ORA", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "SLO", .len = 2},
    {.name = "NOP", .len = 2}, {.name = "ORA", .len = 2},
    {.name = "ASL", .len = 2}, {.name = "SLO", .len = 2},
    {.name = "JSR", .len = 2}, {.name = "AND", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "RLA", .len = 1},
    {.name = "BIT", .len = 1}, {.name = "AND", .len = 1},
    {.name = "ROL", .len = 1}, {.name = "RLA", .len = 1},
    {.name = "PLP", .len = 0}, {.name = "AND", .len = 1},
    {.name = "ROL", .len = 0}, {.name = "ANC", .len = 1},
    {.name = "BIT", .len = 2}, {.name = "AND", .len = 2},
    {.name = "ROL", .len = 2}, {.name = "RLA", .len = 2},
    {.name = "BMI", .len = 1}, {.name = "AND", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "RLA", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "AND", .len = 1},
    {.name = "ROL", .len = 1}, {.name = "RLA", .len = 1},
    {.name = "SEC", .len = 0}, {.name = "AND", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "RLA", .len = 1},
    {.name = "NOP", .len = 2}, {.name = "AND", .len = 2},
    {.name = "ROL", .len = 2}, {.name = "RLA", .len = 2},
    {.name = "RTI", .len = 0}, {.name = "EOR", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "SRE", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "EOR", .len = 1},
    {.name = "LSR", .len = 1}, {.name = "SRE", .len = 1},
    {.name = "PHA", .len = 0}, {.name = "EOR", .len = 1},
    {.name = "LSR", .len = 0}, {.name = "ASL", .len = 1},
    {.name = "JMP", .len = 2}, {.name = "EOR", .len = 2},
    {.name = "LSR", .len = 2}, {.name = "SRE", .len = 2},
    {.name = "BVS", .len = 1}, {.name = "EOR", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "SRE", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "EOR", .len = 1},
    {.name = "LSR", .len = 1}, {.name = "SRE", .len = 1},
    {.name = "CLI", .len = 0}, {.name = "EOR", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "SRE", .len = 2},
    {.name = "NOP", .len = 2}, {.name = "EOR", .len = 2},
    {.name = "LSR", .len = 2}, {.name = "SRE", .len = 2},
    {.name = "RTS", .len = 0}, {.name = "ADC", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "RRA", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "ADC", .len = 1},
    {.name = "ROR", .len = 1}, {.name = "RRA", .len = 1},
    {.name = "PLA", .len = 0}, {.name = "ADC", .len = 1},
    {.name = "ROR", .len = 0}, {.name = "ARR", .len = 1},
    {.name = "JMP", .len = 2}, {.name = "ADC", .len = 2},
    {.name = "ROR", .len = 2}, {.name = "RRA", .len = 2},
    {.name = "BVS", .len = 1}, {.name = "ADC", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "RRA", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "ADC", .len = 1},
    {.name = "ROR", .len = 1}, {.name = "RRA", .len = 1},
    {.name = "SEI", .len = 0}, {.name = "ADC", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "RRA", .len = 2},
    {.name = "NOP", .len = 2}, {.name = "ADC", .len = 2},
    {.name = "ROR", .len = 2}, {.name = "RRA", .len = 2},
    {.name = "NOP", .len = 1}, {.name = "STA", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "SAX", .len = 1},
    {.name = "STY", .len = 1}, {.name = "STA", .len = 1},
    {.name = "STX", .len = 1}, {.name = "SAX", .len = 1},
    {.name = "DEY", .len = 0}, {.name = "NOP", .len = 1},
    {.name = "TXA", .len = 0}, {.name = "XAA", .len = 1},
    {.name = "STY", .len = 2}, {.name = "STA", .len = 2},
    {.name = "STX", .len = 2}, {.name = "SAX", .len = 2},
    {.name = "BCC", .len = 1}, {.name = "STA", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "AHX", .len = 1},
    {.name = "STY", .len = 1}, {.name = "STA", .len = 1},
    {.name = "STX", .len = 1}, {.name = "SAX", .len = 1},
    {.name = "TYA", .len = 0}, {.name = "STA", .len = 2},
    {.name = "TXS", .len = 0}, {.name = "TAS", .len = 2},
    {.name = "SHF", .len = 2}, {.name = "STA", .len = 2},
    {.name = "SHX", .len = 2}, {.name = "AHX", .len = 2},
    {.name = "LDY", .len = 1}, {.name = "LDA", .len = 1},
    {.name = "LDX", .len = 1}, {.name = "LAX", .len = 1},
    {.name = "LDY", .len = 1}, {.name = "LDA", .len = 1},
    {.name = "LDX", .len = 1}, {.name = "LAX", .len = 1},
    {.name = "TAY", .len = 0}, {.name = "LDA", .len = 1},
    {.name = "TAX", .len = 0}, {.name = "LAX", .len = 1},
    {.name = "LDY", .len = 2}, {.name = "LDA", .len = 2},
    {.name = "LDX", .len = 2}, {.name = "LAX", .len = 2},
    {.name = "BCS", .len = 1}, {.name = "LDA", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "LAX", .len = 1},
    {.name = "LDY", .len = 1}, {.name = "LDA", .len = 1},
    {.name = "LDX", .len = 1}, {.name = "LAX", .len = 1},
    {.name = "CLV", .len = 0}, {.name = "LDA", .len = 2},
    {.name = "TSX", .len = 0}, {.name = "LAS", .len = 2},
    {.name = "LDY", .len = 2}, {.name = "LDA", .len = 2},
    {.name = "LDX", .len = 2}, {.name = "LAX", .len = 2},
    {.name = "CPY", .len = 1}, {.name = "CMP", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "DCP", .len = 1},
    {.name = "CPY", .len = 1}, {.name = "CMP", .len = 1},
    {.name = "DEC", .len = 1}, {.name = "DCP", .len = 1},
    {.name = "INY", .len = 0}, {.name = "CMP", .len = 1},
    {.name = "DEX", .len = 0}, {.name = "SBX", .len = 1},
    {.name = "CPY", .len = 2}, {.name = "CMP", .len = 2},
    {.name = "DEC", .len = 2}, {.name = "DCP", .len = 2},
    {.name = "BNE", .len = 1}, {.name = "CMP", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "DCP", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "CMP", .len = 1},
    {.name = "DEC", .len = 1}, {.name = "DCP", .len = 1},
    {.name = "CLD", .len = 0}, {.name = "CMP", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "DCP", .len = 2},
    {.name = "NOP", .len = 2}, {.name = "CMP", .len = 2},
    {.name = "DEC", .len = 2}, {.name = "DCP", .len = 2},
    {.name = "CPX", .len = 1}, {.name = "SBC", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "ISC", .len = 1},
    {.name = "CPX", .len = 1}, {.name = "SBC", .len = 1},
    {.name = "INC", .len = 1}, {.name = "ISC", .len = 1},
    {.name = "INX", .len = 0}, {.name = "SBC", .len = 1},
    {.name = "NOP", .len = 0}, {.name = "SBC", .len = 1},
    {.name = "CPX", .len = 2}, {.name = "SBC", .len = 2},
    {.name = "INC", .len = 2}, {.name = "ISC", .len = 2},
    {.name = "BEQ", .len = 1}, {.name = "SBC", .len = 1},
    {.name = "JAM", .len = 0}, {.name = "ISC", .len = 1},
    {.name = "NOP", .len = 1}, {.name = "SBC", .len = 1},
    {.name = "INC", .len = 1}, {.name = "ISC", .len = 1},
    {.name = "SED", .len = 0}, {.name = "SBC", .len = 2},
    {.name = "NOP", .len = 0}, {.name = "ISC", .len = 2},
    {.name = "NOP", .len = 2}, {.name = "SBC", .len = 2},
    {.name = "INC", .len = 2}, {.name = "ISC", .len = 2},
};

static const char status_reg[8] = {
    'C', 'Z', 'I', 'D', 'B', 'U', 'V', 'N',
};

static void format_status_reg(u8 p, char buf[8]) {
  for (int i = 7; i >= 0; --i) {
    if (p & (1 << i)) {
      buf[i] = status_reg[i];
    } else {
      buf[i] = '.';
    }
  }
}

#define USAGE "Usage: %s [-d] program_file\n"

int main(int argc, char* argv[]) {
  bool debug = false;

  int opt;
  while ((opt = getopt(argc, argv, "d")) != -1) {
    if (opt == 'd') {
      debug = true;
    } else {
      fprintf(stderr, USAGE, argv[0]);
      return 1;
    }
  }

  if ((argc - optind) != 1) {
    fprintf(stderr, USAGE, argv[0]);
    return 1;
  }

  int fd = open(argv[optind], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "error opening %s\n", argv[optind]);
    return 2;
  }

  size_t program_data_size;
  void* program_data = map_program_file(fd, &program_data_size);
  if (!program_data) {
    fprintf(stderr, "error mapping %s\n", argv[optind]);
    goto err;
  }

  const size_t ram_size = 0x10000 - RAM_OFFSET;
  if (program_data_size > (ram_size - 0x0200)) {
    fprintf(stderr, "%s does not fit in RAM\n", argv[1]);
    goto err;
  }

  uint8_t* ram = calloc(ram_size, sizeof(uint8_t));
  if (!ram) {
    fprintf(stderr, "memory alloc error\n");
    goto err;
  }

  ram[0xfffc - RAM_OFFSET] = 0x00;
  ram[0xfffd - RAM_OFFSET] = 0x02;

  memcpy(ram + 0x0100, program_data, program_data_size);
  munmap(program_data, program_data_size);

  struct BusImpl bus_impl = {
      .ram = ram,
  };

  struct Bus bus = {
      .ctx = &bus_impl,
      .read = bus_read,
      .write = bus_write,
  };

  struct Cpu cpu;
  cpu_init(&cpu, &bus);

  u16 pc = 0x0200;
  char p[8];
  for (;;) {
    if (bus_impl.io_full) {
      printf("%c", bus_impl.io_byte);
      bus_impl.io_full = false;
    }

    u8 opcode = cpu_step(&cpu);
    if (debug) {
      struct InstructionInfo* info = instruction_info + opcode;
      fprintf(stderr, "%s", info->name);
      if (info->len >= 1) {
        fprintf(stderr, " %02" PRIX8, bus.read(&bus_impl, pc + 1));
      } else {
        fprintf(stderr, "   ");
      }

      if (info->len == 2) {
        fprintf(stderr, " %02" PRIX8, bus.read(&bus_impl, pc + 2));
      } else {
        fprintf(stderr, "   ");
      }

      format_status_reg(cpu.p, p);
      fprintf(stderr,
              "  PC:%04" PRIX16 " A:%02" PRIX8 " X:%02" PRIX8 " Y:%02" PRIX8
              " S:%02" PRIX8 " P:%c%c%c%c%c%c%c%c\n",
              pc, cpu.a, cpu.x, cpu.y, cpu.s, p[7], p[6], p[5], p[4], p[3],
              p[2], p[1], p[0]);
    }

    if (opcode == 0x00) {
      break;
    }

    pc = cpu.pc;
  }

  free(ram);
  return 0;
err:
  if (program_data) {
    munmap(program_data, program_data_size);
  }

  return 1;
}
