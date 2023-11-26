/******************************************************************************
Filename    : rme_platform_a7m_asm_gcc.s
Author      : pry
Date        : 19/01/2017
Description : The ARMv7-M assembly support of the RME RTOS, for gcc.
******************************************************************************/

/* The ARM Cortex-M3/4/7 Architecture *****************************************
R0-R7:General purpose registers that are accessible. 
R8-R12:general purpose registers that can only be reached by 32-bit instructions.
R13:SP/SP_process/SP_main    Stack pointer
R14:LR                       Link Register(used for returning from a subfunction)
R15:PC                       Program counter.
IPSR                         Interrupt Program Status Register.
APSR                         Application Program Status Register.
EPSR                         Execute Program Status Register.
The above 3 registers are saved into the stack in combination(xPSR).

The ARM Cortex-M4 include a single-precision FPU, and the Cortex-M7 will feature
a double-precision FPU.
******************************************************************************/
    .syntax             unified
    .arch               armv7-m
    .thumb
/* Import ********************************************************************/
    /* Locations provided by the linker */
    .extern             __RME_Stack
    .extern             __RME_Data_Load
    .extern             __RME_Data_Start
    .extern             __RME_Data_End
    .extern             __RME_Zero_Start
    .extern             __RME_Zero_End
    /* Kernel entry */
    .extern             main
    /* Preinitialization routine */
    .extern             __RME_A7M_Lowlvl_Preinit
    /* The system call handler */
    .extern             __RME_A7M_Svc_Handler
    /* The system tick handler */
    .extern             __RME_A7M_Tim_Handler
    /* The memory management fault handler */
    .extern             __RME_A7M_Exc_Handler
    /* The generic interrupt handler for all other vectors. */
    .extern             __RME_A7M_Vct_Handler
/* End Import ****************************************************************/

/* Export ********************************************************************/
    /* Entry point */
    .global             __RME_Entry
    /* Disable all interrupts */
    .global             __RME_Int_Disable
    /* Enable all interrupts */
    .global             __RME_Int_Enable
    /* A full barrier */
    .global             __RME_A7M_Barrier
    /* Full system reset */
    .global             __RME_A7M_Reset
    /* Wait until interrupts happen */
    .global             __RME_A7M_Wait_Int
    /* Get the MSB in a word */
    .global             __RME_A7M_MSB_Get
    /* Entering of the user mode */
    .global             __RME_User_Enter
    /* Clear FPU register contents */
    .global             ___RME_A7M_Thd_Cop_Clear
    /* The FPU register save routine */
    .global             ___RME_A7M_Thd_Cop_Save
    /* The FPU register restore routine */
    .global             ___RME_A7M_Thd_Cop_Load
    /* The MPU setup routines */
    .global             ___RME_A7M_MPU_Set1
    .global             ___RME_A7M_MPU_Set2
    .global             ___RME_A7M_MPU_Set3
    .global             ___RME_A7M_MPU_Set4
    .global             ___RME_A7M_MPU_Set5
    .global             ___RME_A7M_MPU_Set6
    .global             ___RME_A7M_MPU_Set7
    .global             ___RME_A7M_MPU_Set8
    .global             ___RME_A7M_MPU_Set9
    .global             ___RME_A7M_MPU_Set10
    .global             ___RME_A7M_MPU_Set11
    .global             ___RME_A7M_MPU_Set12
    .global             ___RME_A7M_MPU_Set13
    .global             ___RME_A7M_MPU_Set14
    .global             ___RME_A7M_MPU_Set15
    .global             ___RME_A7M_MPU_Set16
/* End Export ****************************************************************/

/* Entry *********************************************************************/
    .section            .text.rme_entry
    .align              3

	.thumb_func
__RME_Entry:
    LDR                 R0, =__RME_A7M_Lowlvl_Preinit
    BLX                 R0
    /* Load data section from flash to RAM */
    LDR                 R0,=__RME_Data_Start
    LDR                 R1,=__RME_Data_End
    LDR                 R2,=__RME_Data_Load
__RME_Data_Load:
    CMP                 R0,R1
    BEQ                 __RME_Data_Done
    LDR					R3,[R2]
    STR                 R3,[R0]
    ADD                 R0,#0x04
    ADD                 R2,#0x04
    B                   __RME_Data_Load
__RME_Data_Done:
    /* Clear bss zero section */
    LDR                 R0,=__RME_Zero_Start
    LDR                 R1,=__RME_Zero_End
    LDR                 R2,=0x00
__RME_Zero_Clear:
    CMP                 R0,R1
    BEQ                 __RME_Zero_Done
    STR                 R2,[R0]
    ADD                 R0,#0x04
    B                   __RME_Zero_Clear
__RME_Zero_Done:
    LDR                 R0, =main
    BX                  R0
/* End Entry *****************************************************************/

/* Vector ********************************************************************/
    .section            .text.rme_vector
    .align              3

