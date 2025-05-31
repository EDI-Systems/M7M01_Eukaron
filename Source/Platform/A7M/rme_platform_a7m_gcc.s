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
    .macro              RME_A7M_SAVE
    PUSH                {R4-R11,LR}         /* Save registers */
    MRS                 R0,PSP
    PUSH                {R0}
    LDR                 R4,=0xE000ED94      /* Turn off MPU in case it sets wrong permission for kernel */
    LDR                 R5,=0x00000000
    STR                 R5,[R4]
    .endm

    .macro              RME_A7M_LOAD
    LDR                 R4,=0xE000ED94      /* Turn MPU back on */
    LDR                 R5,=0x00000005
    STR                 R5,[R4]
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         /* Restore registers */
    .endm

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
    LDR                 R3,[R2]
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
    .word               __RME_Stack         /* Top of Stack */
    .word               __RME_Entry         /* Reset Handler */
    .word               NMI_Handler         /* NMI Handler */
    .word               HardFault_Handler   /* Hard Fault Handler */
    .word               MemManage_Handler   /* MPU Fault Handler */
    .word               BusFault_Handler    /* Bus Fault Handler */
    .word               UsageFault_Handler  /* Usage Fault Handler */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               0                   /* Reserved */
    .word               SVC_Handler         /* SVCall Handler */
    .word               DebugMon_Handler    /* Debug Monitor Handler */
    .word               0                   /* Reserved */
    .word               PendSV_Handler      /* PendSV Handler */
    .word               SysTick_Handler     /* SysTick Handler */

    .word               IRQ0_Handler        /* 240 External Interrupts */
    .word               IRQ1_Handler
    .word               IRQ2_Handler
    .word               IRQ3_Handler
    .word               IRQ4_Handler
    .word               IRQ5_Handler
    .word               IRQ6_Handler
    .word               IRQ7_Handler
    .word               IRQ8_Handler
    .word               IRQ9_Handler

    .word               IRQ10_Handler
    .word               IRQ11_Handler
    .word               IRQ12_Handler
    .word               IRQ13_Handler
    .word               IRQ14_Handler
    .word               IRQ15_Handler
    .word               IRQ16_Handler
    .word               IRQ17_Handler
    .word               IRQ18_Handler
    .word               IRQ19_Handler

    .word               IRQ20_Handler
    .word               IRQ21_Handler
    .word               IRQ22_Handler
    .word               IRQ23_Handler
    .word               IRQ24_Handler
    .word               IRQ25_Handler
    .word               IRQ26_Handler
    .word               IRQ27_Handler
    .word               IRQ28_Handler
    .word               IRQ29_Handler

    .word               IRQ30_Handler
    .word               IRQ31_Handler
    .word               IRQ32_Handler
    .word               IRQ33_Handler
    .word               IRQ34_Handler
    .word               IRQ35_Handler
    .word               IRQ36_Handler
    .word               IRQ37_Handler
    .word               IRQ38_Handler
    .word               IRQ39_Handler

    .word               IRQ40_Handler
    .word               IRQ41_Handler
    .word               IRQ42_Handler
    .word               IRQ43_Handler
    .word               IRQ44_Handler
    .word               IRQ45_Handler
    .word               IRQ46_Handler
    .word               IRQ47_Handler
    .word               IRQ48_Handler
    .word               IRQ49_Handler

    .word               IRQ50_Handler
    .word               IRQ51_Handler
    .word               IRQ52_Handler
    .word               IRQ53_Handler
    .word               IRQ54_Handler
    .word               IRQ55_Handler
    .word               IRQ56_Handler
    .word               IRQ57_Handler
    .word               IRQ58_Handler
    .word               IRQ59_Handler

    .word               IRQ60_Handler
    .word               IRQ61_Handler
    .word               IRQ62_Handler
    .word               IRQ63_Handler
    .word               IRQ64_Handler
    .word               IRQ65_Handler
    .word               IRQ66_Handler
    .word               IRQ67_Handler
    .word               IRQ68_Handler
    .word               IRQ69_Handler

    .word               IRQ70_Handler
    .word               IRQ71_Handler
    .word               IRQ72_Handler
    .word               IRQ73_Handler
    .word               IRQ74_Handler
    .word               IRQ75_Handler
    .word               IRQ76_Handler
    .word               IRQ77_Handler
    .word               IRQ78_Handler
    .word               IRQ79_Handler

    .word               IRQ80_Handler
    .word               IRQ81_Handler
    .word               IRQ82_Handler
    .word               IRQ83_Handler
    .word               IRQ84_Handler
    .word               IRQ85_Handler
    .word               IRQ86_Handler
    .word               IRQ87_Handler
    .word               IRQ88_Handler
    .word               IRQ89_Handler

    .word               IRQ90_Handler
    .word               IRQ91_Handler
    .word               IRQ92_Handler
    .word               IRQ93_Handler
    .word               IRQ94_Handler
    .word               IRQ95_Handler
    .word               IRQ96_Handler
    .word               IRQ97_Handler
    .word               IRQ98_Handler
    .word               IRQ99_Handler

    .word               IRQ100_Handler
    .word               IRQ101_Handler
    .word               IRQ102_Handler
    .word               IRQ103_Handler
    .word               IRQ104_Handler
    .word               IRQ105_Handler
    .word               IRQ106_Handler
    .word               IRQ107_Handler
    .word               IRQ108_Handler
    .word               IRQ109_Handler

    .word               IRQ110_Handler
    .word               IRQ111_Handler
    .word               IRQ112_Handler
    .word               IRQ113_Handler
    .word               IRQ114_Handler
    .word               IRQ115_Handler
    .word               IRQ116_Handler
    .word               IRQ117_Handler
    .word               IRQ118_Handler
    .word               IRQ119_Handler

    .word               IRQ120_Handler
    .word               IRQ121_Handler
    .word               IRQ122_Handler
    .word               IRQ123_Handler
    .word               IRQ124_Handler
    .word               IRQ125_Handler
    .word               IRQ126_Handler
    .word               IRQ127_Handler
    .word               IRQ128_Handler
    .word               IRQ129_Handler

    .word               IRQ130_Handler
    .word               IRQ131_Handler
    .word               IRQ132_Handler
    .word               IRQ133_Handler
    .word               IRQ134_Handler
    .word               IRQ135_Handler
    .word               IRQ136_Handler
    .word               IRQ137_Handler
    .word               IRQ138_Handler
    .word               IRQ139_Handler

    .word               IRQ140_Handler
    .word               IRQ141_Handler
    .word               IRQ142_Handler
    .word               IRQ143_Handler
    .word               IRQ144_Handler
    .word               IRQ145_Handler
    .word               IRQ146_Handler
    .word               IRQ147_Handler
    .word               IRQ148_Handler
    .word               IRQ149_Handler

    .word               IRQ150_Handler
    .word               IRQ151_Handler
    .word               IRQ152_Handler
    .word               IRQ153_Handler
    .word               IRQ154_Handler
    .word               IRQ155_Handler
    .word               IRQ156_Handler
    .word               IRQ157_Handler
    .word               IRQ158_Handler
    .word               IRQ159_Handler

    .word               IRQ160_Handler
    .word               IRQ161_Handler
    .word               IRQ162_Handler
    .word               IRQ163_Handler
    .word               IRQ164_Handler
    .word               IRQ165_Handler
    .word               IRQ166_Handler
    .word               IRQ167_Handler
    .word               IRQ168_Handler
    .word               IRQ169_Handler

    .word               IRQ170_Handler
    .word               IRQ171_Handler
    .word               IRQ172_Handler
    .word               IRQ173_Handler
    .word               IRQ174_Handler
    .word               IRQ175_Handler
    .word               IRQ176_Handler
    .word               IRQ177_Handler
    .word               IRQ178_Handler
    .word               IRQ179_Handler

    .word               IRQ180_Handler
    .word               IRQ181_Handler
    .word               IRQ182_Handler
    .word               IRQ183_Handler
    .word               IRQ184_Handler
    .word               IRQ185_Handler
    .word               IRQ186_Handler
    .word               IRQ187_Handler
    .word               IRQ188_Handler
    .word               IRQ189_Handler

    .word               IRQ190_Handler
    .word               IRQ191_Handler
    .word               IRQ192_Handler
    .word               IRQ193_Handler
    .word               IRQ194_Handler
    .word               IRQ195_Handler
    .word               IRQ196_Handler
    .word               IRQ197_Handler
    .word               IRQ198_Handler
    .word               IRQ199_Handler
    
    .word               IRQ200_Handler
    .word               IRQ201_Handler
    .word               IRQ202_Handler
    .word               IRQ203_Handler
    .word               IRQ204_Handler
    .word               IRQ205_Handler
    .word               IRQ206_Handler
    .word               IRQ207_Handler
    .word               IRQ208_Handler
    .word               IRQ209_Handler

    .word               IRQ210_Handler
    .word               IRQ211_Handler
    .word               IRQ212_Handler
    .word               IRQ213_Handler
    .word               IRQ214_Handler
    .word               IRQ215_Handler
    .word               IRQ216_Handler
    .word               IRQ217_Handler
    .word               IRQ218_Handler
    .word               IRQ219_Handler

    .word               IRQ220_Handler
    .word               IRQ221_Handler
    .word               IRQ222_Handler
    .word               IRQ223_Handler
    .word               IRQ224_Handler
    .word               IRQ225_Handler
    .word               IRQ226_Handler
    .word               IRQ227_Handler
    .word               IRQ228_Handler
    .word               IRQ229_Handler

    .word               IRQ230_Handler
    .word               IRQ231_Handler
    .word               IRQ232_Handler
    .word               IRQ233_Handler
    .word               IRQ234_Handler
    .word               IRQ235_Handler
    .word               IRQ236_Handler
    .word               IRQ237_Handler
    .word               IRQ238_Handler
    .word               IRQ239_Handler

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
    .thumb_func
