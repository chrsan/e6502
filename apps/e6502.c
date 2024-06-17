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
  if (address == 0xffe0) {
    return bus->io_full;
  } else if (address == 0xffe1) {
    return bus->io_byte;
  } else {
    return bus->ram[address];
  }
}

static void bus_write(void* ctx, u16 address, u8 data) {
  struct BusImpl* bus = ctx;
  if (address == 0xffe1) {
    bus->io_byte = data;
    bus->io_full = true;
  } else {
    bus->ram[address] = data;
  }
}

// https://www.pagetable.com/c64ref/6502/
// https://www.nesdev.org/wiki/Visual6502wiki/6502_all_256_Opcodes
static u8 instruction_bytes(u8 opcode) {
  u8 hi = opcode >> 4;
  u8 lo = opcode & 0x0f;
  if (lo == 0x00) {
    if (hi == 0x02) {
      return 3;
    }

    if (hi >= 0x07) {
      return 2;
    }

    return (hi & 1) == 0 ? 1 : 2;
  }

  if (lo == 0x01 || (lo >= 0x03 && lo <= 0x07)) {
    return 2;
  }

  if (lo == 0x02) {
    if (hi <= 0x07) {
      return 1;
    }

    return (hi & 1) == 0 ? 2 : 1;
  }

  if (lo == 0x08 || lo == 0x0a) {
    return 1;
  }

  if (lo == 0x09 || lo == 0x0b) {
    return (hi & 1) == 0 ? 2 : 3;
  }

  return 3;
}

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

  const size_t ram_size = 0x10000;
  if (program_data_size > (ram_size - 0x0200)) {
    fprintf(stderr, "%s does not fit in RAM\n", argv[optind]);
    goto err;
  }

  uint8_t* ram = calloc(ram_size, sizeof(uint8_t));
  if (!ram) {
    fprintf(stderr, "memory alloc error\n");
    goto err;
  }

  ram[0xfffc] = 0x00;
  ram[0xfffd] = 0x02;

  memcpy(ram + 0x0200, program_data, program_data_size);
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
      u8 num_bytes = instruction_bytes(opcode);
      fprintf(stderr, "%s", opcode_name(opcode));
      if (num_bytes >= 2) {
        fprintf(stderr, " %02" PRIX8, bus.read(&bus_impl, pc + 1));
      } else {
        fprintf(stderr, "   ");
      }

      if (num_bytes == 3) {
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
