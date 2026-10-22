;/*****************************************************************************
;Filename    : rme_platform_a7m_iccarm.s
;Author      : pry ; Cline (ported from the armcc variant)
;Date        : 22/10/2026
;Description : The ARMv7-M assembly support of the RME RTOS.
;              This is the IAR (iasmarm) port.
;*****************************************************************************/

;/* Import *******************************************************************/
        ;Preinitialization routine
        EXTERN              __RME_A7M_Lowlvl_Preinit
        ;The IAR C library initialization
        EXTERN              __cmain
        ;The system call handler
        EXTERN              __RME_A7M_Svc_Handler
        ;The system tick handler
        EXTERN              __RME_A7M_Tim_Handler
        ;The memory management fault handler
        EXTERN              __RME_A7M_Exc_Handler
        ;The generic interrupt handler for all other vectors
        EXTERN              __RME_A7M_Vct_Handler
;/* End Import ***************************************************************/

;/* Export *******************************************************************/
        ;Override IAR default startup entry
        PUBLIC              __iar_program_start
        ;Disable all interrupts
        PUBLIC              __RME_Int_Disable
        ;Enable all interrupts
        PUBLIC              __RME_Int_Enable
        ;A full barrier
        PUBLIC              __RME_A7M_Barrier
        ;Full system reset
        PUBLIC              __RME_A7M_Reset
        ;Wait until interrupts happen
        PUBLIC              __RME_A7M_Wait_Int
        ;Get the MSB in a word
        PUBLIC              __RME_A7M_MSB_Get
        ;Entering of the user mode
        PUBLIC              __RME_User_Enter
        ;The reset handler
        PUBLIC              Reset_Handler
        ;The system handlers(the vector table references them)
        PUBLIC              NMI_Handler
        PUBLIC              HardFault_Handler
        PUBLIC              MemManage_Handler
        PUBLIC              BusFault_Handler
        PUBLIC              UsageFault_Handler
        PUBLIC              DebugMon_Handler
        PUBLIC              PendSV_Handler
        PUBLIC              SysTick_Handler
        PUBLIC              SVC_Handler
        ;Clear FPU register contents
        PUBLIC              ___RME_A7M_Thd_Cop_Clear
        ;The FPU register save routine
        PUBLIC              ___RME_A7M_Thd_Cop_Save
        ;The FPU register restore routine
        PUBLIC              ___RME_A7M_Thd_Cop_Load
        PUBLIC              ___RME_A7M_MPU_Set1
        PUBLIC              ___RME_A7M_MPU_Set2
        PUBLIC              ___RME_A7M_MPU_Set3
        PUBLIC              ___RME_A7M_MPU_Set4
        PUBLIC              ___RME_A7M_MPU_Set5
        PUBLIC              ___RME_A7M_MPU_Set6
        PUBLIC              ___RME_A7M_MPU_Set7
        PUBLIC              ___RME_A7M_MPU_Set8
        PUBLIC              ___RME_A7M_MPU_Set9
        PUBLIC              ___RME_A7M_MPU_Set10
        PUBLIC              ___RME_A7M_MPU_Set11
        PUBLIC              ___RME_A7M_MPU_Set12
        PUBLIC              ___RME_A7M_MPU_Set13
        PUBLIC              ___RME_A7M_MPU_Set14
        PUBLIC              ___RME_A7M_MPU_Set15
        PUBLIC              ___RME_A7M_MPU_Set16
;/* End Export ***************************************************************/

;/* Entry ********************************************************************/
        MODULE              RME_A7M
        SECTION             .rme_entry:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

__RME_Entry
Reset_Handler
        LDR                 R0,=__RME_A7M_Lowlvl_Preinit
        BLX                 R0
        LDR                 R0,=__iar_program_start
        BX                  R0

__iar_program_start
;       BL                  __iar_init_vfp      ; Override, don't initialize FPU
        BL                  __cmain
        LTORG
;/* End Entry ****************************************************************/

;/* Save/Restore macros ******************************************************/
        ;Save registers
RME_A7M_SAVE MACRO
        PUSH                {R4-R11,LR}         ;Save registers
        MRS                 R0,PSP
        PUSH                {R0}
        LDR                 R4,=0xE000ED94      ;Turn off MPU in case it sets wrong permission for kernel
        LDR                 R5,=0x00000000
        STR                 R5,[R4]
        ENDM

        ;Restore registers
RME_A7M_LOAD MACRO
        LDR                 R4,=0xE000ED94      ;Turn on MPU
        LDR                 R5,=0x00000005
        STR                 R5,[R4]
        POP                 {R0}
        MSR                 PSP,R0
        POP                 {R4-R11,PC}         ;Restore registers
        ENDM