IRQ0_Handler:
    .thumb_func
IRQ1_Handler:
    .thumb_func
IRQ2_Handler:
    .thumb_func
IRQ3_Handler:
    .thumb_func
IRQ4_Handler:
    .thumb_func
IRQ5_Handler:
    .thumb_func
IRQ6_Handler:
    .thumb_func
IRQ7_Handler:
    .thumb_func
IRQ8_Handler:
    .thumb_func
IRQ9_Handler:

    .thumb_func
IRQ10_Handler:
    .thumb_func
IRQ11_Handler:
    .thumb_func
IRQ12_Handler:
    .thumb_func
IRQ13_Handler:
    .thumb_func
IRQ14_Handler:
    .thumb_func
IRQ15_Handler:
    .thumb_func
IRQ16_Handler:
    .thumb_func
IRQ17_Handler:
    .thumb_func
IRQ18_Handler:
    .thumb_func
IRQ19_Handler:

    .thumb_func
IRQ20_Handler:
    .thumb_func
IRQ21_Handler:
    .thumb_func
IRQ22_Handler:
    .thumb_func
IRQ23_Handler:
    .thumb_func
IRQ24_Handler:
    .thumb_func
IRQ25_Handler:
    .thumb_func
IRQ26_Handler:
    .thumb_func
