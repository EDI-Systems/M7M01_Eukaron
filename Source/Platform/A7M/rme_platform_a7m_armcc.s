;/*****************************************************************************
;Filename    : rme_platform_a7m_asm.s
;Author      : pry
;Date        : 19/01/2017
;Description : The ARMv7-M assembly support of the RME RTOS, for armcc.
;*****************************************************************************/

;/* The ARMv7-M Architecture **************************************************
;R0-R7:General purpose registers that are accessible. 
;R8-R12:general purpose registers that can only be reached by 32-bit instructions.
;R13:SP/SP_process/SP_main    Stack pointer
;R14:LR                       Link Register(used for returning from a subfunction)
;R15:PC                       Program counter.
;IPSR                         Interrupt Program Status Register.
;APSR                         Application Program Status Register.
;EPSR                         Execute Program Status Register.
;The above 3 registers are saved into the stack in combination(xPSR).
;
;The Cortex-M4 include a single-precision FPU, and the Cortex-M7 will feature
;a double-precision FPU.
;*****************************************************************************/

;/* Import *******************************************************************/
    ;Preinitialization routine
    IMPORT              __RME_A7M_Lowlvl_Preinit
    ;The ARM C library entry
    IMPORT              __main
    ;The ARM linker stack segment - provided by linker
    IMPORT              |Image$$ARM_LIB_STACK$$ZI$$Limit|
    ;The system call handler
    IMPORT              __RME_A7M_Svc_Handler
    ;The system tick handler
    IMPORT              __RME_A7M_Tim_Handler
    ;The memory management fault handler
    IMPORT              __RME_A7M_Exc_Handler
    ;The generic interrupt handler for all other vectors
    IMPORT              __RME_A7M_Vct_Handler
;/* End Import ***************************************************************/

;/* Export *******************************************************************/
    ;Disable all interrupts
    EXPORT              __RME_Int_Disable
    ;Enable all interrupts
    EXPORT              __RME_Int_Enable
    ;A full barrier
    EXPORT              __RME_A7M_Barrier
    ;Full system reset
    EXPORT              __RME_A7M_Reset
    ;Wait until interrupts happen
    EXPORT              __RME_A7M_Wait_Int
    ;Get the MSB in a word
    EXPORT              __RME_A7M_MSB_Get
    ;Entering of the user mode
    EXPORT              __RME_User_Enter
    ;Clear FPU register contents
    EXPORT              ___RME_A7M_Thd_Cop_Clear
    ;The FPU register save routine
    EXPORT              ___RME_A7M_Thd_Cop_Save
    ;The FPU register restore routine
    EXPORT              ___RME_A7M_Thd_Cop_Load
    ;The MPU setup routines
    EXPORT              ___RME_A7M_MPU_Set1
    EXPORT              ___RME_A7M_MPU_Set2
    EXPORT              ___RME_A7M_MPU_Set3
    EXPORT              ___RME_A7M_MPU_Set4
    EXPORT              ___RME_A7M_MPU_Set5
    EXPORT              ___RME_A7M_MPU_Set6
    EXPORT              ___RME_A7M_MPU_Set7
    EXPORT              ___RME_A7M_MPU_Set8
    EXPORT              ___RME_A7M_MPU_Set9
    EXPORT              ___RME_A7M_MPU_Set10
    EXPORT              ___RME_A7M_MPU_Set11
    EXPORT              ___RME_A7M_MPU_Set12
    EXPORT              ___RME_A7M_MPU_Set13
    EXPORT              ___RME_A7M_MPU_Set14
    EXPORT              ___RME_A7M_MPU_Set15
    EXPORT              ___RME_A7M_MPU_Set16
;/* End Export ***************************************************************/

;/* Entry ********************************************************************/
    ;The align is "(2^3)/8=1(Byte)."
    AREA                RME_ENTRY,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_Entry             PROC
Reset_Handler
    LDR                 R0,=__RME_A7M_Lowlvl_Preinit
    BLX                 R0
    LDR                 R0,=__main
    BX                  R0
    ENDP
;/* End Entry ****************************************************************/