;/* End Save/Restore macros **************************************************/

;/* Vector *******************************************************************/
        ;Declare the CSTACK segment so that sfe(CSTACK) resolves - the block
        ;itself is defined by the .icf, this empty section gets discarded
        SECTION             CSTACK:DATA:NOROOT(3)
        SECTION             .rme_vector:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

        ;Vector table - the .icf places this section at the ROM start
        DC32                sfe(CSTACK)
        DC32                Reset_Handler       ;Reset Handler
        DC32                NMI_Handler         ;NMI Handler
        DC32                HardFault_Handler   ;Hard Fault Handler
        DC32                MemManage_Handler   ;MPU Fault Handler
        DC32                BusFault_Handler    ;Bus Fault Handler
        DC32                UsageFault_Handler  ;Usage Fault Handler
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                0                   ;Reserved
        DC32                SVC_Handler         ;SVCall Handler
        DC32                DebugMon_Handler    ;Debug Monitor Handler
        DC32                0                   ;Reserved
        DC32                PendSV_Handler      ;PendSV Handler
        DC32                SysTick_Handler     ;SysTick Handler
        DC32                IRQ0_Handler        ;240 External Interrupts
        DC32                IRQ1_Handler
        DC32                IRQ2_Handler
        DC32                IRQ3_Handler
        DC32                IRQ4_Handler
        DC32                IRQ5_Handler
        DC32                IRQ6_Handler
        DC32                IRQ7_Handler
        DC32                IRQ8_Handler
        DC32                IRQ9_Handler
        DC32                IRQ10_Handler        ;240 External Interrupts
        DC32                IRQ11_Handler
        DC32                IRQ12_Handler
        DC32                IRQ13_Handler
        DC32                IRQ14_Handler
        DC32                IRQ15_Handler
        DC32                IRQ16_Handler
        DC32                IRQ17_Handler
        DC32                IRQ18_Handler
        DC32                IRQ19_Handler
        DC32                IRQ20_Handler        ;240 External Interrupts
        DC32                IRQ21_Handler
        DC32                IRQ22_Handler
        DC32                IRQ23_Handler
        DC32                IRQ24_Handler
        DC32                IRQ25_Handler
        DC32                IRQ26_Handler
        DC32                IRQ27_Handler
        DC32                IRQ28_Handler
        DC32                IRQ29_Handler
        DC32                IRQ30_Handler        ;240 External Interrupts
        DC32                IRQ31_Handler
        DC32                IRQ32_Handler
        DC32                IRQ33_Handler
        DC32                IRQ34_Handler
        DC32                IRQ35_Handler
        DC32                IRQ36_Handler
        DC32                IRQ37_Handler
        DC32                IRQ38_Handler
        DC32                IRQ39_Handler
        DC32                IRQ40_Handler        ;240 External Interrupts
        DC32                IRQ41_Handler
        DC32                IRQ42_Handler
        DC32                IRQ43_Handler
        DC32                IRQ44_Handler
        DC32                IRQ45_Handler
        DC32                IRQ46_Handler
        DC32                IRQ47_Handler
        DC32                IRQ48_Handler
        DC32                IRQ49_Handler
        DC32                IRQ50_Handler        ;240 External Interrupts
        DC32                IRQ51_Handler
        DC32                IRQ52_Handler
        DC32                IRQ53_Handler
        DC32                IRQ54_Handler
        DC32                IRQ55_Handler
        DC32                IRQ56_Handler
        DC32                IRQ57_Handler
        DC32                IRQ58_Handler
        DC32                IRQ59_Handler
        DC32                IRQ60_Handler        ;240 External Interrupts
        DC32                IRQ61_Handler
        DC32                IRQ62_Handler
        DC32                IRQ63_Handler
        DC32                IRQ64_Handler
        DC32                IRQ65_Handler
        DC32                IRQ66_Handler
        DC32                IRQ67_Handler
        DC32                IRQ68_Handler
        DC32                IRQ69_Handler
        DC32                IRQ70_Handler        ;240 External Interrupts
        DC32                IRQ71_Handler
        DC32                IRQ72_Handler
        DC32                IRQ73_Handler
        DC32                IRQ74_Handler
        DC32                IRQ75_Handler
        DC32                IRQ76_Handler
        DC32                IRQ77_Handler
        DC32                IRQ78_Handler
        DC32                IRQ79_Handler
        DC32                IRQ80_Handler        ;240 External Interrupts
        DC32                IRQ81_Handler
        DC32                IRQ82_Handler
        DC32                IRQ83_Handler
        DC32                IRQ84_Handler
        DC32                IRQ85_Handler
        DC32                IRQ86_Handler
        DC32                IRQ87_Handler
        DC32                IRQ88_Handler
        DC32                IRQ89_Handler
        DC32                IRQ90_Handler        ;240 External Interrupts
        DC32                IRQ91_Handler
        DC32                IRQ92_Handler
        DC32                IRQ93_Handler
        DC32                IRQ94_Handler
        DC32                IRQ95_Handler
        DC32                IRQ96_Handler
        DC32                IRQ97_Handler
        DC32                IRQ98_Handler
        DC32                IRQ99_Handler
        DC32                IRQ100_Handler        ;240 External Interrupts
        DC32                IRQ101_Handler
        DC32                IRQ102_Handler
        DC32                IRQ103_Handler
        DC32                IRQ104_Handler
        DC32                IRQ105_Handler
        DC32                IRQ106_Handler
        DC32                IRQ107_Handler
        DC32                IRQ108_Handler
        DC32                IRQ109_Handler
        DC32                IRQ110_Handler        ;240 External Interrupts
        DC32                IRQ111_Handler
        DC32                IRQ112_Handler
        DC32                IRQ113_Handler
        DC32                IRQ114_Handler
        DC32                IRQ115_Handler
        DC32                IRQ116_Handler
        DC32                IRQ117_Handler
        DC32                IRQ118_Handler
        DC32                IRQ119_Handler
        DC32                IRQ120_Handler        ;240 External Interrupts
        DC32                IRQ121_Handler
        DC32                IRQ122_Handler
        DC32                IRQ123_Handler
        DC32                IRQ124_Handler
        DC32                IRQ125_Handler
        DC32                IRQ126_Handler
        DC32                IRQ127_Handler
        DC32                IRQ128_Handler
        DC32                IRQ129_Handler
        DC32                IRQ130_Handler        ;240 External Interrupts
        DC32                IRQ131_Handler
        DC32                IRQ132_Handler
        DC32                IRQ133_Handler
        DC32                IRQ134_Handler
        DC32                IRQ135_Handler
        DC32                IRQ136_Handler
        DC32                IRQ137_Handler
        DC32                IRQ138_Handler
        DC32                IRQ139_Handler
        DC32                IRQ140_Handler        ;240 External Interrupts
        DC32                IRQ141_Handler
        DC32                IRQ142_Handler
        DC32                IRQ143_Handler
        DC32                IRQ144_Handler
        DC32                IRQ145_Handler
        DC32                IRQ146_Handler
        DC32                IRQ147_Handler
        DC32                IRQ148_Handler
        DC32                IRQ149_Handler
        DC32                IRQ150_Handler        ;240 External Interrupts
        DC32                IRQ151_Handler
        DC32                IRQ152_Handler
        DC32                IRQ153_Handler
        DC32                IRQ154_Handler
        DC32                IRQ155_Handler
        DC32                IRQ156_Handler
        DC32                IRQ157_Handler
        DC32                IRQ158_Handler
        DC32                IRQ159_Handler
        DC32                IRQ160_Handler        ;240 External Interrupts
        DC32                IRQ161_Handler
        DC32                IRQ162_Handler
        DC32                IRQ163_Handler
        DC32                IRQ164_Handler
        DC32                IRQ165_Handler
        DC32                IRQ166_Handler
        DC32                IRQ167_Handler
        DC32                IRQ168_Handler
        DC32                IRQ169_Handler
        DC32                IRQ170_Handler        ;240 External Interrupts
        DC32                IRQ171_Handler
        DC32                IRQ172_Handler
        DC32                IRQ173_Handler
        DC32                IRQ174_Handler
        DC32                IRQ175_Handler
        DC32                IRQ176_Handler
        DC32                IRQ177_Handler
        DC32                IRQ178_Handler
        DC32                IRQ179_Handler
        DC32                IRQ180_Handler        ;240 External Interrupts
        DC32                IRQ181_Handler
        DC32                IRQ182_Handler
        DC32                IRQ183_Handler
        DC32                IRQ184_Handler
        DC32                IRQ185_Handler
        DC32                IRQ186_Handler
        DC32                IRQ187_Handler
        DC32                IRQ188_Handler
        DC32                IRQ189_Handler
        DC32                IRQ190_Handler        ;240 External Interrupts
        DC32                IRQ191_Handler
        DC32                IRQ192_Handler
        DC32                IRQ193_Handler
        DC32                IRQ194_Handler
        DC32                IRQ195_Handler
        DC32                IRQ196_Handler
        DC32                IRQ197_Handler
        DC32                IRQ198_Handler
        DC32                IRQ199_Handler
        DC32                IRQ200_Handler        ;240 External Interrupts
        DC32                IRQ201_Handler
        DC32                IRQ202_Handler
        DC32                IRQ203_Handler
        DC32                IRQ204_Handler
        DC32                IRQ205_Handler
        DC32                IRQ206_Handler
        DC32                IRQ207_Handler
        DC32                IRQ208_Handler
        DC32                IRQ209_Handler
        DC32                IRQ210_Handler        ;240 External Interrupts
        DC32                IRQ211_Handler
        DC32                IRQ212_Handler
        DC32                IRQ213_Handler
        DC32                IRQ214_Handler
        DC32                IRQ215_Handler
        DC32                IRQ216_Handler
        DC32                IRQ217_Handler
        DC32                IRQ218_Handler
        DC32                IRQ219_Handler
        DC32                IRQ220_Handler        ;240 External Interrupts
        DC32                IRQ221_Handler
        DC32                IRQ222_Handler
        DC32                IRQ223_Handler
        DC32                IRQ224_Handler
        DC32                IRQ225_Handler
        DC32                IRQ226_Handler
        DC32                IRQ227_Handler
        DC32                IRQ228_Handler
        DC32                IRQ229_Handler
        DC32                IRQ230_Handler        ;240 External Interrupts
        DC32                IRQ231_Handler
        DC32                IRQ232_Handler
        DC32                IRQ233_Handler
        DC32                IRQ234_Handler
        DC32                IRQ235_Handler
        DC32                IRQ236_Handler
        DC32                IRQ237_Handler
        DC32                IRQ238_Handler
        DC32                IRQ239_Handler