__RME_Vector:
	.long 				__RME_Stack  		/* Top of Stack */
    .long               __RME_Entry         /* Reset Handler */
    .long               NMI_Handler         /* NMI Handler */
    .long               HardFault_Handler   /* Hard Fault Handler */
    .long               MemManage_Handler   /* MPU Fault Handler */
    .long               BusFault_Handler    /* Bus Fault Handler */
    .long               UsageFault_Handler  /* Usage Fault Handler */
    .long               0                   /* Reserved */
    .long               0                   /* Reserved */
    .long               0                   /* Reserved */
    .long               0                   /* Reserved */
    .long               SVC_Handler         /* SVCall Handler */
    .long               DebugMon_Handler    /* Debug Monitor Handler */
    .long               0                   /* Reserved */
    .long               PendSV_Handler      /* PendSV Handler */
    .long               SysTick_Handler     /* SysTick Handler */

    .long               IRQ0_Handler        /* 240 External Interrupts */
    .long               IRQ1_Handler
    .long               IRQ2_Handler
    .long               IRQ3_Handler
    .long               IRQ4_Handler
    .long               IRQ5_Handler
    .long               IRQ6_Handler
    .long               IRQ7_Handler
    .long               IRQ8_Handler
    .long               IRQ9_Handler

    .long               IRQ10_Handler
    .long               IRQ11_Handler
    .long               IRQ12_Handler
    .long               IRQ13_Handler
    .long               IRQ14_Handler
    .long               IRQ15_Handler
    .long               IRQ16_Handler
    .long               IRQ17_Handler
    .long               IRQ18_Handler
    .long               IRQ19_Handler

    .long               IRQ20_Handler
    .long               IRQ21_Handler
    .long               IRQ22_Handler
    .long               IRQ23_Handler
    .long               IRQ24_Handler
    .long               IRQ25_Handler
    .long               IRQ26_Handler
    .long               IRQ27_Handler
    .long               IRQ28_Handler
    .long               IRQ29_Handler

    .long               IRQ30_Handler
    .long               IRQ31_Handler
    .long               IRQ32_Handler
    .long               IRQ33_Handler
    .long               IRQ34_Handler
    .long               IRQ35_Handler
    .long               IRQ36_Handler
    .long               IRQ37_Handler
    .long               IRQ38_Handler
    .long               IRQ39_Handler

    .long               IRQ40_Handler
    .long               IRQ41_Handler
    .long               IRQ42_Handler
    .long               IRQ43_Handler
    .long               IRQ44_Handler
    .long               IRQ45_Handler
    .long               IRQ46_Handler
    .long               IRQ47_Handler
    .long               IRQ48_Handler
    .long               IRQ49_Handler

    .long               IRQ50_Handler
    .long               IRQ51_Handler
    .long               IRQ52_Handler
    .long               IRQ53_Handler
    .long               IRQ54_Handler
    .long               IRQ55_Handler
    .long               IRQ56_Handler
    .long               IRQ57_Handler
    .long               IRQ58_Handler
    .long               IRQ59_Handler

    .long               IRQ60_Handler
    .long               IRQ61_Handler
    .long               IRQ62_Handler
    .long               IRQ63_Handler
    .long               IRQ64_Handler
    .long               IRQ65_Handler
    .long               IRQ66_Handler
    .long               IRQ67_Handler
    .long               IRQ68_Handler
    .long               IRQ69_Handler

    .long               IRQ70_Handler
    .long               IRQ71_Handler
    .long               IRQ72_Handler
    .long               IRQ73_Handler
    .long               IRQ74_Handler
    .long               IRQ75_Handler
    .long               IRQ76_Handler
    .long               IRQ77_Handler
    .long               IRQ78_Handler
    .long               IRQ79_Handler

    .long               IRQ80_Handler
    .long               IRQ81_Handler
    .long               IRQ82_Handler
    .long               IRQ83_Handler
    .long               IRQ84_Handler
    .long               IRQ85_Handler
    .long               IRQ86_Handler
    .long               IRQ87_Handler
    .long               IRQ88_Handler
    .long               IRQ89_Handler

    .long               IRQ90_Handler
    .long               IRQ91_Handler
    .long               IRQ92_Handler
    .long               IRQ93_Handler
    .long               IRQ94_Handler
    .long               IRQ95_Handler
    .long               IRQ96_Handler
    .long               IRQ97_Handler
    .long               IRQ98_Handler
    .long               IRQ99_Handler

    .long               IRQ100_Handler
    .long               IRQ101_Handler
    .long               IRQ102_Handler
    .long               IRQ103_Handler
    .long               IRQ104_Handler
    .long               IRQ105_Handler
    .long               IRQ106_Handler
    .long               IRQ107_Handler
    .long               IRQ108_Handler
    .long               IRQ109_Handler

    .long               IRQ110_Handler
    .long               IRQ111_Handler
    .long               IRQ112_Handler
    .long               IRQ113_Handler
    .long               IRQ114_Handler
    .long               IRQ115_Handler
    .long               IRQ116_Handler
    .long               IRQ117_Handler
    .long               IRQ118_Handler
    .long               IRQ119_Handler

    .long               IRQ120_Handler
    .long               IRQ121_Handler
    .long               IRQ122_Handler
    .long               IRQ123_Handler
    .long               IRQ124_Handler
    .long               IRQ125_Handler
    .long               IRQ126_Handler
    .long               IRQ127_Handler
    .long               IRQ128_Handler
    .long               IRQ129_Handler

    .long               IRQ130_Handler
    .long               IRQ131_Handler
    .long               IRQ132_Handler
    .long               IRQ133_Handler
    .long               IRQ134_Handler
    .long               IRQ135_Handler
    .long               IRQ136_Handler
    .long               IRQ137_Handler
    .long               IRQ138_Handler
    .long               IRQ139_Handler

    .long               IRQ140_Handler
    .long               IRQ141_Handler
    .long               IRQ142_Handler
    .long               IRQ143_Handler
    .long               IRQ144_Handler
    .long               IRQ145_Handler
    .long               IRQ146_Handler
    .long               IRQ147_Handler
    .long               IRQ148_Handler
    .long               IRQ149_Handler

    .long               IRQ150_Handler
    .long               IRQ151_Handler
    .long               IRQ152_Handler
    .long               IRQ153_Handler
    .long               IRQ154_Handler
    .long               IRQ155_Handler
    .long               IRQ156_Handler
    .long               IRQ157_Handler
    .long               IRQ158_Handler
    .long               IRQ159_Handler

    .long               IRQ160_Handler
    .long               IRQ161_Handler
    .long               IRQ162_Handler
    .long               IRQ163_Handler
    .long               IRQ164_Handler
    .long               IRQ165_Handler
    .long               IRQ166_Handler
    .long               IRQ167_Handler
    .long               IRQ168_Handler
    .long               IRQ169_Handler

    .long               IRQ170_Handler
    .long               IRQ171_Handler
    .long               IRQ172_Handler
    .long               IRQ173_Handler
    .long               IRQ174_Handler
    .long               IRQ175_Handler
    .long               IRQ176_Handler
    .long               IRQ177_Handler
    .long               IRQ178_Handler
    .long               IRQ179_Handler

    .long               IRQ180_Handler
    .long               IRQ181_Handler
    .long               IRQ182_Handler
    .long               IRQ183_Handler
    .long               IRQ184_Handler
    .long               IRQ185_Handler
    .long               IRQ186_Handler
    .long               IRQ187_Handler
    .long               IRQ188_Handler
    .long               IRQ189_Handler

    .long               IRQ190_Handler
    .long               IRQ191_Handler
    .long               IRQ192_Handler
    .long               IRQ193_Handler
    .long               IRQ194_Handler
    .long               IRQ195_Handler
    .long               IRQ196_Handler
    .long               IRQ197_Handler
    .long               IRQ198_Handler
    .long               IRQ199_Handler
    
    .long               IRQ200_Handler
    .long               IRQ201_Handler
    .long               IRQ202_Handler
    .long               IRQ203_Handler
    .long               IRQ204_Handler
    .long               IRQ205_Handler
    .long               IRQ206_Handler
    .long               IRQ207_Handler
    .long               IRQ208_Handler
    .long               IRQ209_Handler

    .long               IRQ210_Handler
    .long               IRQ211_Handler
    .long               IRQ212_Handler
    .long               IRQ213_Handler
    .long               IRQ214_Handler
    .long               IRQ215_Handler
    .long               IRQ216_Handler
    .long               IRQ217_Handler
    .long               IRQ218_Handler
    .long               IRQ219_Handler

    .long               IRQ220_Handler
    .long               IRQ221_Handler
    .long               IRQ222_Handler
    .long               IRQ223_Handler
    .long               IRQ224_Handler
    .long               IRQ225_Handler
    .long               IRQ226_Handler
    .long               IRQ227_Handler
    .long               IRQ228_Handler
    .long               IRQ229_Handler

    .long               IRQ230_Handler
    .long               IRQ231_Handler
    .long               IRQ232_Handler
    .long               IRQ233_Handler
    .long               IRQ234_Handler
    .long               IRQ235_Handler
    .long               IRQ236_Handler
    .long               IRQ237_Handler
    .long               IRQ238_Handler
    .long               IRQ239_Handler

    .weak               IRQ0_Handler        /* 240 External Interrupts */
    .weak               IRQ1_Handler
    .weak               IRQ2_Handler
    .weak               IRQ3_Handler
    .weak               IRQ4_Handler
    .weak               IRQ5_Handler
    .weak               IRQ6_Handler
    .weak               IRQ7_Handler
    .weak               IRQ8_Handler
    .weak               IRQ9_Handler

    .weak               IRQ10_Handler
    .weak               IRQ11_Handler
    .weak               IRQ12_Handler
    .weak               IRQ13_Handler
    .weak               IRQ14_Handler
    .weak               IRQ15_Handler
    .weak               IRQ16_Handler
    .weak               IRQ17_Handler
    .weak               IRQ18_Handler
    .weak               IRQ19_Handler

    .weak               IRQ20_Handler
    .weak               IRQ21_Handler
    .weak               IRQ22_Handler
    .weak               IRQ23_Handler
    .weak               IRQ24_Handler
    .weak               IRQ25_Handler
    .weak               IRQ26_Handler
    .weak               IRQ27_Handler
    .weak               IRQ28_Handler
    .weak               IRQ29_Handler

    .weak               IRQ30_Handler
    .weak               IRQ31_Handler
    .weak               IRQ32_Handler
    .weak               IRQ33_Handler
    .weak               IRQ34_Handler
    .weak               IRQ35_Handler
    .weak               IRQ36_Handler
    .weak               IRQ37_Handler
    .weak               IRQ38_Handler
    .weak               IRQ39_Handler

    .weak               IRQ40_Handler
    .weak               IRQ41_Handler
    .weak               IRQ42_Handler
    .weak               IRQ43_Handler
    .weak               IRQ44_Handler
    .weak               IRQ45_Handler
    .weak               IRQ46_Handler
    .weak               IRQ47_Handler
    .weak               IRQ48_Handler
    .weak               IRQ49_Handler

    .weak               IRQ50_Handler
    .weak               IRQ51_Handler
    .weak               IRQ52_Handler
    .weak               IRQ53_Handler
    .weak               IRQ54_Handler
    .weak               IRQ55_Handler
    .weak               IRQ56_Handler
    .weak               IRQ57_Handler
    .weak               IRQ58_Handler
    .weak               IRQ59_Handler

    .weak               IRQ60_Handler
    .weak               IRQ61_Handler
    .weak               IRQ62_Handler
    .weak               IRQ63_Handler
    .weak               IRQ64_Handler
    .weak               IRQ65_Handler
    .weak               IRQ66_Handler
    .weak               IRQ67_Handler
    .weak               IRQ68_Handler
    .weak               IRQ69_Handler

    .weak               IRQ70_Handler
    .weak               IRQ71_Handler
    .weak               IRQ72_Handler
    .weak               IRQ73_Handler
    .weak               IRQ74_Handler
    .weak               IRQ75_Handler
    .weak               IRQ76_Handler
    .weak               IRQ77_Handler
    .weak               IRQ78_Handler
    .weak               IRQ79_Handler

    .weak               IRQ80_Handler
    .weak               IRQ81_Handler
    .weak               IRQ82_Handler
    .weak               IRQ83_Handler
    .weak               IRQ84_Handler
    .weak               IRQ85_Handler
    .weak               IRQ86_Handler
    .weak               IRQ87_Handler
    .weak               IRQ88_Handler
    .weak               IRQ89_Handler

    .weak               IRQ90_Handler
    .weak               IRQ91_Handler
    .weak               IRQ92_Handler
    .weak               IRQ93_Handler
    .weak               IRQ94_Handler
    .weak               IRQ95_Handler
    .weak               IRQ96_Handler
    .weak               IRQ97_Handler
    .weak               IRQ98_Handler
    .weak               IRQ99_Handler

    .weak               IRQ100_Handler
    .weak               IRQ101_Handler
    .weak               IRQ102_Handler
    .weak               IRQ103_Handler
    .weak               IRQ104_Handler
    .weak               IRQ105_Handler
    .weak               IRQ106_Handler
    .weak               IRQ107_Handler
    .weak               IRQ108_Handler
    .weak               IRQ109_Handler

    .weak               IRQ110_Handler
    .weak               IRQ111_Handler
    .weak               IRQ112_Handler
    .weak               IRQ113_Handler
    .weak               IRQ114_Handler
    .weak               IRQ115_Handler
    .weak               IRQ116_Handler
    .weak               IRQ117_Handler
    .weak               IRQ118_Handler
    .weak               IRQ119_Handler

    .weak               IRQ120_Handler
    .weak               IRQ121_Handler
    .weak               IRQ122_Handler
    .weak               IRQ123_Handler
    .weak               IRQ124_Handler
    .weak               IRQ125_Handler
    .weak               IRQ126_Handler
    .weak               IRQ127_Handler
    .weak               IRQ128_Handler
    .weak               IRQ129_Handler

    .weak               IRQ130_Handler
    .weak               IRQ131_Handler
    .weak               IRQ132_Handler
    .weak               IRQ133_Handler
    .weak               IRQ134_Handler
    .weak               IRQ135_Handler
    .weak               IRQ136_Handler
    .weak               IRQ137_Handler
    .weak               IRQ138_Handler
    .weak               IRQ139_Handler

    .weak               IRQ140_Handler
    .weak               IRQ141_Handler
    .weak               IRQ142_Handler
    .weak               IRQ143_Handler
    .weak               IRQ144_Handler
    .weak               IRQ145_Handler
    .weak               IRQ146_Handler
    .weak               IRQ147_Handler
    .weak               IRQ148_Handler
    .weak               IRQ149_Handler

    .weak               IRQ150_Handler
    .weak               IRQ151_Handler
    .weak               IRQ152_Handler
    .weak               IRQ153_Handler
    .weak               IRQ154_Handler
    .weak               IRQ155_Handler
    .weak               IRQ156_Handler
    .weak               IRQ157_Handler
    .weak               IRQ158_Handler
    .weak               IRQ159_Handler

    .weak               IRQ160_Handler
    .weak               IRQ161_Handler
    .weak               IRQ162_Handler
    .weak               IRQ163_Handler
    .weak               IRQ164_Handler
    .weak               IRQ165_Handler
    .weak               IRQ166_Handler
    .weak               IRQ167_Handler
    .weak               IRQ168_Handler
    .weak               IRQ169_Handler

    .weak               IRQ170_Handler
    .weak               IRQ171_Handler
    .weak               IRQ172_Handler
    .weak               IRQ173_Handler
    .weak               IRQ174_Handler
    .weak               IRQ175_Handler
    .weak               IRQ176_Handler
    .weak               IRQ177_Handler
    .weak               IRQ178_Handler
    .weak               IRQ179_Handler

    .weak               IRQ180_Handler
    .weak               IRQ181_Handler
    .weak               IRQ182_Handler
    .weak               IRQ183_Handler
    .weak               IRQ184_Handler
    .weak               IRQ185_Handler
    .weak               IRQ186_Handler
    .weak               IRQ187_Handler
    .weak               IRQ188_Handler
    .weak               IRQ189_Handler

    .weak               IRQ190_Handler
    .weak               IRQ191_Handler
    .weak               IRQ192_Handler
    .weak               IRQ193_Handler
    .weak               IRQ194_Handler
    .weak               IRQ195_Handler
    .weak               IRQ196_Handler
    .weak               IRQ197_Handler
    .weak               IRQ198_Handler
    .weak               IRQ199_Handler
    
    .weak               IRQ200_Handler
    .weak               IRQ201_Handler
    .weak               IRQ202_Handler
    .weak               IRQ203_Handler
    .weak               IRQ204_Handler
    .weak               IRQ205_Handler
    .weak               IRQ206_Handler
    .weak               IRQ207_Handler
    .weak               IRQ208_Handler
    .weak               IRQ209_Handler

    .weak               IRQ210_Handler
    .weak               IRQ211_Handler
    .weak               IRQ212_Handler
    .weak               IRQ213_Handler
    .weak               IRQ214_Handler
    .weak               IRQ215_Handler
    .weak               IRQ216_Handler
    .weak               IRQ217_Handler
    .weak               IRQ218_Handler
    .weak               IRQ219_Handler

    .weak               IRQ220_Handler
    .weak               IRQ221_Handler
    .weak               IRQ222_Handler
    .weak               IRQ223_Handler
    .weak               IRQ224_Handler
    .weak               IRQ225_Handler
    .weak               IRQ226_Handler
    .weak               IRQ227_Handler
    .weak               IRQ228_Handler
    .weak               IRQ229_Handler

    .weak               IRQ230_Handler
    .weak               IRQ231_Handler
    .weak               IRQ232_Handler
    .weak               IRQ233_Handler
    .weak               IRQ234_Handler
    .weak               IRQ235_Handler
    .weak               IRQ236_Handler
    .weak               IRQ237_Handler
    .weak               IRQ238_Handler
    .weak               IRQ239_Handler

	.thumb_func
