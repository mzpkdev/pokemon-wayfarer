ENTRY(_start)
MEMORY {
  EWRAM (rwx) : ORIGIN = 0x02000000, LENGTH = 256K
  IWRAM (rwx) : ORIGIN = 0x03000000, LENGTH = 32K
  ROM (rx) : ORIGIN = 0x08000000, LENGTH = 32M
}
SECTIONS {
  .text ORIGIN(ROM) : { *(.text.start) *(.text*) *(.rodata*) } > ROM
  .iwram ORIGIN(IWRAM) : AT(ALIGN(LOADADDR(.text) + SIZEOF(.text), 4)) {
    __iwram_start = .; *(.iwram.code) __iwram_end = .;
  } > IWRAM
  __iwram_lma = LOADADDR(.iwram);
  .data : { *(.data*) } > EWRAM AT > ROM
  .bss (NOLOAD) : { __bss_start = .; *(.bss*) *(.sbss*) *(COMMON) __bss_end = .; } > EWRAM
}