;/* Vector *******************************************************************/
    ;Save registers
    MACRO
    RME_A7M_SAVE
    PUSH                {R4-R11,LR}         ;Save registers
    MRS                 R0,PSP
    PUSH                {R0}
    LDR                 R4,=0xE000ED94      ;Turn off MPU in case it sets wrong permission for kernel
    LDR                 R5,=0x00000000
    STR                 R5,[R4]
    MEND

    ;Restore registers
    MACRO
    RME_A7M_LOAD
    LDR                 R4,=0xE000ED94      ;Turn on MPU
    LDR                 R5,=0x00000005
    STR                 R5,[R4]
    POP                 {R0}
    MSR                 PSP,R0
    POP                 {R4-R11,PC}         ;Restore registers
    MEND

    AREA                RME_VECTOR,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

    ;Vector table
    DCD                 |Image$$ARM_LIB_STACK$$ZI$$Limit|
    DCD                 Reset_Handler       ;Reset Handler
    DCD                 NMI_Handler         ;NMI Handler
    DCD                 HardFault_Handler   ;Hard Fault Handler
    DCD                 MemManage_Handler   ;MPU Fault Handler
    DCD                 BusFault_Handler    ;Bus Fault Handler
    DCD                 UsageFault_Handler  ;Usage Fault Handler
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 0                   ;Reserved
    DCD                 SVC_Handler         ;SVCall Handler
    DCD                 DebugMon_Handler    ;Debug Monitor Handler
    DCD                 0                   ;Reserved
    DCD                 PendSV_Handler      ;PendSV Handler
    DCD                 SysTick_Handler     ;SysTick Handler

    DCD                 IRQ0_Handler        ;240 External Interrupts
    DCD                 IRQ1_Handler
    DCD                 IRQ2_Handler
    DCD                 IRQ3_Handler
    DCD                 IRQ4_Handler
    DCD                 IRQ5_Handler
    DCD                 IRQ6_Handler
    DCD                 IRQ7_Handler
    DCD                 IRQ8_Handler
    DCD                 IRQ9_Handler

    DCD                 IRQ10_Handler
    DCD                 IRQ11_Handler
    DCD                 IRQ12_Handler
    DCD                 IRQ13_Handler
    DCD                 IRQ14_Handler
    DCD                 IRQ15_Handler
    DCD                 IRQ16_Handler
    DCD                 IRQ17_Handler
    DCD                 IRQ18_Handler
    DCD                 IRQ19_Handler

    DCD                 IRQ20_Handler
    DCD                 IRQ21_Handler
    DCD                 IRQ22_Handler
    DCD                 IRQ23_Handler
    DCD                 IRQ24_Handler
    DCD                 IRQ25_Handler
    DCD                 IRQ26_Handler
    DCD                 IRQ27_Handler
    DCD                 IRQ28_Handler
    DCD                 IRQ29_Handler

    DCD                 IRQ30_Handler
    DCD                 IRQ31_Handler
    DCD                 IRQ32_Handler
    DCD                 IRQ33_Handler
    DCD                 IRQ34_Handler
    DCD                 IRQ35_Handler
    DCD                 IRQ36_Handler
    DCD                 IRQ37_Handler
    DCD                 IRQ38_Handler
    DCD                 IRQ39_Handler

    DCD                 IRQ40_Handler
    DCD                 IRQ41_Handler
    DCD                 IRQ42_Handler
    DCD                 IRQ43_Handler
    DCD                 IRQ44_Handler
    DCD                 IRQ45_Handler
    DCD                 IRQ46_Handler
    DCD                 IRQ47_Handler
    DCD                 IRQ48_Handler
    DCD                 IRQ49_Handler

    DCD                 IRQ50_Handler
    DCD                 IRQ51_Handler
    DCD                 IRQ52_Handler
    DCD                 IRQ53_Handler
    DCD                 IRQ54_Handler
    DCD                 IRQ55_Handler
    DCD                 IRQ56_Handler
    DCD                 IRQ57_Handler
    DCD                 IRQ58_Handler
    DCD                 IRQ59_Handler

    DCD                 IRQ60_Handler
    DCD                 IRQ61_Handler
    DCD                 IRQ62_Handler
    DCD                 IRQ63_Handler
    DCD                 IRQ64_Handler
    DCD                 IRQ65_Handler
    DCD                 IRQ66_Handler
    DCD                 IRQ67_Handler
    DCD                 IRQ68_Handler
    DCD                 IRQ69_Handler

    DCD                 IRQ70_Handler
    DCD                 IRQ71_Handler
    DCD                 IRQ72_Handler
    DCD                 IRQ73_Handler
    DCD                 IRQ74_Handler
    DCD                 IRQ75_Handler
    DCD                 IRQ76_Handler
    DCD                 IRQ77_Handler
    DCD                 IRQ78_Handler
    DCD                 IRQ79_Handler

    DCD                 IRQ80_Handler
    DCD                 IRQ81_Handler
    DCD                 IRQ82_Handler
    DCD                 IRQ83_Handler
    DCD                 IRQ84_Handler
    DCD                 IRQ85_Handler
    DCD                 IRQ86_Handler
    DCD                 IRQ87_Handler
    DCD                 IRQ88_Handler
    DCD                 IRQ89_Handler

    DCD                 IRQ90_Handler
    DCD                 IRQ91_Handler
    DCD                 IRQ92_Handler
    DCD                 IRQ93_Handler
    DCD                 IRQ94_Handler
    DCD                 IRQ95_Handler
    DCD                 IRQ96_Handler
    DCD                 IRQ97_Handler
    DCD                 IRQ98_Handler
    DCD                 IRQ99_Handler

    DCD                 IRQ100_Handler
    DCD                 IRQ101_Handler
    DCD                 IRQ102_Handler
    DCD                 IRQ103_Handler
    DCD                 IRQ104_Handler
    DCD                 IRQ105_Handler
    DCD                 IRQ106_Handler
    DCD                 IRQ107_Handler
    DCD                 IRQ108_Handler
    DCD                 IRQ109_Handler

    DCD                 IRQ110_Handler
    DCD                 IRQ111_Handler
    DCD                 IRQ112_Handler
    DCD                 IRQ113_Handler
    DCD                 IRQ114_Handler
    DCD                 IRQ115_Handler
    DCD                 IRQ116_Handler
    DCD                 IRQ117_Handler
    DCD                 IRQ118_Handler
    DCD                 IRQ119_Handler

    DCD                 IRQ120_Handler
    DCD                 IRQ121_Handler
    DCD                 IRQ122_Handler
    DCD                 IRQ123_Handler
    DCD                 IRQ124_Handler
    DCD                 IRQ125_Handler
    DCD                 IRQ126_Handler
    DCD                 IRQ127_Handler
    DCD                 IRQ128_Handler
    DCD                 IRQ129_Handler

    DCD                 IRQ130_Handler
    DCD                 IRQ131_Handler
    DCD                 IRQ132_Handler
    DCD                 IRQ133_Handler
    DCD                 IRQ134_Handler
    DCD                 IRQ135_Handler
    DCD                 IRQ136_Handler
    DCD                 IRQ137_Handler
    DCD                 IRQ138_Handler
    DCD                 IRQ139_Handler

    DCD                 IRQ140_Handler
    DCD                 IRQ141_Handler
    DCD                 IRQ142_Handler
    DCD                 IRQ143_Handler
    DCD                 IRQ144_Handler
    DCD                 IRQ145_Handler
    DCD                 IRQ146_Handler
    DCD                 IRQ147_Handler
    DCD                 IRQ148_Handler
    DCD                 IRQ149_Handler

    DCD                 IRQ150_Handler
    DCD                 IRQ151_Handler
    DCD                 IRQ152_Handler
    DCD                 IRQ153_Handler
    DCD                 IRQ154_Handler
    DCD                 IRQ155_Handler
    DCD                 IRQ156_Handler
    DCD                 IRQ157_Handler
    DCD                 IRQ158_Handler
    DCD                 IRQ159_Handler

    DCD                 IRQ160_Handler
    DCD                 IRQ161_Handler
    DCD                 IRQ162_Handler
    DCD                 IRQ163_Handler
    DCD                 IRQ164_Handler
    DCD                 IRQ165_Handler
    DCD                 IRQ166_Handler
    DCD                 IRQ167_Handler
    DCD                 IRQ168_Handler
    DCD                 IRQ169_Handler

    DCD                 IRQ170_Handler
    DCD                 IRQ171_Handler
    DCD                 IRQ172_Handler
    DCD                 IRQ173_Handler
    DCD                 IRQ174_Handler
    DCD                 IRQ175_Handler
    DCD                 IRQ176_Handler
    DCD                 IRQ177_Handler
    DCD                 IRQ178_Handler
    DCD                 IRQ179_Handler

    DCD                 IRQ180_Handler
    DCD                 IRQ181_Handler
    DCD                 IRQ182_Handler
    DCD                 IRQ183_Handler
    DCD                 IRQ184_Handler
    DCD                 IRQ185_Handler
    DCD                 IRQ186_Handler
    DCD                 IRQ187_Handler
    DCD                 IRQ188_Handler
    DCD                 IRQ189_Handler

    DCD                 IRQ190_Handler
    DCD                 IRQ191_Handler
    DCD                 IRQ192_Handler
    DCD                 IRQ193_Handler
    DCD                 IRQ194_Handler
    DCD                 IRQ195_Handler
    DCD                 IRQ196_Handler
    DCD                 IRQ197_Handler
    DCD                 IRQ198_Handler
    DCD                 IRQ199_Handler
    
    DCD                 IRQ200_Handler
    DCD                 IRQ201_Handler
    DCD                 IRQ202_Handler
    DCD                 IRQ203_Handler
    DCD                 IRQ204_Handler
    DCD                 IRQ205_Handler
    DCD                 IRQ206_Handler
    DCD                 IRQ207_Handler
    DCD                 IRQ208_Handler
    DCD                 IRQ209_Handler

    DCD                 IRQ210_Handler
    DCD                 IRQ211_Handler
    DCD                 IRQ212_Handler
    DCD                 IRQ213_Handler
    DCD                 IRQ214_Handler
    DCD                 IRQ215_Handler
    DCD                 IRQ216_Handler
    DCD                 IRQ217_Handler
    DCD                 IRQ218_Handler
    DCD                 IRQ219_Handler

    DCD                 IRQ220_Handler
    DCD                 IRQ221_Handler
    DCD                 IRQ222_Handler
    DCD                 IRQ223_Handler
    DCD                 IRQ224_Handler
    DCD                 IRQ225_Handler
    DCD                 IRQ226_Handler
    DCD                 IRQ227_Handler
    DCD                 IRQ228_Handler
    DCD                 IRQ229_Handler

    DCD                 IRQ230_Handler
    DCD                 IRQ231_Handler
    DCD                 IRQ232_Handler
    DCD                 IRQ233_Handler
    DCD                 IRQ234_Handler
    DCD                 IRQ235_Handler
    DCD                 IRQ236_Handler
    DCD                 IRQ237_Handler
    DCD                 IRQ238_Handler
    DCD                 IRQ239_Handler