IRQ27_Handler:
    .thumb_func
IRQ28_Handler:
    .thumb_func
IRQ29_Handler:

    .thumb_func
IRQ30_Handler:
    .thumb_func
IRQ31_Handler:
    .thumb_func
IRQ32_Handler:
    .thumb_func
IRQ33_Handler:
    .thumb_func
IRQ34_Handler:
    .thumb_func
IRQ35_Handler:
    .thumb_func
IRQ36_Handler:
    .thumb_func
IRQ37_Handler:
    .thumb_func
IRQ38_Handler:
    .thumb_func
IRQ39_Handler:

    .thumb_func
IRQ40_Handler:
    .thumb_func
IRQ41_Handler:
    .thumb_func
IRQ42_Handler:
    .thumb_func
IRQ43_Handler:
    .thumb_func
IRQ44_Handler:
    .thumb_func
IRQ45_Handler:
    .thumb_func
IRQ46_Handler:
    .thumb_func
IRQ47_Handler:
    .thumb_func
IRQ48_Handler:
    .thumb_func
IRQ49_Handler:

    .thumb_func
IRQ50_Handler:
    .thumb_func
IRQ51_Handler:
    .thumb_func
IRQ52_Handler:
    .thumb_func
IRQ53_Handler:
    .thumb_func