;/* Weak default handlers ****************************************************/        PUBWEAK             IRQ0_Handler
        PUBWEAK             IRQ1_Handler
        PUBWEAK             IRQ2_Handler
        PUBWEAK             IRQ3_Handler
        PUBWEAK             IRQ4_Handler
        PUBWEAK             IRQ5_Handler
        PUBWEAK             IRQ6_Handler
        PUBWEAK             IRQ7_Handler
        PUBWEAK             IRQ8_Handler
        PUBWEAK             IRQ9_Handler
        PUBWEAK             IRQ10_Handler
        PUBWEAK             IRQ11_Handler
        PUBWEAK             IRQ12_Handler
        PUBWEAK             IRQ13_Handler
        PUBWEAK             IRQ14_Handler
        PUBWEAK             IRQ15_Handler
        PUBWEAK             IRQ16_Handler
        PUBWEAK             IRQ17_Handler
        PUBWEAK             IRQ18_Handler
        PUBWEAK             IRQ19_Handler
        PUBWEAK             IRQ20_Handler
        PUBWEAK             IRQ21_Handler
        PUBWEAK             IRQ22_Handler
        PUBWEAK             IRQ23_Handler
        PUBWEAK             IRQ24_Handler
        PUBWEAK             IRQ25_Handler
        PUBWEAK             IRQ26_Handler
        PUBWEAK             IRQ27_Handler
        PUBWEAK             IRQ28_Handler
        PUBWEAK             IRQ29_Handler
        PUBWEAK             IRQ30_Handler
        PUBWEAK             IRQ31_Handler
        PUBWEAK             IRQ32_Handler
        PUBWEAK             IRQ33_Handler
        PUBWEAK             IRQ34_Handler
        PUBWEAK             IRQ35_Handler
        PUBWEAK             IRQ36_Handler
        PUBWEAK             IRQ37_Handler
        PUBWEAK             IRQ38_Handler
        PUBWEAK             IRQ39_Handler
        PUBWEAK             IRQ40_Handler
        PUBWEAK             IRQ41_Handler
        PUBWEAK             IRQ42_Handler
        PUBWEAK             IRQ43_Handler
        PUBWEAK             IRQ44_Handler
        PUBWEAK             IRQ45_Handler
        PUBWEAK             IRQ46_Handler
        PUBWEAK             IRQ47_Handler
        PUBWEAK             IRQ48_Handler
        PUBWEAK             IRQ49_Handler
        PUBWEAK             IRQ50_Handler
        PUBWEAK             IRQ51_Handler
        PUBWEAK             IRQ52_Handler
        PUBWEAK             IRQ53_Handler
        PUBWEAK             IRQ54_Handler
        PUBWEAK             IRQ55_Handler
        PUBWEAK             IRQ56_Handler
        PUBWEAK             IRQ57_Handler
        PUBWEAK             IRQ58_Handler
        PUBWEAK             IRQ59_Handler
        PUBWEAK             IRQ60_Handler
        PUBWEAK             IRQ61_Handler
        PUBWEAK             IRQ62_Handler
        PUBWEAK             IRQ63_Handler
        PUBWEAK             IRQ64_Handler
        PUBWEAK             IRQ65_Handler
        PUBWEAK             IRQ66_Handler
        PUBWEAK             IRQ67_Handler
        PUBWEAK             IRQ68_Handler
        PUBWEAK             IRQ69_Handler
        PUBWEAK             IRQ70_Handler
        PUBWEAK             IRQ71_Handler
        PUBWEAK             IRQ72_Handler
        PUBWEAK             IRQ73_Handler
        PUBWEAK             IRQ74_Handler
        PUBWEAK             IRQ75_Handler
        PUBWEAK             IRQ76_Handler
        PUBWEAK             IRQ77_Handler
        PUBWEAK             IRQ78_Handler
        PUBWEAK             IRQ79_Handler
        PUBWEAK             IRQ80_Handler
        PUBWEAK             IRQ81_Handler
        PUBWEAK             IRQ82_Handler
        PUBWEAK             IRQ83_Handler
        PUBWEAK             IRQ84_Handler
        PUBWEAK             IRQ85_Handler
        PUBWEAK             IRQ86_Handler
        PUBWEAK             IRQ87_Handler
        PUBWEAK             IRQ88_Handler
        PUBWEAK             IRQ89_Handler
        PUBWEAK             IRQ90_Handler
        PUBWEAK             IRQ91_Handler
        PUBWEAK             IRQ92_Handler
        PUBWEAK             IRQ93_Handler
        PUBWEAK             IRQ94_Handler
        PUBWEAK             IRQ95_Handler
        PUBWEAK             IRQ96_Handler
        PUBWEAK             IRQ97_Handler
        PUBWEAK             IRQ98_Handler
        PUBWEAK             IRQ99_Handler
        PUBWEAK             IRQ100_Handler
        PUBWEAK             IRQ101_Handler
        PUBWEAK             IRQ102_Handler
        PUBWEAK             IRQ103_Handler
        PUBWEAK             IRQ104_Handler
        PUBWEAK             IRQ105_Handler
        PUBWEAK             IRQ106_Handler
        PUBWEAK             IRQ107_Handler
        PUBWEAK             IRQ108_Handler
        PUBWEAK             IRQ109_Handler
        PUBWEAK             IRQ110_Handler
        PUBWEAK             IRQ111_Handler
        PUBWEAK             IRQ112_Handler
        PUBWEAK             IRQ113_Handler
        PUBWEAK             IRQ114_Handler
        PUBWEAK             IRQ115_Handler
        PUBWEAK             IRQ116_Handler
        PUBWEAK             IRQ117_Handler
        PUBWEAK             IRQ118_Handler
        PUBWEAK             IRQ119_Handler
        PUBWEAK             IRQ120_Handler
        PUBWEAK             IRQ121_Handler
        PUBWEAK             IRQ122_Handler
        PUBWEAK             IRQ123_Handler
        PUBWEAK             IRQ124_Handler
        PUBWEAK             IRQ125_Handler
        PUBWEAK             IRQ126_Handler
        PUBWEAK             IRQ127_Handler
        PUBWEAK             IRQ128_Handler
        PUBWEAK             IRQ129_Handler
        PUBWEAK             IRQ130_Handler
        PUBWEAK             IRQ131_Handler
        PUBWEAK             IRQ132_Handler
        PUBWEAK             IRQ133_Handler
        PUBWEAK             IRQ134_Handler
        PUBWEAK             IRQ135_Handler
        PUBWEAK             IRQ136_Handler
        PUBWEAK             IRQ137_Handler
        PUBWEAK             IRQ138_Handler
        PUBWEAK             IRQ139_Handler
        PUBWEAK             IRQ140_Handler
        PUBWEAK             IRQ141_Handler
        PUBWEAK             IRQ142_Handler
        PUBWEAK             IRQ143_Handler
        PUBWEAK             IRQ144_Handler
        PUBWEAK             IRQ145_Handler
        PUBWEAK             IRQ146_Handler
        PUBWEAK             IRQ147_Handler
        PUBWEAK             IRQ148_Handler
        PUBWEAK             IRQ149_Handler
        PUBWEAK             IRQ150_Handler
        PUBWEAK             IRQ151_Handler
        PUBWEAK             IRQ152_Handler
        PUBWEAK             IRQ153_Handler
        PUBWEAK             IRQ154_Handler
        PUBWEAK             IRQ155_Handler
        PUBWEAK             IRQ156_Handler
        PUBWEAK             IRQ157_Handler
        PUBWEAK             IRQ158_Handler
        PUBWEAK             IRQ159_Handler
        PUBWEAK             IRQ160_Handler
        PUBWEAK             IRQ161_Handler
        PUBWEAK             IRQ162_Handler
        PUBWEAK             IRQ163_Handler
        PUBWEAK             IRQ164_Handler
        PUBWEAK             IRQ165_Handler
        PUBWEAK             IRQ166_Handler
        PUBWEAK             IRQ167_Handler
        PUBWEAK             IRQ168_Handler
        PUBWEAK             IRQ169_Handler
        PUBWEAK             IRQ170_Handler
        PUBWEAK             IRQ171_Handler
        PUBWEAK             IRQ172_Handler
        PUBWEAK             IRQ173_Handler
        PUBWEAK             IRQ174_Handler
        PUBWEAK             IRQ175_Handler
        PUBWEAK             IRQ176_Handler
        PUBWEAK             IRQ177_Handler
        PUBWEAK             IRQ178_Handler
        PUBWEAK             IRQ179_Handler
        PUBWEAK             IRQ180_Handler
        PUBWEAK             IRQ181_Handler
        PUBWEAK             IRQ182_Handler
        PUBWEAK             IRQ183_Handler
        PUBWEAK             IRQ184_Handler
        PUBWEAK             IRQ185_Handler
        PUBWEAK             IRQ186_Handler
        PUBWEAK             IRQ187_Handler
        PUBWEAK             IRQ188_Handler
        PUBWEAK             IRQ189_Handler
        PUBWEAK             IRQ190_Handler
        PUBWEAK             IRQ191_Handler
        PUBWEAK             IRQ192_Handler
        PUBWEAK             IRQ193_Handler
        PUBWEAK             IRQ194_Handler
        PUBWEAK             IRQ195_Handler
        PUBWEAK             IRQ196_Handler
        PUBWEAK             IRQ197_Handler
        PUBWEAK             IRQ198_Handler
        PUBWEAK             IRQ199_Handler
        PUBWEAK             IRQ200_Handler
        PUBWEAK             IRQ201_Handler
        PUBWEAK             IRQ202_Handler
        PUBWEAK             IRQ203_Handler
        PUBWEAK             IRQ204_Handler
        PUBWEAK             IRQ205_Handler
        PUBWEAK             IRQ206_Handler
        PUBWEAK             IRQ207_Handler
        PUBWEAK             IRQ208_Handler
        PUBWEAK             IRQ209_Handler
        PUBWEAK             IRQ210_Handler
        PUBWEAK             IRQ211_Handler
        PUBWEAK             IRQ212_Handler
        PUBWEAK             IRQ213_Handler
        PUBWEAK             IRQ214_Handler
        PUBWEAK             IRQ215_Handler
        PUBWEAK             IRQ216_Handler
        PUBWEAK             IRQ217_Handler
        PUBWEAK             IRQ218_Handler
        PUBWEAK             IRQ219_Handler
        PUBWEAK             IRQ220_Handler
        PUBWEAK             IRQ221_Handler
        PUBWEAK             IRQ222_Handler
        PUBWEAK             IRQ223_Handler
        PUBWEAK             IRQ224_Handler
        PUBWEAK             IRQ225_Handler
        PUBWEAK             IRQ226_Handler
        PUBWEAK             IRQ227_Handler
        PUBWEAK             IRQ228_Handler
        PUBWEAK             IRQ229_Handler
        PUBWEAK             IRQ230_Handler
        PUBWEAK             IRQ231_Handler
        PUBWEAK             IRQ232_Handler
        PUBWEAK             IRQ233_Handler
        PUBWEAK             IRQ234_Handler
        PUBWEAK             IRQ235_Handler
        PUBWEAK             IRQ236_Handler
        PUBWEAK             IRQ237_Handler
        PUBWEAK             IRQ238_Handler
        PUBWEAK             IRQ239_Handler

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
        MRS                 R1,XPSR             ;Pass in the interrupt number
        UBFX                R1,R1,#0,#9
        SUB                 R1,R1,#16           ;The IRQ0's number is 16; subtract
        BL                  __RME_A7M_Vct_Handler

        RME_A7M_LOAD
        LTORG