Default_Handler         PROC                ;240 External Interrupts
    EXPORT              IRQ0_Handler        [WEAK]
    EXPORT              IRQ1_Handler        [WEAK]
    EXPORT              IRQ2_Handler        [WEAK]
    EXPORT              IRQ3_Handler        [WEAK]
    EXPORT              IRQ4_Handler        [WEAK]
    EXPORT              IRQ5_Handler        [WEAK]
    EXPORT              IRQ6_Handler        [WEAK]
    EXPORT              IRQ7_Handler        [WEAK]
    EXPORT              IRQ8_Handler        [WEAK]
    EXPORT              IRQ9_Handler        [WEAK]

    EXPORT              IRQ10_Handler       [WEAK]
    EXPORT              IRQ11_Handler       [WEAK]
    EXPORT              IRQ12_Handler       [WEAK]
    EXPORT              IRQ13_Handler       [WEAK]
    EXPORT              IRQ14_Handler       [WEAK]
    EXPORT              IRQ15_Handler       [WEAK]
    EXPORT              IRQ16_Handler       [WEAK]
    EXPORT              IRQ17_Handler       [WEAK]
    EXPORT              IRQ18_Handler       [WEAK]
    EXPORT              IRQ19_Handler       [WEAK]

    EXPORT              IRQ20_Handler       [WEAK]
    EXPORT              IRQ21_Handler       [WEAK]
    EXPORT              IRQ22_Handler       [WEAK]
    EXPORT              IRQ23_Handler       [WEAK]
    EXPORT              IRQ24_Handler       [WEAK]
    EXPORT              IRQ25_Handler       [WEAK]
    EXPORT              IRQ26_Handler       [WEAK]
    EXPORT              IRQ27_Handler       [WEAK]
    EXPORT              IRQ28_Handler       [WEAK]
    EXPORT              IRQ29_Handler       [WEAK]

    EXPORT              IRQ30_Handler       [WEAK]
    EXPORT              IRQ31_Handler       [WEAK]
    EXPORT              IRQ32_Handler       [WEAK]
    EXPORT              IRQ33_Handler       [WEAK]
    EXPORT              IRQ34_Handler       [WEAK]
    EXPORT              IRQ35_Handler       [WEAK]
    EXPORT              IRQ36_Handler       [WEAK]
    EXPORT              IRQ37_Handler       [WEAK]
    EXPORT              IRQ38_Handler       [WEAK]
    EXPORT              IRQ39_Handler       [WEAK]

    EXPORT              IRQ40_Handler       [WEAK]
    EXPORT              IRQ41_Handler       [WEAK]
    EXPORT              IRQ42_Handler       [WEAK]
    EXPORT              IRQ43_Handler       [WEAK]
    EXPORT              IRQ44_Handler       [WEAK]
    EXPORT              IRQ45_Handler       [WEAK]
    EXPORT              IRQ46_Handler       [WEAK]
    EXPORT              IRQ47_Handler       [WEAK]
    EXPORT              IRQ48_Handler       [WEAK]
    EXPORT              IRQ49_Handler       [WEAK]

    EXPORT              IRQ50_Handler       [WEAK]
    EXPORT              IRQ51_Handler       [WEAK]
    EXPORT              IRQ52_Handler       [WEAK]
    EXPORT              IRQ53_Handler       [WEAK]
    EXPORT              IRQ54_Handler       [WEAK]
    EXPORT              IRQ55_Handler       [WEAK]
    EXPORT              IRQ56_Handler       [WEAK]
    EXPORT              IRQ57_Handler       [WEAK]
    EXPORT              IRQ58_Handler       [WEAK]
    EXPORT              IRQ59_Handler       [WEAK]

    EXPORT              IRQ60_Handler       [WEAK]
    EXPORT              IRQ61_Handler       [WEAK]
    EXPORT              IRQ62_Handler       [WEAK]
    EXPORT              IRQ63_Handler       [WEAK]
    EXPORT              IRQ64_Handler       [WEAK]
    EXPORT              IRQ65_Handler       [WEAK]
    EXPORT              IRQ66_Handler       [WEAK]
    EXPORT              IRQ67_Handler       [WEAK]
    EXPORT              IRQ68_Handler       [WEAK]
    EXPORT              IRQ69_Handler       [WEAK]

    EXPORT              IRQ70_Handler       [WEAK]
    EXPORT              IRQ71_Handler       [WEAK]
    EXPORT              IRQ72_Handler       [WEAK]
    EXPORT              IRQ73_Handler       [WEAK]
    EXPORT              IRQ74_Handler       [WEAK]
    EXPORT              IRQ75_Handler       [WEAK]
    EXPORT              IRQ76_Handler       [WEAK]
    EXPORT              IRQ77_Handler       [WEAK]
    EXPORT              IRQ78_Handler       [WEAK]
    EXPORT              IRQ79_Handler       [WEAK]

    EXPORT              IRQ80_Handler       [WEAK]
    EXPORT              IRQ81_Handler       [WEAK]
    EXPORT              IRQ82_Handler       [WEAK]
    EXPORT              IRQ83_Handler       [WEAK]
    EXPORT              IRQ84_Handler       [WEAK]
    EXPORT              IRQ85_Handler       [WEAK]
    EXPORT              IRQ86_Handler       [WEAK]
    EXPORT              IRQ87_Handler       [WEAK]
    EXPORT              IRQ88_Handler       [WEAK]
    EXPORT              IRQ89_Handler       [WEAK]

    EXPORT              IRQ90_Handler       [WEAK]
    EXPORT              IRQ91_Handler       [WEAK]
    EXPORT              IRQ92_Handler       [WEAK]
    EXPORT              IRQ93_Handler       [WEAK]
    EXPORT              IRQ94_Handler       [WEAK]
    EXPORT              IRQ95_Handler       [WEAK]
    EXPORT              IRQ96_Handler       [WEAK]
    EXPORT              IRQ97_Handler       [WEAK]
    EXPORT              IRQ98_Handler       [WEAK]
    EXPORT              IRQ99_Handler       [WEAK]

    EXPORT              IRQ100_Handler      [WEAK]
    EXPORT              IRQ101_Handler      [WEAK]
    EXPORT              IRQ102_Handler      [WEAK]
    EXPORT              IRQ103_Handler      [WEAK]
    EXPORT              IRQ104_Handler      [WEAK]
    EXPORT              IRQ105_Handler      [WEAK]
    EXPORT              IRQ106_Handler      [WEAK]
    EXPORT              IRQ107_Handler      [WEAK]
    EXPORT              IRQ108_Handler      [WEAK]
    EXPORT              IRQ109_Handler      [WEAK]

    EXPORT              IRQ110_Handler      [WEAK]
    EXPORT              IRQ111_Handler      [WEAK]
    EXPORT              IRQ112_Handler      [WEAK]
    EXPORT              IRQ113_Handler      [WEAK]
    EXPORT              IRQ114_Handler      [WEAK]
    EXPORT              IRQ115_Handler      [WEAK]
    EXPORT              IRQ116_Handler      [WEAK]
    EXPORT              IRQ117_Handler      [WEAK]
    EXPORT              IRQ118_Handler      [WEAK]
    EXPORT              IRQ119_Handler      [WEAK]

    EXPORT              IRQ120_Handler      [WEAK]
    EXPORT              IRQ121_Handler      [WEAK]
    EXPORT              IRQ122_Handler      [WEAK]
    EXPORT              IRQ123_Handler      [WEAK]
    EXPORT              IRQ124_Handler      [WEAK]
    EXPORT              IRQ125_Handler      [WEAK]
    EXPORT              IRQ126_Handler      [WEAK]
    EXPORT              IRQ127_Handler      [WEAK]
    EXPORT              IRQ128_Handler      [WEAK]
    EXPORT              IRQ129_Handler      [WEAK]

    EXPORT              IRQ130_Handler      [WEAK]
    EXPORT              IRQ131_Handler      [WEAK]
    EXPORT              IRQ132_Handler      [WEAK]
    EXPORT              IRQ133_Handler      [WEAK]
    EXPORT              IRQ134_Handler      [WEAK]
    EXPORT              IRQ135_Handler      [WEAK]
    EXPORT              IRQ136_Handler      [WEAK]
    EXPORT              IRQ137_Handler      [WEAK]
    EXPORT              IRQ138_Handler      [WEAK]
    EXPORT              IRQ139_Handler      [WEAK]

    EXPORT              IRQ140_Handler      [WEAK]
    EXPORT              IRQ141_Handler      [WEAK]
    EXPORT              IRQ142_Handler      [WEAK]
    EXPORT              IRQ143_Handler      [WEAK]
    EXPORT              IRQ144_Handler      [WEAK]
    EXPORT              IRQ145_Handler      [WEAK]
    EXPORT              IRQ146_Handler      [WEAK]
    EXPORT              IRQ147_Handler      [WEAK]
    EXPORT              IRQ148_Handler      [WEAK]
    EXPORT              IRQ149_Handler      [WEAK]

    EXPORT              IRQ150_Handler      [WEAK]
    EXPORT              IRQ151_Handler      [WEAK]
    EXPORT              IRQ152_Handler      [WEAK]
    EXPORT              IRQ153_Handler      [WEAK]
    EXPORT              IRQ154_Handler      [WEAK]
    EXPORT              IRQ155_Handler      [WEAK]
    EXPORT              IRQ156_Handler      [WEAK]
    EXPORT              IRQ157_Handler      [WEAK]
    EXPORT              IRQ158_Handler      [WEAK]
    EXPORT              IRQ159_Handler      [WEAK]

    EXPORT              IRQ160_Handler      [WEAK]
    EXPORT              IRQ161_Handler      [WEAK]
    EXPORT              IRQ162_Handler      [WEAK]
    EXPORT              IRQ163_Handler      [WEAK]
    EXPORT              IRQ164_Handler      [WEAK]
    EXPORT              IRQ165_Handler      [WEAK]
    EXPORT              IRQ166_Handler      [WEAK]
    EXPORT              IRQ167_Handler      [WEAK]
    EXPORT              IRQ168_Handler      [WEAK]
    EXPORT              IRQ169_Handler      [WEAK]

    EXPORT              IRQ170_Handler      [WEAK]
    EXPORT              IRQ171_Handler      [WEAK]
    EXPORT              IRQ172_Handler      [WEAK]
    EXPORT              IRQ173_Handler      [WEAK]
    EXPORT              IRQ174_Handler      [WEAK]
    EXPORT              IRQ175_Handler      [WEAK]
    EXPORT              IRQ176_Handler      [WEAK]
    EXPORT              IRQ177_Handler      [WEAK]
    EXPORT              IRQ178_Handler      [WEAK]
    EXPORT              IRQ179_Handler      [WEAK]

    EXPORT              IRQ180_Handler      [WEAK]
    EXPORT              IRQ181_Handler      [WEAK]
    EXPORT              IRQ182_Handler      [WEAK]
    EXPORT              IRQ183_Handler      [WEAK]
    EXPORT              IRQ184_Handler      [WEAK]
    EXPORT              IRQ185_Handler      [WEAK]
    EXPORT              IRQ186_Handler      [WEAK]
    EXPORT              IRQ187_Handler      [WEAK]
    EXPORT              IRQ188_Handler      [WEAK]
    EXPORT              IRQ189_Handler      [WEAK]

    EXPORT              IRQ190_Handler      [WEAK]
    EXPORT              IRQ191_Handler      [WEAK]
    EXPORT              IRQ192_Handler      [WEAK]
    EXPORT              IRQ193_Handler      [WEAK]
    EXPORT              IRQ194_Handler      [WEAK]
    EXPORT              IRQ195_Handler      [WEAK]
    EXPORT              IRQ196_Handler      [WEAK]
    EXPORT              IRQ197_Handler      [WEAK]
    EXPORT              IRQ198_Handler      [WEAK]
    EXPORT              IRQ199_Handler      [WEAK]
    
    EXPORT              IRQ200_Handler      [WEAK]
    EXPORT              IRQ201_Handler      [WEAK]
    EXPORT              IRQ202_Handler      [WEAK]
    EXPORT              IRQ203_Handler      [WEAK]
    EXPORT              IRQ204_Handler      [WEAK]
    EXPORT              IRQ205_Handler      [WEAK]
    EXPORT              IRQ206_Handler      [WEAK]
    EXPORT              IRQ207_Handler      [WEAK]
    EXPORT              IRQ208_Handler      [WEAK]
    EXPORT              IRQ209_Handler      [WEAK]

    EXPORT              IRQ210_Handler      [WEAK]
    EXPORT              IRQ211_Handler      [WEAK]
    EXPORT              IRQ212_Handler      [WEAK]
    EXPORT              IRQ213_Handler      [WEAK]
    EXPORT              IRQ214_Handler      [WEAK]
    EXPORT              IRQ215_Handler      [WEAK]
    EXPORT              IRQ216_Handler      [WEAK]
    EXPORT              IRQ217_Handler      [WEAK]
    EXPORT              IRQ218_Handler      [WEAK]
    EXPORT              IRQ219_Handler      [WEAK]

    EXPORT              IRQ220_Handler      [WEAK]
    EXPORT              IRQ221_Handler      [WEAK]
    EXPORT              IRQ222_Handler      [WEAK]
    EXPORT              IRQ223_Handler      [WEAK]
    EXPORT              IRQ224_Handler      [WEAK]
    EXPORT              IRQ225_Handler      [WEAK]
    EXPORT              IRQ226_Handler      [WEAK]
    EXPORT              IRQ227_Handler      [WEAK]
    EXPORT              IRQ228_Handler      [WEAK]
    EXPORT              IRQ229_Handler      [WEAK]

    EXPORT              IRQ230_Handler      [WEAK]
    EXPORT              IRQ231_Handler      [WEAK]
    EXPORT              IRQ232_Handler      [WEAK]
    EXPORT              IRQ233_Handler      [WEAK]
    EXPORT              IRQ234_Handler      [WEAK]
    EXPORT              IRQ235_Handler      [WEAK]
    EXPORT              IRQ236_Handler      [WEAK]
    EXPORT              IRQ237_Handler      [WEAK]
    EXPORT              IRQ238_Handler      [WEAK]
    EXPORT              IRQ239_Handler      [WEAK]
                