IRQ54_Handler:
    .thumb_func
IRQ55_Handler:
    .thumb_func
IRQ56_Handler:
    .thumb_func
IRQ57_Handler:
    .thumb_func
IRQ58_Handler:
    .thumb_func
IRQ59_Handler:

    .thumb_func
IRQ60_Handler:
    .thumb_func
IRQ61_Handler:
    .thumb_func
IRQ62_Handler:
    .thumb_func
IRQ63_Handler:
    .thumb_func
IRQ64_Handler:
    .thumb_func
IRQ65_Handler:
    .thumb_func
IRQ66_Handler:
    .thumb_func
IRQ67_Handler:
    .thumb_func
IRQ68_Handler:
    .thumb_func
IRQ69_Handler:
 
    .thumb_func
IRQ70_Handler:
    .thumb_func
IRQ71_Handler:
    .thumb_func
IRQ72_Handler:
    .thumb_func
IRQ73_Handler:
    .thumb_func
IRQ74_Handler:
    .thumb_func
IRQ75_Handler:
    .thumb_func
IRQ76_Handler:
    .thumb_func
IRQ77_Handler:
    .thumb_func
IRQ78_Handler:
    .thumb_func
IRQ79_Handler:

    .thumb_func
IRQ80_Handler:
    .thumb_func
IRQ81_Handler:
    .thumb_func
IRQ82_Handler:
    .thumb_func
IRQ83_Handler:
    .thumb_func
IRQ84_Handler:
    .thumb_func
IRQ85_Handler:
    .thumb_func
IRQ86_Handler:
    .thumb_func
IRQ87_Handler:
    .thumb_func
IRQ88_Handler:
    .thumb_func
IRQ89_Handler:
 
    .thumb_func
IRQ90_Handler:
    .thumb_func
IRQ91_Handler:
    .thumb_func
IRQ92_Handler:
    .thumb_func
IRQ93_Handler:
    .thumb_func
IRQ94_Handler:
    .thumb_func
IRQ95_Handler:
    .thumb_func
IRQ96_Handler:
    .thumb_func
IRQ97_Handler:
    .thumb_func
IRQ98_Handler:
    .thumb_func
IRQ99_Handler:

    .thumb_func
IRQ100_Handler:
    .thumb_func
IRQ101_Handler:
    .thumb_func
IRQ102_Handler:
    .thumb_func
IRQ103_Handler:
    .thumb_func
IRQ104_Handler:
    .thumb_func
IRQ105_Handler:
    .thumb_func
IRQ106_Handler:
    .thumb_func
IRQ107_Handler:
    .thumb_func
IRQ108_Handler:
    .thumb_func
IRQ109_Handler:

    .thumb_func
IRQ110_Handler:
    .thumb_func
IRQ111_Handler:
    .thumb_func
IRQ112_Handler:
    .thumb_func
IRQ113_Handler:
    .thumb_func
IRQ114_Handler:
    .thumb_func
IRQ115_Handler:
    .thumb_func
IRQ116_Handler:
    .thumb_func
IRQ117_Handler:
    .thumb_func
IRQ118_Handler:
    .thumb_func
IRQ119_Handler:

    .thumb_func
IRQ120_Handler:
    .thumb_func
IRQ121_Handler:
    .thumb_func
IRQ122_Handler:
    .thumb_func