;/* End Weak default handlers ************************************************/

;/* Function:__RME_Int_Disable ************************************************
;Description : The function for disabling all interrupts.
;*****************************************************************************/
        SECTION             .text:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

__RME_Int_Disable
        CPSID               I
        BX                  LR
;/* End Function:__RME_Int_Disable *******************************************/

;/* Function:__RME_Int_Enable *************************************************
;Description : The function for enabling all interrupts.
;*****************************************************************************/
__RME_Int_Enable
        CPSIE               I
        BX                  LR
;/* End Function:__RME_Int_Enable ********************************************/

;/* Function:__RME_A7M_Barrier ************************************************
;Description : A full data/instruction barrier.
;*****************************************************************************/
__RME_A7M_Barrier
        DSB
        ISB
        BX                  LR
;/* End Function:__RME_A7M_Barrier *******************************************/

;/* Function:__RME_A7M_Reset **************************************************
;Description : A full system reset.
;*****************************************************************************/
__RME_A7M_Reset
        ;Disable all interrupts
        CPSID               I
        ;ARMv7-M Standard system reset
        LDR                 R0,=0xE000ED0C
        LDR                 R1,=0x05FA0004
        STR                 R1,[R0]
        ISB
        ;Deadloop
        B                   .
        LTORG
;/* End Function:__RME_A7M_Reset *********************************************/