IRQ0_Handler
IRQ1_Handler
IRQ2_Handler
IRQ3_Handler
IRQ4_Handler
IRQ5_Handler
IRQ6_Handler
IRQ7_Handler
IRQ8_Handler
IRQ9_Handler

IRQ10_Handler
IRQ11_Handler
IRQ12_Handler
IRQ13_Handler
IRQ14_Handler
IRQ15_Handler
IRQ16_Handler
IRQ17_Handler
IRQ18_Handler
IRQ19_Handler

IRQ20_Handler
IRQ21_Handler
IRQ22_Handler
IRQ23_Handler
IRQ24_Handler
IRQ25_Handler
IRQ26_Handler
IRQ27_Handler
IRQ28_Handler
IRQ29_Handler

IRQ30_Handler
IRQ31_Handler
IRQ32_Handler
IRQ33_Handler
IRQ34_Handler
IRQ35_Handler
IRQ36_Handler
IRQ37_Handler
IRQ38_Handler
IRQ39_Handler

IRQ40_Handler
IRQ41_Handler
IRQ42_Handler
IRQ43_Handler
IRQ44_Handler
IRQ45_Handler
IRQ46_Handler
IRQ47_Handler
IRQ48_Handler
IRQ49_Handler

IRQ50_Handler
IRQ51_Handler
IRQ52_Handler
IRQ53_Handler
IRQ54_Handler
IRQ55_Handler
IRQ56_Handler
IRQ57_Handler
IRQ58_Handler
IRQ59_Handler