Default_Handler:
IRQ0_Handler:
IRQ1_Handler:
IRQ2_Handler:
IRQ3_Handler:
IRQ4_Handler:
IRQ5_Handler:
IRQ6_Handler:
IRQ7_Handler:
IRQ8_Handler:
IRQ9_Handler:

IRQ10_Handler:
IRQ11_Handler:
IRQ12_Handler:
IRQ13_Handler:
IRQ14_Handler:
IRQ15_Handler:
IRQ16_Handler:
IRQ17_Handler:
IRQ18_Handler:
IRQ19_Handler:

IRQ20_Handler:
IRQ21_Handler:
IRQ22_Handler:
IRQ23_Handler:
IRQ24_Handler:
IRQ25_Handler:
IRQ26_Handler:
IRQ27_Handler:
IRQ28_Handler:
IRQ29_Handler:

IRQ30_Handler:
IRQ31_Handler:
IRQ32_Handler:
IRQ33_Handler:
IRQ34_Handler:
IRQ35_Handler:
IRQ36_Handler:
IRQ37_Handler:
IRQ38_Handler:
IRQ39_Handler:

IRQ40_Handler:
IRQ41_Handler:
IRQ42_Handler:
IRQ43_Handler:
IRQ44_Handler:
IRQ45_Handler:
IRQ46_Handler:
IRQ47_Handler:
IRQ48_Handler:
IRQ49_Handler:

