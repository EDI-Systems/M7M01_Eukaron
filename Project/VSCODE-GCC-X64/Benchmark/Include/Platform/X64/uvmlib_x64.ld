OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_UVM_Entry)

/* We guarantee that at the end of canonical address there are always 8 pages
 * available starting at 0x00. */
SECTIONS
{   
	/* Link the kernel at this address: "." means the current address */
    /* Must be equal to KERNLINK */
	. = 0x20000000;

	PROVIDE(begin = .);

	.text : {
	    *uvmlib_x64_asm.o(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
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
		*(.bss)
		*(COMMON)
	}
	
	. = ALIGN(0x1000);

	PROVIDE(end = .);

	/DISCARD/ : {
		*(.eh_frame .rela.eh_frame .note.GNU-stack)
	}
}