IRQ60_Handler
IRQ61_Handler
IRQ62_Handler
IRQ63_Handler
IRQ64_Handler
IRQ65_Handler
IRQ66_Handler
IRQ67_Handler
IRQ68_Handler
IRQ69_Handler
 
IRQ70_Handler
IRQ71_Handler
IRQ72_Handler
IRQ73_Handler
IRQ74_Handler
IRQ75_Handler
IRQ76_Handler
IRQ77_Handler
IRQ78_Handler
IRQ79_Handler

IRQ80_Handler
IRQ81_Handler
IRQ82_Handler
IRQ83_Handler
IRQ84_Handler
IRQ85_Handler
IRQ86_Handler
IRQ87_Handler
IRQ88_Handler
IRQ89_Handler
 
IRQ90_Handler
IRQ91_Handler
IRQ92_Handler
IRQ93_Handler
IRQ94_Handler
IRQ95_Handler
IRQ96_Handler
IRQ97_Handler
IRQ98_Handler
IRQ99_Handler

IRQ100_Handler
IRQ101_Handler
IRQ102_Handler
IRQ103_Handler
IRQ104_Handler
IRQ105_Handler
IRQ106_Handler
IRQ107_Handler
IRQ108_Handler
IRQ109_Handler

IRQ110_Handler
IRQ111_Handler
IRQ112_Handler
IRQ113_Handler
IRQ114_Handler
IRQ115_Handler
IRQ116_Handler
IRQ117_Handler
IRQ118_Handler
IRQ119_Handler

