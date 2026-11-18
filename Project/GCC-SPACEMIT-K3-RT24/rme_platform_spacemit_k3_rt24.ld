/******************************************************************************
Filename    : rme_platform_spacemit_k3_rt24.ld
Author      : yhy
Date        : 12/11/2026
Licence     : LGPL v3+; see COPYING for details.
Description : The linker script for the RV64V platform on SpacemiT K3 RT24
              (Banana Pi BPI-SM10). The kernel runs in the Sv39 high
              canonical half, so its VMA is VA_BASE + physical. The AP loader
              places it at physical 0x110000000 (LMA), and the M-mode firmware
              enters it in S-mode with the MMU off. RME builds the bootstrap
              tables in S-mode and switches to the high VA.
              The user init program (init.bin) is embedded in the kernel
              image before .bss. __RME_Boot maps it to user VA 0x20000000.
******************************************************************************/

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Kernel high-half virtual base and physical load address */
__RME_VA_BASE = 0xFFFFFFC000000000;
__RME_PA_BASE = 0x110000000;

SECTIONS
{
  /* High-half VMA; LMA of every loaded section is VMA - __RME_VA_BASE */
  . = __RME_VA_BASE + __RME_PA_BASE;

  PROVIDE(begin = .);

  /* The entry section must come first so that the image starts exactly at
   * physical 0x110000000, where the M-mode firmware enters S-mode. */
  .text : AT(ADDR(.text) - __RME_VA_BASE) {
    KEEP(*(.text._start))
    *(.text .text.* .gnu.linkonce.t.*)
  }

  PROVIDE(etext = .);

  . = ALIGN(0x1000);

  .rodata : AT(ADDR(.rodata) - __RME_VA_BASE) {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }

  . = ALIGN(0x1000);

  PROVIDE(data = .);

  .data : AT(ADDR(.data) - __RME_VA_BASE) {
    *(.data .data.*)
  }

  . = ALIGN(0x1000);

  PROVIDE(edata = .);

  .sdata : AT(ADDR(.sdata) - __RME_VA_BASE) {
    *(.sdata .sdata.*)
  }

  . = ALIGN(0x1000);

  /* Put init next to the loaded sections so the flat binary has no large gap. */
  .init_image : AT(ADDR(.init_image) - __RME_VA_BASE) {
    __RME_Init_Image = .;
    KEEP(*(.init_image))
    . = ALIGN(8);
  }
  /* Reserve the rest of the 128 KiB user mapping for startup to clear. */
  .init_reserve (NOLOAD) : {
    . += MAX(0x20000, SIZEOF(.init_image)) - SIZEOF(.init_image);
  }

  . = ALIGN(0x1000);

  .bss (NOLOAD) : {
    *(.bss .bss.* .sbss .sbss.* COMMON)
  }

  . = ALIGN(0x1000);

  /* Keep the common assembly symbol; RT24 boots with a reserved SRAM root
   * built by RME itself in S-mode, not supplied by either firmware. */
  .bootpgt (NOLOAD) : {
    *(.bootpgt)
  }

  /* Reserve the kernel stack - grows downwards from __RME_Stack */
  . = ALIGN(16);
  __RME_Stack_Base = .;
  . += 0x10000;
  __RME_Stack = .;
  PROVIDE(end = .);

  /DISCARD/ : {
    *(.eh_frame .eh_frame.* .rela.eh_frame .note.* .comment)
  }
}

/* Symbols referenced by rme_platform_rv64v_gcc.s. There is no .data copy: the
 * high-half mapping points straight at the physical load address. */
__RME_Global     = ADDR(.data);
__RME_Zero_Start = ADDR(.init_image) + SIZEOF(.init_image);
__RME_Zero_End   = ADDR(.bss) + SIZEOF(.bss);

/* End Of File ***************************************************************/