;/* Function:__RME_A7M_Wait_Int ***********************************************
;Description : Wait until a new interrupt comes, to save power.
;*****************************************************************************/
__RME_A7M_Wait_Int
        WFE
        BX                  LR
;/* End Function:__RME_A7M_Wait_Int ******************************************/

;/* Function:__RME_A7M_MSB_Get ************************************************
;Description : Get the MSB of the word.
;*****************************************************************************/
__RME_A7M_MSB_Get
        CLZ                 R1,R0
        MOV                 R0,#31
        SUB                 R0,R0,R1
        BX                  LR
;/* End Function:__RME_A7M_MSB_Get *******************************************/

;/* Function:__RME_User_Enter *************************************************
;Description : Entering of the user mode, after the system finish its preliminary
;              booting. The function shall never return. This function should only
;              be used to boot the first process in the system.
;Input       : ptr_t Entry - The user execution startpoint.
;              ptr_t Stack - The user stack.
;              ptr_t CPUID - The CPUID.
;*****************************************************************************/
__RME_User_Enter
        MSR                 PSP,R1              ;Set the stack pointer
        MOV                 R4,#0x03            ;Unprevileged thread mode
        MSR                 CONTROL,R4
        ISB
        MOV                 R1,R0               ;Save the entry to R1
        MOV                 R0,R2               ;Save CPUID(0) to R0
        BLX                 R1                  ;Branch to target
