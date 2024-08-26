/******************************************************************************
Filename   : rme_platform_wchd8c_gcc.s
Author     : pry
Date       : 24/06/2017
Licence    : The Unlicense; see LICENSE for details.
Description: The boot stub source file for all WCH QingKe cores. It claims to
             be RISC-V, yet it has multiple pecularities that must be carefully
             considered when the port is used:
              -----------------------------------------------------------------
              1. Esoteric memory configuration
              This chip is esoteric in the sense that it integrates an SPI
              flash chiplet with the main chip that only have 320k SRAM, which
              is different from most such chips where manufacturers allow using
              an arbitrary size external SPI flash. To imitate a product with
              real embedded flash, WCH choose not to add ICache or prefetcher
              (which is the common case for these chips) but to copy a portion
              of the flash onto the 320k SRAM and use it as code memory. Hence
              the SRAM must be split into "FLASH" and user-usable "SRAM", and
              the possible configurations include:
              192k Flash + 128k RAM
              224k Flash + 96k RAM
              256k Flash + 64k RAM
              288k Flash + 32k RAM
              Care must be taken to select the correct configuration.
              -----------------------------------------------------------------
              2. Nonstandard CSRs that outsmart themselves
              It has non-standard CSRs called GINTENR and INTSYSCR, and these
              are URW. They give user-level code perfect denial-of-service
              attack surface. For serious applications, the binaries must be
              scanned to confirm that they do not contain any CSR writes.
              -----------------------------------------------------------------
              3. Nonstandard interrupt entry behavior
              This chip is also nonstandard in terms of interrupt entry 
              behavior. Upon interrupt entry, it will NOT disable interrupts,
              and leave the MIE bit set. According to WCH, this facilitates the
              so-called "M-mode interrupt preemption". However, such preemption
              will NOT work at all unless the processor guarantees to execute at
              least one instruction between interrupts. To see why, consider a
              situation where a low-priority interrupt is immediately followed
              by a high-priority one. The low-priority interrupt will never get
              a chance to save its mepc even if its first instructions are to do
              so or disable interrupts, because the high-priority interrupt WILL
              preempt it before it can even execute one instruction.
              The remedy here is to (1) disable interrupt preemption altogether,
              or (2) believe that somehow WCH allows low-level interrupt to at
              least execute one instruction before it gets preempted, so we can
              disable interrupts when we enter interrupts, save mepc, and reenable
              interrupts. The (2) will require some modifications to the standard
              context switch code, and we refrain from doing so.
              -----------------------------------------------------------------
              4. Nonstandard PMP exception behavior
              The WCH manual also says that its PMP access permission faults
              are "asychronous imprecise", against the RISC-V standard
              requirement that the PMP violations are always trapped precisely 
              at the processor. This is a serious deviation from the RISC-V
              standard, potentially jeoparding the whole dynamic loading
              scheme. Extensive testing show that albeit the processor always
              stopped precisely at the offending instruction and will try to
              execute the offending instruction again (which is what we want
              to have), the instruction following that offending instruction
              is already commited, causing it to be executed again on exception
              return. The Nuclei N300 series just seem to have the same issue.
              A general workaround for these processors are:
              (1) In any init boot code, insert enough number of NOPs after any
                  potentially dynamic accesses.
              (2) When accessing any data structure, make sure the operations
                  that follow the access are idempotent.
              (3) If (2) is hard to guarantee, just read through the whole
                  memory range and make sure all pages are loaded into the
                  PMP cache, and the PMP cache must be large enough to hold
                  all these pages.
              (4) If (3) is unrealistic, enable RME_PGT_RAW_USER altogether.
              -----------------------------------------------------------------
              5. Nonstandard PMP access control behavior
              The WCH PMP has a hidden "background range" that allow all U-mode
              accesses by default. That said, when all PMP regions are "OFF", 
              the processor does not block any U-mode accesses. This is a clear
              deviation from the RISC-V standard, which require all such accesses
              to fail. The workaround is that we will have to declare the chip as
              having 3 MPU regions, and the last region will always be programmed
              to block all accesses.
******************************************************************************/