IRQ50_Handler:
IRQ51_Handler:
IRQ52_Handler:
IRQ53_Handler:
IRQ54_Handler:
IRQ55_Handler:
IRQ56_Handler:
IRQ57_Handler:
IRQ58_Handler:
IRQ59_Handler:

IRQ60_Handler:
IRQ61_Handler:
IRQ62_Handler:
IRQ63_Handler:
IRQ64_Handler:
IRQ65_Handler:
IRQ66_Handler:
IRQ67_Handler:
IRQ68_Handler:
IRQ69_Handler:
 
IRQ70_Handler:
IRQ71_Handler:
IRQ72_Handler:
IRQ73_Handler:
IRQ74_Handler:
IRQ75_Handler:
IRQ76_Handler:
IRQ77_Handler:
IRQ78_Handler:
IRQ79_Handler:

IRQ80_Handler:
IRQ81_Handler:
IRQ82_Handler:
IRQ83_Handler:
IRQ84_Handler:
IRQ85_Handler:
IRQ86_Handler:
IRQ87_Handler:
IRQ88_Handler:
IRQ89_Handler:
 
IRQ90_Handler:
IRQ91_Handler:
IRQ92_Handler:
IRQ93_Handler:
IRQ94_Handler:
IRQ95_Handler:
IRQ96_Handler:
IRQ97_Handler:
IRQ98_Handler:
IRQ99_Handler:

IRQ100_Handler:
IRQ101_Handler:
IRQ102_Handler:
IRQ103_Handler:
IRQ104_Handler:
IRQ105_Handler:
IRQ106_Handler:
IRQ107_Handler:
IRQ108_Handler:
IRQ109_Handler:

IRQ110_Handler:
IRQ111_Handler:
IRQ112_Handler:
IRQ113_Handler:
IRQ114_Handler:
IRQ115_Handler:
IRQ116_Handler:
IRQ117_Handler:
IRQ118_Handler:
IRQ119_Handler:

IRQ120_Handler:
IRQ121_Handler:
IRQ122_Handler:
IRQ123_Handler:
IRQ124_Handler:
IRQ125_Handler:
IRQ126_Handler:
IRQ127_Handler:
IRQ128_Handler:
IRQ129_Handler:

IRQ130_Handler:
IRQ131_Handler:
IRQ132_Handler:
IRQ133_Handler:
IRQ134_Handler:
IRQ135_Handler:
IRQ136_Handler:
IRQ137_Handler:
IRQ138_Handler:
IRQ139_Handler:

IRQ140_Handler:
IRQ141_Handler:
IRQ142_Handler:
IRQ143_Handler:
IRQ144_Handler:
IRQ145_Handler:
IRQ146_Handler:
IRQ147_Handler:
IRQ148_Handler:
IRQ149_Handler:

IRQ150_Handler:
IRQ151_Handler:
IRQ152_Handler:
IRQ153_Handler:
IRQ154_Handler:
IRQ155_Handler:
IRQ156_Handler:
IRQ157_Handler:
IRQ158_Handler:
IRQ159_Handler:

IRQ160_Handler:
IRQ161_Handler:
IRQ162_Handler:
IRQ163_Handler:
IRQ164_Handler:
IRQ165_Handler:
IRQ166_Handler:
IRQ167_Handler:
IRQ168_Handler:
IRQ169_Handler:
 
IRQ170_Handler:
IRQ171_Handler:
IRQ172_Handler:
IRQ173_Handler:
IRQ174_Handler:
IRQ175_Handler:
IRQ176_Handler:
IRQ177_Handler:
IRQ178_Handler:
IRQ179_Handler:

IRQ180_Handler:
IRQ181_Handler:
IRQ182_Handler:
IRQ183_Handler:
IRQ184_Handler:
IRQ185_Handler:
IRQ186_Handler:
IRQ187_Handler:
IRQ188_Handler:
IRQ189_Handler:
 
IRQ190_Handler:
IRQ191_Handler:
IRQ192_Handler:
IRQ193_Handler:
IRQ194_Handler:
IRQ195_Handler:
IRQ196_Handler:
IRQ197_Handler:
IRQ198_Handler:
IRQ199_Handler:
                
IRQ200_Handler:
IRQ201_Handler:
IRQ202_Handler:
IRQ203_Handler:
IRQ204_Handler:
IRQ205_Handler:
IRQ206_Handler:
IRQ207_Handler:
IRQ208_Handler:
IRQ209_Handler:

IRQ210_Handler:
IRQ211_Handler:
IRQ212_Handler:
IRQ213_Handler:
IRQ214_Handler:
IRQ215_Handler:
IRQ216_Handler:
IRQ217_Handler:
IRQ218_Handler:
IRQ219_Handler:

IRQ220_Handler:
IRQ221_Handler:
IRQ222_Handler:
IRQ223_Handler:
IRQ224_Handler:
IRQ225_Handler:
IRQ226_Handler:
IRQ227_Handler:
IRQ228_Handler:
IRQ229_Handler:

IRQ230_Handler:
IRQ231_Handler:
IRQ232_Handler:
IRQ233_Handler:
IRQ234_Handler:
IRQ235_Handler:
IRQ236_Handler:
IRQ237_Handler:
IRQ238_Handler:
IRQ239_Handler:
    PUSH                {R4-R11,LR}         /* Save registers */
    MRS                 R0,PSP
    PUSH                {R0}
    
    MOV                 R0,SP               /* Pass in the regs */
    MRS                 R1,xPSR             /* Pass in the interrupt number */
    UBFX                R1,R1,#0,#9
    SUB                 R1,#16              /* The IRQ0's starting number is 16; subtract */
    BL                  __RME_A7M_Vct_Handler
    
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         /* Restore registers */
/* End Vector ****************************************************************/

/* Function:__RME_Int_Disable *************************************************
Description : The function for disabling all interrupts.
Input       : None.
Output      : None. 
Return      : None.                                
******************************************************************************/
    .section            .text.__rme_int_disable
    .align              3

	.thumb_func
__RME_Int_Disable:
    CPSID               I 
    BX                  LR                                                 
/* End Function:__RME_Int_Disable ********************************************/

/* Function:__RME_Int_Enable **************************************************
Description : The function for enabling all interrupts.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_int_enable
    .align              3

	.thumb_func
__RME_Int_Enable:
    CPSIE               I 
    BX                  LR
/* End Function:__RME_Int_Enable *********************************************/

/* Function:__RME_A7M_Barrier *************************************************
Description : A full data/instruction barrier.
Input       : None.
Output      : None.    
Return      : None.
******************************************************************************/
    .section            .text.__rme_a7m_barrier
    .align              3

	.thumb_func