;/* End Function:__RME_User_Enter ********************************************/

;/* Function:SysTick_Handler **************************************************
;Description : The System Tick Timer handler routine. This will in fact call a
;              C function to resolve the system service routines.
;*****************************************************************************/
SysTick_Handler
        RME_A7M_SAVE

        MOV                 R0,SP               ;Pass in the regs
        BL                  __RME_A7M_Tim_Handler

        RME_A7M_LOAD
;/* End Function:SysTick_Handler *********************************************/

;/* Function:SVC_Handler ******************************************************
;Description : The SVC handler routine. This will in fact call a C function to
;              resolve the system service routines.
;*****************************************************************************/
SVC_Handler
        RME_A7M_SAVE

        MOV                 R0,SP               ;Pass in the regs
        BL                  __RME_A7M_Svc_Handler

        RME_A7M_LOAD
;/* End Function:SVC_Handler *************************************************/

;/* Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler **************
;Description : The multi-purpose handler routine. This will in fact call
;              a C function to resolve the system service routines.
;*****************************************************************************/
NMI_Handler
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
        LTORG
;/* End Function:NMI/HardFault/MemManage/BusFault/UsageFault_Handler *********/

;/* Function:___RME_A7M_Thd_Cop_Clear *****************************************
;Description : Clean up the coprocessor state so that the FP information is not
;              leaked when switching from a FPU-enabled thread to a FPU-disabled
;              thread. DC16 is used to avoid errors when FPU not enabled.
;*****************************************************************************/
___RME_A7M_Thd_Cop_Clear
        LDR                 R0,=COP_CLEAR
        BIC                 R0,R0,#0x01         ;Strip the Thumb bit IAR sets on code-section
                                                ;labels - otherwise VLDMIA below is unaligned
        DC16                0xEC90              ;VLDMIA    R0,{S0-S31}
        DC16                0x0A20              ;Clear all the FPU registers
        MOV                 R0,#0               ;Clear FPSCR as well
        DC16                0xEEE1              ;VMSR      FPSCR,R0
        DC16                0x0A10
        BX                  LR
        LTORG