/* Import ********************************************************************/
    .extern         __RME_Entry_After
/* End Import ****************************************************************/

/* Export ********************************************************************/
    .global         __RME_Entry
/* End Export ****************************************************************/

/* Startup *******************************************************************/
    .section        .text.rme_entry
    .option         norelax
    .option         norvc

__RME_Entry:
    J               __RME_WCHD8C_Entry
    /* Weird nonstandard constants that the architecture would require */
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00000013
    .word           0x00100073
    .align          1

__RME_WCHD8C_Entry:
    /* Chip-specific CORECFGR(0x0BC0): default value 0x1F */
    LI              t0,0x1F
    CSRW            0x0BC0,t0
    /* Chip-specific INTSYSCR(0x0804): no hardware stack/nesting */
    LI              t0,0x0000E000
    CSRW            0x0804,t0
    /* Enter machine mode, FPU/interrupt disabled for now */
    LI              t0,0x1880
    CSRW            mstatus,t0
    /* Waste the last PMP region to block all user-level accesses */
    LI              t0,0x18000000
    CSRW            pmpcfg0,t0
    LI              t0,0x1FFFFFFF
    CSRW            pmpaddr3,t0
    /* Set vector table address with "number" mode, a WCH extension */
    LA              t0,__RME_Vector
    ORI             t0,t0,3
    CSRW            mtvec,t0
    /* Jump to real entry that further initializes */
    J               __RME_Entry_After
/* End Startup ***************************************************************/

/* Vector ********************************************************************/
    .section        .text.rme_vector
    .option         norvc
    .align          1

