/******************************************************************************
Filename    : rme_platform_spacemit_k3.ld
Author      : yhy
Date        : 20/10/2026
Licence     : LGPL v3+; see COPYING for details.
Description : The linker script for the RV64V platform on SpacemiT K3
              (Banana Pi BPI-SM10). The kernel runs in the Sv39 high
              canonical half, so its VMA is VA_BASE + physical, while U-Boot
              loads and enters it in S-mode at the low physical address
              0x102200000 (LMA). A small assembly trampoline builds the
              bootstrap tables and switches to the high VA.
              The user init program (init.bin) is embedded in the kernel
              image as a .init_image section at the fixed physical address
              0x102600000, which __RME_Boot maps to the user VA 0x20000000.
******************************************************************************/

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Kernel high-half virtual base and physical load address */
__RME_VA_BASE = 0xFFFFFFC000000000;
__RME_PA_BASE = 0x102200000;

/* Physical address where the user init program is embedded - must match
 * RME_RV64V_INIT_PA in rme_platform_spacemit_k3.h. It is kept 4MB above the
 * kernel load address so the kernel .bss (KOT bitmap and per-CPU stacks) does
 * not overlap it. */
__RME_INIT_PA_BASE = 0x102600000;

SECTIONS
{
  /* High-half VMA; LMA of every loaded section is VMA - __RME_VA_BASE */
  . = __RME_VA_BASE + __RME_PA_BASE;

  PROVIDE(begin = .);

  /* The entry section must come first so that the image starts exactly at the
   * physical 0x102200000, where U-Boot jumps to. */
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

  /* Embedded user init program - placed at the fixed physical address
   * __RME_INIT_PA_BASE (see chip header). U-Boot loads the whole kernel
   * image including this blob, so no external loader is needed on the
   * board; __RME_Boot maps PA 0x102600000 to the user VA 0x20000000. */
  . = __RME_VA_BASE + __RME_INIT_PA_BASE;
  .init_image : AT(ADDR(.init_image) - __RME_VA_BASE) {
    __RME_Init_Image = .;
    *(.init_image)
    __RME_Init_Image_End = .;
  }

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
