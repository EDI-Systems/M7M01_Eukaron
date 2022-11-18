/*
 * memoryIntCode.x -- memory configuration for TC275x (CPU0)
 *
 * Copyright (C) 2001 - 2013 HighTec EDV-Systeme GmbH.
 *
 */
__USTACK_SIZE = 4K;
__ISTACK_SIZE = 256;
__HEAP_MIN = 4K;
__CSA_SIZE = 4K;
__TRICORE_DERIVATE_MEMORY_MAP__ = 0x2700;
/* the external RAM description */
__EXT_CODE_RAM_BEGIN = 0;
__EXT_CODE_RAM_SIZE = 0 ;
__EXT_DATA_RAM_BEGIN = 0;
__EXT_DATA_RAM_SIZE = 0;
__RAM_END = __EXT_DATA_RAM_BEGIN + __EXT_DATA_RAM_SIZE;
/* internal FLASH description */
__INT_CODE_FLASH_BEGIN = 0x80000000;
__INT_CODE_FLASH_SIZE = 0x200000;
__INT_CODE_FLASH_BANK1_BEGIN = 0x80200000;
__INT_CODE_FLASH_BANK1_SIZE = 0x200000;
__INT_DATA_FLASH_BEGIN = 0xaf000000;
__INT_DATA_FLASH_SIZE = 384K;
__INT_DATA_FLASH_BANK1_BEGIN = 0xaf110000;
__INT_DATA_FLASH_BANK1_SIZE = 64K;
/* the internal ram description */
__INT_CODE_RAM_BEGIN = 0xc0000000;
__INT_CODE_RAM_SIZE = 24K;
__INT_DATA_RAM_BEGIN = 0xd0000000;
__INT_DATA_RAM_SIZE = 112K;
/* the pcp memory description */
__PCP_CODE_RAM_BEGIN = 0;
__PCP_CODE_RAM_SIZE = 0;
__PCP_DATA_RAM_BEGIN = 0;
__PCP_DATA_RAM_SIZE = 0;
MEMORY
{
  PMU_PFLASH0 (rx!p): org = 0x80000000, len = 0x200000
  PMU_PFLASH1 (rx!p): org = 0x80200000, len = 0x200000
  PMU_DFLASH0 (w!xp): org = 0xaf000000, len = 384K
  PMU_DFLASH1 (w!xp): org = 0xaf110000, len = 64K
  PMI_SPRAM (rx!p): org = 0xc0000000, len = 24K
  DMI_LDRAM (w!xp): org = 0xd0000000, len = 112K
  PCP_PRAM (wp!x): org = 0, len = 0
  PCP_CMEM (rpx): org = 0, len = 0
}
REGION_ALIAS("DATA_MEM", DMI_LDRAM)
REGION_ALIAS("CODE_MEM", PMI_SPRAM)
REGION_ALIAS("SDATA_MEM", DMI_LDRAM)
REGION_ALIAS("BSS_MEM", DMI_LDRAM)
REGION_ALIAS("ZDATA_MEM", DMI_LDRAM)
REGION_ALIAS("CSA_MEM", DMI_LDRAM)
REGION_ALIAS("PCP_CODE", PCP_CMEM)
REGION_ALIAS("PCP_DATA", PCP_PRAM)
/* the symbol __TRICORE_DERIVATE_NAME__ will be defined in the crt0.S and is
 * tested here to confirm that this memory map and the startup file will
 * fit together
*/
_. = ASSERT ((__TRICORE_DERIVATE_MEMORY_MAP__ == __TRICORE_DERIVATE_NAME__), "Using wrong Memory Map. This Map is for TC27XX");
INSERT BEFORE .startup