__RME_A7M_Barrier:       
    DSB                 SY
    ISB                 SY
    BX                  LR
    B                   .
/* End Function:__RME_A7M_Barrier ********************************************/

/* Function:__RME_A7M_Reset ***************************************************
Description : A full system reset.
Input       : None.
Output      : None.    
Return      : None.
******************************************************************************/
    .section            .text.__rme_a7m_reset
    .align              3

	.thumb_func
__RME_A7M_Reset:
    /* Disable all interrupts */
    CPSID               I
    /* ARMv7-M Standard system reset */
    LDR                 R0,=0xE000ED0C
    LDR                 R1,=0x05FA0004
    STR                 R1,[R0]
    ISB
    /* Deadloop */
    B                   .
/* End Function:__RME_A7M_Reset **********************************************/

/* Function:__RME_A7M_Wait_Int ************************************************
Description : Wait until a new interrupt comes, to save power.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_a7m_wait_int
    .align              3

	.thumb_func
__RME_A7M_Wait_Int:
    WFE 
    BX                  LR
/* End Function:__RME_A7M_Wait_Int *******************************************/

/* Function:__RME_A7M_MSB_Get *************************************************
Description : Get the MSB of the word.
Input       : ptr_t Val - The value.
Output      : None.
Return      : ptr_t - The MSB position.
******************************************************************************/
    .section            .text.__rme_a7m_msb_get
    .align              3

	.thumb_func
__RME_A7M_MSB_Get:
    CLZ                 R1,R0
    MOV                 R0,#31
    SUB                 R0,R1
    BX                  LR
/* End Function:__RME_A7M_MSB_Get ********************************************/

/* Function:__RME_User_Enter **************************************************
Description : Entering of the user mode, after the system finish its preliminary
              booting. The function shall never return. This function should only
              be used to boot the first process in the system.
Input       : ptr_t Entry - The user execution startpoint.
              ptr_t Stack - The user stack.
              ptr_t CPUID - The CPUID.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.__rme_user_enter
    .align              3

	.thumb_func
__RME_User_Enter:
    MSR                 PSP,R1              /* Set the stack pointer */
    MOV                 R4,#0x03            /* Unprevileged thread mode */
    MSR                 CONTROL,R4
    ISB
    MOV                 R1,R0               /* Save the entry to R1 */
    MOV                 R0,R2               /* Save CPUID(0) to R0 */
    BLX                 R1                  /* Branch to our target */
/* End Function:__RME_User_Enter *********************************************/

/* Function:SysTick_Handler ***************************************************
Description : The System Tick Timer handler routine. This will in fact call a
              C function to resolve the system service routines.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.systick_handler
    .align              3

	.thumb_func
SysTick_Handler:
    PUSH                {R4-R11,LR}         /* Save registers */
    MRS                 R0,PSP
    PUSH                {R0}
    
    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Tim_Handler
    
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         /* Restore registers */
/* End Function:SysTick_Handler **********************************************/

/* Function:SVC_Handler *******************************************************
Description : The SVC handler routine. This will in fact call a C function to resolve
              the system service routines.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.svc_handler
    .align              3

	.thumb_func
SVC_Handler:
    PUSH                {R4-R11,LR}         /* Save registers */
    MRS                 R0,PSP
    PUSH                {R0}
    
    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Svc_Handler
    
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         /* Restore registers */
/* End Function:SVC_Handler **************************************************/

/* Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler ***************
Description : The multi-purpose handler routine. This will in fact call
              a C function to resolve the system service routines.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.system_handler
    .align              3

	.thumb_func
NMI_Handler:
    NOP
PendSV_Handler:
    NOP
DebugMon_Handler:
    NOP
HardFault_Handler:
    NOP
MemManage_Handler:
    NOP
BusFault_Handler:
    NOP
UsageFault_Handler:
    PUSH                {R4-R11,LR}         /* Save registers */
    MRS                 R0,PSP
    PUSH                {R0}
    
    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Exc_Handler
    
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         /* Restore registers */
/* End Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler **********/

/* Function:___RME_A7M_Thd_Cop_Clear ******************************************
Description : Clean up the coprocessor state so that the FP information is not
              leaked when switching from a fpu-enabled thread to a fpu-disabled
              thread.             
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_a7m_thd_cop_clear
    .align              3

	.thumb_func
___RME_A7M_Thd_Cop_Clear:            
    /* Use DCI to avoid compilation errors when FPU not enabled */
    LDR                 R0,=COP_CLEAR
    .short              0xEC90              /* VLDMIA    R0,{S0-S31} */
    .short              0x0A20              /* Clear all the FPU registers */
    MOV                 R0,#0               /* Clear FPSCR as well */
    .short              0xEEE1              /* VMSR      FPSCR, R0 */
    .short              0x0A10
    BX                  LR
COP_CLEAR:
    .space               32*4
/* End Function:___RME_A7M_Thd_Cop_Clear *************************************/

