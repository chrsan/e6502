project('e6502', 'c')

e6502_includes = include_directories('include')

e6502_sources = files(
  'src/cpu.c',
  'src/instr.c',
  'src/op.c',
)

e6502_library = library(
  'e6502',
  e6502_sources,
  include_directories: e6502_includes,
)

e6502_dependency = declare_dependency(
  include_directories: e6502_includes,
  link_with: e6502_library,
)

executable(
  'e6502',
  files('apps/e6502.c'),
  dependencies: e6502_dependency,
)
