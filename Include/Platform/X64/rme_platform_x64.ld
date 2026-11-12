/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

/* It seemed that we cannot directly link kernel to that address as it forces a custom gcc. So what's the plot? */

/* OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386") */
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

/*virtual address 0xFFFFFFFF80100000, load address 0x00100000.*/
__RME_X64_Mboot_Load_Addr = 0x00100000;


SECTIONS
{
	/* Link the kernel at this address: "." means the current address */
    /* Must be equal to KERNLINK */
	. = 0xFFFFFFFF80100000;

	PROVIDE(begin = .);

	.multiboot2 ALIGN(8) : AT(__RME_X64_Mboot_Load_Addr) {
		KEEP(*(.multiboot2))
	}

	/* The bootstrap page tables occupy seven pages starting at 0x101000. */
	.text ALIGN(0x1000) : AT(__RME_X64_Mboot_Load_Addr + (ADDR(.text) - begin)) {
	    *platform_x64_gcc.o(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
		*(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
	}

	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	/* Adjust the address for the data segment to the next page */
	. = ALIGN(0x1000);

	/* Conventionally, Unix linkers provide pseudo-symbols
	 * etext, edata, and end, at the end of the text, data, and bss.
	 * For the kernel mapping, we need the address at the beginning
	 * of the data section, but that's not one of the conventional
	 * symbols, because the convention started before there was a
	 * read-only rodata section between text and data. */
	PROVIDE(data = .);

	/* The data segment */
	.data : {
		*(.data)
	}

	. = ALIGN(0x1000);

	PROVIDE(edata = .);

    . = ALIGN(0x1000);

	.bss : {
        *rme_kernel.o(.bss)
        *rme_platform_x64.o(.bss)
        *rme_platform_x64_gcc.o(.bss)
		*(COMMON)
	}

	PROVIDE(end = .);
	
    /* This mapping starts from 16MB, and includes all the kernel memory
     * that should not be touched by grub. We init it when we run our kernel */

	/DISCARD/ : {
		*(.eh_frame .rela.eh_frame .note.*)
	}
}

__RME_X64_Boot_Text_Load = LOADADDR(.text);
ASSERT(SIZEOF(.multiboot2) > 0 && SIZEOF(.multiboot2) <= 0x1000, "Invalid Multiboot2 header size")
ASSERT(__RME_X64_Boot_Text_Load == 0x101000, "Bootstrap page tables must start at 0x101000")