COP_CLEAR
        DC32                0,0,0,0,0,0,0,0
        DC32                0,0,0,0,0,0,0,0
        DC32                0,0,0,0,0,0,0,0
        DC32                0,0,0,0,0,0,0,0
;/* End Function:___RME_A7M_Thd_Cop_Clear ************************************/

;/* Function:___RME_A7M_Thd_Cop_Save ******************************************
;Description : Save the coprocessor context on switch.
;Input       : R0 - The pointer to the coprocessor struct.
;*****************************************************************************/
___RME_A7M_Thd_Cop_Save
        DC16                0xEC80              ;VSTMIA    R0,{S16-S31}
        DC16                0x8A10              ;Save all the FPU registers
        BX                  LR
;/* End Function:___RME_A7M_Thd_Cop_Save *************************************/

;/* Function:___RME_A7M_Thd_Cop_Load ******************************************
;Description : Restore the coprocessor context on switch.
;Input       : R0 - The pointer to the coprocessor struct.
;*****************************************************************************/
___RME_A7M_Thd_Cop_Load
        DC16                0xEC90              ;VLDMIA    R0,{S16-S31}
        DC16                0x8A10              ;Restore all the FPU registers
        BX                  LR
;/* End Function:___RME_A7M_Thd_Cop_Load *************************************/