IRQ120_Handler
IRQ121_Handler
IRQ122_Handler
IRQ123_Handler
IRQ124_Handler
IRQ125_Handler
IRQ126_Handler
IRQ127_Handler
IRQ128_Handler
IRQ129_Handler

IRQ130_Handler
IRQ131_Handler
IRQ132_Handler
IRQ133_Handler
IRQ134_Handler
IRQ135_Handler
IRQ136_Handler
IRQ137_Handler
IRQ138_Handler
IRQ139_Handler

IRQ140_Handler
IRQ141_Handler
IRQ142_Handler
IRQ143_Handler
IRQ144_Handler
IRQ145_Handler
IRQ146_Handler
IRQ147_Handler
IRQ148_Handler
IRQ149_Handler

IRQ150_Handler
IRQ151_Handler
IRQ152_Handler
IRQ153_Handler
IRQ154_Handler
IRQ155_Handler
IRQ156_Handler
IRQ157_Handler
IRQ158_Handler
IRQ159_Handler

IRQ160_Handler
IRQ161_Handler
IRQ162_Handler
IRQ163_Handler
IRQ164_Handler
IRQ165_Handler
IRQ166_Handler
IRQ167_Handler
IRQ168_Handler
IRQ169_Handler
 
IRQ170_Handler
IRQ171_Handler
IRQ172_Handler
IRQ173_Handler
IRQ174_Handler
IRQ175_Handler
IRQ176_Handler
IRQ177_Handler
IRQ178_Handler
IRQ179_Handler