IRQ123_Handler:
    .thumb_func
IRQ124_Handler:
    .thumb_func
IRQ125_Handler:
    .thumb_func
IRQ126_Handler:
    .thumb_func
IRQ127_Handler:
    .thumb_func
IRQ128_Handler:
    .thumb_func
IRQ129_Handler:

    .thumb_func
IRQ130_Handler:
    .thumb_func
IRQ131_Handler:
    .thumb_func
IRQ132_Handler:
    .thumb_func
IRQ133_Handler:
    .thumb_func
IRQ134_Handler:
    .thumb_func
IRQ135_Handler:
    .thumb_func
IRQ136_Handler:
    .thumb_func
IRQ137_Handler:
    .thumb_func
IRQ138_Handler:
    .thumb_func
IRQ139_Handler:

    .thumb_func
IRQ140_Handler:
    .thumb_func
IRQ141_Handler:
    .thumb_func
IRQ142_Handler:
    .thumb_func
IRQ143_Handler:
    .thumb_func
IRQ144_Handler:
    .thumb_func
IRQ145_Handler:
    .thumb_func
IRQ146_Handler:
    .thumb_func
IRQ147_Handler:
    .thumb_func
IRQ148_Handler:
    .thumb_func
IRQ149_Handler:

    .thumb_func
IRQ150_Handler:
    .thumb_func
IRQ151_Handler:
    .thumb_func
IRQ152_Handler:
    .thumb_func
IRQ153_Handler:
    .thumb_func
IRQ154_Handler:
    .thumb_func
IRQ155_Handler:
    .thumb_func
IRQ156_Handler:
    .thumb_func
IRQ157_Handler:
    .thumb_func
IRQ158_Handler:
    .thumb_func
IRQ159_Handler:

    .thumb_func
IRQ160_Handler:
    .thumb_func
IRQ161_Handler:
    .thumb_func
IRQ162_Handler:
    .thumb_func
IRQ163_Handler:
    .thumb_func
IRQ164_Handler:
    .thumb_func
IRQ165_Handler:
    .thumb_func
IRQ166_Handler:
    .thumb_func
IRQ167_Handler:
    .thumb_func
IRQ168_Handler:
    .thumb_func
IRQ169_Handler:
 
    .thumb_func
IRQ170_Handler:
    .thumb_func
IRQ171_Handler:
    .thumb_func
IRQ172_Handler:
    .thumb_func
IRQ173_Handler:
    .thumb_func
IRQ174_Handler:
    .thumb_func
IRQ175_Handler:
    .thumb_func
IRQ176_Handler:
    .thumb_func
IRQ177_Handler:
    .thumb_func
IRQ178_Handler:
    .thumb_func
IRQ179_Handler:

    .thumb_func
IRQ180_Handler:
    .thumb_func
IRQ181_Handler:
    .thumb_func
IRQ182_Handler:
    .thumb_func
IRQ183_Handler:
    .thumb_func
IRQ184_Handler:
    .thumb_func
IRQ185_Handler:
    .thumb_func
IRQ186_Handler:
    .thumb_func
IRQ187_Handler:
    .thumb_func
IRQ188_Handler:
    .thumb_func
IRQ189_Handler:

    .thumb_func
IRQ190_Handler:
    .thumb_func
IRQ191_Handler:
    .thumb_func
IRQ192_Handler:
    .thumb_func
IRQ193_Handler:
    .thumb_func
IRQ194_Handler:
    .thumb_func
IRQ195_Handler:
    .thumb_func
IRQ196_Handler:
    .thumb_func
IRQ197_Handler:
    .thumb_func
IRQ198_Handler:
    .thumb_func
IRQ199_Handler:

    .thumb_func
IRQ200_Handler:
    .thumb_func
IRQ201_Handler:
    .thumb_func
IRQ202_Handler:
    .thumb_func
IRQ203_Handler:
    .thumb_func
IRQ204_Handler:
    .thumb_func
IRQ205_Handler:
    .thumb_func
