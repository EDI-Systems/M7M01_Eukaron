/*====================================================================
* Project:  Board Support Package (BSP)
* Function: C interface for TriCore trap and interrupt handlers
*
* Copyright HighTec EDV-Systeme GmbH 1982-2014
*====================================================================*/

#ifdef TRIBOARD_TC275B
#include "tc27xb/IfxCpu_reg.h"
#include "tc27xb/IfxCpu_bf.h"
#else
#if defined(APPKIT_TC275TU_C) || defined(TRIBOARD_TC275C)
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"
#else
#include "tc27xa/IfxCpu_reg.h"
#include "tc27xa/IfxCpu_bf.h"
#endif /* APPKIT_TC275TU_C || TRIBOARD_TC275C */
#endif /* TRIBOARD_TC275B */

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>
#include "cint.h"

/* This variable is set to 1 after the vectabs are initialized.  */

static int _init_vectab_initialized[3] = {0, 0, 0};

/* This structure describes interrupt handlers and their arguments.  */

typedef struct _Hnd_arg
{
	void (*hnd_handler)(int);
	int hnd_arg;
} Hnd_arg;

/* This array holds the functions to be called when a trap occurs. */

void (*Tdisptab[MAX_TRAPS])(int tin);

/* This array holds the functions to be called when an interrupt occurs.  */

Hnd_arg Cdisptab[MAX_INTRS];

void __int_handler(int arg);

/* This is the default trap vector table, which consists of eight
   entries, each consisting of eight words (32 bytes).  The table
   must be put into the section ".traptab", must be aligned to a
   256-byte boundary, and the entry symbol (trap class 0 -- reset)
   must be called "TriCore_trap_table", as it is referenced below
   in order to program the BTV register.  */

__asm (	".section .traptab, \"ax\", @progbits\n"
		".align 8\n"
		".global TriCore_trap_table\n"
	"TriCore_trap_table:\n"
	  );