IRQ180_Handler
IRQ181_Handler
IRQ182_Handler
IRQ183_Handler
IRQ184_Handler
IRQ185_Handler
IRQ186_Handler
IRQ187_Handler
IRQ188_Handler
IRQ189_Handler
 
IRQ190_Handler
IRQ191_Handler
IRQ192_Handler
IRQ193_Handler
IRQ194_Handler
IRQ195_Handler
IRQ196_Handler
IRQ197_Handler
IRQ198_Handler
IRQ199_Handler
                
IRQ200_Handler
IRQ201_Handler
IRQ202_Handler
IRQ203_Handler
IRQ204_Handler
IRQ205_Handler
IRQ206_Handler
IRQ207_Handler
IRQ208_Handler
IRQ209_Handler

IRQ210_Handler
IRQ211_Handler
IRQ212_Handler
IRQ213_Handler
IRQ214_Handler
IRQ215_Handler
IRQ216_Handler
IRQ217_Handler
IRQ218_Handler
IRQ219_Handler

IRQ220_Handler
IRQ221_Handler
IRQ222_Handler
IRQ223_Handler
IRQ224_Handler
IRQ225_Handler
IRQ226_Handler
IRQ227_Handler
IRQ228_Handler
IRQ229_Handler

IRQ230_Handler
IRQ231_Handler
IRQ232_Handler
IRQ233_Handler
IRQ234_Handler
IRQ235_Handler
IRQ236_Handler
IRQ237_Handler
IRQ238_Handler
IRQ239_Handler
    RME_A7M_SAVE
    
    MOV                 R0,SP               ;Pass in the regs
    MRS                 R1,xPSR             ;Pass in the interrupt number
    UBFX                R1,R1,#0,#9
    SUB                 R1,#16              ;The IRQ0's number is 16; subtract
    BL                  __RME_A7M_Vct_Handler

    RME_A7M_LOAD
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Vector ***************************************************************/

;/* Function:__RME_Int_Disable ************************************************
;Description : The function for disabling all interrupts.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_INT_DISABLE,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_Int_Disable       PROC
    CPSID               I 
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_Int_Disable *******************************************/

;/* Function:__RME_Int_Enable *************************************************
;Description : The function for enabling all interrupts.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_INT_ENABLE,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_Int_Enable        PROC
    CPSIE               I 
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_Int_Enable ********************************************/

;/* Function:__RME_A7M_Barrier ************************************************
;Description : A full data/instruction barrier.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A7M_BARRIER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A7M_Barrier       PROC
    DSB                 SY
    ISB                 SY
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A7M_Barrier *******************************************/

;/* Function:__RME_A7M_Reset **************************************************
;Description : A full system reset.
;Input       : None.
;Output      : None.    
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A7M_RESET,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A7M_Reset         PROC
    ;Disable all interrupts
    CPSID               I
    ;ARMv7-M Standard system reset
    LDR                 R0,=0xE000ED0C
    LDR                 R1,=0x05FA0004
    STR                 R1,[R0]
    ISB
    ;Deadloop
    B                   .
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A7M_Reset *********************************************/

;/* Function:__RME_A7M_Wait_Int ***********************************************
;Description : Wait until a new interrupt comes, to save power.
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_A7M_WAIT_INT,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A7M_Wait_Int      PROC
    WFE
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A7M_Wait_Int ******************************************/

;/* Function:__RME_A7M_MSB_Get ************************************************
;Description : Get the MSB of the word.
;Input       : ptr_t Val - The value.
;Output      : None.
;Return      : ptr_t - The MSB position.
;*****************************************************************************/
    AREA                __RME_A7M_MSB_GET,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_A7M_MSB_Get       PROC
    CLZ                 R1,R0
    MOV                 R0,#31
    SUB                 R0,R1
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_A7M_MSB_Get *******************************************/

;/* Function:__RME_User_Enter *************************************************
;Description : Entering of the user mode, after the system finish its preliminary
;              booting. The function shall never return. This function should only
;              be used to boot the first process in the system.
;Input       : ptr_t Entry - The user execution startpoint.
;              ptr_t Stack - The user stack.
;              ptr_t CPUID - The CPUID.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                __RME_USER_ENTER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

__RME_User_Enter        PROC
    MSR                 PSP,R1              ;Set the stack pointer
    MOV                 R4,#0x03            ;Unprevileged thread mode
    MSR                 CONTROL,R4
    ISB
    MOV                 R1,R0               ;Save the entry to R1
    MOV                 R0,R2               ;Save CPUID(0) to R0
    BLX                 R1                  ;Branch to target
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:__RME_User_Enter ********************************************/

;/* Function:SysTick_Handler **************************************************
;Description : The System Tick Timer handler routine. This will in fact call a
;              C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SYSTICK_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

SysTick_Handler         PROC
    RME_A7M_SAVE
    
    MOV                 R0,SP               ;Pass in the regs
    BL                  __RME_A7M_Tim_Handler
    
    RME_A7M_LOAD
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:SysTick_Handler *********************************************/

;/* Function:SVC_Handler ******************************************************
;Description : The SVC handler routine. This will in fact call a C function to
;              resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SVC_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

SVC_Handler             PROC
    RME_A7M_SAVE
    
    MOV                 R0, SP              ;Pass in the regs
    BL                  __RME_A7M_Svc_Handler
    
    RME_A7M_LOAD
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:SVC_Handler *************************************************/