IRQ206_Handler:
    .thumb_func
IRQ207_Handler:
    .thumb_func
IRQ208_Handler:
    .thumb_func
IRQ209_Handler:

    .thumb_func
IRQ210_Handler:
    .thumb_func
IRQ211_Handler:
    .thumb_func
IRQ212_Handler:
    .thumb_func
IRQ213_Handler:
    .thumb_func
IRQ214_Handler:
    .thumb_func
IRQ215_Handler:
    .thumb_func
IRQ216_Handler:
    .thumb_func
IRQ217_Handler:
    .thumb_func
IRQ218_Handler:
    .thumb_func
IRQ219_Handler:

    .thumb_func
IRQ220_Handler:
    .thumb_func
IRQ221_Handler:
    .thumb_func
IRQ222_Handler:
    .thumb_func
IRQ223_Handler:
    .thumb_func
IRQ224_Handler:
    .thumb_func
IRQ225_Handler:
    .thumb_func
IRQ226_Handler:
    .thumb_func
IRQ227_Handler:
    .thumb_func
IRQ228_Handler:
    .thumb_func
IRQ229_Handler:

    .thumb_func
IRQ230_Handler:
    .thumb_func
IRQ231_Handler:
    .thumb_func
IRQ232_Handler:
    .thumb_func
IRQ233_Handler:
    .thumb_func
IRQ234_Handler:
    .thumb_func
IRQ235_Handler:
    .thumb_func
IRQ236_Handler:
    .thumb_func
IRQ237_Handler:
    .thumb_func
IRQ238_Handler:
    .thumb_func
IRQ239_Handler:
    RME_A7M_SAVE

    MOV                 R0,SP               /* Pass in the regs */
    MRS                 R1,xPSR             /* Pass in the interrupt number */
    UBFX                R1,R1,#0,#9
    SUB                 R1,#16              /* The IRQ0's starting number is 16; subtract */
    BL                  __RME_A7M_Vct_Handler

    RME_A7M_LOAD
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
    RME_A7M_SAVE

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Tim_Handler

    RME_A7M_LOAD
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
    RME_A7M_SAVE

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Svc_Handler

    RME_A7M_LOAD
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
    .thumb_func
PendSV_Handler:
    NOP
    .thumb_func
DebugMon_Handler:
    NOP
    .thumb_func
HardFault_Handler:
    NOP
    .thumb_func
MemManage_Handler:
    NOP
    .thumb_func
BusFault_Handler:
    NOP
    .thumb_func
UsageFault_Handler:
    RME_A7M_SAVE

    MOV                 R0,SP               /* Pass in the regs */
    BL                  __RME_A7M_Exc_Handler

    RME_A7M_LOAD
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
    .hword              0xEC90              /* VLDMIA    R0,{S0-S31} */
    .hword              0x0A20              /* Clear all the FPU registers */
    MOV                 R0,#0               /* Clear FPSCR as well */
    .hword              0xEEE1              /* VMSR      FPSCR, R0 */
    .hword              0x0A10
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
    .hword              0xEC80              /* VSTMIA    R0,{S16-S31} */
    .hword              0x8A10              /* Save all the FPU registers */
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
    .hword              0xEC90            /* VLDMIA    R0,{S16-S31} */
    .hword              0x8A10            /* Restore all the FPU registers */
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
    LDR                 R1,=0xE000ED9C      /* The base address of MPU RBAR and all 4 registers */
    .endm
    
    .macro              MPU_SET
    LDMIA               R0!,{R2-R3}         /* Read settings */
    STMIA               R1,{R2-R3}          /* Program */
    .endm
    
    .macro              MPU_SET2
    LDMIA               R0!,{R2-R5}
    STMIA               R1,{R2-R5} 
    .endm
    
    .macro              MPU_SET3
    LDMIA               R0!,{R2-R7}
    STMIA               R1,{R2-R7}
    .endm
    
    .macro              MPU_SET4
    LDMIA               R0!,{R2-R9}
    STMIA               R1,{R2-R9}
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