;/* Function:___RME_A7M_MPU_Set ***********************************************
;Description : Set the MPU context. 1-to-16-region versions are all declared here.
;              Note that the ARMv7-M may support more than 16 regions. But we only
;              support 16 regions in our port.
;Input       : R0 - The pointer to the MPU content.
;*****************************************************************************/
        SECTION             .rme_mpu:CODE:NOROOT(3)
        THUMB
        REQUIRE8
        PRESERVE8

MPU_PRE MACRO
        PUSH                {R4-R9}             ;Save registers
        LDR                 R1,=0xE000ED9C      ;The base address of MPU RBAR and all 4 registers
        ENDM

MPU_SET MACRO
        LDMIA               R0!,{R2-R3}         ;Read settings
        STMIA               R1,{R2-R3}          ;Program
        ENDM

MPU_SET2 MACRO
        LDMIA               R0!,{R2-R5}
        STMIA               R1,{R2-R5}
        ENDM

MPU_SET3 MACRO
        LDMIA               R0!,{R2-R7}
        STMIA               R1,{R2-R7}
        ENDM

MPU_SET4 MACRO
        LDMIA               R0!,{R2-R9}
        STMIA               R1,{R2-R9}
        ENDM

MPU_POST MACRO
        POP                 {R4-R9}             ;Restore registers
        ISB                                     ;Barrier
        BX                  LR
        ENDM
;Region version 1
___RME_A7M_MPU_Set1
        MPU_PRE
        MPU_SET
        MPU_POST

;Region version 2
___RME_A7M_MPU_Set2
        MPU_PRE
        MPU_SET2
        MPU_POST

;Region version 3
___RME_A7M_MPU_Set3
        MPU_PRE
        MPU_SET3
        MPU_POST

;Region version 4
___RME_A7M_MPU_Set4
        MPU_PRE
        MPU_SET4
        MPU_POST

;Region version 5
___RME_A7M_MPU_Set5
        MPU_PRE
        MPU_SET4
        MPU_SET
        MPU_POST

;Region version 6
___RME_A7M_MPU_Set6
        MPU_PRE
        MPU_SET4
        MPU_SET2
        MPU_POST

;Region version 7
___RME_A7M_MPU_Set7
        MPU_PRE
        MPU_SET4
        MPU_SET3
        MPU_POST

;Region version 8
___RME_A7M_MPU_Set8
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_POST

;Region version 9
___RME_A7M_MPU_Set9
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET
        MPU_POST

;Region version 10
___RME_A7M_MPU_Set10
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET2
        MPU_POST

;Region version 11
___RME_A7M_MPU_Set11
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET3
        MPU_POST

;Region version 12
___RME_A7M_MPU_Set12
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_POST

;Region version 13
___RME_A7M_MPU_Set13
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET
        MPU_POST

;Region version 14
___RME_A7M_MPU_Set14
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET2
        MPU_POST

;Region version 15
___RME_A7M_MPU_Set15
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET3
        MPU_POST

;Region version 16
___RME_A7M_MPU_Set16
        MPU_PRE
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_SET4
        MPU_POST

        LTORG
;/* End Function:___RME_A7M_MPU_Set ******************************************/

        END
;/* End Of File **************************************************************/

;/* Copyright (C) Evo-Devo Instrum. All rights reserved **********************/