;/* Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler **************
;Description : The multi-purpose handler routine. This will in fact call
;              a C function to resolve the system service routines.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                SYSTEM_HANDLER,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

NMI_Handler             PROC
    NOP
PendSV_Handler
    NOP
DebugMon_Handler
    NOP
HardFault_Handler
    NOP
MemManage_Handler
    NOP
BusFault_Handler
    NOP
UsageFault_Handler
    RME_A7M_SAVE
    
    MOV                 R0,SP               ;Pass in the regs
    BL                  __RME_A7M_Exc_Handler

    RME_A7M_LOAD
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler *********/

;/* Function:___RME_A7M_Thd_Cop_Clear *****************************************
;Description : Clean up the coprocessor state so that the FP information is not
;              leaked when switching from a FPU-enabled thread to a FPU-disabled
;              thread.             
;Input       : None.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                ___RME_A7M_THD_COP_CLEAR,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_Thd_Cop_Clear PROC               
    ;Use DCI to avoid compilation errors when FPU not enabled
    LDR                 R0,=COP_CLEAR
    DCI                 0xEC90              ;VLDMIA    R0,{S0-S31}
    DCI                 0x0A20              ;Clear all the FPU registers
    MOV                 R0,#0               ;Clear FPSCR as well
    DCI                 0xEEE1              ;VMSR      FPSCR,R0
    DCI                 0x0A10
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
COP_CLEAR
    SPACE               32*4
;/* End Function:___RME_A7M_Thd_Cop_Clear ************************************/

;/* Function:___RME_A7M_Thd_Cop_Save ******************************************
;Description : Save the coprocessor context on switch.         
;Input       : R0 - The pointer to the coprocessor struct.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                ___RME_A7M_THD_COP_SAVE,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_Thd_Cop_Save PROC
    ;Use DCI to avoid compilation errors when FPU not enabled
    DCI                 0xEC80              ;VSTMIA    R0,{S16-S31}
    DCI                 0x8A10              ;Save all the FPU registers
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:___RME_A7M_Thd_Cop_Save *************************************/

;/* Function:___RME_A7M_Thd_Cop_Load ******************************************
;Description : Restore the coprocessor context on switch.
;Input       : R0 - The pointer to the coprocessor struct.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    AREA                ___RME_A7M_THD_COP_LOAD,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_Thd_Cop_Load PROC
    ;Use DCI to avoid compilation errors when FPU not enabled
    DCI                 0xEC90              ;VLDMIA    R0,{S16-S31}
    DCI                 0x8A10              ;Restore all the FPU registers
    BX                  LR
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:___RME_A7M_Thd_Cop_Load *************************************/

;/* Function:___RME_A7M_MPU_Set ***********************************************
;Description : Set the MPU context. 1-to-16-region versions are all declared here.
;              Note that the ARMv7-M may support more than 16 regions. But we only
;              support 16 regions in our port.
;Input       : R0 - The pointer to the MPU content.
;Output      : None.
;Return      : None.
;*****************************************************************************/
    MACRO
    MPU_PRE
    PUSH                {R4-R9}             ;Save registers
    LDR                 R1,=0xE000ED9C      ;The base address of MPU RBAR and all 4 registers
    MEND
    
    MACRO
    MPU_SET
    LDMIA               R0!,{R2-R3}         ;Read settings
    STMIA               R1,{R2-R3}          ;Program
    MEND
    
    MACRO
    MPU_SET2
    LDMIA               R0!,{R2-R5}
    STMIA               R1,{R2-R5} 
    MEND
    
    MACRO
    MPU_SET3
    LDMIA               R0!,{R2-R7}
    STMIA               R1,{R2-R7}
    MEND
    
    MACRO
    MPU_SET4
    LDMIA               R0!,{R2-R9}
    STMIA               R1,{R2-R9}
    MEND
    
    MACRO
    MPU_POST
    POP                 {R4-R9}             ;Restore registers
    ISB                                     ;Barrier
    BX                  LR
    MEND

;1-region version
    AREA                ___RME_A7M_MPU_SET1,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set1     PROC
    MPU_PRE
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;2-region version
    AREA                ___RME_A7M_MPU_SET2,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set2     PROC
    MPU_PRE
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;3-region version
    AREA                ___RME_A7M_MPU_SET3,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set3     PROC
    MPU_PRE
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;4-region version
    AREA                ___RME_A7M_MPU_SET4,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set4     PROC
    MPU_PRE
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;5-region version
    AREA                ___RME_A7M_MPU_SET5,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set5     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;6-region version
    AREA                ___RME_A7M_MPU_SET6,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set6     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;7-region version
    AREA                ___RME_A7M_MPU_SET7,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set7     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;8-region version
    AREA                ___RME_A7M_MPU_SET8,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set8     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;9-region version
    AREA                ___RME_A7M_MPU_SET9,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set9     PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;10-region version
    AREA                ___RME_A7M_MPU_SET10,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set10    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;11-region version
    AREA                ___RME_A7M_MPU_SET11,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set11    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;12-region version
    AREA                ___RME_A7M_MPU_SET12,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set12    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;13-region version
    AREA                ___RME_A7M_MPU_SET13,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set13    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;14-region version
    AREA                ___RME_A7M_MPU_SET14,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set14    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET2
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;15-region version
    AREA                ___RME_A7M_MPU_SET15,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set15    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET3
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN

;16-region version
    AREA                ___RME_A7M_MPU_SET16,CODE,READONLY,ALIGN=3
    THUMB
    REQUIRE8
    PRESERVE8

___RME_A7M_MPU_Set16    PROC
    MPU_PRE
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_SET4
    MPU_POST
    ENDP
    ALIGN
    LTORG
    ALIGN
;/* End Function:___RME_A7M_MPU_Set ******************************************/
    END
;/* End Of File **************************************************************/

;/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