# define DEFINE_TRAP(i)									\
	__asm (".global __trap_" #i);						\
	__asm ("__trap_" #i ":");							\
	__asm ("svlcx");									\
	__asm ("movh.a %a15,hi:Tdisptab+(4*" #i ")");		\
	__asm ("ld.w %d4,[%a15]lo:Tdisptab+(4*" #i ")");	\
	__asm ("mov.a %a15, %d4");							\
	__asm ("mov %d4,%d15");								\
	__asm ("calli %a15");								\
	__asm ("rslcx");									\
	__asm ("rfe");										\
	__asm (".align 5")


DEFINE_TRAP(0);		/* trap class 0 (Reset) */
DEFINE_TRAP(1);		/* trap class 1 (Internal Protection Traps) */
DEFINE_TRAP(2);		/* trap class 2 (Instruction Errors) */
DEFINE_TRAP(3);		/* trap class 3 (Context Management) */
DEFINE_TRAP(4);		/* trap class 4 (System Bus and Peripheral Errors) */
DEFINE_TRAP(5);		/* trap class 5 (Assertion Traps) */
DEFINE_TRAP(6);		/* trap class 6 (System Call) */
DEFINE_TRAP(7);		/* trap class 7 (Non-Maskable Interrupt) */

__asm (".text");

/* Install TRAPHANDLER for trap TRAPNO.  */

int _install_trap_handler(int trapno, void (*traphandler)(int))
{
	int CpuId = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;

	if ((trapno < 0) || (trapno >= MAX_TRAPS) || !_init_vectab_initialized[CpuId])
		return 0;

	Tdisptab[trapno] = traphandler;

	return 1;
}

/* The default handlers for class 0-7 traps; if the CPU is programmed to
   honor debug instructions, program control will return to the debugger
   and you can see which trap was triggered.  */

/* Reset  */

static void __class_0_trap_handler(int tin)
{
	(void)tin;
	__asm volatile ("debug"); /* RESET  */
}

/* Internal Protection traps  */

static void __class_1_trap_handler(int tin)
{
	switch (tin)
	{
		case 1:
			__asm volatile ("debug"); /* PRIV -- Privileged Instruction  */
			break;

		case 2:
			__asm volatile ("debug"); /* MPR -- MemProt: Read Access  */
			break;

		case 3:
			__asm volatile ("debug"); /* MPW -- MemProt: Write Access  */
			break;

		case 4:
			__asm volatile ("debug"); /* MPX -- MemProt: Execution Access  */
			break;

		case 5:
			__asm volatile ("debug"); /* MPP -- MemProt: Peripheral Access  */
			break;

		case 6:
			__asm volatile ("debug"); /* MPN -- MemProt: Null Address  */
			break;

		case 7:
			__asm volatile ("debug"); /* GRPW -- Global Register Write Prot  */
			break;
	}
}

/* Instruction Errors  */

static void __class_2_trap_handler(int tin)
{
	switch (tin)
	{
		case 1:
			__asm volatile ("debug"); /* IOPC -- Illegal Opcode  */
			break;

		case 2:
			__asm volatile ("debug"); /* UOPC -- Unimplemented Opcode  */
			break;

		case 3:
			__asm volatile ("debug"); /* OPD -- Invalid Operand Specification  */
			break;

		case 4:
			__asm volatile ("debug"); /* ALN -- Data Address Alignment Error  */
			break;

		case 5:
			__asm volatile ("debug"); /* MEM -- Invalid Local Memory Address  */
			break;
	}
}

/* Context Management  */

static void __class_3_trap_handler(int tin)
{
	switch (tin)
	{
		case 1:
			__asm volatile ("debug"); /* FCD -- Free Context List Depleted  */
			break;

		case 2:
			__asm volatile ("debug"); /* CDO -- Call Depth Overflow  */
			break;

		case 3:
			__asm volatile ("debug"); /* CDU -- Call Depth Underflow  */
			break;

		case 4:
			__asm volatile ("debug"); /* FCU -- Free Context List Underflow  */
			break;

		case 5:
			__asm volatile ("debug"); /* CSU -- Context List Underflow  */
			break;

		case 6:
			__asm volatile ("debug"); /* CTYP -- Context Type Error  */
			break;

		case 7:
			__asm volatile ("debug"); /* NEST -- Nesting Error (RFE)  */
			break;
	}
}

/* System Bus and Peripheral Errors  */

static void __class_4_trap_handler(int tin)
{
	switch (tin)
	{
		case 1:
			__asm volatile ("debug"); /* PRVP -- PrivViol on Peripheral Access  */
			break;

		case 2:
			__asm volatile ("debug"); /* BUS -- Bus Error  */
			break;

		case 3:
			__asm volatile ("debug"); /* PARI -- Parity/CRC Error  */
			break;

		case 4:
			__asm volatile ("debug"); /* BLTO -- Bus Lock Time-Out  */
			break;

		case 5:
			__asm volatile ("debug"); /* PKEY -- KeyViol for ProtPeriph (src)  */
			break;
	}
}

/* Assertion Traps  */

static void __class_5_trap_handler(int tin)
{
	switch (tin)
	{
		case 1:
			__asm volatile ("debug"); /* OVF -- Arithmetic Overflow  */
			break;

		case 2:
			__asm volatile ("debug"); /* SOVF -- Sticky Arithmetic Overflow  */
			break;
	}
}

/* System Call #tin  */

static void __class_6_trap_handler(int tin)
{
	(void)tin;
	__asm volatile ("debug"); /* System Call #tin  */
}

/* Non-maskable Interrupt  */

static void __class_7_trap_handler(int tin)
{
	(void)tin;
	__asm volatile ("debug"); /* NMI -- Non-maskable Interrupt  */
}

/* This is the default interrupt vector table, which consists of 32
   entries, each consisting of eight words (32 bytes).  The table
   must be put into the section ".inttab", must be aligned to a
   8 KB boundary, and the entry symbol (virtual interrupt 0)
   must be called "TriCore_int_table", as it is referenced below
   in order to program the BIV register.  Note: This is only a
   sample implementation which can handle up to 31 interrupts.
   If you need more interrupts, you need to expand the table in the
   obvious way and adjust the alignment as explained in the "TriCore
   uC-DSP Architecture Manual".  */

__asm (	".section .inttab, \"ax\", @progbits\n"
		".align 13\n"
		".global TriCore_int_table\n"
	"TriCore_int_table:\n"
	  );

__asm (	"debug		# int 0\n"
		".align 5\n"
	  );

#define DEFINE_INT(i)							\
	__asm (".global __interrupt_" #i);			\
	__asm ("__interrupt_" #i ":");				\
	__asm ("bisr " #i);							\
	__asm ("movh.a %a15,hi:Cdisptab+8*"#i);		\
	__asm ("lea %a15,[%a15]lo:Cdisptab+8*"#i);	\
	__asm ("ld.a %a14,[%a15+]");				\
	__asm ("ld.w %d4,[%a15]");					\
	__asm ("calli %a14");						\
	__asm ("rslcx");							\
	__asm ("rfe");								\
	__asm (".align 5")


DEFINE_INT(1);
DEFINE_INT(2);
DEFINE_INT(3);
DEFINE_INT(4);
DEFINE_INT(5);
DEFINE_INT(6);
DEFINE_INT(7);
DEFINE_INT(8);
DEFINE_INT(9);
DEFINE_INT(10);
DEFINE_INT(11);
DEFINE_INT(12);
DEFINE_INT(13);
DEFINE_INT(14);
DEFINE_INT(15);
DEFINE_INT(16);
DEFINE_INT(17);
DEFINE_INT(18);
DEFINE_INT(19);
DEFINE_INT(20);
DEFINE_INT(21);
DEFINE_INT(22);
DEFINE_INT(23);
DEFINE_INT(24);
DEFINE_INT(25);
DEFINE_INT(26);
DEFINE_INT(27);
DEFINE_INT(28);
DEFINE_INT(29);
DEFINE_INT(30);
DEFINE_INT(31);
#if MAX_INTRS > 32
DEFINE_INT(32);
DEFINE_INT(33);
DEFINE_INT(34);
DEFINE_INT(35);
DEFINE_INT(36);
DEFINE_INT(37);
DEFINE_INT(38);
DEFINE_INT(39);
DEFINE_INT(40);
DEFINE_INT(41);
DEFINE_INT(42);
DEFINE_INT(43);
DEFINE_INT(44);
DEFINE_INT(45);
DEFINE_INT(46);
DEFINE_INT(47);
DEFINE_INT(48);
DEFINE_INT(49);
DEFINE_INT(50);
DEFINE_INT(51);
DEFINE_INT(52);
DEFINE_INT(53);
DEFINE_INT(54);
DEFINE_INT(55);
DEFINE_INT(56);
DEFINE_INT(57);
DEFINE_INT(58);
DEFINE_INT(59);
DEFINE_INT(60);
DEFINE_INT(61);
DEFINE_INT(62);
DEFINE_INT(63);
DEFINE_INT(64);
DEFINE_INT(65);
DEFINE_INT(66);
DEFINE_INT(67);
DEFINE_INT(68);
DEFINE_INT(69);
DEFINE_INT(70);
DEFINE_INT(71);
DEFINE_INT(72);
DEFINE_INT(73);
DEFINE_INT(74);
DEFINE_INT(75);
DEFINE_INT(76);
DEFINE_INT(77);
DEFINE_INT(78);
DEFINE_INT(79);
DEFINE_INT(80);
DEFINE_INT(81);
DEFINE_INT(82);
DEFINE_INT(83);
DEFINE_INT(84);
DEFINE_INT(85);
DEFINE_INT(86);
DEFINE_INT(87);
DEFINE_INT(88);
DEFINE_INT(89);
DEFINE_INT(90);
DEFINE_INT(91);
DEFINE_INT(92);
DEFINE_INT(93);
DEFINE_INT(94);
DEFINE_INT(95);
DEFINE_INT(96);
DEFINE_INT(97);
DEFINE_INT(98);
DEFINE_INT(99);
DEFINE_INT(100);
DEFINE_INT(101);
DEFINE_INT(102);
DEFINE_INT(103);
DEFINE_INT(104);
DEFINE_INT(105);
DEFINE_INT(106);
DEFINE_INT(107);
DEFINE_INT(108);
DEFINE_INT(109);
DEFINE_INT(110);
DEFINE_INT(111);
DEFINE_INT(112);
DEFINE_INT(113);
DEFINE_INT(114);
DEFINE_INT(115);
DEFINE_INT(116);
DEFINE_INT(117);
DEFINE_INT(118);
DEFINE_INT(119);
DEFINE_INT(120);
DEFINE_INT(121);
DEFINE_INT(122);
DEFINE_INT(123);
DEFINE_INT(124);
DEFINE_INT(125);
DEFINE_INT(126);
DEFINE_INT(127);
DEFINE_INT(128);
DEFINE_INT(129);
DEFINE_INT(130);
DEFINE_INT(131);
DEFINE_INT(132);
DEFINE_INT(133);
DEFINE_INT(134);
DEFINE_INT(135);
DEFINE_INT(136);
DEFINE_INT(137);
DEFINE_INT(138);
DEFINE_INT(139);
DEFINE_INT(140);
DEFINE_INT(141);
DEFINE_INT(142);
DEFINE_INT(143);
DEFINE_INT(144);
DEFINE_INT(145);
DEFINE_INT(146);
DEFINE_INT(147);
DEFINE_INT(148);
DEFINE_INT(149);
DEFINE_INT(150);
DEFINE_INT(151);
DEFINE_INT(152);
DEFINE_INT(153);
DEFINE_INT(154);
DEFINE_INT(155);
DEFINE_INT(156);
DEFINE_INT(157);
DEFINE_INT(158);
DEFINE_INT(159);
DEFINE_INT(160);
DEFINE_INT(161);
DEFINE_INT(162);
DEFINE_INT(163);
DEFINE_INT(164);
DEFINE_INT(165);
DEFINE_INT(166);
DEFINE_INT(167);
DEFINE_INT(168);
DEFINE_INT(169);
DEFINE_INT(170);
DEFINE_INT(171);
DEFINE_INT(172);
DEFINE_INT(173);
DEFINE_INT(174);
DEFINE_INT(175);
DEFINE_INT(176);
DEFINE_INT(177);
DEFINE_INT(178);
DEFINE_INT(179);
DEFINE_INT(180);
DEFINE_INT(181);
DEFINE_INT(182);
DEFINE_INT(183);
DEFINE_INT(184);
DEFINE_INT(185);
DEFINE_INT(186);
DEFINE_INT(187);
DEFINE_INT(188);
DEFINE_INT(189);
DEFINE_INT(190);
DEFINE_INT(191);
DEFINE_INT(192);
DEFINE_INT(193);
DEFINE_INT(194);
DEFINE_INT(195);
DEFINE_INT(196);
DEFINE_INT(197);
DEFINE_INT(198);
DEFINE_INT(199);
DEFINE_INT(200);
DEFINE_INT(201);
DEFINE_INT(202);
DEFINE_INT(203);
DEFINE_INT(204);
DEFINE_INT(205);
DEFINE_INT(206);
DEFINE_INT(207);
DEFINE_INT(208);
DEFINE_INT(209);
DEFINE_INT(210);
DEFINE_INT(211);
DEFINE_INT(212);
DEFINE_INT(213);
DEFINE_INT(214);
DEFINE_INT(215);
DEFINE_INT(216);
DEFINE_INT(217);
DEFINE_INT(218);
DEFINE_INT(219);
DEFINE_INT(220);
DEFINE_INT(221);
DEFINE_INT(222);
DEFINE_INT(223);
DEFINE_INT(224);
DEFINE_INT(225);
DEFINE_INT(226);
DEFINE_INT(227);
DEFINE_INT(228);
DEFINE_INT(229);
DEFINE_INT(230);
DEFINE_INT(231);
DEFINE_INT(232);
DEFINE_INT(233);
DEFINE_INT(234);
DEFINE_INT(235);
DEFINE_INT(236);
DEFINE_INT(237);
DEFINE_INT(238);
DEFINE_INT(239);
DEFINE_INT(240);
DEFINE_INT(241);
DEFINE_INT(242);
DEFINE_INT(243);
DEFINE_INT(244);
DEFINE_INT(245);
DEFINE_INT(246);
DEFINE_INT(247);
DEFINE_INT(248);
DEFINE_INT(249);
DEFINE_INT(250);
DEFINE_INT(251);
DEFINE_INT(252);
DEFINE_INT(253);
DEFINE_INT(254);
DEFINE_INT(255);
#endif

__asm (".text");


/* The default handler for interrupts.  */

void __int_handler(int arg)
{
	/* Just ignore this interrupt.  */
	(void)arg;
}

/* Install INTHANDLER for interrupt INTNO and remember ARG for later use.  */

int _install_int_handler(int intno, void (*inthandler)(int), int arg)
{
	int CpuId = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;

	if ((intno < 0) || (intno >= MAX_INTRS) || !_init_vectab_initialized[CpuId])
		return 0;

	Cdisptab[intno].hnd_handler = inthandler;
	Cdisptab[intno].hnd_arg = arg;

	return 1;
}

/* This initializes the C interrupt interface.  */

void _init_vectab(void);

extern int TriCore_trap_table[];
extern int TriCore_int_table[];

void _init_vectab(void)
{
	int CpuId = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	register int *vptr;
	int vecno;

	if (_init_vectab_initialized[CpuId])
		return;

	/* Set BTV and BIV registers.  */
	unlock_wdtcon();
	vptr = TriCore_trap_table;
	__asm volatile ("mtcr $btv,%0" : : "d" (vptr));
	vptr = TriCore_int_table;
	__asm volatile ("mtcr $biv,%0" : : "d" (vptr));
	lock_wdtcon();

	/* Initialize the trap handlers.  */
	Tdisptab[0] = __class_0_trap_handler;
	Tdisptab[1] = __class_1_trap_handler;
	Tdisptab[2] = __class_2_trap_handler;
	Tdisptab[3] = __class_3_trap_handler;
	Tdisptab[4] = __class_4_trap_handler;
	Tdisptab[5] = __class_5_trap_handler;
	Tdisptab[6] = __class_6_trap_handler;
	Tdisptab[7] = __class_7_trap_handler;

	/* Initialize the interrupt handlers.  */
	for (vecno = 0; vecno < MAX_INTRS; ++vecno)
	{
		Cdisptab[vecno].hnd_handler = __int_handler;
		Cdisptab[vecno].hnd_arg = vecno;
	}

	_init_vectab_initialized[CpuId] = 1;
}