/* Function:___RME_A7M_Thd_Cop_Save *******************************************
Description : Save the coprocessor context on switch.         
Input       : R0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_a7m_thd_cop_save
    .align              3

	.thumb_func
___RME_A7M_Thd_Cop_Save:
    /* Use DCI to avoid compilation errors when FPU not enabled */
    .short              0xEC80              /* VSTMIA    R0,{S16-S31} */
    .short              0x8A10              /* Save all the FPU registers */
    BX                  LR
/* End Function:___RME_A7M_Thd_Cop_Save **************************************/

/* Function:___RME_A7M_Thd_Cop_Load *******************************************
Description : Restore the coprocessor context on switch.             
Input       : R0 - The pointer to the coprocessor struct.
Output      : None.
Return      : None.
******************************************************************************/
    .section            .text.___rme_a7m_thd_cop_load
    .align              3

	.thumb_func
___RME_A7M_Thd_Cop_Load:
/* Use DCI to avoid compilation errors when FPU not enabled*/
    .short              0xEC90            /* VLDMIA    R0,{S16-S31} */
    .short              0x8A10            /* Restore all the FPU registers */
    BX                  LR
/* End Function:___RME_A7M_Thd_Cop_Load **************************************/

/* Function:___RME_A7M_MPU_Set ************************************************
Description : Set the MPU context. We write 8 registers at a time to increase efficiency.            
Input       : R0 - The pointer to the MPU content.
Output      : None.
Return      : None.
******************************************************************************/
    .macro              MPU_PRE
    PUSH                {R4-R9}             /* Save registers */
    LDR                 R1, =0xE000ED9C     /* The base address of MPU RBAR and all 4 registers */
    .endm
    
    .macro              MPU_SET
    LDMIA               R0!, {R2-R3}        /* Read settings */
    STMIA               R1, {R2-R3}         /* Program */
    .endm
    
    .macro              MPU_SET2
    LDMIA               R0!, {R2-R5}
    STMIA               R1, {R2-R5} 
    .endm
    
    .macro              MPU_SET3
    LDMIA               R0!, {R2-R7}
    STMIA               R1, {R2-R7}
    .endm
    
    .macro              MPU_SET4
    LDMIA               R0!, {R2-R9}
    STMIA               R1, {R2-R9}
    .endm
    
    .macro              MPU_POST
    POP                 {R4-R9}             /* Restore registers */
    ISB                                     /* Barrier */
    BX                  LR
    .endm
                        
/* 1-region version */
    .section            .text.___rme_a7m_mpu_set1
    .align              3

	.thumb_func
___RME_A7M_MPU_Set1:
    MPU_PRE
    MPU_SET
    MPU_POST

/* 2-region version */
    .section            .text.___rme_a7m_mpu_set2
    .align              3

	.thumb_func
___RME_A7M_MPU_Set2:
    MPU_PRE
    MPU_SET2
    MPU_POST

/* 3-region version */
    .section            .text.___rme_a7m_mpu_set3
    .align              3

	.thumb_func
___RME_A7M_MPU_Set3:
    MPU_PRE
    MPU_SET3
    MPU_POST

/* 4-region version */
    .section            .text.___rme_a7m_mpu_set4
    .align              3

	.thumb_func
___RME_A7M_MPU_Set4:
    MPU_PRE
    MPU_SET4
    MPU_POST

/* 5-region version */
    .section            .text.___rme_a7m_mpu_set5
    .align              3

	.thumb_func
___RME_A7M_MPU_Set5:
    MPU_PRE
    MPU_SET4
    MPU_SET
    MPU_POST

/* 6-region version */
    .section            .text.___rme_a7m_mpu_set6
    .align              3

	.thumb_func
___RME_A7M_MPU_Set6:
    MPU_PRE
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 7-region version */
    .section            .text.___rme_a7m_mpu_set7
    .align              3

	.thumb_func
___RME_A7M_MPU_Set7:
    MPU_PRE
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 8-region version */
    .section            .text.___rme_a7m_mpu_set8
    .align              3

	.thumb_func
___RME_A7M_MPU_Set8:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_POST

/* 9-region version */
    .section            .text.___rme_a7m_mpu_set9
    .align              3

	.thumb_func
___RME_A7M_MPU_Set9:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST

/* 10-region version */
    .section            .text.___rme_a7m_mpu_set10
    .align              3

	.thumb_func
___RME_A7M_MPU_Set10:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 11-region version */
    .section            .text.___rme_a7m_mpu_set11
    .align              3

	.thumb_func
___RME_A7M_MPU_Set11:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 12-region version */
    .section            .text.___rme_a7m_mpu_set12
    .align              3

	.thumb_func
___RME_A7M_MPU_Set12:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST

/* 13-region version */
    .section            .text.___rme_a7m_mpu_set13
    .align              3

	.thumb_func
___RME_A7M_MPU_Set13:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST

/* 14-region version */
    .section            .text.___rme_a7m_mpu_set14
    .align              3

	.thumb_func
___RME_A7M_MPU_Set14:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST

/* 15-region version */
    .section            .text.___rme_a7m_mpu_set15
    .align              3

	.thumb_func
___RME_A7M_MPU_Set15:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST

/* 16-region version */
    .section            .text.___rme_a7m_mpu_set16
    .align              3

	.thumb_func
___RME_A7M_MPU_Set16:
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
/* End Function:___RME_A7M_MPU_Set *******************************************/

	.end
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

