/******************************************************************************
Filename    : rme_platform_qemu_virt.ld
Author      : yhy
Date        : 29/09/2026
Licence     : LGPL v3+; see COPYING for details.
Description : The linker script for the RV64V platform on QEMU virt.
              The kernel runs in the Sv39 high canonical half, so its VMA is
              VA_BASE + physical, while OpenSBI loads and enters it at the low
              physical address 0x80200000 (LMA). A small assembly trampoline
              builds the bootstrap tables and switches to the high VA. The
              symbols used by rme_platform_rv64v_gcc.s are provided at the
              bottom.
******************************************************************************/

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Kernel high-half virtual base and physical load address */
__RME_VA_BASE = 0xFFFFFFC000000000;
__RME_PA_BASE = 0x80200000;

SECTIONS
{
  /* High-half VMA; LMA of every loaded section is VMA - __RME_VA_BASE */
  . = __RME_VA_BASE + __RME_PA_BASE;

  PROVIDE(begin = .);

  /* The entry section must come first so that the image starts exactly at the
   * physical 0x80200000, where OpenSBI jumps to. */
  .text : AT(ADDR(.text) - __RME_VA_BASE) {
    *(.text._start)
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

  .bss (NOLOAD) : {
    *(.bss .bss.* COMMON)
  }

  . = ALIGN(0x1000);

  /* Bootstrap root table - no load image, programmed by _start itself */
  .bootpgt (NOLOAD) : {
    *(.bootpgt)
  }

  . = ALIGN(0x1000);
  PROVIDE(end = .);

  /* Reserve the kernel stack - grows downwards from __RME_Stack */
  . = ALIGN(16);
  __RME_Stack_Base = .;
  . += 0x10000;
  __RME_Stack = .;

  /DISCARD/ : {
    *(.eh_frame .rela.eh_frame .note.*)
  }
}

/* Symbols referenced by rme_platform_rv64v_gcc.s. There is no .data copy: the
 * high-half mapping points straight at the physical load address. */
__RME_Global     = ADDR(.data);
__RME_Zero_Start = ADDR(.bss);
__RME_Zero_End   = ADDR(.bss) + SIZEOF(.bss);

/* End Of File ***************************************************************/