__RME_Vector:
    /* System vectors - these are totally nonstandard, implementing a "numeric"
     * mode similar to ARMv7-M that is not even in the RISC-V spec */
    .word           __RME_Entry
    .word           0
    .word           NMI_Handler                 /* NMI */
    .word           EXC_Handler                 /* Hard Fault */
    .word           0
    .word           ECALLM_Handler              /* Ecall M Mode */
    .word           0
    .word           0
    .word           ECALLU_Handler              /* Ecall U Mode */
    .word           BKPT_Handler                /* Break Point */
    .word           0
    .word           0
    .word           SYSTICK_Handler             /* SysTick */
    .word           0
    .word           SWI_Handler                 /* SWI */
    .word           0

    /* External Interrupts - 240 */
    .word           IRQ0_Handler
    .word           IRQ1_Handler
    .word           IRQ2_Handler
    .word           IRQ3_Handler
    .word           IRQ4_Handler
    .word           IRQ5_Handler
    .word           IRQ6_Handler
    .word           IRQ7_Handler
    .word           IRQ8_Handler
    .word           IRQ9_Handler

    .word           IRQ10_Handler
    .word           IRQ11_Handler
    .word           IRQ12_Handler
    .word           IRQ13_Handler
    .word           IRQ14_Handler
    .word           IRQ15_Handler
    .word           IRQ16_Handler
    .word           IRQ17_Handler
    .word           IRQ18_Handler
    .word           IRQ19_Handler

    .word           IRQ20_Handler
    .word           IRQ21_Handler
    .word           IRQ22_Handler
    .word           IRQ23_Handler
    .word           IRQ24_Handler
    .word           IRQ25_Handler
    .word           IRQ26_Handler
    .word           IRQ27_Handler
    .word           IRQ28_Handler
    .word           IRQ29_Handler

    .word           IRQ30_Handler
    .word           IRQ31_Handler
    .word           IRQ32_Handler
    .word           IRQ33_Handler
    .word           IRQ34_Handler
    .word           IRQ35_Handler
    .word           IRQ36_Handler
    .word           IRQ37_Handler
    .word           IRQ38_Handler
    .word           IRQ39_Handler

    .word           IRQ40_Handler
    .word           IRQ41_Handler
    .word           IRQ42_Handler
    .word           IRQ43_Handler
    .word           IRQ44_Handler
    .word           IRQ45_Handler
    .word           IRQ46_Handler
    .word           IRQ47_Handler
    .word           IRQ48_Handler
    .word           IRQ49_Handler

    .word           IRQ50_Handler
    .word           IRQ51_Handler
    .word           IRQ52_Handler
    .word           IRQ53_Handler
    .word           IRQ54_Handler
    .word           IRQ55_Handler
    .word           IRQ56_Handler
    .word           IRQ57_Handler
    .word           IRQ58_Handler
    .word           IRQ59_Handler

    .word           IRQ60_Handler
    .word           IRQ61_Handler
    .word           IRQ62_Handler
    .word           IRQ63_Handler
    .word           IRQ64_Handler
    .word           IRQ65_Handler
    .word           IRQ66_Handler
    .word           IRQ67_Handler
    .word           IRQ68_Handler
    .word           IRQ69_Handler

    .word           IRQ70_Handler
    .word           IRQ71_Handler
    .word           IRQ72_Handler
    .word           IRQ73_Handler
    .word           IRQ74_Handler
    .word           IRQ75_Handler
    .word           IRQ76_Handler
    .word           IRQ77_Handler
    .word           IRQ78_Handler
    .word           IRQ79_Handler

    .word           IRQ80_Handler
    .word           IRQ81_Handler
    .word           IRQ82_Handler
    .word           IRQ83_Handler
    .word           IRQ84_Handler
    .word           IRQ85_Handler
    .word           IRQ86_Handler
    .word           IRQ87_Handler
    .word           IRQ88_Handler
    .word           IRQ89_Handler

    .word           IRQ90_Handler
    .word           IRQ91_Handler
    .word           IRQ92_Handler
    .word           IRQ93_Handler
    .word           IRQ94_Handler
    .word           IRQ95_Handler
    .word           IRQ96_Handler
    .word           IRQ97_Handler
    .word           IRQ98_Handler
    .word           IRQ99_Handler


    .word           IRQ100_Handler
    .word           IRQ101_Handler
    .word           IRQ102_Handler
    .word           IRQ103_Handler
    .word           IRQ104_Handler
    .word           IRQ105_Handler
    .word           IRQ106_Handler
    .word           IRQ107_Handler
    .word           IRQ108_Handler
    .word           IRQ109_Handler

    .word           IRQ110_Handler
    .word           IRQ111_Handler
    .word           IRQ112_Handler
    .word           IRQ113_Handler
    .word           IRQ114_Handler
    .word           IRQ115_Handler
    .word           IRQ116_Handler
    .word           IRQ117_Handler
    .word           IRQ118_Handler
    .word           IRQ119_Handler

    .word           IRQ120_Handler
    .word           IRQ121_Handler
    .word           IRQ122_Handler
    .word           IRQ123_Handler
    .word           IRQ124_Handler
    .word           IRQ125_Handler
    .word           IRQ126_Handler
    .word           IRQ127_Handler
    .word           IRQ128_Handler
    .word           IRQ129_Handler

    .word           IRQ130_Handler
    .word           IRQ131_Handler
    .word           IRQ132_Handler
    .word           IRQ133_Handler
    .word           IRQ134_Handler
    .word           IRQ135_Handler
    .word           IRQ136_Handler
    .word           IRQ137_Handler
    .word           IRQ138_Handler
    .word           IRQ139_Handler

    .word           IRQ140_Handler
    .word           IRQ141_Handler
    .word           IRQ142_Handler
    .word           IRQ143_Handler
    .word           IRQ144_Handler
    .word           IRQ145_Handler
    .word           IRQ146_Handler
    .word           IRQ147_Handler
    .word           IRQ148_Handler
    .word           IRQ149_Handler

    .word           IRQ150_Handler
    .word           IRQ151_Handler
    .word           IRQ152_Handler
    .word           IRQ153_Handler
    .word           IRQ154_Handler
    .word           IRQ155_Handler
    .word           IRQ156_Handler
    .word           IRQ157_Handler
    .word           IRQ158_Handler
    .word           IRQ159_Handler

    .word           IRQ160_Handler
    .word           IRQ161_Handler
    .word           IRQ162_Handler
    .word           IRQ163_Handler
    .word           IRQ164_Handler
    .word           IRQ165_Handler
    .word           IRQ166_Handler
    .word           IRQ167_Handler
    .word           IRQ168_Handler
    .word           IRQ169_Handler

    .word           IRQ170_Handler
    .word           IRQ171_Handler
    .word           IRQ172_Handler
    .word           IRQ173_Handler
    .word           IRQ174_Handler
    .word           IRQ175_Handler
    .word           IRQ176_Handler
    .word           IRQ177_Handler
    .word           IRQ178_Handler
    .word           IRQ179_Handler

    .word           IRQ180_Handler
    .word           IRQ181_Handler
    .word           IRQ182_Handler
    .word           IRQ183_Handler
    .word           IRQ184_Handler
    .word           IRQ185_Handler
    .word           IRQ186_Handler
    .word           IRQ187_Handler
    .word           IRQ188_Handler
    .word           IRQ189_Handler

    .word           IRQ190_Handler
    .word           IRQ191_Handler
    .word           IRQ192_Handler
    .word           IRQ193_Handler
    .word           IRQ194_Handler
    .word           IRQ195_Handler
    .word           IRQ196_Handler
    .word           IRQ197_Handler
    .word           IRQ198_Handler
    .word           IRQ199_Handler

    .word           IRQ200_Handler
    .word           IRQ201_Handler
    .word           IRQ202_Handler
    .word           IRQ203_Handler
    .word           IRQ204_Handler
    .word           IRQ205_Handler
    .word           IRQ206_Handler
    .word           IRQ207_Handler
    .word           IRQ208_Handler
    .word           IRQ209_Handler

    .word           IRQ210_Handler
    .word           IRQ211_Handler
    .word           IRQ212_Handler
    .word           IRQ213_Handler
    .word           IRQ214_Handler
    .word           IRQ215_Handler
    .word           IRQ216_Handler
    .word           IRQ217_Handler
    .word           IRQ218_Handler
    .word           IRQ219_Handler

    .word           IRQ220_Handler
    .word           IRQ221_Handler
    .word           IRQ222_Handler
    .word           IRQ223_Handler
    .word           IRQ224_Handler
    .word           IRQ225_Handler
    .word           IRQ226_Handler
    .word           IRQ227_Handler
    .word           IRQ228_Handler
    .word           IRQ229_Handler

    .word           IRQ230_Handler
    .word           IRQ231_Handler
    .word           IRQ232_Handler
    .word           IRQ233_Handler
    .word           IRQ234_Handler
    .word           IRQ235_Handler
    .word           IRQ236_Handler
    .word           IRQ237_Handler
    .word           IRQ238_Handler
    .word           IRQ239_Handler

    .weak           NMI_Handler
    .weak           EXC_Handler
    .weak           ECALLM_Handler
    .weak           ECALLU_Handler
    .weak           BKPT_Handler
    .weak           SYSTICK_Handler
    .weak           SWI_Handler

    .weak           IRQ0_Handler
    .weak           IRQ1_Handler
    .weak           IRQ2_Handler
    .weak           IRQ3_Handler
    .weak           IRQ4_Handler
    .weak           IRQ5_Handler
    .weak           IRQ6_Handler
    .weak           IRQ7_Handler
    .weak           IRQ8_Handler
    .weak           IRQ9_Handler

    .weak           IRQ10_Handler
    .weak           IRQ11_Handler
    .weak           IRQ12_Handler
    .weak           IRQ13_Handler
    .weak           IRQ14_Handler
    .weak           IRQ15_Handler
    .weak           IRQ16_Handler
    .weak           IRQ17_Handler
    .weak           IRQ18_Handler
    .weak           IRQ19_Handler

    .weak           IRQ20_Handler
    .weak           IRQ21_Handler
    .weak           IRQ22_Handler
    .weak           IRQ23_Handler
    .weak           IRQ24_Handler
    .weak           IRQ25_Handler
    .weak           IRQ26_Handler
    .weak           IRQ27_Handler
    .weak           IRQ28_Handler
    .weak           IRQ29_Handler

    .weak           IRQ30_Handler
    .weak           IRQ31_Handler
    .weak           IRQ32_Handler
    .weak           IRQ33_Handler
    .weak           IRQ34_Handler
    .weak           IRQ35_Handler
    .weak           IRQ36_Handler
    .weak           IRQ37_Handler
    .weak           IRQ38_Handler
    .weak           IRQ39_Handler

    .weak           IRQ40_Handler
    .weak           IRQ41_Handler
    .weak           IRQ42_Handler
    .weak           IRQ43_Handler
    .weak           IRQ44_Handler
    .weak           IRQ45_Handler
    .weak           IRQ46_Handler
    .weak           IRQ47_Handler
    .weak           IRQ48_Handler
    .weak           IRQ49_Handler

    .weak           IRQ50_Handler
    .weak           IRQ51_Handler
    .weak           IRQ52_Handler
    .weak           IRQ53_Handler
    .weak           IRQ54_Handler
    .weak           IRQ55_Handler
    .weak           IRQ56_Handler
    .weak           IRQ57_Handler
    .weak           IRQ58_Handler
    .weak           IRQ59_Handler

    .weak           IRQ60_Handler
    .weak           IRQ61_Handler
    .weak           IRQ62_Handler
    .weak           IRQ63_Handler
    .weak           IRQ64_Handler
    .weak           IRQ65_Handler
    .weak           IRQ66_Handler
    .weak           IRQ67_Handler
    .weak           IRQ68_Handler
    .weak           IRQ69_Handler

    .weak           IRQ70_Handler
    .weak           IRQ71_Handler
    .weak           IRQ72_Handler
    .weak           IRQ73_Handler
    .weak           IRQ74_Handler
    .weak           IRQ75_Handler
    .weak           IRQ76_Handler
    .weak           IRQ77_Handler
    .weak           IRQ78_Handler
    .weak           IRQ79_Handler

    .weak           IRQ80_Handler
    .weak           IRQ81_Handler
    .weak           IRQ82_Handler
    .weak           IRQ83_Handler
    .weak           IRQ84_Handler
    .weak           IRQ85_Handler
    .weak           IRQ86_Handler
    .weak           IRQ87_Handler
    .weak           IRQ88_Handler
    .weak           IRQ89_Handler

    .weak           IRQ90_Handler
    .weak           IRQ91_Handler
    .weak           IRQ92_Handler
    .weak           IRQ93_Handler
    .weak           IRQ94_Handler
    .weak           IRQ95_Handler
    .weak           IRQ96_Handler
    .weak           IRQ97_Handler
    .weak           IRQ98_Handler
    .weak           IRQ99_Handler


    .weak           IRQ100_Handler
    .weak           IRQ101_Handler
    .weak           IRQ102_Handler
    .weak           IRQ103_Handler
    .weak           IRQ104_Handler
    .weak           IRQ105_Handler
    .weak           IRQ106_Handler
    .weak           IRQ107_Handler
    .weak           IRQ108_Handler
    .weak           IRQ109_Handler

    .weak           IRQ110_Handler
    .weak           IRQ111_Handler
    .weak           IRQ112_Handler
    .weak           IRQ113_Handler
    .weak           IRQ114_Handler
    .weak           IRQ115_Handler
    .weak           IRQ116_Handler
    .weak           IRQ117_Handler
    .weak           IRQ118_Handler
    .weak           IRQ119_Handler

    .weak           IRQ120_Handler
    .weak           IRQ121_Handler
    .weak           IRQ122_Handler
    .weak           IRQ123_Handler
    .weak           IRQ124_Handler
    .weak           IRQ125_Handler
    .weak           IRQ126_Handler
    .weak           IRQ127_Handler
    .weak           IRQ128_Handler
    .weak           IRQ129_Handler

    .weak           IRQ130_Handler
    .weak           IRQ131_Handler
    .weak           IRQ132_Handler
    .weak           IRQ133_Handler
    .weak           IRQ134_Handler
    .weak           IRQ135_Handler
    .weak           IRQ136_Handler
    .weak           IRQ137_Handler
    .weak           IRQ138_Handler
    .weak           IRQ139_Handler

    .weak           IRQ140_Handler
    .weak           IRQ141_Handler
    .weak           IRQ142_Handler
    .weak           IRQ143_Handler
    .weak           IRQ144_Handler
    .weak           IRQ145_Handler
    .weak           IRQ146_Handler
    .weak           IRQ147_Handler
    .weak           IRQ148_Handler
    .weak           IRQ149_Handler

    .weak           IRQ150_Handler
    .weak           IRQ151_Handler
    .weak           IRQ152_Handler
    .weak           IRQ153_Handler
    .weak           IRQ154_Handler
    .weak           IRQ155_Handler
    .weak           IRQ156_Handler
    .weak           IRQ157_Handler
    .weak           IRQ158_Handler
    .weak           IRQ159_Handler

    .weak           IRQ160_Handler
    .weak           IRQ161_Handler
    .weak           IRQ162_Handler
    .weak           IRQ163_Handler
    .weak           IRQ164_Handler
    .weak           IRQ165_Handler
    .weak           IRQ166_Handler
    .weak           IRQ167_Handler
    .weak           IRQ168_Handler
    .weak           IRQ169_Handler

    .weak           IRQ170_Handler
    .weak           IRQ171_Handler
    .weak           IRQ172_Handler
    .weak           IRQ173_Handler
    .weak           IRQ174_Handler
    .weak           IRQ175_Handler
    .weak           IRQ176_Handler
    .weak           IRQ177_Handler
    .weak           IRQ178_Handler
    .weak           IRQ179_Handler

    .weak           IRQ180_Handler
    .weak           IRQ181_Handler
    .weak           IRQ182_Handler
    .weak           IRQ183_Handler
    .weak           IRQ184_Handler
    .weak           IRQ185_Handler
    .weak           IRQ186_Handler
    .weak           IRQ187_Handler
    .weak           IRQ188_Handler
    .weak           IRQ189_Handler

    .weak           IRQ190_Handler
    .weak           IRQ191_Handler
    .weak           IRQ192_Handler
    .weak           IRQ193_Handler
    .weak           IRQ194_Handler
    .weak           IRQ195_Handler
    .weak           IRQ196_Handler
    .weak           IRQ197_Handler
    .weak           IRQ198_Handler
    .weak           IRQ199_Handler

    .weak           IRQ200_Handler
    .weak           IRQ201_Handler
    .weak           IRQ202_Handler
    .weak           IRQ203_Handler
    .weak           IRQ204_Handler
    .weak           IRQ205_Handler
    .weak           IRQ206_Handler
    .weak           IRQ207_Handler
    .weak           IRQ208_Handler
    .weak           IRQ209_Handler

    .weak           IRQ210_Handler
    .weak           IRQ211_Handler
    .weak           IRQ212_Handler
    .weak           IRQ213_Handler
    .weak           IRQ214_Handler
    .weak           IRQ215_Handler
    .weak           IRQ216_Handler
    .weak           IRQ217_Handler
    .weak           IRQ218_Handler
    .weak           IRQ219_Handler

    .weak           IRQ220_Handler
    .weak           IRQ221_Handler
    .weak           IRQ222_Handler
    .weak           IRQ223_Handler
    .weak           IRQ224_Handler
    .weak           IRQ225_Handler
    .weak           IRQ226_Handler
    .weak           IRQ227_Handler
    .weak           IRQ228_Handler
    .weak           IRQ229_Handler

    .weak           IRQ230_Handler
    .weak           IRQ231_Handler
    .weak           IRQ232_Handler
    .weak           IRQ233_Handler
    .weak           IRQ234_Handler
    .weak           IRQ235_Handler
    .weak           IRQ236_Handler
    .weak           IRQ237_Handler
    .weak           IRQ238_Handler
    .weak           IRQ239_Handler

Default_Handler:
NMI_Handler:
EXC_Handler:
ECALLM_Handler:
ECALLU_Handler:
BKPT_Handler:
SYSTICK_Handler:
SWI_Handler:

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
    J               __RME_RV32P_Handler
/* End Vector ****************************************************************/

    .end
/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/

