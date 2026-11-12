/******************************************************************************
Filename    : platform_x64.c
Author      : pry
Date        : 01/04/2017
Licence     : The Unlicense; see LICENSE for details.
Description : The hardware abstraction layer for ACPI compliant x86-64 machines.
              TODO list:
              1. Fix all the system call/int gates, make sure that they cannot cause DOS.
              2. Implement NMI to a different stack, thus the NMI will not currupt the
                 kernel stack. The NMI stack does not have to be large. Use NMIw/LAPIC
                 timer as watchdog on each core, and if something bad happens we just reboot.
                 we also need to display whether this is due to hardware fault or something
                 else.
              3. Fix error handling - now they just display info.
              4. Test multi-core stuff.
              5. User-level: Consider running VMs. We support FULL virtualization
                 instead of paravirtualization. Use virtualization extensions to do that.
              6. Display something on screen at system boot-up. A console will be great.
              7. Consider how do we pass COMPLEX system configuration data to the INIT
                 process. We have multi-sockets and this is going to be very complex if
                 not well handled.
              8. Look into FPU support. Also, FPU support on x86-64 should be reexamined
                 if possible. The current FPU support is purely theoretical.
              9. Consider using the AML interpreter. Or we will not be able to detect
                 more complex peripherals.
              10. Run this on a real machine to watch output.
              11. Consider complex PCI device support and driver compatibility.
              12. Make the system NUMA-aware.
******************************************************************************/

/* Include *******************************************************************/
#define __HDR_DEF__
#include "Platform/X64/rme_platform_x64.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_DEF__

#define __HDR_STRUCT__
#include "Platform/X64/rme_platform_x64.h"
#include "Kernel/rme_kernel.h"
#undef __HDR_STRUCT__

/* Private include */
#include "Platform/X64/rme_platform_x64.h"

#define __HDR_PUBLIC__
#include "Kernel/rme_kernel.h"
#undef __HDR_PUBLIC__
/* End Include ***************************************************************/

/* Function:main **************************************************************
Description : The entrance of the operating system.
Input       : rme_ptr_t MBInfo - The multiboot information structure's physical address.
Output      : None.
Return      : int - This function never returns.
******************************************************************************/
int main(rme_ptr_t MBInfo)
{
    RME_X64_MBInfo_Phys=MBInfo;
    /* The main function of the kernel - we will start our kernel boot here */
    _RME_Kmain(RME_KOM_STACK_ADDR);
    return 0;
}
/* End Function:main *********************************************************/


/* Multiboot2 tags used before switching to the kernel page tables. */
struct RME_X64_Boot_Info
{
    const struct multiboot_tag_mmap* MMap;
    const struct multiboot_tag_framebuffer* Framebuffer;
    const struct multiboot_tag* ACPI_Old;
    const struct multiboot_tag* ACPI_New;
    rme_ptr_t EFI;
};

static rme_ret_t __RME_X64_Boot_Info_Read(struct RME_X64_Boot_Info* Boot)
{
    const struct multiboot_info* Info;
    const struct multiboot_tag* Tag;
    rme_ptr_t Base=RME_X64_MBInfo_Phys;
    rme_ptr_t Offset;
    rme_ptr_t Size;
    rme_ptr_t Next;
    const rme_ptr_t Limit=1ULL<<32;

    if(Base==0 || (Base&(MULTIBOOT_TAG_ALIGN-1U))!=0 ||
       Base>Limit-sizeof(*Info))
        return -1;

    Info=(const struct multiboot_info*)RME_X64_PA2VA(Base);
    Size=Info->total_size;
    if(Size<sizeof(*Info)+sizeof(*Tag) || Size>Limit-Base ||
       (Size&(MULTIBOOT_TAG_ALIGN-1U))!=0)
        return -1;

    for(Offset=sizeof(*Info);Size-Offset>=sizeof(*Tag);Offset=Next)
    {
        Tag=(const struct multiboot_tag*)((rme_ptr_t)Info+Offset);
        if(Tag->size<sizeof(*Tag) || Tag->size>Size-Offset)
            return -1;
        Next=Offset+RME_ROUND_UP((rme_ptr_t)Tag->size,3);
        if(Next>Size)
            return -1;
        switch(Tag->type)
        {
            case MULTIBOOT_TAG_END:
                return (Tag->size==sizeof(*Tag) && Next==Size && Boot->MMap!=0)?0:-1;
            case MULTIBOOT_TAG_MMAP:
                if(Tag->size<sizeof(*Boot->MMap))
                    return -1;
                Boot->MMap=(const struct multiboot_tag_mmap*)Tag;
                break;
            case MULTIBOOT_TAG_FRAMEBUFFER:
                if(Tag->size>=sizeof(*Boot->Framebuffer))
                    Boot->Framebuffer=(const struct multiboot_tag_framebuffer*)Tag;
                break;
            case MULTIBOOT_TAG_EFI64:
                Boot->EFI=1;
                break;
            case MULTIBOOT_TAG_ACPI_OLD:
                Boot->ACPI_Old=Tag;
                break;
            case MULTIBOOT_TAG_ACPI_NEW:
                Boot->ACPI_New=Tag;
                break;
            default:
                break;
        }
    }

    return -1;
}

#define RME_X64_GLYPH_SIZE 8U
#define RME_X64_TEXT_LINE  12U

/* Printable ASCII from font8x8_basic by Daniel Hepper, based on the
 * public-domain VGA font by Marcel Sondaar and IBM. Each row is LSB-first.
 * https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h */
static const rme_u8_t RME_X64_Font[95][RME_X64_GLYPH_SIZE]=
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 0x20 */
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, /* ! */
    { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* " */
    { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, /* # */
    { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, /* $ */
    { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, /* % */
    { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, /* & */
    { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, /* ' */
    { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, /* ( */
    { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, /* ) */
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, /* 0x2A */
    { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, /* + */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06}, /* , */
    { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, /* - */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, /* . */
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, /* 0x2F */
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, /* 0 */
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, /* 1 */
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, /* 2 */
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, /* 3 */
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, /* 4 */
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, /* 5 */
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, /* 6 */
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, /* 7 */
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, /* 8 */
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, /* 9 */
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, /* : */
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06}, /* ; */
    { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, /* < */
    { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, /* = */
    { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, /* > */
    { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, /* ? */
    { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, /* @ */
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, /* A */
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, /* B */
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, /* C */
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, /* D */
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, /* E */
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, /* F */
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, /* G */
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, /* H */
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, /* I */
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, /* J */
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, /* K */
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, /* L */
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, /* M */
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, /* N */
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, /* O */
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, /* P */
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, /* Q */
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, /* R */
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, /* S */
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, /* T */
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, /* U */
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, /* V */
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, /* W */
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, /* X */
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, /* Y */
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, /* Z */
    { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00}, /* [ */
    { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00}, /* 0x5C */
    { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00}, /* ] */
    { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, /* ^ */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, /* _ */
    { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, /* ` */
    { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, /* a */
    { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, /* b */
    { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, /* c */
    { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, /* d */
    { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, /* e */
    { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, /* f */
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, /* g */
    { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, /* h */
    { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, /* i */
    { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, /* j */
    { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, /* k */
    { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, /* l */
    { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, /* m */
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, /* n */
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, /* o */
    { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, /* p */
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, /* q */
    { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, /* r */
    { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, /* s */
    { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, /* t */
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, /* u */
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, /* v */
    { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, /* w */
    { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, /* x */
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, /* y */
    { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, /* z */
    { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00}, /* { */
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00}, /* | */
    { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00}, /* } */
    { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* ~ */
};

enum RME_X64_Boot_Stage
{
    RME_X64_BOOT_GOP,
    RME_X64_BOOT_ACPI,
    RME_X64_BOOT_MMU,
    RME_X64_BOOT_IRQ,
    RME_X64_BOOT_SMP,
    RME_X64_BOOT_UVM,
    RME_X64_BOOT_DONE
};

enum RME_X64_Boot_Color
{
    RME_X64_BOOT_PANEL,
    RME_X64_BOOT_DIM,
    RME_X64_BOOT_LABEL,
    RME_X64_BOOT_READY,
    RME_X64_BOOT_ACTIVE,
    RME_X64_BOOT_COLORS
};

#define RME_X64_BOOT_ROWS 6U

static struct
{
    volatile rme_u32_t* Buffer;
    rme_ptr_t Stride;
    rme_ptr_t Scale;
    rme_ptr_t Top;
    rme_ptr_t Boot_Width;
    rme_u32_t Boot_Color[RME_X64_BOOT_COLORS];
    rme_ptr_t Cols;
    rme_ptr_t Rows;
    rme_ptr_t Col;
    rme_ptr_t Row;
    rme_u32_t Foreground;
    rme_u32_t Background;
} RME_X64_RGB_Console;

/* Bootstrap PML4 slot 510 holds two temporary ACPI windows and one
 * framebuffer window. Only the framebuffer window is kept in the final map. */
#define RME_X64_BOOT_WINDOW_SLOT 510U
#define RME_X64_BOOT_WINDOW_BASE 0xFFFFFF0000000000ULL
#define RME_X64_RGB_WINDOW       2U

static rme_ptr_t RME_X64_Boot_PDP[512] __attribute__((aligned(4096)));
static rme_ptr_t RME_X64_Boot_PD[3][512] __attribute__((aligned(4096)));

/* Function:__RME_X64_Boot_Window_Map *****************************************
Description : Map a physical range in one of the reserved bootstrap windows.
Input       : rme_ptr_t Phys - The physical address of the first byte.
              rme_ptr_t Size - The length in bytes.
              rme_ptr_t Window - The ACPI or framebuffer window index.
              rme_ptr_t Flags - Additional leaf page permissions and cache flags.
Output      : None.
Return      : rme_ptr_t - The mapped virtual address, or zero on failure.
******************************************************************************/
rme_ptr_t __RME_X64_Boot_Window_Map(rme_ptr_t Phys, rme_ptr_t Size,
                                  rme_ptr_t Window, rme_ptr_t Flags)
{
    const rme_ptr_t Page=1ULL<<21;
    const rme_ptr_t Window_Size=1ULL<<30;
    rme_ptr_t* PML4=(rme_ptr_t*)RME_X64_PA2VA(0x101000);
    rme_ptr_t Phys_Bits=RME_X64_EXT(RME_X64_CPUID_E8_VA_PA_SIZE,0)&0xFFU;
    rme_ptr_t Limit=1ULL<<((Phys_Bits>=32 && Phys_Bits<=52)?Phys_Bits:36);
    rme_ptr_t Offset=Phys&(Page-1U);
    rme_ptr_t VA=RME_X64_BOOT_WINDOW_BASE+Window*Window_Size;
    rme_ptr_t* PD;

    if(Window>=sizeof(RME_X64_Boot_PD)/sizeof(RME_X64_Boot_PD[0]) ||
       Phys==0 || Size==0 || Phys>=Limit || Size>Limit-Phys ||
       Size>Window_Size-Offset)
        return 0;

    PD=RME_X64_Boot_PD[Window];
    RME_X64_Boot_PDP[Window]=RME_X64_TEXT_VA2PA(PD)|RME_X64_MMU_P|RME_X64_MMU_RW;
    PML4[RME_X64_BOOT_WINDOW_SLOT]=RME_X64_TEXT_VA2PA(RME_X64_Boot_PDP)|
                                   RME_X64_MMU_P|RME_X64_MMU_RW;
    Phys-=Offset;
    for(rme_ptr_t Index=0;Index*Page<Size+Offset;Index++)
    {
        PD[Index]=(Phys+Index*Page)|RME_X64_MMU_P|RME_X64_MMU_PDE_SUP|RME_X64_MMU_NX|Flags;
        __asm__ volatile("invlpg (%0)"::"r"(VA+Index*Page):"memory");
    }

    return VA+Offset;
}
/* End Function:__RME_X64_Boot_Window_Map ************************************/

/* Function:__RME_X64_RGB_Init ***********************************************
Description : Map a 32-bit RGB framebuffer and initialize its ASCII console.
Input       : const struct multiboot_tag_framebuffer* Framebuffer - The mode.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_RGB_Init(const struct multiboot_tag_framebuffer* Framebuffer)
{
    const struct multiboot_framebuffer_rgb* RGB=(const void*)(Framebuffer+1);
    const rme_u32_t Palette[RME_X64_BOOT_COLORS]=
    {
        0x0C1927, 0x21384C, 0x9AB4CC, 0x52E4C4, 0xFFD28A
    };
    rme_u32_t Mask[3];
    rme_ptr_t Scale=(Framebuffer->width>=1024U && Framebuffer->height>=768U)?2U:1U;
    rme_ptr_t Cell_Width=RME_X64_GLYPH_SIZE*Scale;
    rme_ptr_t Cell_Height=RME_X64_TEXT_LINE*Scale;
    volatile rme_u32_t* Buffer;

    if(Framebuffer->tag.size<sizeof(*Framebuffer)+sizeof(*RGB) ||
       Framebuffer->bpp!=32U || ((Framebuffer->addr|Framebuffer->pitch)&3U)!=0 ||
       Framebuffer->pitch<(rme_ptr_t)Framebuffer->width*sizeof(*Buffer) ||
       Framebuffer->width<3U*Cell_Width || Framebuffer->height<3U*Cell_Height)
        return;

    const rme_u8_t Position[3]={RGB->red_position,RGB->green_position,RGB->blue_position};
    const rme_u8_t Size[3]={RGB->red_size,RGB->green_size,RGB->blue_size};
    rme_u32_t Used=0;
    for(rme_ptr_t Index=0;Index<3U;Index++)
    {
        if(Size[Index]==0 || Position[Index]+Size[Index]>32U)
            return;
        Mask[Index]=(rme_u32_t)(((1ULL<<Size[Index])-1U)<<Position[Index]);
        if((Used&Mask[Index])!=0)
            return;
        Used|=Mask[Index];
    }

    const rme_ptr_t Four_GiB=1ULL<<32;
    rme_ptr_t Phys=Framebuffer->addr;
    rme_ptr_t Length=(rme_ptr_t)Framebuffer->pitch*Framebuffer->height;
    rme_ptr_t VA;

    if(Phys<Four_GiB && Length<=Four_GiB-Phys)
        VA=RME_X64_PA2VA(Phys);
    else
        VA=__RME_X64_Boot_Window_Map(Phys,Length,RME_X64_RGB_WINDOW,
                                     RME_X64_MMU_RW|RME_X64_MMU_PCD|RME_X64_MMU_PWT);
    if(VA==0)
        return;

    Buffer=(volatile rme_u32_t*)VA;
    RME_X64_RGB_Console.Stride=Framebuffer->pitch/sizeof(*Buffer);
    RME_X64_RGB_Console.Scale=Scale;
    RME_X64_RGB_Console.Cols=Framebuffer->width/Cell_Width-2U;
    RME_X64_RGB_Console.Rows=Framebuffer->height/Cell_Height-2U;
    RME_X64_RGB_Console.Top=Cell_Height;
    /* Keep the splash above the scrolling console. Leave small modes to text. */
    if(Framebuffer->width/Scale>=480U && RME_X64_RGB_Console.Rows>=24U)
    {
        RME_X64_RGB_Console.Boot_Width=Framebuffer->width/Scale;
        RME_X64_RGB_Console.Top+=RME_X64_BOOT_ROWS*Cell_Height;
        RME_X64_RGB_Console.Rows-=RME_X64_BOOT_ROWS;
        for(rme_ptr_t Color=0;Color<RME_X64_BOOT_COLORS;Color++)
        {
            rme_u32_t Pixel=0;
            for(rme_ptr_t Index=0;Index<3U;Index++)
            {
                rme_ptr_t Channel=(Palette[Color]>>((2U-Index)*8U))&0xFFU;
                rme_ptr_t Max=Mask[Index]>>Position[Index];
                Pixel|=(rme_u32_t)(((Max*Channel+127U)/255U)<<Position[Index]);
            }
            RME_X64_RGB_Console.Boot_Color[Color]=Pixel;
        }
    }
    RME_X64_RGB_Console.Foreground=Used;
    RME_X64_RGB_Console.Background=((Mask[0]>>4)&Mask[0])|
                                   ((Mask[1]>>4)&Mask[1])|
                                   ((Mask[2]>>3)&Mask[2]);

    for(rme_ptr_t Y=0;Y<Framebuffer->height;Y++)
    {
        for(rme_ptr_t X=0;X<Framebuffer->width;X++)
            Buffer[X]=RME_X64_RGB_Console.Background;
        Buffer+=RME_X64_RGB_Console.Stride;
    }
    RME_X64_RGB_Console.Buffer=(volatile rme_u32_t*)VA;
    __RME_X64_Boot_Progress(RME_X64_BOOT_GOP);
}
/* End Function:__RME_X64_RGB_Init *******************************************/

/* Function:__RME_X64_RGB_Glyph *********************************************
Description : Draw a scaled glyph from the built-in font.
Input       : volatile rme_u32_t* Pixel - The top left pixel of the glyph.
              rme_u8_t Char - The printable ASCII character.
              rme_ptr_t Scale - The pixel scale factor.
              rme_u32_t Ink - The foreground color.
              rme_u32_t Paper - The background color.
Output      : None.
Return      : None.
******************************************************************************/
static void __RME_X64_RGB_Glyph(volatile rme_u32_t* Pixel, rme_u8_t Char,
                               rme_ptr_t Scale, rme_u32_t Ink, rme_u32_t Paper)
{
    rme_ptr_t Width=RME_X64_GLYPH_SIZE*Scale;

    for(rme_ptr_t Y=0;Y<Width;Y++)
    {
        rme_u8_t Bits=RME_X64_Font[Char-' '][Y/Scale];
        for(rme_ptr_t X=0;X<Width;X++)
            Pixel[X]=((Bits>>(X/Scale))&1U)?Ink:Paper;
        Pixel+=RME_X64_RGB_Console.Stride;
    }
}
/* End Function:__RME_X64_RGB_Glyph ******************************************/

/* Function:__RME_X64_RGB_Putchar ********************************************
Description : Draw a debug character and scroll the console when it fills.
Input       : rme_u8_t Char - The character to display.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_RGB_Putchar(rme_u8_t Char)
{
    rme_ptr_t Scale=RME_X64_RGB_Console.Scale;
    rme_ptr_t Stride=RME_X64_RGB_Console.Stride;
    rme_ptr_t Cell_Width=RME_X64_GLYPH_SIZE*Scale;
    rme_ptr_t Cell_Height=RME_X64_TEXT_LINE*Scale;
    volatile rme_u32_t* Origin=RME_X64_RGB_Console.Buffer+RME_X64_RGB_Console.Top*Stride+Cell_Width;
    volatile rme_u32_t* Pixel;

    if(Char=='\n')
    {
        RME_X64_RGB_Console.Col=0;
        RME_X64_RGB_Console.Row++;
    }
    else if(Char>=' ')
    {
        if(Char>'~')
            Char='?';

        /* Center the glyph vertically in the cell. */
        Pixel=Origin+(RME_X64_RGB_Console.Row*Cell_Height+2U*Scale)*Stride+
              RME_X64_RGB_Console.Col*Cell_Width;
        __RME_X64_RGB_Glyph(Pixel,Char,Scale,RME_X64_RGB_Console.Foreground,
                            RME_X64_RGB_Console.Background);

        RME_X64_RGB_Console.Col++;
        if(RME_X64_RGB_Console.Col==RME_X64_RGB_Console.Cols)
        {
            RME_X64_RGB_Console.Col=0;
            RME_X64_RGB_Console.Row++;
        }
    }

    if(RME_X64_RGB_Console.Row==RME_X64_RGB_Console.Rows)
    {
        rme_ptr_t Width=RME_X64_RGB_Console.Cols*Cell_Width;
        rme_ptr_t Height=(RME_X64_RGB_Console.Rows-1U)*Cell_Height;

        for(rme_ptr_t Y=0;Y<Height;Y++)
        {
            for(rme_ptr_t X=0;X<Width;X++)
                Origin[X]=Origin[Cell_Height*Stride+X];
            Origin+=Stride;
        }
        for(rme_ptr_t Y=0;Y<Cell_Height;Y++)
        {
            for(rme_ptr_t X=0;X<Width;X++)
                Origin[X]=RME_X64_RGB_Console.Background;
            Origin+=Stride;
        }
        RME_X64_RGB_Console.Row--;
    }
}
/* End Function:__RME_X64_RGB_Putchar ****************************************/

/* Function:__RME_X64_Boot_Rect **********************************************
Description : Fill a rectangle inside the splash.
Input       : rme_ptr_t X, Y - The top left corner in unscaled pixels.
              rme_ptr_t Width, Height - The size in unscaled pixels.
              rme_ptr_t Color - The splash palette index.
Output      : None.
Return      : None.
******************************************************************************/
static void __RME_X64_Boot_Rect(rme_ptr_t X, rme_ptr_t Y, rme_ptr_t Width,
                                rme_ptr_t Height, rme_ptr_t Color)
{
    rme_ptr_t Scale=RME_X64_RGB_Console.Scale;
    rme_ptr_t Stride=RME_X64_RGB_Console.Stride;
    rme_u32_t Ink=RME_X64_RGB_Console.Boot_Color[Color];
    volatile rme_u32_t* Pixel=RME_X64_RGB_Console.Buffer+Y*Scale*Stride+X*Scale;

    for(rme_ptr_t Row=0;Row<Height*Scale;Row++)
    {
        for(rme_ptr_t Col=0;Col<Width*Scale;Col++)
            Pixel[Col]=Ink;
        Pixel+=Stride;
    }
}
/* End Function:__RME_X64_Boot_Rect ******************************************/

/* Function:__RME_X64_Boot_Text **********************************************
Description : Draw printable ASCII text inside the splash.
Input       : rme_ptr_t X, Y - The top left corner in unscaled pixels.
              rme_ptr_t Zoom - The glyph scale relative to the splash scale.
              rme_ptr_t Color - The splash palette index.
              const char* Text - The text to draw.
Output      : None.
Return      : None.
******************************************************************************/
static void __RME_X64_Boot_Text(rme_ptr_t X, rme_ptr_t Y, rme_ptr_t Zoom,
                                rme_ptr_t Color, const char* Text)
{
    rme_ptr_t Scale=RME_X64_RGB_Console.Scale;
    rme_ptr_t Glyph_Scale=Scale*Zoom;
    volatile rme_u32_t* Pixel=RME_X64_RGB_Console.Buffer+
                              Y*Scale*RME_X64_RGB_Console.Stride+X*Scale;

    while(*Text!='\0')
    {
        __RME_X64_RGB_Glyph(Pixel,(rme_u8_t)*Text++,Glyph_Scale,
                            RME_X64_RGB_Console.Boot_Color[Color],
                            RME_X64_RGB_Console.Boot_Color[RME_X64_BOOT_PANEL]);
        Pixel+=RME_X64_GLYPH_SIZE*Glyph_Scale;
    }
}
/* End Function:__RME_X64_Boot_Text ******************************************/

/* Function:__RME_X64_Boot_Progress ******************************************
Description : Show the current initialization stage above the boot log.
Input       : rme_ptr_t Stage - The current stage, or DONE before entering UVM.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_Boot_Progress(rme_ptr_t Stage)
{
    static const char* const Label[]={"GOP","ACPI","MMU","IRQ","SMP","UVM"};
    static const char* const Status[]=
    {
        "Framebuffer ready", "Reading firmware tables", "Building kernel page tables",
        "Setting up interrupts", "Starting secondary CPUs", "Preparing initial process",
        "Entering user mode"
    };
    rme_ptr_t Width=RME_X64_RGB_Console.Boot_Width;
    rme_ptr_t Segment;

    if(Width==0)
        return;

    if(Stage==RME_X64_BOOT_GOP)
    {
        __RME_X64_Boot_Rect(0,0,Width,RME_X64_BOOT_ROWS*RME_X64_TEXT_LINE,RME_X64_BOOT_PANEL);
        __RME_X64_Boot_Rect(0,0,3,RME_X64_BOOT_ROWS*RME_X64_TEXT_LINE,RME_X64_BOOT_READY);
        __RME_X64_Boot_Text(12,10,3,RME_X64_BOOT_READY,"RME");
        __RME_X64_Boot_Text(98,10,1,RME_X64_BOOT_LABEL,"X86-64 MICROKERNEL");
    }

    __RME_X64_Boot_Rect(98,26,Width-250U,8,RME_X64_BOOT_PANEL);
    __RME_X64_Boot_Text(98,26,1,(Stage==RME_X64_BOOT_DONE)?RME_X64_BOOT_READY:
                                                          RME_X64_BOOT_ACTIVE,Status[Stage]);
    Segment=(Width-24U)/RME_X64_BOOT_DONE;
    for(rme_ptr_t Index=0;Index<RME_X64_BOOT_DONE;Index++)
    {
        rme_ptr_t X=12U+Segment*Index;
        rme_ptr_t Color=(Index<Stage)?RME_X64_BOOT_READY:RME_X64_BOOT_DIM;
        if(Index==Stage)
            Color=RME_X64_BOOT_ACTIVE;

        __RME_X64_Boot_Text(X,55,1,(Index>Stage)?RME_X64_BOOT_LABEL:Color,Label[Index]);
        __RME_X64_Boot_Rect(X,66,Segment-4U,3,Color);
    }
}
/* End Function:__RME_X64_Boot_Progress *************************************/

/* Function:__RME_X64_Boot_CPUs **********************************************
Description : Show one pixel tile per logical CPU as the BSP starts the APs.
Input       : rme_ptr_t Ready - The number of CPUs that finished initialization.
              rme_ptr_t Total - The number of enabled CPUs in the MADT.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_Boot_CPUs(rme_ptr_t Ready, rme_ptr_t Total)
{
    char Text[]="CPU 000 / 000";
    rme_ptr_t Width=RME_X64_RGB_Console.Boot_Width;
    rme_ptr_t Cols=(Total<=32U)?16U:32U;
    rme_ptr_t X=Width-140U;

    if(Width==0)
        return;

    for(rme_ptr_t Digit=0,Place=100U;Digit<3U;Digit++,Place/=10U)
    {
        Text[4U+Digit]='0'+(Ready/Place)%10U;
        Text[10U+Digit]='0'+(Total/Place)%10U;
    }
    __RME_X64_Boot_Text(X,10,1,RME_X64_BOOT_LABEL,Text);
    for(rme_ptr_t Index=0;Index<Total;Index++)
    {
        rme_ptr_t Color=(Index<Ready)?RME_X64_BOOT_READY:RME_X64_BOOT_DIM;
        if(Index==Ready)
            Color=RME_X64_BOOT_ACTIVE;

        __RME_X64_Boot_Rect(X+(Index%Cols)*4U,21U+(Index/Cols)*4U,3,3,Color);
    }
}
/* End Function:__RME_X64_Boot_CPUs *****************************************/

static void __RME_X64_Framebuffer_Init(const struct multiboot_tag_framebuffer* Framebuffer)
{
    const rme_ptr_t Four_GiB=1ULL<<32;
    rme_ptr_t Size;

    if(Framebuffer==0)
        return;

    RME_DBG_S("\r\nMultiboot2 framebuffer: PA 0x");
    RME_DBG_H(Framebuffer->addr);
    RME_DBG_S(", type ");
    RME_DBG_I(Framebuffer->type);
    RME_DBG_S(", width ");
    RME_DBG_I(Framebuffer->width);
    RME_DBG_S(", height ");
    RME_DBG_I(Framebuffer->height);
    RME_DBG_S(", pitch ");
    RME_DBG_I(Framebuffer->pitch);
    RME_DBG_S(", bpp ");
    RME_DBG_I(Framebuffer->bpp);

    Size=(rme_ptr_t)Framebuffer->pitch*Framebuffer->height;
    if(Framebuffer->addr==0 || Size==0)
        return;

    if(Framebuffer->type==MULTIBOOT_FRAMEBUFFER_RGB)
    {
        const struct multiboot_framebuffer_rgb* RGB=(const void*)(Framebuffer+1);

        if(Framebuffer->tag.size>=sizeof(*Framebuffer)+sizeof(*RGB))
        {
            RME_DBG_S(", RGB positions ");
            RME_DBG_I(RGB->red_position);
            RME_DBG_S("/");
            RME_DBG_I(RGB->green_position);
            RME_DBG_S("/");
            RME_DBG_I(RGB->blue_position);
            __RME_X64_RGB_Init(Framebuffer);
        }
        if(RME_X64_RGB_Console.Buffer==0)
            RME_DBG_S("\r\nUnable to initialize RGB framebuffer");
        return;
    }

    /* The debug character console writes EGA text cells. */
    if(Framebuffer->addr>=Four_GiB || Size>Four_GiB-Framebuffer->addr ||
       Framebuffer->type!=MULTIBOOT_FRAMEBUFFER_EGA_TEXT ||
       Framebuffer->bpp!=16U || Framebuffer->width==0 ||
       Framebuffer->pitch<(rme_ptr_t)Framebuffer->width*sizeof(*vga_buffer) ||
       ((Framebuffer->addr|Framebuffer->pitch)&1U)!=0)
        return;

    vga_buffer=(volatile rme_u16_t*)RME_X64_PA2VA(Framebuffer->addr);
    vga_width=Framebuffer->width;
    vga_height=Framebuffer->height;
    vga_stride=Framebuffer->pitch/sizeof(*vga_buffer);
}

/* Function:__RME_X64_Boot_Message *******************************************
Description : Show a boot checkpoint on the RGB console.
Input       : const char* Message - The text to display.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_Boot_Message(const char* Message)
{
    if(RME_X64_RGB_Console.Buffer==0)
        return;

    while(*Message!='\0')
        __RME_X64_RGB_Putchar((rme_u8_t)*Message++);
    __RME_X64_RGB_Putchar('\n');
}
/* End Function:__RME_X64_Boot_Message ***************************************/

/* Function: __RME_Kfn_Handler ************************************************
Description : Display a user-mode debug character. Sub-ID 1 also sends it to COM1.
******************************************************************************/
rme_ret_t __RME_Kfn_Handler(struct RME_Cap_Cpt* Cpt, struct RME_Reg_Struct* Reg,rme_ptr_t FuncID, rme_ptr_t SubID,rme_ptr_t Param1, rme_ptr_t Param2)
{
    char Char=(char)Param1;
    rme_ret_t Result=0;

    if(RME_X64_RGB_Console.Buffer!=0)
        __RME_X64_RGB_Putchar((rme_u8_t)Char);
    else if(vga_buffer!=0 && Char!='\r')
    {
        if(vga_row>=vga_height)
            Result=-1;
        else
        {
            if(Char!='\n')
            {
                vga_buffer[vga_row*vga_stride+vga_col]=(0x1F<<8)|(rme_u8_t)Char;
                vga_col++;
            }

            if(Char=='\n' || vga_col==vga_width)
            {
                vga_col=0;
                vga_row++;
            }
        }
    }

    /* Older UVM images using sub-ID 0 already write to COM1. */
    if(SubID==1U)
        __RME_Putchar(Char);

    return Result;
}
/* End Function: __RME_Kfn_Handler ********************************************/

/* Function:__RME_Putchar *****************************************************
Description : Send a character to COM1 when the UART is ready.
Input       : char Char - The character to print.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Putchar(char Char)
{
    rme_ptr_t Status;

    if(RME_X64_UART_Exist==0)
        return 0;

    /* Bound the wait so a stalled UART cannot stop the kernel or the console. */
    for(rme_ptr_t Try=0;Try<4096U;Try++)
    {
        Status=__RME_X64_In(RME_X64_COM1+5);
        if(Status==0xFFU)
            break;
        if((Status&0x20U)!=0)
        {
            __RME_X64_Out(RME_X64_COM1,Char);
            break;
        }
    }

    return 0;
}
/* End Function:__RME_Putchar ************************************************/

/* Function:__RME_X64_UART_Init ***********************************************
Description : Initialize the UART of X64 platform.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_UART_Init(void)
{
    /* Disable interrupts */
    __RME_X64_Out(RME_X64_COM1+1, 0);
    /* Unlock divisor */
    __RME_X64_Out(RME_X64_COM1+3, 0x80);
    /* Set baudrate - on some computer, the hardware only support this reliably */
    __RME_X64_Out(RME_X64_COM1+0, 115200/9600);
    __RME_X64_Out(RME_X64_COM1+1, 0);
    /* Lock divisor, 8 data bits, 1 stop bit, parity off */
    __RME_X64_Out(RME_X64_COM1+3, 0x03);
    /* Turn on the FIFO */
    __RME_X64_Out(RME_X64_COM1+2, 0xC7);
    /* Turn off all model control, fully asynchronous */
    __RME_X64_Out(RME_X64_COM1+4, 0);

    /* If status is 0xFF, no serial port */
    if(__RME_X64_In(RME_X64_COM1+5)==0xFF)
        RME_X64_UART_Exist=0;
    else
        RME_X64_UART_Exist=1;
}
/* End Function:__RME_X64_UART_Init ******************************************/

/* The two RSDP formats share the first 20 bytes. */
#define RME_X64_RSDP_V1_SIZE 20U

/* Function:__RME_X64_ACPI_Checksum ********************************************
Description : Calculate the sum of the bytes in an ACPI structure modulo 256.
Input       : const void* Table - The address of the ACPI structure.
              rme_ptr_t Size - The number of bytes to include in the sum.
Output      : None.
Return      : rme_ptr_t - The checksum. Zero indicates a valid checksum.
******************************************************************************/

static rme_ptr_t __RME_X64_ACPI_Checksum(const void* Table, rme_ptr_t Size)
{
    const rme_u8_t* Bytes=Table;
    rme_u8_t Sum=0;

    for(rme_ptr_t Index=0;Index<Size;Index++)
        Sum+=Bytes[Index];

    return Sum;
}

/* End Function:__RME_X64_ACPI_Checksum ******************************************/

/* Function:__RME_X64_RSDP_Validate *******************************************
Description : Validate the RSDP signature and the checksum of its first 20 bytes.
              When requested, also check the ACPI revision, the declared length,
              and the checksum of the full RSDP.
Input       : const void* Data - The address of the RSDP data.
              rme_ptr_t Size - The number of bytes available at Data.
              rme_ptr_t Extended - Nonzero to require the ACPI 2.0 or later format.
Output      : None.
Return      : const struct RME_X64_ACPI_RDSP_Desc* - The RSDP pointer if valid, or null.
******************************************************************************/

static const struct RME_X64_ACPI_RDSP_Desc* __RME_X64_RSDP_Validate(
    const void* Data, rme_ptr_t Size, rme_ptr_t Extended)
{
    const struct RME_X64_ACPI_RDSP_Desc* RSDP=Data;

    if(Size<RME_X64_RSDP_V1_SIZE || _RME_Memcmp(RSDP->Signature,"RSD PTR ",8)!=0 ||
       __RME_X64_ACPI_Checksum(RSDP,RME_X64_RSDP_V1_SIZE)!=0)
        return 0;

    if(Extended!=0 &&
       (Size<sizeof(*RSDP) || RSDP->Revision<2 || RSDP->Length<sizeof(*RSDP) ||
        RSDP->Length>Size || __RME_X64_ACPI_Checksum(RSDP,RSDP->Length)!=0))
        return 0;

    return RSDP;
}

/* End Function:RME_X64_ACPI_RDSP_Desc ******************************************/

/* Function:__RME_X64_RSDP_Scan ***********************************************
Description : Scan for a valid RSDP structure in the given physical memory segment.
Input       : rme_ptr_t Base - The base address of the physical memory segment.
              rme_ptr_t Base - The length of the memory segment.
Output      : None.
Return      : struct RME_X64_ACPI_RSDP_Desc* - The descriptor physical address.
******************************************************************************/

const struct RME_X64_ACPI_RDSP_Desc* __RME_X64_RSDP_Scan(rme_ptr_t Base,
                                                        rme_ptr_t Size)
{
    const struct RME_X64_ACPI_RDSP_Desc* RSDP;

    for(rme_ptr_t Offset=0;Offset+RME_X64_RSDP_V1_SIZE<=Size;Offset+=16)
    {
        RSDP=__RME_X64_RSDP_Validate((const void*)RME_X64_PA2VA(Base+Offset),Size-Offset,0);
        if(RSDP!=0)
            return RSDP;
    }

    return 0;
}
/* End Function:__RME_X64_RSDP_Scan ******************************************/

/* Function:__RME_X64_RSDP_Find ***********************************************
Description : Find a valid RSDP structure and return it.
Input       : None.
Output      : None.
Return      : struct RME_X64_ACPI_RSDP_Desc* - The descriptor address.
******************************************************************************/

const struct RME_X64_ACPI_RDSP_Desc* __RME_X64_RSDP_Find(void)
{
    const struct RME_X64_ACPI_RDSP_Desc* RSDP;
    /* The BIOS Data Area stores the EBDA segment at physical 0x40E. */
    rme_ptr_t Base=(rme_ptr_t)*(const rme_u16_t*)RME_X64_PA2VA(0x40E)<<4;

    if(Base!=0)
    {
        RSDP=__RME_X64_RSDP_Scan(Base,1024);
        if(RSDP!=0)
            return RSDP;
    }

    return __RME_X64_RSDP_Scan(0xE0000,0x20000);
}

/* End Function:__RME_X64_RSDP_Find ******************************************/

/* Function:__RME_X64_ACPI_Map ************************************************
Description : Map a physical ACPI table range for early boot access. Reuse the
              direct mapping if the entire range is below 4 GiB. Otherwise use
              the selected temporary window.
Input       : rme_ptr_t Phys - The physical address of the first byte.
              rme_ptr_t Size - The number of bytes to map.
              rme_ptr_t Window - The temporary window index, 0 or 1.
Output      : None.
Return      : const void* - The virtual address of the first byte, or null if
              the range cannot be mapped. A temporary mapping is valid until
              the window is remapped or the bootstrap page tables are replaced.
******************************************************************************/

static const void* __RME_X64_ACPI_Map(rme_ptr_t Phys, rme_ptr_t Size, rme_ptr_t Window)
{
    const rme_ptr_t Four_GiB=1ULL<<32;

    if(Phys==0 || Size==0)
        return 0;
    if(Phys<Four_GiB && Size<=Four_GiB-Phys)
        return (const void*)RME_X64_PA2VA(Phys);

    return (const void*)__RME_X64_Boot_Window_Map(Phys,Size,Window,0);
}

/* Function:__RME_X64_ACPI_Table **********************************************
Description : Map the ACPI table header to read its length, then map the full
              table and validate its checksum.
Input       : rme_ptr_t Phys - The physical address of the ACPI table.
              rme_ptr_t Window - The temporary mapping window index, 0 or 1.
Output      : None.
Return      : const struct RME_X64_ACPI_Desc_Hdr* - The mapped table header,
              or null if the mapping, table length, or checksum is invalid.
******************************************************************************/

static const struct RME_X64_ACPI_Desc_Hdr* __RME_X64_ACPI_Table(rme_ptr_t Phys,
                                                               rme_ptr_t Window)
{
    const struct RME_X64_ACPI_Desc_Hdr* Header;
    rme_ptr_t Size;

    Header=__RME_X64_ACPI_Map(Phys,sizeof(*Header),Window);
    if(Header==0 || Header->Length<sizeof(*Header))
        return 0;

    Size=Header->Length;
    Header=__RME_X64_ACPI_Map(Phys,Size,Window);
    if(Header==0 || __RME_X64_ACPI_Checksum(Header,Size)!=0)
        return 0;

    return Header;
}

/* End Function:__RME_X64_ACPI_Table  ******************************************/

/* Function:__RME_X64_ACPI_Error *********************************************
Description : Report an ACPI error on the boot console and debug port.
Input       : const char* Message - The error message.
Output      : None.
Return      : rme_ret_t - Always -1.
******************************************************************************/
rme_ret_t __RME_X64_ACPI_Error(const char* Message)
{
    __RME_X64_Boot_Message(Message);
    RME_DBG_S("\r\n");
    RME_DBG_S(Message);
    return -1;
}
/* End Function:__RME_X64_ACPI_Error *****************************************/

/* Function:__RME_X64_SMP_Detect **********************************************
Description : Detect the SMP configuration in the system and set up the per-CPU info.
Input       : const struct RME_X64_ACPI_MADT_Hdr* MADT - The pointer to the MADT header.
Output      : None.
Return      : rme_ret_t - If successful, 0; else -1.
******************************************************************************/
rme_ret_t __RME_X64_SMP_Detect(const struct RME_X64_ACPI_MADT_Hdr* MADT)
{
    const struct RME_X64_ACPI_MADT_LAPIC_Record* LAPIC;
    const struct RME_X64_ACPI_MADT_X2APIC_Record* X2APIC;
    const struct RME_X64_ACPI_MADT_IOAPIC_Record* IOAPIC;
    const struct RME_X64_ACPI_MADT_SRC_OVERRIDE_Record* OVERRIDE;
    rme_ptr_t Length;
    rme_ptr_t Count;
    rme_u32_t ID;
    rme_u32_t Flags;
    rme_ptr_t X2APIC_Count=0;
    rme_ptr_t Need_X2APIC;
    const rme_u8_t* Ptr;
    const rme_u8_t* End;

    if(MADT->Header.Length<sizeof(*MADT))
        return __RME_X64_ACPI_Error("ACPI: Invalid MADT header");

    RME_X64_LAPIC_Addr=MADT->LAPIC_Addr_Phys;
    Need_X2APIC=__RME_X64_Read_MSR(RME_X64_MSR_IA32_APIC_BASE)&RME_X64_APIC_BASE_X2APIC;

    /* Where does the actual table contents start? */
    Ptr=MADT->Table;
    /* Where does it end? */
    End=Ptr+MADT->Header.Length-sizeof(struct RME_X64_ACPI_MADT_Hdr);

    RME_X64_Num_IOAPIC=0;
    RME_X64_Num_CPU=0;
    while(Ptr<End)
    {
        /* See if we have finished scanning the table */
        if((End-Ptr)<2)
            return __RME_X64_ACPI_Error("ACPI: Truncated MADT record");
        Length=Ptr[1];
        if(Length<2 || Length>(rme_ptr_t)(End-Ptr))
            return __RME_X64_ACPI_Error("ACPI: Invalid MADT record length");

        /* See what is in the table */
        switch(Ptr[0])
        {
            case RME_X64_MADT_LAPIC:
            case RME_X64_MADT_X2APIC:
            {
                if(Ptr[0]==RME_X64_MADT_LAPIC)
                {
                    if(Length<sizeof(*LAPIC))
                        return __RME_X64_ACPI_Error("ACPI: Invalid LAPIC record");
                    LAPIC=(const struct RME_X64_ACPI_MADT_LAPIC_Record*)Ptr;
                    ID=LAPIC->APIC_ID;
                    Flags=LAPIC->Flags;
                }
                else
                {
                    if(Length<sizeof(*X2APIC))
                        return __RME_X64_ACPI_Error("ACPI: Invalid x2APIC record");
                    X2APIC=(const struct RME_X64_ACPI_MADT_X2APIC_Record*)Ptr;
                    ID=X2APIC->APIC_ID;
                    Flags=X2APIC->Flags;
                }

                if((Flags&RME_X64_APIC_LAPIC_ENABLED)==0)
                    break;

                if(ID==0xFFFFFFFFU ||
                   (Ptr[0]==RME_X64_MADT_LAPIC && ID==RME_X64_XAPIC_BCAST_ID))
                    return __RME_X64_ACPI_Error("ACPI: Reserved APIC ID");
                if(RME_X64_Num_CPU>=RME_X64_CPU_NUM)
                    return __RME_X64_ACPI_Error("ACPI: Too many enabled CPUs");
                for(Count=0;Count<RME_X64_Num_CPU;Count++)
                {
                    if(RME_X64_CPU_Info[Count].LAPIC_ID==ID)
                        return __RME_X64_ACPI_Error("ACPI: Duplicate APIC ID");
                }

                if(Ptr[0]==RME_X64_MADT_X2APIC)
                    X2APIC_Count++;
                if(ID>=RME_X64_XAPIC_BCAST_ID)
                    Need_X2APIC=1;

                RME_DBG_S("\n\rACPI: CPU ");
                RME_DBG_I((rme_cnt_t)RME_X64_Num_CPU);
                if(Ptr[0]==RME_X64_MADT_X2APIC)
                    RME_DBG_S(", x2APIC ID ");
                else
                    RME_DBG_S(", LAPIC ID ");
                RME_DBG_I(ID);

                RME_X64_CPU_Info[RME_X64_Num_CPU].LAPIC_ID=ID;
                RME_X64_CPU_Info[RME_X64_Num_CPU].Boot_Done=0;
                RME_X64_Num_CPU++;
                break;
            }
            /* This is an IOAPIC */
            case RME_X64_MADT_IOAPIC:
            {
                IOAPIC=(const struct RME_X64_ACPI_MADT_IOAPIC_Record*)Ptr;
                /* Is the length correct? */
                if(Length<sizeof(struct RME_X64_ACPI_MADT_IOAPIC_Record))
                    break;

                RME_DBG_S("\n\rACPI: IOAPIC ");
                RME_DBG_I((rme_cnt_t)RME_X64_Num_IOAPIC);
                RME_DBG_S(" @ ");
                RME_DBG_H(IOAPIC->Addr);
                RME_DBG_S(", ID ");
                RME_DBG_I(IOAPIC->ID);
                RME_DBG_S(", IBASE ");
                RME_DBG_I(IOAPIC->Interrupt_Base);

                /* Support multiple APICS */
                if(RME_X64_Num_IOAPIC!=0)
                {
                    RME_DBG_S("Warning: multiple ioapics are not supported - currently we will not initialize IOAPIC > 1\n");
                }
                else
                {
                    RME_X64_IOAPIC_Info[RME_X64_Num_IOAPIC].IOAPIC_ID=IOAPIC->ID;
                }

                if(RME_X64_Num_IOAPIC>=RME_X64_IOAPIC_NUM)
                    return __RME_X64_ACPI_Error("ACPI: Too many I/O APICs");
                RME_X64_Num_IOAPIC++;
                break;
            }
            /* This is interrupt override information */
            case RME_X64_MADT_INT_SRC_OVERRIDE:
            {
                OVERRIDE=(const struct RME_X64_ACPI_MADT_SRC_OVERRIDE_Record*)Ptr;
                if(Length<sizeof(struct RME_X64_ACPI_MADT_SRC_OVERRIDE_Record))
                    break;
                RME_DBG_S("\n\rACPI: OVERRIDE Bus ");
                RME_DBG_I(OVERRIDE->Bus);
                RME_DBG_S(", Source ");
                RME_DBG_H(OVERRIDE->Source);
                RME_DBG_S(", GSI ");
                RME_DBG_I(OVERRIDE->GS_Interrupt);
                RME_DBG_S(", Flags ");
                RME_DBG_I(OVERRIDE->MPS_Int_Flags);

                break;
            }
            /* All other types are ignored */
            default:break;
        }
        
        Ptr+=Length;
    }

    if(RME_X64_Num_CPU==0)
        return __RME_X64_ACPI_Error("ACPI: MADT has no enabled CPU");
    if(RME_X64_Num_IOAPIC==0)
        return __RME_X64_ACPI_Error("ACPI: MADT has no I/O APIC");
    if(Need_X2APIC!=0 &&
       (RME_X64_FUNC(RME_X64_CPUID_1_INFO_FEATURE,2)&RME_X64_1_ECX_X2APIC)==0)
        return __RME_X64_ACPI_Error("ACPI: CPU does not support x2APIC");
    if(Need_X2APIC==0 && (RME_X64_LAPIC_Addr==0 || (RME_X64_LAPIC_Addr&0xFFFU)!=0))
        return __RME_X64_ACPI_Error("ACPI: Invalid LAPIC address");

    RME_X64_X2APIC=(Need_X2APIC!=0);
    if(X2APIC_Count!=0)
        __RME_X64_Boot_Message("ACPI: MADT has x2APIC CPUs");

    RME_DBG_S("\r\nACPI: Enabled CPUs ");
    RME_DBG_I(RME_X64_Num_CPU);
    RME_DBG_S(", x2APIC entries ");
    RME_DBG_I(X2APIC_Count);
    return 0;
}
/* End Function:__RME_X64_SMP_Detect *****************************************/

/* Function:__RME_X64_ACPI_Debug **********************************************
Description : Print the information about the ACPI table entry.
Input       : const struct RME_X64_ACPI_Desc_Hdr* Header - The table header.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_ACPI_Debug(const struct RME_X64_ACPI_Desc_Hdr* Header)
{
    rme_u8_t Signature[5];
    rme_u8_t ID[7];
    rme_u8_t Table_ID[9];
    rme_u8_t Creator[5];
    rme_ptr_t OEM_Rev;
    rme_ptr_t Creator_Rev;

    /* Copy everything into our buffer */
    __builtin_memcpy(Signature, Header->Signature, 4);
    Signature[4]='\0';
    __builtin_memcpy(ID, Header->OEM_ID, 6);
    ID[6]='\0';
    __builtin_memcpy(Table_ID, Header->OEM_Table_ID, 8);
    Table_ID[8]='\0';
    __builtin_memcpy(Creator, Header->Creator_ID, 4);
    Creator[4]='\0';

    OEM_Rev=Header->OEM_Revision;
    Creator_Rev=Header->Creator_Revision;

    /* And print these entries */
    RME_DBG_S("\n\rACPI:");
    RME_DBG_S(Signature);
    RME_DBG_S(", ");
    RME_DBG_S(ID);
    RME_DBG_S(", ");
    RME_DBG_S(Table_ID);
    RME_DBG_S(", ");
    RME_DBG_I(OEM_Rev);
    RME_DBG_S(", ");
    RME_DBG_S(Creator);
    RME_DBG_S(", ");
    RME_DBG_I(Creator_Rev);
    RME_DBG_S(".");
}
/* End Function:__RME_X64_ACPI_Debug *****************************************/

/* Function:__RME_X64_ACPI_Root ***********************************************
Description : Validate the RSDT or XSDT and walk its table entries. Find the
              MADT and use it to detect the CPU and APIC configuration.
Input       : rme_ptr_t Phys - The physical address of the root table.
              rme_ptr_t Entry_Size - The entry size in bytes, 4 for RSDT or
                                    8 for XSDT.
              const char* Signature - The expected root table signature,
                                     "RSDT" or "XSDT".
Output      : None.
Return      : rme_ret_t - 0 if MADT parsing succeeds, or -1 if the root table
                          is invalid, no valid MADT is found, or parsing fails.
******************************************************************************/

static rme_ret_t __RME_X64_ACPI_Root(rme_ptr_t Phys, rme_ptr_t Entry_Size,
                                      const char* Signature)
{
    const struct RME_X64_ACPI_Desc_Hdr* Root=__RME_X64_ACPI_Table(Phys,0);
    const struct RME_X64_ACPI_Desc_Hdr* Header;
    rme_ptr_t Child;

    if(Phys==0)
        return -1;
    if(Root==0 || _RME_Memcmp(Root->Signature,Signature,4)!=0 ||
       (Root->Length-sizeof(*Root))%Entry_Size!=0)
        return __RME_X64_ACPI_Error((Entry_Size==8)?"ACPI: Invalid XSDT":"ACPI: Invalid RSDT");

    RME_DBG_S("\r\nACPI root ");
    RME_DBG_S(Signature);
    RME_DBG_S(" PA 0x");
    RME_DBG_H(Phys);

    for(rme_ptr_t Offset=sizeof(*Root);Offset<Root->Length;Offset+=Entry_Size)
    {
        if(Entry_Size==8)
            __builtin_memcpy(&Child,(const rme_u8_t*)Root+Offset,8);
        else
        {
            rme_u32_t Child32;
            __builtin_memcpy(&Child32,(const rme_u8_t*)Root+Offset,4);
            Child=Child32;
        }
        Header=__RME_X64_ACPI_Table(Child,1);
        if(Header==0)
            continue;

        __RME_X64_ACPI_Debug(Header);
        if(_RME_Memcmp(Header->Signature,"APIC",4)==0)
            return __RME_X64_SMP_Detect((const struct RME_X64_ACPI_MADT_Hdr*)Header);
    }

    return __RME_X64_ACPI_Error("ACPI: No valid MADT in root table");
}
/* End Function:__RME_X64_ACPI_Root *****************************************/

/* Function:__RME_X64_ACPI_Init ***********************************************
Description : Find and validate the RSDP, then parse the MADT to detect the
              CPU and APIC configuration. Try Multiboot2 tag 15 first,
              using the XSDT and then the RSDT. If that fails, try tag 14
              and its RSDT. On BIOS boots, fall back to scanning for the RSDP.
Input       : const struct RME_X64_Boot_Info* Boot - The parsed boot information.
Output      : None.
Return      : rme_ret_t - 0 if MADT parsing succeeds, or -1 if all attempts fail.
******************************************************************************/

rme_ret_t __RME_X64_ACPI_Init(const struct RME_X64_Boot_Info* Boot)
{
    const struct RME_X64_ACPI_RDSP_Desc* RSDP;
    const struct multiboot_tag* Tag=Boot->ACPI_New;

    if(Tag!=0)
    {
        RSDP=__RME_X64_RSDP_Validate(Tag+1,Tag->size-sizeof(*Tag),1);
        if(RSDP!=0)
        {
            RME_DBG_S("\r\nACPI RSDP from Multiboot2 tag 15");
            if(__RME_X64_ACPI_Root(RSDP->XSDT_Addr_Phys,8,"XSDT")==0 ||
               __RME_X64_ACPI_Root(RSDP->RSDT_Addr_Phys,4,"RSDT")==0)
                return 0;
        }
        else
            __RME_X64_ACPI_Error("ACPI: Invalid RSDP in tag 15");
    }

    Tag=Boot->ACPI_Old;
    if(Tag!=0)
    {
        RSDP=__RME_X64_RSDP_Validate(Tag+1,Tag->size-sizeof(*Tag),0);
        if(RSDP!=0)
        {
            RME_DBG_S("\r\nACPI RSDP from Multiboot2 tag 14");
            if(__RME_X64_ACPI_Root(RSDP->RSDT_Addr_Phys,4,"RSDT")==0)
                return 0;
        }
        else
            __RME_X64_ACPI_Error("ACPI: Invalid RSDP in tag 14");
    }

    if(Boot->ACPI_New==0 && Boot->ACPI_Old==0)
        __RME_X64_ACPI_Error("ACPI: No Multiboot2 ACPI tag");

    if(Boot->EFI==0)
    {
        RSDP=__RME_X64_RSDP_Find();
        if(RSDP!=0)
        {
            RME_DBG_S("\r\nACPI RSDP from BIOS scan");
            return __RME_X64_ACPI_Root(RSDP->RSDT_Addr_Phys,4,"RSDT");
        }
    }

    return -1;
}

/* End Function:__RME_X64_ACPI_Init  *****************************************/

/* Function:__RME_X64_Feature_Get *********************************************
Description : Use the CPUID instruction extensively to get all the processor
              information. We assume that all processors installed have the same
              features.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_Feature_Get(void)
{
    rme_ptr_t Count;

    /* What's the maximum feature? */
    RME_X64_Feature.Max_Func=__RME_X64_CPUID_Get(RME_X64_CPUID_0_VENDOR_ID,
                                                 (rme_ptr_t*)&(RME_X64_Feature.Func[0][1]),
                                                 (rme_ptr_t*)&(RME_X64_Feature.Func[0][2]),
                                                 (rme_ptr_t*)&(RME_X64_Feature.Func[0][3]));
    RME_X64_Feature.Func[0][0]=RME_X64_Feature.Max_Func;

    /* Get all the feature bits */
    for(Count=1;Count<=RME_X64_Feature.Max_Func &&
                Count<sizeof(RME_X64_Feature.Func)/sizeof(RME_X64_Feature.Func[0]);Count++)
    {
        RME_X64_Feature.Func[Count][0]=__RME_X64_CPUID_Get(Count,
                                                           (rme_ptr_t*)&(RME_X64_Feature.Func[Count][1]),
                                                           (rme_ptr_t*)&(RME_X64_Feature.Func[Count][2]),
                                                           (rme_ptr_t*)&(RME_X64_Feature.Func[Count][3]));
    }

    /* What's the maximum extended feature? */
    RME_X64_Feature.Max_Ext=__RME_X64_CPUID_Get(RME_X64_CPUID_E0_EXT_MAX,
                                                (rme_ptr_t*)&(RME_X64_Feature.Ext[0][1]),
                                                (rme_ptr_t*)&(RME_X64_Feature.Ext[0][2]),
                                                (rme_ptr_t*)&(RME_X64_Feature.Ext[0][3]));
    RME_X64_Feature.Ext[0][0]=RME_X64_Feature.Max_Ext;


    /* Get all the feature bits */
    for(Count=1;RME_X64_CPUID_E0_EXT_MAX+Count<=RME_X64_Feature.Max_Ext &&
                Count<sizeof(RME_X64_Feature.Ext)/sizeof(RME_X64_Feature.Ext[0]);Count++)
    {
        RME_X64_Feature.Ext[Count][0]=__RME_X64_CPUID_Get(RME_X64_CPUID_E0_EXT_MAX|Count,
                                                          (rme_ptr_t*)&(RME_X64_Feature.Ext[Count][1]),
                                                          (rme_ptr_t*)&(RME_X64_Feature.Ext[Count][2]),
                                                          (rme_ptr_t*)&(RME_X64_Feature.Ext[Count][3]));
    }

    /* TODO: Check these flags. If not satisfied, we hang immediately. */
}
/* End Function:__RME_X64_Feature_Get ****************************************/

/* Function:__RME_X64_Mem_Init ************************************************
Description : Initialize the memory map, and get the size of kernel object
              allocation registration table(Kot).
Input       : const struct multiboot_tag_mmap* Map - Multiboot2 memory map tag.
Output      : None.
Return      : None.
******************************************************************************/
/* We place this here because these are never exported, and are local to this
 * file. This is a little workaround for the header inclusion problem */
struct __RME_X64_Mem
{
    struct RME_List Head;
    rme_ptr_t Start_Addr;
    rme_ptr_t Length;
};
/* The header of the physical memory linked list */
volatile struct RME_List RME_X64_Phys_Mem;
/* The BIOS wouldn't really report more than 1024 blocks of memory */
struct __RME_X64_Mem RME_X64_Mem[1024];

static rme_ret_t __RME_X64_Mem_Init(const struct multiboot_tag_mmap* Map)
{
    const struct multiboot_mmap_entry* Entry;
    volatile struct RME_List* Trav_Ptr;
    rme_ptr_t Size=Map->tag.size-sizeof(*Map);
    rme_ptr_t MMap_Cnt;
    rme_ptr_t Info_Cnt=0;

    if(Map->entry_size<sizeof(*Entry) || Map->entry_version!=0 ||
       Size%Map->entry_size!=0)
        return -1;

    __RME_List_Crt((struct RME_List*)&RME_X64_Phys_Mem);

    for(rme_ptr_t Offset=0;Offset<Size;Offset+=Map->entry_size)
    {
        Entry=(const struct multiboot_mmap_entry*)((const rme_u8_t*)Map->entries+Offset);
        RME_DBG_S("\n\rPhysical memory: 0x");
        RME_DBG_H(Entry->addr);
        RME_DBG_S(", 0x");
        RME_DBG_H(Entry->len);
        RME_DBG_S(", ");
        RME_DBG_H(Entry->type);

        if(Entry->type!=MULTIBOOT_MEMORY_AVAILABLE || Entry->len==0)
            continue;
        if(Entry->addr>~(rme_ptr_t)0-Entry->len ||
           Info_Cnt>=sizeof(RME_X64_Mem)/sizeof(RME_X64_Mem[0]))
            return -1;

        Trav_Ptr=RME_X64_Phys_Mem.Next;
        while(Trav_Ptr!=&RME_X64_Phys_Mem)
        {
            if(((struct __RME_X64_Mem*)Trav_Ptr)->Start_Addr>Entry->addr)
                break;
            Trav_Ptr=Trav_Ptr->Next;
        }
        RME_X64_Mem[Info_Cnt].Start_Addr=Entry->addr;
        RME_X64_Mem[Info_Cnt].Length=Entry->len;
        __RME_List_Ins(&RME_X64_Mem[Info_Cnt].Head,Trav_Ptr->Prev,(struct RME_List*)Trav_Ptr);
        Info_Cnt++;
    }

    /* Merge overlapping or adjacent available ranges. */
    Trav_Ptr=RME_X64_Phys_Mem.Next;
    while(Trav_Ptr!=&RME_X64_Phys_Mem && Trav_Ptr->Next!=&RME_X64_Phys_Mem)
    {
        struct __RME_X64_Mem* Current=(struct __RME_X64_Mem*)Trav_Ptr;
        struct __RME_X64_Mem* Next=(struct __RME_X64_Mem*)Trav_Ptr->Next;
        rme_ptr_t Next_End=Next->Start_Addr+Next->Length;

        if(Current->Start_Addr+Current->Length>=Next->Start_Addr)
        {
            if(Next_End>Current->Start_Addr+Current->Length)
                Current->Length=Next_End-Current->Start_Addr;
            __RME_List_Del(&Current->Head,Next->Head.Next);
            continue;
        }
        Trav_Ptr=Trav_Ptr->Next;
    }

    /* Calculate total memory */
    MMap_Cnt=0;
    Trav_Ptr=RME_X64_Phys_Mem.Next;
    while(Trav_Ptr!=&RME_X64_Phys_Mem)
    {
        MMap_Cnt+=((struct __RME_X64_Mem*)(Trav_Ptr))->Length;
        Trav_Ptr=Trav_Ptr->Next;
    }
    RME_DBG_S("\n\rTotal physical memory: 0x");
    RME_DBG_H(MMap_Cnt);

    /* At least 256MB memory required on x64 architecture */
    if(MMap_Cnt<RME_POW2(RME_PGT_SIZE_256M))
        return -1;

    /* Kernel virtual memory layout */
    RME_X64_Layout.Kot_Start=(rme_ptr_t)RME_KOT_VA_BASE;
    /* +1G in cases where we have > 3GB memory for covering the memory hole */
    Info_Cnt=(MMap_Cnt>3*RME_POW2(RME_PGT_SIZE_1G))?(MMap_Cnt+RME_POW2(RME_PGT_SIZE_1G)):MMap_Cnt;
    RME_X64_Layout.Kot_Size=((Info_Cnt>>RME_KOM_SLOT_ORDER)>>RME_WORD_ORDER)+1;

    /* Calculate the per-CPU data structure size - each CPU have two 4k pages */
    RME_X64_Layout.PerCPU_Start=RME_ROUND_UP(RME_X64_Layout.Kot_Start+RME_X64_Layout.Kot_Size,RME_PGT_SIZE_4K);
    RME_X64_Layout.PerCPU_Size=2*RME_POW2(RME_PGT_SIZE_4K)*RME_X64_Num_CPU;

    /* Now decide the size of the stack */
    RME_X64_Layout.Stack_Size=RME_X64_Num_CPU<<RME_X64_KSTACK_ORDER;
    return 0;
}
/* End Function:__RME_X64_Mem_Init *******************************************/

/* Function:__RME_X64_CPU_Local_Init ******************************************
Description : Initialize CPU-local data structures. The layout of each CPU-local
              data structure is:
              |       4kB      |    1kB    |  3kB-3*8Bytes  |   3*8Bytes   |
              |    IDT[255:0]  |  GDT/TSS  |  RME_CPU_Local | RME_X64_Temp |
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_CPU_Local_Init(void)
{
    volatile rme_u16_t Desc[5];
    struct RME_X64_IDT_Entry* IDT_Table;
    struct RME_X64_Temp* Temp;
    struct RME_CPU_Local* CPU_Local;
    rme_ptr_t* GDT_Table;
    rme_ptr_t TSS_Table;
    rme_cnt_t Count;

    IDT_Table=(struct RME_X64_IDT_Entry*)RME_X64_CPU_LOCAL_BASE(RME_X64_CPU_Cnt);
    /* Clean up the whole IDT */
    for(Count=0;Count<256;Count++)
        IDT_Table[Count].Type_Attr=0;
    /* Install the vectors - only the INT3 is trap (for debugging), all other ones are interrupt */
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_DE, RME_X64_IDT_VECT, __RME_X64_FAULT_DE_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_TRAP_DB, RME_X64_IDT_VECT, __RME_X64_TRAP_DB_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_INT_NMI, RME_X64_IDT_VECT, __RME_X64_INT_NMI_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_TRAP_BP, RME_X64_IDT_TRAP, __RME_X64_TRAP_BP_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_TRAP_OF, RME_X64_IDT_VECT, __RME_X64_TRAP_OF_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_BR, RME_X64_IDT_VECT, __RME_X64_FAULT_BR_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_UD, RME_X64_IDT_VECT, __RME_X64_FAULT_UD_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_NM, RME_X64_IDT_VECT, __RME_X64_FAULT_NM_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_ABORT_DF, RME_X64_IDT_VECT, __RME_X64_ABORT_DF_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_ABORT_OLD_MF, RME_X64_IDT_VECT, __RME_X64_ABORT_OLD_MF_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_TS, RME_X64_IDT_VECT, __RME_X64_FAULT_TS_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_NP, RME_X64_IDT_VECT, __RME_X64_FAULT_NP_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_SS, RME_X64_IDT_VECT, __RME_X64_FAULT_SS_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_GP, RME_X64_IDT_VECT, __RME_X64_FAULT_GP_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_PF, RME_X64_IDT_VECT, __RME_X64_FAULT_PF_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_MF, RME_X64_IDT_VECT, __RME_X64_FAULT_MF_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_AC, RME_X64_IDT_VECT, __RME_X64_FAULT_AC_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_ABORT_MC, RME_X64_IDT_VECT, __RME_X64_ABORT_MC_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_XM, RME_X64_IDT_VECT, __RME_X64_FAULT_XM_Handler);
    RME_X64_SET_IDT(IDT_Table, RME_X64_FAULT_VE, RME_X64_IDT_VECT, __RME_X64_FAULT_VE_Handler);
    /* Install user handlers */
    RME_X64_USER_IDT(IDT_Table, 32); RME_X64_USER_IDT(IDT_Table, 33);
    RME_X64_USER_IDT(IDT_Table, 34); RME_X64_USER_IDT(IDT_Table, 35);
    RME_X64_USER_IDT(IDT_Table, 36); RME_X64_USER_IDT(IDT_Table, 37);
    RME_X64_USER_IDT(IDT_Table, 38); RME_X64_USER_IDT(IDT_Table, 39);

    RME_X64_USER_IDT(IDT_Table, 40); RME_X64_USER_IDT(IDT_Table, 41);
    RME_X64_USER_IDT(IDT_Table, 42); RME_X64_USER_IDT(IDT_Table, 43);
    RME_X64_USER_IDT(IDT_Table, 44); RME_X64_USER_IDT(IDT_Table, 45);
    RME_X64_USER_IDT(IDT_Table, 46); RME_X64_USER_IDT(IDT_Table, 47);
    RME_X64_USER_IDT(IDT_Table, 48); RME_X64_USER_IDT(IDT_Table, 49);

    RME_X64_USER_IDT(IDT_Table, 50); RME_X64_USER_IDT(IDT_Table, 51);
    RME_X64_USER_IDT(IDT_Table, 52); RME_X64_USER_IDT(IDT_Table, 53);
    RME_X64_USER_IDT(IDT_Table, 54); RME_X64_USER_IDT(IDT_Table, 55);
    RME_X64_USER_IDT(IDT_Table, 56); RME_X64_USER_IDT(IDT_Table, 57);
    RME_X64_USER_IDT(IDT_Table, 58); RME_X64_USER_IDT(IDT_Table, 59);

    RME_X64_USER_IDT(IDT_Table, 60); RME_X64_USER_IDT(IDT_Table, 61);
    RME_X64_USER_IDT(IDT_Table, 62); RME_X64_USER_IDT(IDT_Table, 63);
    RME_X64_USER_IDT(IDT_Table, 64); RME_X64_USER_IDT(IDT_Table, 65);
    RME_X64_USER_IDT(IDT_Table, 66); RME_X64_USER_IDT(IDT_Table, 67);
    RME_X64_USER_IDT(IDT_Table, 68); RME_X64_USER_IDT(IDT_Table, 69);

    RME_X64_USER_IDT(IDT_Table, 70); RME_X64_USER_IDT(IDT_Table, 71);
    RME_X64_USER_IDT(IDT_Table, 72); RME_X64_USER_IDT(IDT_Table, 73);
    RME_X64_USER_IDT(IDT_Table, 74); RME_X64_USER_IDT(IDT_Table, 75);
    RME_X64_USER_IDT(IDT_Table, 76); RME_X64_USER_IDT(IDT_Table, 77);
    RME_X64_USER_IDT(IDT_Table, 78); RME_X64_USER_IDT(IDT_Table, 79);

    RME_X64_USER_IDT(IDT_Table, 80); RME_X64_USER_IDT(IDT_Table, 81);
    RME_X64_USER_IDT(IDT_Table, 82); RME_X64_USER_IDT(IDT_Table, 83);
    RME_X64_USER_IDT(IDT_Table, 84); RME_X64_USER_IDT(IDT_Table, 85);
    RME_X64_USER_IDT(IDT_Table, 86); RME_X64_USER_IDT(IDT_Table, 87);
    RME_X64_USER_IDT(IDT_Table, 88); RME_X64_USER_IDT(IDT_Table, 89);

    RME_X64_USER_IDT(IDT_Table, 90); RME_X64_USER_IDT(IDT_Table, 91);
    RME_X64_USER_IDT(IDT_Table, 92); RME_X64_USER_IDT(IDT_Table, 93);
    RME_X64_USER_IDT(IDT_Table, 94); RME_X64_USER_IDT(IDT_Table, 95);
    RME_X64_USER_IDT(IDT_Table, 96); RME_X64_USER_IDT(IDT_Table, 97);
    RME_X64_USER_IDT(IDT_Table, 98); RME_X64_USER_IDT(IDT_Table, 99);

    RME_X64_USER_IDT(IDT_Table, 100); RME_X64_USER_IDT(IDT_Table, 101);
    RME_X64_USER_IDT(IDT_Table, 102); RME_X64_USER_IDT(IDT_Table, 103);
    RME_X64_USER_IDT(IDT_Table, 104); RME_X64_USER_IDT(IDT_Table, 105);
    RME_X64_USER_IDT(IDT_Table, 106); RME_X64_USER_IDT(IDT_Table, 107);
    RME_X64_USER_IDT(IDT_Table, 108); RME_X64_USER_IDT(IDT_Table, 109);

    RME_X64_USER_IDT(IDT_Table, 110); RME_X64_USER_IDT(IDT_Table, 111);
    RME_X64_USER_IDT(IDT_Table, 112); RME_X64_USER_IDT(IDT_Table, 113);
    RME_X64_USER_IDT(IDT_Table, 114); RME_X64_USER_IDT(IDT_Table, 115);
    RME_X64_USER_IDT(IDT_Table, 116); RME_X64_USER_IDT(IDT_Table, 117);
    RME_X64_USER_IDT(IDT_Table, 118); RME_X64_USER_IDT(IDT_Table, 119);

    RME_X64_USER_IDT(IDT_Table, 120); RME_X64_USER_IDT(IDT_Table, 121);
    RME_X64_USER_IDT(IDT_Table, 122); RME_X64_USER_IDT(IDT_Table, 123);
    RME_X64_USER_IDT(IDT_Table, 124); RME_X64_USER_IDT(IDT_Table, 125);
    RME_X64_USER_IDT(IDT_Table, 126); RME_X64_USER_IDT(IDT_Table, 127);
    RME_X64_USER_IDT(IDT_Table, 128); RME_X64_USER_IDT(IDT_Table, 129);

    RME_X64_USER_IDT(IDT_Table, 130); RME_X64_USER_IDT(IDT_Table, 131);
    RME_X64_USER_IDT(IDT_Table, 132); RME_X64_USER_IDT(IDT_Table, 133);
    RME_X64_USER_IDT(IDT_Table, 134); RME_X64_USER_IDT(IDT_Table, 135);
    RME_X64_USER_IDT(IDT_Table, 136); RME_X64_USER_IDT(IDT_Table, 137);
    RME_X64_USER_IDT(IDT_Table, 138); RME_X64_USER_IDT(IDT_Table, 139);

    RME_X64_USER_IDT(IDT_Table, 140); RME_X64_USER_IDT(IDT_Table, 141);
    RME_X64_USER_IDT(IDT_Table, 142); RME_X64_USER_IDT(IDT_Table, 143);
    RME_X64_USER_IDT(IDT_Table, 144); RME_X64_USER_IDT(IDT_Table, 145);
    RME_X64_USER_IDT(IDT_Table, 146); RME_X64_USER_IDT(IDT_Table, 147);
    RME_X64_USER_IDT(IDT_Table, 148); RME_X64_USER_IDT(IDT_Table, 149);

    RME_X64_USER_IDT(IDT_Table, 150); RME_X64_USER_IDT(IDT_Table, 151);
    RME_X64_USER_IDT(IDT_Table, 152); RME_X64_USER_IDT(IDT_Table, 153);
    RME_X64_USER_IDT(IDT_Table, 154); RME_X64_USER_IDT(IDT_Table, 155);
    RME_X64_USER_IDT(IDT_Table, 156); RME_X64_USER_IDT(IDT_Table, 157);
    RME_X64_USER_IDT(IDT_Table, 158); RME_X64_USER_IDT(IDT_Table, 159);

    RME_X64_USER_IDT(IDT_Table, 160); RME_X64_USER_IDT(IDT_Table, 161);
    RME_X64_USER_IDT(IDT_Table, 162); RME_X64_USER_IDT(IDT_Table, 163);
    RME_X64_USER_IDT(IDT_Table, 164); RME_X64_USER_IDT(IDT_Table, 165);
    RME_X64_USER_IDT(IDT_Table, 166); RME_X64_USER_IDT(IDT_Table, 167);
    RME_X64_USER_IDT(IDT_Table, 168); RME_X64_USER_IDT(IDT_Table, 169);

    RME_X64_USER_IDT(IDT_Table, 170); RME_X64_USER_IDT(IDT_Table, 171);
    RME_X64_USER_IDT(IDT_Table, 172); RME_X64_USER_IDT(IDT_Table, 173);
    RME_X64_USER_IDT(IDT_Table, 174); RME_X64_USER_IDT(IDT_Table, 175);
    RME_X64_USER_IDT(IDT_Table, 176); RME_X64_USER_IDT(IDT_Table, 177);
    RME_X64_USER_IDT(IDT_Table, 178); RME_X64_USER_IDT(IDT_Table, 179);

    RME_X64_USER_IDT(IDT_Table, 180); RME_X64_USER_IDT(IDT_Table, 181);
    RME_X64_USER_IDT(IDT_Table, 182); RME_X64_USER_IDT(IDT_Table, 183);
    RME_X64_USER_IDT(IDT_Table, 184); RME_X64_USER_IDT(IDT_Table, 185);
    RME_X64_USER_IDT(IDT_Table, 186); RME_X64_USER_IDT(IDT_Table, 187);
    RME_X64_USER_IDT(IDT_Table, 188); RME_X64_USER_IDT(IDT_Table, 189);

    RME_X64_USER_IDT(IDT_Table, 190); RME_X64_USER_IDT(IDT_Table, 191);
    RME_X64_USER_IDT(IDT_Table, 192); RME_X64_USER_IDT(IDT_Table, 193);
    RME_X64_USER_IDT(IDT_Table, 194); RME_X64_USER_IDT(IDT_Table, 195);
    RME_X64_USER_IDT(IDT_Table, 196); RME_X64_USER_IDT(IDT_Table, 197);
    RME_X64_USER_IDT(IDT_Table, 198); RME_X64_USER_IDT(IDT_Table, 199);

    RME_X64_USER_IDT(IDT_Table, 200); RME_X64_USER_IDT(IDT_Table, 201);
    RME_X64_USER_IDT(IDT_Table, 202); RME_X64_USER_IDT(IDT_Table, 203);
    RME_X64_USER_IDT(IDT_Table, 204); RME_X64_USER_IDT(IDT_Table, 205);
    RME_X64_USER_IDT(IDT_Table, 206); RME_X64_USER_IDT(IDT_Table, 207);
    RME_X64_USER_IDT(IDT_Table, 208); RME_X64_USER_IDT(IDT_Table, 209);

    RME_X64_USER_IDT(IDT_Table, 210); RME_X64_USER_IDT(IDT_Table, 211);
    RME_X64_USER_IDT(IDT_Table, 212); RME_X64_USER_IDT(IDT_Table, 213);
    RME_X64_USER_IDT(IDT_Table, 214); RME_X64_USER_IDT(IDT_Table, 215);
    RME_X64_USER_IDT(IDT_Table, 216); RME_X64_USER_IDT(IDT_Table, 217);
    RME_X64_USER_IDT(IDT_Table, 218); RME_X64_USER_IDT(IDT_Table, 219);

    RME_X64_USER_IDT(IDT_Table, 220); RME_X64_USER_IDT(IDT_Table, 221);
    RME_X64_USER_IDT(IDT_Table, 222); RME_X64_USER_IDT(IDT_Table, 223);
    RME_X64_USER_IDT(IDT_Table, 224); RME_X64_USER_IDT(IDT_Table, 225);
    RME_X64_USER_IDT(IDT_Table, 226); RME_X64_USER_IDT(IDT_Table, 227);
    RME_X64_USER_IDT(IDT_Table, 228); RME_X64_USER_IDT(IDT_Table, 229);

    RME_X64_USER_IDT(IDT_Table, 230); RME_X64_USER_IDT(IDT_Table, 231);
    RME_X64_USER_IDT(IDT_Table, 232); RME_X64_USER_IDT(IDT_Table, 233);
    RME_X64_USER_IDT(IDT_Table, 234); RME_X64_USER_IDT(IDT_Table, 235);
    RME_X64_USER_IDT(IDT_Table, 236); RME_X64_USER_IDT(IDT_Table, 237);
    RME_X64_USER_IDT(IDT_Table, 238); RME_X64_USER_IDT(IDT_Table, 239);

    RME_X64_USER_IDT(IDT_Table, 240); RME_X64_USER_IDT(IDT_Table, 241);
    RME_X64_USER_IDT(IDT_Table, 242); RME_X64_USER_IDT(IDT_Table, 243);
    RME_X64_USER_IDT(IDT_Table, 244); RME_X64_USER_IDT(IDT_Table, 245);
    RME_X64_USER_IDT(IDT_Table, 246); RME_X64_USER_IDT(IDT_Table, 247);
    RME_X64_USER_IDT(IDT_Table, 248); RME_X64_USER_IDT(IDT_Table, 249);

    RME_X64_USER_IDT(IDT_Table, 250); RME_X64_USER_IDT(IDT_Table, 251);
    RME_X64_USER_IDT(IDT_Table, 252); RME_X64_USER_IDT(IDT_Table, 253);
    RME_X64_USER_IDT(IDT_Table, 254); RME_X64_USER_IDT(IDT_Table, 255);

    /* Replace systick handler with customized ones - spurious interrupts
     * and IPIs are handled in the general interrupt path. SysTick handler
     * is only processed by the first processor, so we don't register it
     * for other auxiliary processors */
    if(RME_X64_CPU_Cnt==0)
        RME_X64_SET_IDT(IDT_Table, RME_X64_INT_SYSTICK, RME_X64_IDT_VECT, SysTick_Handler);
    /* Register SMP handlers */
    RME_X64_SET_IDT(IDT_Table, RME_X64_INT_SMP_SYSTICK, RME_X64_IDT_VECT, SysTick_SMP_Handler);

    /* Load the IDT */
    Desc[0]=RME_POW2(RME_PGT_SIZE_4K)-1;
    Desc[1]=(rme_ptr_t)IDT_Table;
    Desc[2]=((rme_ptr_t)IDT_Table)>>16;
    Desc[3]=((rme_ptr_t)IDT_Table)>>32;
    Desc[4]=((rme_ptr_t)IDT_Table)>>48;
    __RME_X64_IDT_Load((rme_ptr_t*)Desc);
    GDT_Table=(rme_ptr_t*)(RME_X64_CPU_LOCAL_BASE(RME_X64_CPU_Cnt)+RME_POW2(RME_PGT_SIZE_4K));
    TSS_Table=(rme_ptr_t)(RME_X64_CPU_LOCAL_BASE(RME_X64_CPU_Cnt)+RME_POW2(RME_PGT_SIZE_4K)+16*sizeof(rme_ptr_t));
    /* Dummy entry */
    GDT_Table[0]=0x0000000000000000ULL;
    /* Kernel code, DPL=0, R/X */
    GDT_Table[1]=0x0020980000000000ULL;
    /* Kernel data, DPL=0, W */
    GDT_Table[2]=0x0000920000000000ULL;
    /* Unused entry - this is for sysret instruction's requirement */
    GDT_Table[3]=0x0000000000000000ULL;
    /* User data, DPL=3, W */
    GDT_Table[4]=0x0000F20000000000ULL;
    /* User code, DPL=3, R/X */
    GDT_Table[5]=0x0020F80000000000ULL;
    /* TSS */
    GDT_Table[6]=(0x0067)|((TSS_Table&0xFFFFFFULL)<<16)|(0x0089ULL<<40)|(((TSS_Table>>24)&0xFFULL)<<56);
    GDT_Table[7]=(TSS_Table>>32);
    /* Load the GDT */
    Desc[0]=8*sizeof(rme_ptr_t)-1;
    Desc[1]=(rme_ptr_t)GDT_Table;
    Desc[2]=((rme_ptr_t)GDT_Table)>>16;
    Desc[3]=((rme_ptr_t)GDT_Table)>>32;
    Desc[4]=((rme_ptr_t)GDT_Table)>>48;
    __RME_X64_GDT_Load((rme_ptr_t*)Desc);
    /* Set the RSP to TSS */
    ((rme_u32_t*)TSS_Table)[1]=RME_X64_KSTACK(RME_X64_CPU_Cnt);
    ((rme_u32_t*)TSS_Table)[2]=RME_X64_KSTACK(RME_X64_CPU_Cnt)>>32;
    /* IO Map Base = End of TSS (What's this?) */
    ((rme_u32_t*)TSS_Table)[16]=0x00680000;
    __RME_X64_TSS_Load(6*sizeof(rme_ptr_t));
    /* Initialize the RME per-cpu data here */
    CPU_Local=(struct RME_CPU_Local*)(RME_X64_CPU_LOCAL_BASE(RME_X64_CPU_Cnt)+
    		                          RME_POW2(RME_PGT_SIZE_4K)+
									  RME_POW2(RME_PGT_SIZE_1K));
    _RME_CPU_Local_Init(CPU_Local,RME_X64_CPU_Cnt);

    /* Initialize x64 specific CPU-local data structure */
    Temp=(struct RME_X64_Temp*)(RME_X64_CPU_LOCAL_BASE(RME_X64_CPU_Cnt+1)-sizeof(struct RME_X64_Temp));
    Temp->CPU_Local_Addr=(rme_ptr_t)CPU_Local;
    Temp->Kernel_SP=RME_X64_KSTACK(RME_X64_CPU_Cnt);
    Temp->Temp_User_SP=0;

    /* Set the base of GS to this memory */
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_KERNEL_GS_BASE, (rme_ptr_t)IDT_Table);
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_GS_BASE, (rme_ptr_t)IDT_Table);
    /* Enable SYSCALL/SYSRET */
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_EFER,__RME_X64_Read_MSR(RME_X64_MSR_IA32_EFER)|RME_X64_MSR_IA32_EFER_SCE);
    /* Set up SYSCALL/SYSRET parameters */
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_LSTAR, (rme_ptr_t)SVC_Handler);
    /* Clear IF, DF and TF on SYSCALL entry; leave the MSR's reserved bits zero. */
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_FMASK, RME_X64_RFLAGS_IF|RME_X64_RFLAGS_DF|RME_X64_RFLAGS_TF);
    /* The SYSRET, when returning to user mode in 64-bit, will load the SS from +8, and CS from +16.
     * The original place for CS is reserved for 32-bit usages and is thus not usable by 64-bit */
    __RME_X64_Write_MSR(RME_X64_MSR_IA32_STAR, (((rme_ptr_t)RME_X64_SEG_EMPTY)<<48)|(((rme_ptr_t)RME_X64_SEG_KERNEL_CODE)<<32));
}
/* End Function:__RME_X64_CPU_Local_Init *************************************/

/* Function:__RME_X64_CPU_Local_Get_By_CPUID **********************************
Description : Given the CPUID of a CPU, get its RME CPU-local data structure.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
struct RME_CPU_Local* __RME_X64_CPU_Local_Get_By_CPUID(rme_ptr_t CPUID)
{
	return (struct RME_CPU_Local*)(RME_X64_CPU_LOCAL_BASE(CPUID)+
			                       RME_POW2(RME_PGT_SIZE_4K)+
								   RME_POW2(RME_PGT_SIZE_1K));
}
/* End Function:__RME_X64_CPU_Local_Get_By_CPUID *****************************/

/* Function:__RME_X64_LAPIC_Read **********************************************
Description : Read a 32-bit local APIC register in the selected mode.
Input       : rme_ptr_t Reg - The register offset in 32-bit words.
Output      : None.
Return      : rme_u32_t - The register value.
******************************************************************************/
rme_u32_t __RME_X64_LAPIC_Read(rme_ptr_t Reg)
{
    if(RME_X64_X2APIC!=0)
        return (rme_u32_t)__RME_X64_Read_MSR(RME_X64_MSR_X2APIC(Reg));
    return RME_X64_LAPIC_READ(Reg);
}
/* End Function:__RME_X64_LAPIC_Read *****************************************/

/* Function:__RME_X64_LAPIC_Write *********************************************
Description : Write a 32-bit local APIC register in the selected mode.
Input       : rme_ptr_t Reg - The register offset in 32-bit words.
              rme_u32_t Value - The register value.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_LAPIC_Write(rme_ptr_t Reg, rme_u32_t Value)
{
    if(RME_X64_X2APIC!=0)
        __RME_X64_Write_MSR(RME_X64_MSR_X2APIC(Reg), Value);
    else
        RME_X64_LAPIC_WRITE(Reg, Value);
}
/* End Function:__RME_X64_LAPIC_Write ****************************************/

/* Function:__RME_X64_LAPIC_IPI ***********************************************
Description : Send an interprocessor interrupt in the selected APIC mode.
Input       : rme_u32_t ID - The destination APIC ID, ignored for shorthand.
              rme_u32_t Command - The low half of the interrupt command.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_LAPIC_IPI(rme_u32_t ID, rme_u32_t Command)
{
    if(RME_X64_X2APIC!=0)
    {
        /* Publish shared data before the non-serializing ICR write. */
        __asm__ volatile("mfence; lfence":::"memory");
        __RME_X64_Write_MSR(RME_X64_MSR_X2APIC(RME_X64_LAPIC_ICRLO),
                            ((rme_ptr_t)ID<<32)|Command);
    }
    else
    {
        while((RME_X64_LAPIC_READ(RME_X64_LAPIC_ICRLO)&RME_X64_LAPIC_ICRLO_DELIVS)!=0);
        RME_X64_LAPIC_WRITE(RME_X64_LAPIC_ICRHI, ID<<24);
        RME_X64_LAPIC_WRITE(RME_X64_LAPIC_ICRLO, Command);
    }
}
/* End Function:__RME_X64_LAPIC_IPI ******************************************/

/* Function:__RME_X64_LAPIC_Ack ***********************************************
Description : Acknowledge the interrupt on LAPIC.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_LAPIC_Ack(void)
{
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_EOI, 0);
}
/* End Function:__RME_X64_LAPIC_Ack ******************************************/

/* Function:__RME_X64_LAPIC_Init **********************************************
Description : Initialize LAPIC controllers - this will be run once on everycore.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_LAPIC_Init(void)
{
    rme_ptr_t Base=__RME_X64_Read_MSR(RME_X64_MSR_IA32_APIC_BASE);

    /* APs may start in xAPIC mode even if the boot processor uses x2APIC. */
    if((Base&RME_X64_APIC_BASE_ENABLE)==0)
    {
        Base|=RME_X64_APIC_BASE_ENABLE;
        __RME_X64_Write_MSR(RME_X64_MSR_IA32_APIC_BASE, Base);
    }
    if(RME_X64_X2APIC!=0 && (Base&RME_X64_APIC_BASE_X2APIC)==0)
        __RME_X64_Write_MSR(RME_X64_MSR_IA32_APIC_BASE, Base|RME_X64_APIC_BASE_X2APIC);

    /* Enable local APIC; set spurious interrupt vector to 32 */
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_SVR, RME_X64_LAPIC_SVR_ENABLE|RME_X64_INT_SPUR);

    /* Disable local interrupt lines */
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_LINT0, RME_X64_LAPIC_MASKED);
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_LINT1, RME_X64_LAPIC_MASKED);

    /* Disable performance counter overflow interrupts when there is one */
    if(((__RME_X64_LAPIC_Read(RME_X64_LAPIC_VER)>>16)&0xFF)>=4)
        __RME_X64_LAPIC_Write(RME_X64_LAPIC_PCINT, RME_X64_LAPIC_MASKED);

    /* Map error interrupt to IRQ_ERROR */
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_ERROR, RME_X64_INT_ERROR);

    /* Clear error status register (requires back-to-back writes) */
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_ESR, 0);
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_ESR, 0);

    /* Acknowledge any outstanding interrupts */
    __RME_X64_LAPIC_Ack();

    /* The legacy arbitration synchronization IPI is only for xAPIC. */
    if(RME_X64_X2APIC==0)
        __RME_X64_LAPIC_IPI(0, RME_X64_LAPIC_ICRLO_BCAST|RME_X64_LAPIC_ICRLO_INIT|
                               RME_X64_LAPIC_ICRLO_LEVEL);

    /* Enable interrupts on the APIC */
    __RME_X64_LAPIC_Write(RME_X64_LAPIC_TPR, 0);
}
/* End Function:__RME_X64_LAPIC_Init *****************************************/

/* Function:__RME_X64_PIC_Init ************************************************
Description : Initialize PIC controllers - we just disable it once and for all.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_PIC_Init(void)
{
    /* Mask all interrupts */
    __RME_X64_Out(RME_X64_PIC1+1, 0xFF);
    __RME_X64_Out(RME_X64_PIC2+1, 0xFF);

    /* Set up master (8259A-1) */
    __RME_X64_Out(RME_X64_PIC1, 0x11);
    __RME_X64_Out(RME_X64_PIC1+1, RME_X64_INT_USER(0));
    __RME_X64_Out(RME_X64_PIC1+1, 1<<2);
    __RME_X64_Out(RME_X64_PIC1+1, 0x3);

    /* Set up slave (8259A-2) */
    __RME_X64_Out(RME_X64_PIC2, 0x11);
    __RME_X64_Out(RME_X64_PIC2+1, RME_X64_INT_USER(8));
    __RME_X64_Out(RME_X64_PIC2+1, 2);
    __RME_X64_Out(RME_X64_PIC2+1, 0x3);

    __RME_X64_Out(RME_X64_PIC1, 0x68);
    __RME_X64_Out(RME_X64_PIC1, 0x0A);

    __RME_X64_Out(RME_X64_PIC2, 0x68);
    __RME_X64_Out(RME_X64_PIC2, 0x0A);

    /* Mask all interrupts - we do not use the PIC at all */
    __RME_X64_Out(RME_X64_PIC1+1, 0xFF);
    __RME_X64_Out(RME_X64_PIC2+1, 0xFF);
}
/* End Function:__RME_X64_PIC_Init *******************************************/

/* Function:__RME_X64_IOAPIC_Int_Enable ***************************************
Description : Enable a specific vector on one CPU.
Input       : rme_ptr_t IRQ - The user vector to enable.
              rme_ptr_t CPUID - The CPU to enable this IRQ on.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_IOAPIC_Int_Enable(rme_ptr_t IRQ, rme_ptr_t CPUID)
{
    /* Mark interrupt edge-triggered, active high, enabled, and routed to the
     * given cpunum, which happens to be that cpu's APIC ID. */
    RME_X64_IOAPIC_WRITE(RME_X64_IOAPIC_REG_TABLE+(IRQ<<1),RME_X64_INT_USER(IRQ));
    RME_X64_IOAPIC_WRITE(RME_X64_IOAPIC_REG_TABLE+(IRQ<<1)+1,CPUID<<24);
}
/* End Function:__RME_X64_IOAPIC_Int_Enable **********************************/

/* Function:__RME_X64_IOAPIC_Int_Disable **************************************
Description : Disable a specific vector.
Input       : rme_ptr_t IRQ - The user vector to enable.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_IOAPIC_Int_Disable(rme_ptr_t IRQ)
{
    /* Mark interrupt edge-triggered, active high, enabled, and routed to the
     * given cpunum, which happens to be that cpu's APIC ID. */
    RME_X64_IOAPIC_WRITE(RME_X64_IOAPIC_REG_TABLE+(IRQ<<1),RME_X64_IOAPIC_INT_DISABLED|RME_X64_INT_USER(IRQ));
    RME_X64_IOAPIC_WRITE(RME_X64_IOAPIC_REG_TABLE+(IRQ<<1)+1,0);
}
/* End Function:__RME_X64_IOAPIC_Int_Disable *********************************/

/* Function:__RME_X64_IOAPIC_Init *********************************************
Description : Initialize IOAPIC controllers - this will be run once only.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_IOAPIC_Init(void)
{
    rme_ptr_t Max_Int;
    rme_ptr_t IOAPIC_ID;
    rme_cnt_t Count;
    /* IOAPIC initialization */
    RME_X64_IOAPIC_READ(RME_X64_IOAPIC_REG_VER,Max_Int);
    Max_Int=((Max_Int>>16)&0xFF);
    RME_DBG_S("\n\rMax interupt number is: ");
    RME_DBG_I(Max_Int);
    RME_X64_IOAPIC_READ(RME_X64_IOAPIC_REG_ID,IOAPIC_ID);
    IOAPIC_ID>>=24;
    /* This is not necessarily true when we have >1 IOAPICs */
    /* RME_ASSERT(IOAPIC_ID==RME_X64_IOAPIC_Info[0].IOAPIC_ID); */
    RME_DBG_S("\n\rIOAPIC ID is: ");
    RME_DBG_I(IOAPIC_ID);

    /* Disable all interrupts */
    for(Count=0;Count<=Max_Int;Count++)
        __RME_X64_IOAPIC_Int_Disable(Count);
}
/* End Function:__RME_X64_IOAPIC_Init ****************************************/

/* Function:__RME_X64_SMP_Delay ***********************************************
Description : Wait between startup IPIs before the scheduler timer is started.
Input       : rme_ptr_t Microseconds - The delay, between 1 and 10000 us.
              rme_ptr_t TSC_Hz - The TSC frequency, or 0 if not enumerated.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_SMP_Delay(rme_ptr_t Microseconds, rme_ptr_t TSC_Hz)
{
    rme_ptr_t Start;
    rme_ptr_t Port;
    rme_ptr_t Count;
    rme_ptr_t Retry;

    if(TSC_Hz!=0)
    {
        Count=(TSC_Hz*Microseconds+999999U)/1000000U;
        Start=RME_TIMESTAMP;
        while(RME_TIMESTAMP-Start<Count)
            __asm__ volatile("pause");
        return;
    }

    /* Fall back to PIT channel 2 and keep the speaker off. */
    Port=__RME_X64_In(0x61);
    Count=(1193182U*Microseconds+999999U)/1000000U;
    __RME_X64_Out(0x61, Port&~3U);
    __RME_X64_Out(RME_X64_PIT_CMD, 0xB0);
    __RME_X64_Out(RME_X64_PIT_CH2, Count&0xFFU);
    __RME_X64_Out(RME_X64_PIT_CH2, Count>>8);
    __RME_X64_Out(0x61, (Port&~2U)|1U);
    for(Retry=0;Retry<1000000U;Retry++)
    {
        if((__RME_X64_In(0x61)&0x20U)!=0)
            break;
    }
    __RME_X64_Out(0x61, Port);
    if(Retry==1000000U)
    {
        __RME_X64_ACPI_Error("APIC: No working startup timer");
        for(;;)
            __RME_X64_Halt();
    }
}
/* End Function:__RME_X64_SMP_Delay ******************************************/

/* Function:__RME_X64_SMP_Init ************************************************
Description : Start all other processors, one by one. We cannot start all of them
              at once because of the stupid self modifying code of X64!
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_SMP_Init(void)
{
    rme_u8_t* Code;
    rme_cnt_t Count;
    rme_u16_t* Warm_Reset;
    rme_u32_t ID;
    rme_ptr_t Retry;
    rme_ptr_t Denominator;
    rme_ptr_t Numerator=0;
    rme_ptr_t Crystal_Hz=0;
    rme_ptr_t Reserved=0;
    rme_ptr_t TSC_Hz=0;
    rme_u32_t BSP_ID=__RME_X64_LAPIC_Read(RME_X64_LAPIC_ID);

    /* Keep the boot processor at index 0 regardless of MADT entry order. */
    if(RME_X64_X2APIC==0)
        BSP_ID>>=24;
    for(Count=0;Count<RME_X64_Num_CPU;Count++)
    {
        if(RME_X64_CPU_Info[Count].LAPIC_ID==BSP_ID)
            break;
    }
    if(Count==RME_X64_Num_CPU || BSP_ID>=RME_X64_XAPIC_BCAST_ID)
    {
        if(Count==RME_X64_Num_CPU)
            __RME_X64_ACPI_Error("APIC: Boot CPU is missing from MADT");
        else
            __RME_X64_ACPI_Error("APIC: Boot CPU needs I/O interrupt remapping");
        for(;;)
            __RME_X64_Halt();
    }
    RME_X64_CPU_Info[Count].LAPIC_ID=RME_X64_CPU_Info[0].LAPIC_ID;
    RME_X64_CPU_Info[0].LAPIC_ID=BSP_ID;

    if(RME_X64_Num_CPU>1 && RME_X64_Feature.Max_Func>=RME_X64_CPUID_15_TSC_RATIO)
    {
        Denominator=__RME_X64_CPUID_Get(RME_X64_CPUID_15_TSC_RATIO,&Numerator,&Crystal_Hz,&Reserved);
        if(Denominator!=0 && Numerator!=0 && Crystal_Hz!=0)
            TSC_Hz=Crystal_Hz*Numerator/Denominator;
    }

    /* Write entry code to unused memory at 0x7000 */
    Code=(rme_u8_t*)RME_X64_PA2VA(0x7000);
    for(Count=0;Count<sizeof(RME_X64_Boot_Code);Count++)
        Code[Count]=RME_X64_Boot_Code[Count];

    /* Start the CPUs one by one - the first one is ourself */
    RME_X64_CPU_Cnt=1;
    __RME_X64_Boot_CPUs(1,RME_X64_Num_CPU);
    for(Count=1;Count<RME_X64_Num_CPU;Count++)
    {
        RME_DBG_S("\n\rBooting CPU ");
        RME_DBG_I(Count);
        /* Temporary stack */
        *(rme_u32_t*)(Code-4)=0x8000;
        *(rme_u32_t*)(Code-8)=RME_X64_TEXT_VA2PA(__RME_X64_SMP_Boot_32);
        *(rme_ptr_t*)(Code-16)=RME_X64_KSTACK(Count);

        /* Initialize CMOS shutdown code to 0AH */
        __RME_X64_Out(RME_X64_RTC_CMD,0xF);
        __RME_X64_Out(RME_X64_RTC_DATA,0xA);
        /* Warm reset vector point to AP code */
        Warm_Reset=(rme_u16_t*)RME_X64_PA2VA((0x40<<4|0x67));
        Warm_Reset[0]=0;
        Warm_Reset[1]=0x7000>>4;

        ID=(rme_u32_t)RME_X64_CPU_Info[Count].LAPIC_ID;
        /* Send INIT (level-triggered) interrupt to reset other CPU */
        __RME_X64_LAPIC_IPI(ID, RME_X64_LAPIC_ICRLO_INIT|
                               RME_X64_LAPIC_ICRLO_LEVEL|RME_X64_LAPIC_ICRLO_ASSERT);
        if(RME_X64_X2APIC==0)
        {
            __RME_X64_SMP_Delay(200,TSC_Hz);
            __RME_X64_LAPIC_IPI(ID, RME_X64_LAPIC_ICRLO_INIT|RME_X64_LAPIC_ICRLO_LEVEL);
        }
        __RME_X64_SMP_Delay(10000,TSC_Hz);

        /* Send startup IPI twice according to Intel manuals */
        __RME_X64_LAPIC_IPI(ID, RME_X64_LAPIC_ICRLO_STARTUP|
                               RME_X64_LAPIC_ICRLO_ASSERT|(0x7000>>12));
        __RME_X64_SMP_Delay(200,TSC_Hz);
        __RME_X64_LAPIC_IPI(ID, RME_X64_LAPIC_ICRLO_STARTUP|
                               RME_X64_LAPIC_ICRLO_ASSERT|(0x7000>>12));
        __RME_X64_SMP_Delay(200,TSC_Hz);

        /* Wait up to one second for the AP to finish its initialization. */
        for(Retry=0;Retry<100 && RME_X64_CPU_Info[Count].Boot_Done==0;Retry++)
            __RME_X64_SMP_Delay(10000,TSC_Hz);
        if(RME_X64_CPU_Info[Count].Boot_Done==0)
        {
            RME_DBG_S("\r\nAPIC: Startup timeout, APIC ID ");
            RME_DBG_I(ID);
            __RME_X64_Boot_Message("APIC: Secondary CPU startup timed out");
            for(;;)
                __RME_X64_Halt();
        }
        RME_X64_CPU_Cnt++;
        __RME_X64_Boot_CPUs(RME_X64_CPU_Cnt,RME_X64_Num_CPU);
    }
}
/* End Function:__RME_X64_SMP_Init *******************************************/

/* Function:__RME_X64_SMP_Tick ************************************************
Description : Send IPI to all other cores,to run their handler on the time.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_SMP_Tick(void)
{
    /* Is this a SMP? */
    if(RME_X64_Num_CPU>1)
        __RME_X64_LAPIC_IPI(0, RME_X64_LAPIC_ICRLO_EXC_SELF|RME_X64_LAPIC_ICRLO_FIXED|
                              RME_X64_LAPIC_ICRLO_ASSERT|RME_X64_INT_SMP_SYSTICK);
}
/* End Function:__RME_X64_SMP_Tick *******************************************/

/* Function:__RME_X64_Timer_Init **********************************************
Description : Initialize the on-board timer. We use the PIT because it is stable;
              Then we let the main CPU send out timer IPI interrupts to all other
              CPUs.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_X64_Timer_Init(void)
{
    /* For timer interrupts, they will always be handled by core 1, and all the other
     * cores should receive a IPI for that, so their scheduler can look after their
     * threads. We are using square wave mode. */
    __RME_X64_Out(RME_X64_PIT_CMD,0x34);
    __RME_X64_Out(RME_X64_PIT_CH0,(1193182/2/RME_X64_TIMER_FREQ)&0xFF);
    __RME_X64_Out(RME_X64_PIT_CH0,((1193182/2/RME_X64_TIMER_FREQ)>>8)&0xFF);
}
/* End Function:__RME_X64_Timer_Init *****************************************/

/* Function:__RME_Lowlvl_Init **********************************************
Description : Initialize the low-level hardware.
Input       : None.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Lowlvl_Init(void)
{
    struct RME_X64_Boot_Info Boot={0};

    __RME_X64_UART_Init();
    RME_DBG_S("\r\nRME_Kmain: Multiboot2");
    if(__RME_X64_Boot_Info_Read(&Boot)!=0)
    {
        RME_DBG_S("\r\nInvalid Multiboot2 information or missing memory map");
        for(;;)
            __RME_X64_Halt();
    }

    __RME_X64_Feature_Get();
    __RME_X64_Framebuffer_Init(Boot.Framebuffer);
    __RME_X64_Boot_Progress(RME_X64_BOOT_ACPI);
    __RME_X64_Boot_Message("RME: GOP ready, initializing ACPI");
    if(__RME_X64_ACPI_Init(&Boot)!=0)
    {
        __RME_X64_Boot_Message("RME: ACPI initialization failed");
        RME_DBG_S("\r\nACPI initialization failed");
        for(;;)
            __RME_X64_Halt();
    }
    __RME_X64_Boot_Message((RME_X64_X2APIC!=0)?"APIC: x2APIC mode":"APIC: xAPIC mode");
    __RME_X64_Boot_Progress(RME_X64_BOOT_MMU);
    __RME_X64_Boot_Message("RME: ACPI ready, reading memory map");
    RME_DBG_S("\r\nACPI initialization succeeded");
    if(RME_X64_X2APIC!=0)
        RME_DBG_S("\r\nAPIC: x2APIC mode");
    else
        RME_DBG_S("\r\nAPIC: xAPIC mode");

    if(__RME_X64_Mem_Init(Boot.MMap)!=0)
    {
        __RME_X64_Boot_Message("RME: Invalid Multiboot2 memory map");
        RME_DBG_S("\r\nInvalid Multiboot2 memory map");
        for(;;)
            __RME_X64_Halt();
    }

    return 0;
}
/* End Function:__RME_Lowlvl_Init *****************************************/

/* Function:__RME_Pgt_Kom_Init *********************************************
Description : Initialize the kernel mapping tables, so it can be added to all the
              top-level page tables. Currently this have no consideration for >1TB
              RAM, and is not NUMA-aware.
Input       : None.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Kom_Init(void)
{
    rme_cnt_t PML4_Cnt;
    rme_cnt_t PDP_Cnt;
    rme_cnt_t PDE_Cnt;
    rme_cnt_t Addr_Cnt;
    volatile struct __RME_X64_Mem* Mem;
    rme_ptr_t Start;
    rme_ptr_t End;
    rme_ptr_t First_End;
    const rme_ptr_t Four_GiB=1ULL<<32;

    __RME_X64_Boot_Message("RME: Setting up kernel page tables");

    /* Find the contiguous low-memory range that holds the fixed KOT and Kom1. */
    Mem=(struct __RME_X64_Mem*)RME_X64_Phys_Mem.Next;
    while(Mem!=(struct __RME_X64_Mem*)&RME_X64_Phys_Mem &&
          Mem->Start_Addr+Mem->Length<=RME_X64_KOT_OFFSET)
        Mem=(struct __RME_X64_Mem*)Mem->Head.Next;

    RME_X64_Layout.Kom1_Start[0]=RME_X64_Layout.PerCPU_Start+RME_X64_Layout.PerCPU_Size;
    First_End=0;
    if(Mem!=(struct __RME_X64_Mem*)&RME_X64_Phys_Mem && Mem->Start_Addr<=RME_X64_KOT_OFFSET)
    {
        End=Mem->Start_Addr+Mem->Length;
        if(End>Four_GiB)
            End=Four_GiB;
        First_End=RME_ROUND_DOWN(End-RME_X64_KOT_OFFSET,RME_PGT_SIZE_2M);
    }
    if(First_End<=RME_X64_VA2PA(RME_X64_Layout.Kom1_Start[0]))
    {
        __RME_X64_Boot_Message("RME: Insufficient contiguous low kernel memory");
        RME_DBG_S("\r\nMultiboot2 memory map cannot accommodate the low kernel memory");
        for(;;)
            __RME_X64_Halt();
    }

    /* Now initialize the kernel object allocation table */
    _RME_Kot_Init(RME_X64_Layout.Kot_Size/sizeof(rme_ptr_t));
    /* Reset PCID counter */
    RME_X64_PCID_Inc=0;

    /* Create the frame for kernel page tables */
    for(PML4_Cnt=0;PML4_Cnt<256;PML4_Cnt++)
    {
        RME_X64_Kpgt.PML4[PML4_Cnt]=RME_X64_MMU_ADDR(RME_X64_TEXT_VA2PA(&(RME_X64_Kpgt.PDP[PML4_Cnt][0])))|
        		                    RME_X64_MMU_KERN_PML4;

        for(PDP_Cnt=0;PDP_Cnt<512;PDP_Cnt++)
            RME_X64_Kpgt.PDP[PML4_Cnt][PDP_Cnt]=RME_X64_MMU_KERN_PDP;
    }

    /* Map in the first 4GB as linear mappings as always, 4 super pages, including the device hole.
     * We need to detect whether the 1GB page is supported. If not, we just map the initial tables
     * in, and we know where they are hard-coded in the assembly file */
    if((RME_X64_EXT(RME_X64_CPUID_E1_INFO_FEATURE,3)&RME_X64_E1_EDX_PDPE1GB)!=0)
    {
        /* Can use 1GB pages */
        RME_DBG_S("\n\rThis CPU have 1GB superpage support");
        RME_X64_Kpgt.PDP[0][0]|=RME_X64_MMU_ADDR(0)|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][1]|=RME_X64_MMU_ADDR(RME_POW2(RME_PGT_SIZE_1G))|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][2]|=RME_X64_MMU_ADDR(2*RME_POW2(RME_PGT_SIZE_1G))|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
        /* We need to mark the device hole as unbufferable */
        RME_X64_Kpgt.PDP[0][3]|=RME_X64_MMU_ADDR(3*RME_POW2(RME_PGT_SIZE_1G))|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][3]|=RME_X64_MMU_PWT|RME_X64_MMU_PCD;

        /* Map the first 2GB to the last position too, where the kernel text segment is at */
        RME_X64_Kpgt.PDP[255][510]|=RME_X64_MMU_ADDR(0)|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[255][511]|=RME_X64_MMU_ADDR(RME_POW2(RME_PGT_SIZE_1G))|RME_X64_MMU_PDE_SUP|RME_X64_MMU_P;
    }
    else
    {
        RME_DBG_S("\n\rThis CPU do not have 1GB superpage support");
        /* Cannot use 1GB pages, we revert to 2MB pages used during kernel startup */
        RME_X64_Kpgt.PDP[0][0]|=0x104000|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][1]|=0x105000|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][2]|=0x106000|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[0][3]|=0x107000|RME_X64_MMU_PCD|RME_X64_MMU_PWT|RME_X64_MMU_P;

        /* Map the first 2GB to the last position too, where the kernel text segment is at */
        RME_X64_Kpgt.PDP[255][510]|=0x104000|RME_X64_MMU_P;
        RME_X64_Kpgt.PDP[255][511]|=0x105000|RME_X64_MMU_P;
    }

    /* Add complete 2 MiB pages from the remaining ranges below 4 GiB. */
    Addr_Cnt=1;
    while(Mem->Start_Addr+Mem->Length<=Four_GiB)
    {
        Mem=(struct __RME_X64_Mem*)Mem->Head.Next;
        if(Mem==(struct __RME_X64_Mem*)&RME_X64_Phys_Mem || Mem->Start_Addr>=Four_GiB)
            break;

        Start=RME_ROUND_UP(Mem->Start_Addr,RME_PGT_SIZE_2M);
        End=Mem->Start_Addr+Mem->Length;
        End=RME_ROUND_DOWN((End<Four_GiB)?End:Four_GiB,RME_PGT_SIZE_2M);
        if(Start>=End || End-Start<RME_POW2(RME_PGT_SIZE_4M))
            continue;
        if(Addr_Cnt>=RME_X64_KOM1_MAXSEGS)
        {
            RME_DBG_S("\r\nThe memory under 4G is too fragmented. Aborting.");
            for(;;)
                __RME_X64_Halt();
        }
        RME_X64_Layout.Kom1_Start[Addr_Cnt]=RME_X64_PA2VA(Start);
        RME_X64_Layout.Kom1_Size[Addr_Cnt]=End-Start;
        Addr_Cnt++;
    }
    RME_X64_Layout.Kom1_Trunks=Addr_Cnt;

    /* This is the hole */
    RME_X64_Layout.Hole_Start=(Addr_Cnt==1)?RME_X64_PA2VA(First_End):
        RME_X64_Layout.Kom1_Start[Addr_Cnt-1]+RME_X64_Layout.Kom1_Size[Addr_Cnt-1];
    RME_X64_Layout.Hole_Size=RME_POW2(RME_PGT_SIZE_4G)-RME_X64_VA2PA(RME_X64_Layout.Hole_Start);

    /* Create kernel page mappings for memory above 4GB - we assume only one segment below 4GB */
    RME_X64_Layout.Kpgtbl_Start=RME_X64_Layout.Kom1_Start[0];
    RME_X64_Layout.Kom2_Start=RME_X64_PA2VA(RME_POW2(RME_PGT_SIZE_4G));
    RME_X64_Layout.Kom2_Size=0;

    /* We have filled the first 4 1GB superpages */
    PML4_Cnt=0;
    PDP_Cnt=3;
    PDE_Cnt=511;
    while(Mem!=(struct __RME_X64_Mem*)(&RME_X64_Phys_Mem))
    {
        Start=(Mem->Start_Addr<Four_GiB)?Four_GiB:Mem->Start_Addr;
        Start=RME_ROUND_UP(Start,RME_PGT_SIZE_2M);
        End=RME_ROUND_DOWN(Mem->Start_Addr+Mem->Length,RME_PGT_SIZE_2M);
        if(Start>=End || End-Start<RME_POW2(RME_PGT_SIZE_4M))
        {
            Mem=(struct __RME_X64_Mem*)Mem->Head.Next;
            continue;
        }

        /* Pack RAM above 4 GiB into the existing Kom2 virtual range. */
        for(rme_ptr_t Phys=Start;Phys<End;Phys+=RME_POW2(RME_PGT_SIZE_2M))
        {
            PDE_Cnt++;
            if(PDE_Cnt==512)
            {
                PDE_Cnt=0;
                PDP_Cnt++;
                if(PDP_Cnt==512)
                {
                    PDP_Cnt=0;
                    PML4_Cnt++;
                }
                /* Map this PDE into the PDP */
                RME_X64_Kpgt.PDP[PML4_Cnt][PDP_Cnt]|=RME_X64_MMU_ADDR(RME_X64_VA2PA(RME_X64_Layout.Kom1_Start[0]))|RME_X64_MMU_P;
            }

            ((rme_ptr_t*)(RME_X64_Layout.Kom1_Start[0]))[0]=RME_X64_MMU_ADDR(Phys)|RME_X64_MMU_KERN_PDE;
            RME_X64_Layout.Kom1_Start[0]+=sizeof(rme_ptr_t);
            RME_X64_Layout.Kom2_Size+=RME_POW2(RME_PGT_SIZE_2M);
        }

        Mem=(struct __RME_X64_Mem*)(Mem->Head.Next);
    }

    /* Preserve the RGB MMIO window in every kernel page table. */
    if(RME_X64_Boot_PDP[RME_X64_RGB_WINDOW]!=0)
        RME_X64_Kpgt.PDP[RME_X64_BOOT_WINDOW_SLOT-256U][RME_X64_RGB_WINDOW]=
            RME_X64_Boot_PDP[RME_X64_RGB_WINDOW];

    /* Copy the new page tables to the temporary entries, so that we can boot SMP */
    for(PML4_Cnt=0;PML4_Cnt<256;PML4_Cnt++)
        ((rme_ptr_t*)RME_X64_PA2VA(0x101000))[PML4_Cnt+256]=RME_X64_Kpgt.PML4[PML4_Cnt];

    /* Page table allocation finished. Now need to align Kom1 to 2MB page boundary */
    RME_X64_Layout.Kom1_Start[0]=RME_ROUND_UP(RME_X64_Layout.Kom1_Start[0],RME_PGT_SIZE_2M);
    RME_X64_Layout.Kom1_Size[0]=First_End-RME_X64_VA2PA(RME_X64_Layout.Kom1_Start[0]);

    /* All memory is mapped. Now figure out the size of kernel stacks */
    RME_X64_Layout.Kpgtbl_Size=RME_X64_Layout.Kom1_Start[0]-RME_X64_Layout.Kpgtbl_Start;

    /* See if we are allocating the stack from Kom2 or Kom1 */
    if(RME_X64_Layout.Kom2_Size==0)
    {
        RME_X64_Layout.Stack_Start=RME_ROUND_DOWN(RME_X64_Layout.Kom1_Start[0]+RME_X64_Layout.Kom1_Size[0]-1,RME_X64_KSTACK_ORDER);
        RME_X64_Layout.Stack_Start-=RME_X64_Layout.Stack_Size;
        RME_X64_Layout.Kom1_Size[0]=RME_X64_Layout.Stack_Start-RME_X64_Layout.Kom1_Start[0];
    }


    else
    {
        RME_X64_Layout.Stack_Start=RME_ROUND_DOWN(RME_X64_Layout.Kom2_Start+RME_X64_Layout.Kom2_Size-1,RME_X64_KSTACK_ORDER);
        RME_X64_Layout.Stack_Start-=RME_X64_Layout.Stack_Size;
        RME_X64_Layout.Kom2_Size=RME_X64_Layout.Stack_Start-RME_X64_Layout.Kom2_Start;
    }

    /* Now report all mapping info */
    RME_DBG_S("\n\r\n\rKot_Start:     0x");
    RME_DBG_H(RME_X64_Layout.Kot_Start);
    RME_DBG_S("\n\rKot_Size:      0x");
    RME_DBG_H(RME_X64_Layout.Kot_Size);
    RME_DBG_S("\n\rPerCPU_Start:    0x");
    RME_DBG_H(RME_X64_Layout.PerCPU_Start);
    RME_DBG_S("\n\rPerCPU_Size:     0x");
    RME_DBG_H(RME_X64_Layout.PerCPU_Size);
    RME_DBG_S("\n\rKpgtbl_Start:    0x");
    RME_DBG_H(RME_X64_Layout.Kpgtbl_Start);
    RME_DBG_S("\n\rKpgtbl_Size:     0x");
    RME_DBG_H(RME_X64_Layout.Kpgtbl_Size);
    for(Addr_Cnt=0;Addr_Cnt<RME_X64_Layout.Kom1_Trunks;Addr_Cnt++)
    {
        RME_DBG_S("\n\rKom1_Start[");
        RME_DBG_I(Addr_Cnt);
        RME_DBG_S("]:  0x");
        RME_DBG_H(RME_X64_Layout.Kom1_Start[Addr_Cnt]);
        RME_DBG_S("\n\rKom1_Size[");
        RME_DBG_I(Addr_Cnt);
        RME_DBG_S("]:   0x");
        RME_DBG_H(RME_X64_Layout.Kom1_Size[Addr_Cnt]);
    }
    RME_DBG_S("\n\rHole_Start:      0x");
    RME_DBG_H(RME_X64_Layout.Hole_Start);
    RME_DBG_S("\n\rHole_Size:       0x");
    RME_DBG_H(RME_X64_Layout.Hole_Size);
    RME_DBG_S("\n\rKom2_Start:     0x");
    RME_DBG_H(RME_X64_Layout.Kom2_Start);
    RME_DBG_S("\n\rKom2_Size:      0x");
    RME_DBG_H(RME_X64_Layout.Kom2_Size);
    RME_DBG_S("\n\rStack_Start:     0x");
    RME_DBG_H(RME_X64_Layout.Stack_Start);
    RME_DBG_S("\n\rStack_Size:      0x");
    RME_DBG_H(RME_X64_Layout.Stack_Size);

    return 0;
}
/* End Function:__RME_Pgt_Kom_Init ****************************************/

/* Function:__RME_SMP_Low_Level_Init ******************************************
Description : Low-level initialization for all other cores.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
rme_ptr_t __RME_SMP_Low_Level_Init(void)
{
    struct RME_CPU_Local* CPU_Local;

    /* Initialize all vector tables */
    __RME_X64_CPU_Local_Init();
    /* Initialize LAPIC */
    __RME_X64_LAPIC_Init();

    /* Check to see if we are booting this correctly */
    CPU_Local=RME_CPU_LOCAL();
    RME_ASSERT(CPU_Local->CPUID==RME_X64_CPU_Cnt);

    RME_X64_CPU_Info[RME_X64_CPU_Cnt].Boot_Done=1;
    /* Spin until the global CPU counter is zero again, which means the booting
     * processor has done booting and we can proceed now */
    while(RME_X64_CPU_Cnt!=0);

    /* The booting CPU must have created everything necessary for us. Let's
     * check to see if they are there. */
    RME_ASSERT(CPU_Local->Thd_Cur!=0);
    RME_ASSERT(CPU_Local->Sig_Tim!=0);
    RME_ASSERT(CPU_Local->Sig_Vct!=0);

    /* Change page tables */
    __RME_Pgt_Set((CPU_Local->Thd_Cur)->Sched.Prc->Pgt);
    /* Boot into the init thread - never returns */
    __RME_Enter_User_Mode(RME_X64_UVM_BASE, RME_X64_UVM_BASE+RME_X64_USTACK(CPU_Local->CPUID),
                          CPU_Local->CPUID);

    return 0;
}
/* End Function:__RME_SMP_Low_Level_Init *************************************/

/* Function:__RME_Boot ********************************************************
Description : Boot the first process in the system.
Input       : None.
Output      : None.
Return      : rme_ptr_t - Always 0.
******************************************************************************/
rme_ptr_t __RME_Boot(void)
{
    rme_ptr_t Cur_Addr;
    rme_cnt_t Count;
    rme_cnt_t Kom1_Cnt;
    rme_ptr_t Phys_Addr;
    rme_ptr_t Page_Ptr;
    struct RME_Cap_Cpt* Cpt;
    struct RME_CPU_Local* CPU_Local;
    __RME_X64_Boot_Progress(RME_X64_BOOT_IRQ);
    __RME_X64_Boot_Message("RME: Initializing interrupt controllers");

    /* Initialize our own CPU-local data structures */
    RME_X64_CPU_Cnt = 0;
    RME_DBG_S("\r\nCPU 0 local IDT/GDT init");
    __RME_X64_CPU_Local_Init();
    /* Initialize interrupt controllers (PIC, LAPIC, IOAPIC) */
    RME_DBG_S("\r\nCPU 0 LAPIC init");
    __RME_X64_LAPIC_Init();
    RME_DBG_S("\r\nPIC init");
    __RME_X64_PIC_Init();
    RME_DBG_S("\r\nIOAPIC init");
    __RME_X64_IOAPIC_Init();
    /* Start other processors, if there are any. They will keep spinning until
     * the booting processor finish all its work. */
    __RME_X64_Boot_Progress(RME_X64_BOOT_SMP);
    __RME_X64_Boot_Message("RME: Starting secondary CPUs");
    __RME_X64_SMP_Init();
    __RME_X64_Boot_Progress(RME_X64_BOOT_UVM);
    __RME_X64_Boot_Message("RME: Preparing the initial process");

    /* Create all initial tables in Kom1, which is sure to be present */
    Cur_Addr=RME_X64_Layout.Kom1_Start[0]+16*RME_POW2(RME_PGT_SIZE_2M);
    RME_DBG_S("\r\nKot registration start offset: 0x");
    RME_DBG_H(((Cur_Addr-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER)/8);

    /* Create the capability table for the init process - always 16 */
    Cpt=(struct RME_Cap_Cpt*)Cur_Addr;
    RME_ASSERT(_RME_Cpt_Boot_Init(RME_BOOT_INIT_CPT,Cur_Addr,16)==RME_BOOT_INIT_CPT);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(16));
    /* Create the capability table for initial page tables - now we are only
     * adding 2MB pages. There will be 1 PML4, 16 PDP, and 16*512=8192 PGD.
     * This should provide support for up to 4TB of memory, which will be sufficient
     * for at least a decade. These data structures will eat 32MB of memory, which
     * is fine */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_TBL_PGT, Cur_Addr, 1+16+8192)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(1+16+8192));
    /* Align the address to 4096 to prepare for page table creation */
    Cur_Addr=RME_ROUND_UP(Cur_Addr,12);
    /* Create PML4 */
    RME_DBG_S("\r\nPML4 address:");
    RME_DBG_H(Cur_Addr);
    RME_ASSERT(_RME_Pgt_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_PGT, RME_BOOT_PML4,
                                   Cur_Addr, 0, RME_PGT_TOP, RME_PGT_SIZE_512G, RME_PGT_NUM_512)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_TOP(RME_PGT_NUM_512));
    /* Create all our 16 PDPs, and cons them into the PML4 */
    for(Count=0;Count<16;Count++)
    {
        RME_ASSERT(_RME_Pgt_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_PGT, RME_BOOT_PDP(Count),
                                       Cur_Addr, (rme_ptr_t)RME_POW2(RME_PGT_SIZE_512G)*Count, RME_PGT_NOM, RME_PGT_SIZE_1G, RME_PGT_NUM_512)==0);
        Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_NOM(RME_PGT_NUM_512));
        RME_ASSERT(_RME_Pgt_Boot_Con(RME_X64_CPT, RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PML4), Count,
                                       RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PDP(Count)), RME_PGT_ALL_PERM)==0);
    }

    /* Create 8192 PDEs, and cons them into their respective PDPs */
    for(Count=0;Count<8192;Count++)
    {
        RME_ASSERT(_RME_Pgt_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_PGT, RME_BOOT_PDE(Count),
                                       Cur_Addr, (rme_ptr_t)RME_POW2(RME_PGT_SIZE_1G)*Count, RME_PGT_NOM, RME_PGT_SIZE_2M, RME_PGT_NUM_512)==0);
        Cur_Addr+=RME_KOM_ROUND(RME_PGT_SIZE_NOM(RME_PGT_NUM_512));
        RME_ASSERT(_RME_Pgt_Boot_Con(RME_X64_CPT, RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PDP(Count>>9)), Count&0x1FF,
                                       RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PDE(Count)), RME_PGT_ALL_PERM)==0);
    }

    /* Map all the Kom1 that we have into it */
    Page_Ptr=0;
    for(Kom1_Cnt=0;Kom1_Cnt<RME_X64_Layout.Kom1_Trunks;Kom1_Cnt++)
    {
        for(Count=0;Count<RME_X64_Layout.Kom1_Size[Kom1_Cnt];Count+=RME_POW2(RME_PGT_SIZE_2M))
        {
            Phys_Addr=RME_X64_VA2PA(RME_X64_Layout.Kom1_Start[Kom1_Cnt])+Count;
            RME_ASSERT(_RME_Pgt_Boot_Add(RME_X64_CPT, RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PDE(Page_Ptr>>9)),
                                           Phys_Addr, Page_Ptr&0x1FF, RME_PGT_ALL_PERM)==0);
            Page_Ptr++;
        }
    }
    RME_DBG_S("\r\nKom1 pages: 0x");
    RME_DBG_H(Page_Ptr);
    RME_DBG_S(", [0x0, 0x");
    RME_DBG_H(Page_Ptr*RME_POW2(RME_PGT_SIZE_2M)+RME_POW2(RME_PGT_SIZE_2M)-1);
    RME_DBG_S("]");

    /* Map the Kom2 in - don't want lookups, we know where they are. Offset by 2048 because they are mapped above 4G */
    RME_DBG_S("\r\nKom2 pages: 0x");
    RME_DBG_H(RME_X64_Layout.Kom2_Size/RME_POW2(RME_PGT_SIZE_2M));
    RME_DBG_S(", [0x");
    RME_DBG_H(Page_Ptr*RME_POW2(RME_PGT_SIZE_2M)+RME_POW2(RME_PGT_SIZE_2M));
    RME_DBG_S(", 0x");
    for(Count=2048;Count<(RME_X64_Layout.Kom2_Size/RME_POW2(RME_PGT_SIZE_2M)+2048);Count++)
    {
        Phys_Addr=RME_X64_PA2VA(RME_X64_MMU_ADDR(RME_X64_Kpgt.PDP[Count>>18][(Count>>9)&0x1FF]));
        Phys_Addr=RME_X64_MMU_ADDR(((rme_ptr_t*)Phys_Addr)[Count&0x1FF]);
        RME_ASSERT(_RME_Pgt_Boot_Add(RME_X64_CPT, RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PDE(Page_Ptr>>9)),
                                       Phys_Addr, Page_Ptr&0x1FF, RME_PGT_ALL_PERM)==0);
        Page_Ptr++;
    }
    RME_DBG_H(Page_Ptr*RME_POW2(RME_PGT_SIZE_2M)+RME_POW2(RME_PGT_SIZE_2M)-1);
    RME_DBG_S("]");

    /* Activate the first process - This process cannot be deleted */
    RME_ASSERT(_RME_Prc_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_PRC,
                                  RME_BOOT_INIT_CPT, RME_CAPID(RME_BOOT_TBL_PGT,RME_BOOT_PML4))==0);
    /* Create the initial kernel function capability */
    RME_ASSERT(_RME_Kfn_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_INIT_KERN)==0);

    /* Create a capability table for initial kernel memory capabilities. We need a few for Kom1, and another one for Kom2 */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_TBL_KOM, Cur_Addr, RME_X64_KOM1_MAXSEGS+1)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_X64_KOM1_MAXSEGS+1));
    /* Create Kom1 capabilities - can create page tables here */
    for(Count=0;Count<RME_X64_Layout.Kom1_Trunks;Count++)
    {
        RME_ASSERT(_RME_Kom_Boot_Crt(RME_X64_CPT,
                                      RME_BOOT_TBL_KOM, Count,
                                      RME_X64_Layout.Kom1_Start[Count],
                                      RME_X64_Layout.Kom1_Start[Count]+RME_X64_Layout.Kom1_Size[Count],
                                      RME_KOM_FLAG_ALL)==0);
    }
    /* Create Kom2 capability - cannot create page tables here */
    RME_ASSERT(_RME_Kom_Boot_Crt(RME_X64_CPT,
                                  RME_BOOT_TBL_KOM, RME_X64_KOM1_MAXSEGS,
                                  RME_X64_Layout.Kom2_Start,
                                  RME_X64_Layout.Kom2_Start+RME_X64_Layout.Kom2_Size,
                                  RME_KOM_FLAG_CPT|RME_KOM_FLAG_THD|RME_KOM_FLAG_INV)==0);

    /* Create the initial kernel endpoints for timer ticks */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_TBL_TIMER, Cur_Addr, RME_X64_Num_CPU)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_X64_Num_CPU));
    for(Count=0;Count<RME_X64_Num_CPU;Count++)
    {
    	CPU_Local=__RME_X64_CPU_Local_Get_By_CPUID(Count);
    	CPU_Local->Sig_Tim=&(RME_CAP_GETOBJ(&(RME_X64_CPT[RME_BOOT_TBL_TIMER]), struct RME_Cap_Sig*)[Count]);
        RME_ASSERT(_RME_Sig_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_TIMER, Count)==0);
    }

    /* Create the initial kernel endpoints for all other interrupts */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_TBL_INT, Cur_Addr, RME_X64_Num_CPU)==0);
    Cur_Addr+=RME_KOT_VA_BASE_ROUND(RME_CPT_SIZE(RME_X64_Num_CPU));
    for(Count=0;Count<RME_X64_Num_CPU;Count++)
    {
    	CPU_Local=__RME_X64_CPU_Local_Get_By_CPUID(Count);
    	CPU_Local->Sig_Vct=&(RME_CAP_GETOBJ(&(RME_X64_CPT[RME_BOOT_TBL_INT]), struct RME_Cap_Sig*)[Count]);
        RME_ASSERT(_RME_Sig_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_INT, Count)==0);
    }

    /* Activate the first thread, and set its priority */
    RME_ASSERT(_RME_Cpt_Boot_Crt(RME_X64_CPT, RME_BOOT_INIT_CPT, RME_BOOT_TBL_THD, Cur_Addr, RME_X64_Num_CPU)==0);
    Cur_Addr+=RME_KOM_ROUND(RME_CPT_SIZE(RME_X64_Num_CPU));
    for(Count=0;Count<RME_X64_Num_CPU;Count++)
    {
    	CPU_Local=__RME_X64_CPU_Local_Get_By_CPUID(Count);
        RME_ASSERT(_RME_Thd_Boot_Crt(RME_X64_CPT, RME_BOOT_TBL_THD, Count, RME_BOOT_INIT_PRC, Cur_Addr, 0, CPU_Local)>=0);
        Cur_Addr+=RME_KOM_ROUND(RME_THD_SIZE(Count));
    }

    RME_DBG_S("\r\nKot registration end offset: 0x");
    RME_DBG_H(((Cur_Addr-RME_KOM_VA_BASE)>>RME_KOM_SLOT_ORDER)/8);
    RME_DBG_S("\r\nKom1 frontier: 0x");
    RME_DBG_H(Cur_Addr);

    /* Print sizes and halt */
    RME_DBG_S("\r\nThread object size: ");
    RME_DBG_I(sizeof(struct RME_Thd_Struct)/sizeof(rme_ptr_t));
    RME_DBG_S("\r\nInvocation object size: ");
    RME_DBG_I(sizeof(struct RME_Inv_Struct)/sizeof(rme_ptr_t));

    /* Initialize the timer and start its interrupt routing */
    RME_DBG_S("\r\nTimer init");
    __RME_X64_Timer_Init();
    __RME_X64_IOAPIC_Int_Enable(2,RME_X64_CPU_Info[0].LAPIC_ID);
    /* Change page tables */
    RME_DBG_S("\r\nInit page table address:");
    RME_DBG_H(RME_CAP_GETOBJ((RME_CPU_LOCAL()->Thd_Cur)->Sched.Prc->Pgt,rme_ptr_t));
    __RME_Pgt_Set((RME_CPU_LOCAL()->Thd_Cur)->Sched.Prc->Pgt);

    /* Load the init process at its linked virtual address. */
    extern const unsigned char UVM_Init[];
    _RME_Memcpy((void*)RME_X64_UVM_BASE,(void*)UVM_Init,RME_POW2(RME_PGT_SIZE_2M));

    __RME_X64_Boot_Progress(RME_X64_BOOT_DONE);
    __RME_X64_Boot_Message("RME: Entering UVM");
    /* Release secondary processors after the init image is ready. */
    RME_X64_CPU_Cnt=0;
    __RME_Enter_User_Mode(RME_X64_UVM_BASE, RME_X64_UVM_BASE+RME_X64_USTACK(0), 0);
    return 0;
}
/* End Function:__RME_Boot ***************************************************/

/* Function:__RME_Reboot ******************************************************
Description : Reboot the machine, abandon all operating system states.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Reboot(void)
{
    /* Currently we cannot parse th FADT yet. We need these info to shutdown the machine */
    /* outportb(FADT->ResetReg.Address, FADT->ResetValue); */
    RME_ASSERT(RME_WORD_BITS!=RME_POW2(RME_WORD_ORDER));
}
/* End Function:__RME_Reboot *************************************************/

/* Function:__RME_Shutdown ****************************************************
Description : Shutdown the machine, abandon all operating system states.
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Shutdown(void)
{
    /* Currently we cannot parse th DSDT yet. We need these info to shutdown the machine */
    /* outw(PM1a_CNT,SLP_TYPa|SLP_EN) */
    RME_ASSERT(RME_WORD_BITS!=RME_POW2(RME_WORD_ORDER));
}
/* End Function:__RME_Shutdown ***********************************************/

/* Function:__RME_Get_Syscall_Param *******************************************
Description : Get the system call parameters from the stack frame.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : rme_ptr_t* Svc - The system service number.
              rme_ptr_t* Capid - The capability ID number.
              rme_ptr_t* Param - The parameters.
Return      : None.
******************************************************************************/
void __RME_Get_Syscall_Param(struct RME_Reg_Struct* Reg, rme_ptr_t* Svc, rme_ptr_t* Capid, rme_ptr_t* Param)
{
    *Svc=(Reg->RDI)>>32;
    *Capid=(Reg->RDI)&0xFFFFFFFF;
    Param[0]=Reg->RSI;
    Param[1]=Reg->RDX;
    Param[2]=Reg->R8;
}
/* End Function:__RME_Get_Syscall_Param **************************************/

/* Function:__RME_Set_Syscall_Retval ******************************************
Description : Set the system call return value to the stack frame. This function 
              may carry up to 4 return values. If the last 3 is not needed, just set
              them to zero.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Set_Syscall_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval)
{
    Reg->RAX=(rme_ptr_t)Retval;
}
/* End Function:__RME_Set_Syscall_Retval *************************************/

/* Function:__RME_Thd_Reg_Init ************************************************
Description : Initialize the register set for the thread.
Input       : rme_ptr_t Entry - The thread entry address.
              rme_ptr_t Stack - The thread stack address.
              rme_ptr_t Param - The parameter to pass to it.
Output      : struct RME_Reg_Struct* Reg - The register set content generated.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Init(rme_ptr_t Attr,rme_ptr_t Entry, rme_ptr_t Stack, rme_ptr_t Param, struct RME_Reg_Struct* Reg)
{
    /* We use the SYSRET path on creation if possible */
    Reg->INT_NUM=0x10000;
    Reg->ERROR_CODE=0;
    Reg->RIP=Entry;
    Reg->CS=RME_X64_SEG_USER_CODE;
    /* IOPL 3, IF */
    Reg->RFLAGS=0x3200;
    Reg->RSP=Stack;
    Reg->SS=RME_X64_SEG_USER_DATA;
    /* Pass the parameter */
    Reg->RDI=Param;
}
/* End Function:__RME_Thd_Reg_Init *******************************************/

/* Function:__RME_Thd_Reg_Copy ************************************************
Description : Copy one set of registers into another.
Input       : struct RME_Reg_Struct* Src - The source register set.
Output      : struct RME_Reg_Struct* Dst - The destination register set.
Return      : None.
******************************************************************************/
void __RME_Thd_Reg_Copy(struct RME_Reg_Struct* Dst, struct RME_Reg_Struct* Src)
{
    /* Make sure that the ordering is the same so the compiler can optimize */
    Dst->RAX=Src->RAX;
    Dst->RBX=Src->RBX;
    Dst->RCX=Src->RCX;
    Dst->RDX=Src->RDX;
    Dst->RSI=Src->RSI;
    Dst->RDI=Src->RDI;
    Dst->RBP=Src->RBP;
    Dst->R8=Src->R8;
    Dst->R9=Src->R9;
    Dst->R10=Src->R10;
    Dst->R11=Src->R11;
    Dst->R12=Src->R12;
    Dst->R13=Src->R13;
    Dst->R14=Src->R14;
    Dst->R15=Src->R15;
    /* Don't worry about user modifying INTNUM. If he or she did that it will corrupt userspace */
    Dst->INT_NUM=Src->INT_NUM;
    Dst->ERROR_CODE=Src->ERROR_CODE;
    /* This will always be canonical upon SYSRET, because we will truncate in on return */
    Dst->RIP=Src->RIP;
    Dst->CS=Src->CS;
    Dst->RFLAGS=Src->RFLAGS;
    Dst->RSP=Src->RSP;
    Dst->SS=Src->SS;
}
/* End Function:__RME_Thd_Reg_Copy *******************************************/

/* Function:__RME_Thd_Cop_Init ************************************************
Description : Initialize the coprocessor register set for the thread.
Input       : struct RME_Reg_Struct* Reg - The register struct to help initialize the coprocessor.
Output      : struct RME_Reg_Cop_Struct* Cop_Reg - The register set content generated.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Init(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    /* Empty function, return immediately. The FPU contents is not predictable */
}
/* End Function:__RME_Thd_Cop_Reg_Init ***************************************/

/* Function:__RME_Thd_Cop_Save ************************************************
Description : Save the co-op register sets. This operation is flexible - If the
              program does not use the FPU, we do not save its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Save(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    /* Not used for now */
}
/* End Function:__RME_Thd_Cop_Save *******************************************/

/* Function:__RME_Thd_Cop_Restore *********************************************
Description : Restore the co-op register sets. This operation is flexible - If the
              FPU is not used, we do not restore its context.
Input       : struct RME_Reg_Struct* Reg - The context, used to decide whether
                                           to save the context of the coprocessor.
Output      : struct RME_Cop_Struct* Cop_Reg - The pointer to the coprocessor contents.
Return      : None.
******************************************************************************/
void __RME_Thd_Cop_Restore(struct RME_Reg_Struct* Reg, struct RME_Cop_Struct* Cop_Reg)
{
    /* Not used for now */
}
/* End Function:__RME_Thd_Cop_Restore ****************************************/

/* Function:__RME_Inv_Reg_Save ************************************************
Description : Save the necessary registers on invocation for returning. Only the
              registers that will influence program control flow will be saved.
Input       : struct RME_Reg_Struct* Reg - The register set.
Output      : struct RME_Iret_Struct* Ret - The invocation return register context.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Save(struct RME_Iret_Struct* Ret, struct RME_Reg_Struct* Reg)
{
    Ret->RIP=Reg->RIP;
    Ret->RSP=Reg->RSP;
}
/* End Function:__RME_Inv_Reg_Save *******************************************/

/* Function:__RME_Inv_Reg_Restore *********************************************
Description : Restore the necessary registers for returning from an invocation.
Input       : struct RME_Iret_Struct* Ret - The invocation return register context.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Inv_Reg_Restore(struct RME_Reg_Struct* Reg, struct RME_Iret_Struct* Ret)
{
    Reg->RIP=Ret->RIP;
    Reg->RSP=Ret->RSP;
}
/* End Function:__RME_Inv_Reg_Restore ****************************************/

/* Function:__RME_Set_Inv_Retval **********************************************
Description : Set the invocation return value to the stack frame.
Input       : rme_ret_t Retval - The return value.
Output      : struct RME_Reg_Struct* Reg - The register set.
Return      : None.
******************************************************************************/
void __RME_Set_Inv_Retval(struct RME_Reg_Struct* Reg, rme_ret_t Retval)
{
    Reg->RDI=(rme_ptr_t)Retval;
}
/* End Function:__RME_Set_Inv_Retval *****************************************/

/* Crap for test */
void write_string( int colour, const char *string, rme_ptr_t pos)
{
    volatile char *video = (volatile char*)RME_X64_PA2VA(pos+0xB8000);
    while( *string != 0 )
    {
        *video++ = *string++;
        *video++ = colour;
    }
}

void NDBG(void)
{
    write_string( 0x07, "Here", 0);
}

/* Function:__RME_Kern_Func_Handler *******************************************
Description : Handle kernel function calls.
Input       : struct RME_Cap_Cpt* Cpt - The current capability table.
              struct RME_Reg_Struct* Reg - The current register set.
              rme_ptr_t Func_ID - The function ID.
              rme_ptr_t Sub_ID - The sub function ID.
              rme_ptr_t Param1 - The first parameter.
              rme_ptr_t Param2 - The second parameter.
Output      : None.
Return      : rme_ret_t - The value that the function returned.
******************************************************************************/
rme_ret_t __RME_Kern_Func_Handler(struct RME_Cap_Cpt* Cpt, struct RME_Reg_Struct* Reg,
                                  rme_ptr_t Func_ID, rme_ptr_t Sub_ID, rme_ptr_t Param1, rme_ptr_t Param2)
{
    /* Now always call the HALT */
    char String[16];

    String[0]=Param1/10000000+'0';
    String[1]=(Param1/1000000)%10+'0';
    String[2]=(Param1/100000)%10+'0';
    String[3]=(Param1/10000)%10+'0';
    String[4]=(Param1/1000)%10+'0';
    String[5]=(Param1/100)%10+'0';
    String[6]=(Param1/10)%10+'0';
    String[7]=(Param1)%10+'0';
    String[8]='\0';
    write_string(Func_ID, (const char *)String, Sub_ID);

    //__RME_X64_Halt();
    return 0;
}
/* End Function:__RME_Kern_Func_Handler **************************************/

/* Function:__RME_X64_Fault_Handler *******************************************
Description : The fault handler of RME. In x64, this is used to handle multiple
              faults.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
              rme_ptr_t Reason - The fault source.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void __RME_X64_Fault_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Reason)
{
    /* Not handling faults */
    RME_DBG_S("\n\r\n\r*** Fault: ");RME_DBG_I(Reason);RME_DBG_S(" - ");
    /* When handling debug exceptions, note CVE 2018-8897, we may get something at
     * kernel level - If this is what we have, the user must have touched SS + INT */
    /* Print reason */
    switch(Reason)
    {
        case RME_X64_FAULT_DE:RME_DBG_S("Divide error");break;
        case RME_X64_TRAP_DB:RME_DBG_S("Debug exception");break;
        case RME_X64_INT_NMI:RME_DBG_S("NMI error");break;
        case RME_X64_TRAP_BP:RME_DBG_S("Debug breakpoint");break;
        case RME_X64_TRAP_OF:RME_DBG_S("Overflow exception");break;
        case RME_X64_FAULT_BR:RME_DBG_S("Bound range exception");break;
        case RME_X64_FAULT_UD:RME_DBG_S("Undefined instruction");break;
        case RME_X64_FAULT_NM:RME_DBG_S("Device not available");break;
        case RME_X64_ABORT_DF:RME_DBG_S("Double(nested) fault exception");break;
        case RME_X64_ABORT_OLD_MF:RME_DBG_S("Coprocessor overrun - not used later on");break;
        case RME_X64_FAULT_TS:RME_DBG_S("Invalid TSS exception");break;
        case RME_X64_FAULT_NP:RME_DBG_S("Segment not present");break;
        case RME_X64_FAULT_SS:RME_DBG_S("Stack fault exception");break;
        case RME_X64_FAULT_GP:RME_DBG_S("General protection exception");break;
        case RME_X64_FAULT_PF:RME_DBG_S("Page fault exception");break;
        case RME_X64_FAULT_MF:RME_DBG_S("X87 FPU floating-point error:");break;
        case RME_X64_FAULT_AC:RME_DBG_S("Alignment check exception");break;
        case RME_X64_ABORT_MC:RME_DBG_S("Machine check exception");break;
        case RME_X64_FAULT_XM:RME_DBG_S("SIMD floating-point exception");break;
        case RME_X64_FAULT_VE:RME_DBG_S("Virtualization exception");break;
        default:RME_DBG_S("Unknown exception");break;
    }
    /* Print all registers */
    RME_DBG_S("\n\rRAX:        0x");RME_DBG_H(Reg->RAX);
    RME_DBG_S("\n\rRBX:        0x");RME_DBG_H(Reg->RBX);
    RME_DBG_S("\n\rRCX:        0x");RME_DBG_H(Reg->RCX);
    RME_DBG_S("\n\rRDX:        0x");RME_DBG_H(Reg->RDX);
    RME_DBG_S("\n\rRSI:        0x");RME_DBG_H(Reg->RSI);
    RME_DBG_S("\n\rRDI:        0x");RME_DBG_H(Reg->RDI);
    RME_DBG_S("\n\rRBP:        0x");RME_DBG_H(Reg->RBP);
    RME_DBG_S("\n\rR8:         0x");RME_DBG_H(Reg->R8);
    RME_DBG_S("\n\rR9:         0x");RME_DBG_H(Reg->R9);
    RME_DBG_S("\n\rR10:        0x");RME_DBG_H(Reg->R10);
    RME_DBG_S("\n\rR11:        0x");RME_DBG_H(Reg->R11);
    RME_DBG_S("\n\rR12:        0x");RME_DBG_H(Reg->R12);
    RME_DBG_S("\n\rR13:        0x");RME_DBG_H(Reg->R13);
    RME_DBG_S("\n\rR14:        0x");RME_DBG_H(Reg->R14);
    RME_DBG_S("\n\rR15:        0x");RME_DBG_H(Reg->R15);
    RME_DBG_S("\n\rINT_NUM:    0x");RME_DBG_H(Reg->INT_NUM);
    RME_DBG_S("\n\rERROR_CODE: 0x");RME_DBG_H(Reg->ERROR_CODE);
    RME_DBG_S("\n\rRIP:        0x");RME_DBG_H(Reg->RIP);
    RME_DBG_S("\n\rCS:         0x");RME_DBG_H(Reg->CS);
    RME_DBG_S("\n\rRFLAGS:     0x");RME_DBG_H(Reg->RFLAGS);
    RME_DBG_S("\n\rRSP:        0x");RME_DBG_H(Reg->RSP);
    RME_DBG_S("\n\rSS:         0x");RME_DBG_H(Reg->SS);
    RME_DBG_S("\n\rHang");

    while(1);
}
/* End Function:__RME_X64_Fault_Handler **************************************/

/* Function:__RME_X64_Generic_Handler *****************************************
Description : The generic interrupt handler of RME for x64.
Input       : struct RME_Reg_Struct* Reg - The register set when entering the handler.
              rme_ptr_t Int_Num - The interrupt number.
Output      : struct RME_Reg_Struct* Reg - The register set when exiting the handler.
Return      : None.
******************************************************************************/
void __RME_X64_Generic_Handler(struct RME_Reg_Struct* Reg, rme_ptr_t Int_Num)
{
    /* Not handling interrupts */
    RME_DBG_S("\r\nGeneral int:");
    RME_DBG_I(Int_Num);

    switch(Int_Num)
    {
        /* Is this a generic IPI from other processors? */

        default:break;
    }
    /* Remember to perform context switch after any kernel sends */
}
/* End Function:__RME_X64_Generic_Handler ************************************/

/* Function:__RME_Pgt_Set ***************************************************
Description : Set the processor's page table.
Input       : rme_ptr_t Pgt - The virtual address of the page table.
Output      : None.
Return      : None.
******************************************************************************/
void __RME_Pgt_Set(struct RME_Cap_Pgt* Pgt)
{
    __RME_X64_Pgt_Set(RME_X64_VA2PA(RME_CAP_GETOBJ(Pgt,rme_ptr_t)));
}
/* End Function:__RME_Pgt_Set **********************************************/

/* Function:__RME_Pgt_Check *************************************************
Description : Check if the page table parameters are feasible, according to the
              parameters. This is only used in page table creation.
Input       : rme_ptr_t Base_Addr - The start mapping address.
              rme_ptr_t Is_Top - The top-level flag,
              rme_ptr_t Size_Order - The size order of the page directory.
              rme_ptr_t Num_Order - The number order of the page directory.
              rme_ptr_t Vaddr - The virtual address of the page directory.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Check(rme_ptr_t Base_Addr, rme_ptr_t Is_Top,
                            rme_ptr_t Size_Order, rme_ptr_t Num_Order, rme_ptr_t Vaddr)
{
    /* Is the table address aligned to 4kB? */
    if((Vaddr&0xFFF)!=0)
        return RME_ERR_HAL_FAIL;

    /* Is the size order allowed? */
    if((Size_Order!=RME_PGT_SIZE_512G)&&(Size_Order!=RME_PGT_SIZE_1G)&&
       (Size_Order!=RME_PGT_SIZE_2M)&&(Size_Order!=RME_PGT_SIZE_4K))
        return RME_ERR_HAL_FAIL;

    /* Is the top-level relationship correct? */
    if(((Size_Order==RME_PGT_SIZE_512G)^(Is_Top!=0))!=0)
        return RME_ERR_HAL_FAIL;

    /* Is the number order allowed? */
    if(Num_Order!=RME_PGT_NUM_512)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Check ********************************************/

/* Function:__RME_Pgt_Init **************************************************
Description : Initialize the page table data structure, according to the capability.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Init(struct RME_Cap_Pgt* Pgt_Op)
{
    rme_cnt_t Count;
    rme_ptr_t* Ptr;
    
    /* Get the actual table */
    Ptr=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* Hopefully the compiler optimize this to rep stos */
    for(Count=0;Count<256;Count++)
        Ptr[Count]=0;

    /* Hopefully the compiler optimize this to rep movs */
    if((Pgt_Op->Base&RME_PGT_TOP)!=0)
    {
        for(;Count<512;Count++)
            Ptr[Count]=RME_X64_Kpgt.PML4[Count-256];
    }
    else
    {
        for(;Count<512;Count++)
            Ptr[Count]=0;
    }

    return 0;
}
/* End Function:__RME_Pgt_Init *********************************************/

/* Function:__RME_Pgt_Del_Check *********************************************
Description : Check if the page table can be deleted.
Input       : struct RME_Cap_Pgt Pgt_Op* - The capability to the page table to operate on.
Output      : None.
Return      : rme_ptr_t - If can be deleted, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Del_Check(struct RME_Cap_Pgt* Pgt_Op)
{

    return 0;
}
/* End Function:__RME_Pgt_Del_Check ****************************************/

/* Function:__RME_Pgt_Page_Map **********************************************
Description : Map a page into the page table. This architecture requires that the mapping is
              always at least readable.
Input       : struct RME_Cap_Pgt* - The cap ability to the page table to operate on.
              rme_ptr_t Paddr - The physical address to map to. If we are unmapping, this have no effect.
              rme_ptr_t Pos - The position in the page table.
              rme_ptr_t Flags - The RME standard page attributes. Need to translate them into
                                architecture specific page table's settings.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Page_Map(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Paddr, rme_ptr_t Pos, rme_ptr_t Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t X64_Flags;

    /* It should at least be readable */
    if((Flags&RME_PGT_READ)==0)
        return RME_ERR_HAL_FAIL;

    /* Are we trying to map into the kernel space on the top level? */
    if(((Pgt_Op->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* Generate flags */
    if(RME_PGT_SZORD(Pgt_Op->Order)==RME_PGT_SIZE_4K)
        X64_Flags=RME_X64_MMU_ADDR(Paddr)|RME_X64_PGFLG_RME2NAT(Flags)|RME_X64_MMU_US;
    else
        X64_Flags=RME_X64_MMU_ADDR(Paddr)|RME_X64_PGFLG_RME2NAT(Flags)|RME_X64_MMU_PDE_SUP|RME_X64_MMU_US;

    /* Try to map it in */
    if(RME_COMP_SWAP(&(Table[Pos]),0,X64_Flags)==0)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Page_Map *****************************************/

/* Function:__RME_Pgt_Page_Unmap ********************************************
Description : Unmap a page from the page table.
Input       : struct RME_Cap_Pgt* - The capability to the page table to operate on.
              rme_ptr_t Pos - The position in the page table.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Page_Unmap(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;

    /* Are we trying to unmap the kernel space on the top level? */
    if(((Pgt_Op->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);

    /* Make sure that there is something */
    Temp=Table[Pos];
    if(Temp==0)
        return RME_ERR_HAL_FAIL;

    /* Is this a page directory? We cannot unmap page directories like this */
    if((RME_PGT_SZORD(Pgt_Op->Order)!=RME_PGT_SIZE_4K)&&((Temp&RME_X64_MMU_PDE_SUP)==0))
        return RME_ERR_HAL_FAIL;

    /* Try to unmap it. Use CAS just in case */
    if(RME_COMP_SWAP(&(Table[Pos]),Temp,0)==0)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Page_Unmap ***************************************/

/* Function:__RME_Pgt_Pgdir_Map *********************************************
Description : Map a page directory into the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table.
              rme_ptr_t Pos - The position in the destination page table.
              rme_ptr_t Flags - The RME standard flags for the child page table.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Pgdir_Map(struct RME_Cap_Pgt* Pgt_Parent, rme_ptr_t Pos,
                                struct RME_Cap_Pgt* Pgt_Child, rme_ptr_t Flags)
{
    rme_ptr_t* Parent_Table;
    rme_ptr_t* Child_Table;
    rme_ptr_t X64_Flags;

    /* It should at least be readable */
    if((Flags&RME_PGT_READ)==0)
        return RME_ERR_HAL_FAIL;

    /* Are we trying to map into the kernel space on the top level? */
    if(((Pgt_Parent->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Parent_Table=RME_CAP_GETOBJ(Pgt_Parent,rme_ptr_t*);
    Child_Table=RME_CAP_GETOBJ(Pgt_Child,rme_ptr_t*);

    /* Generate the content */
    X64_Flags=RME_X64_MMU_ADDR(RME_X64_VA2PA(Child_Table))|RME_X64_PGFLG_RME2NAT(Flags)|RME_X64_MMU_US;

    /* Try to map it in - may need to increase some count */
    if(RME_COMP_SWAP(&(Parent_Table[Pos]),0,X64_Flags)==0)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Map ****************************************/

/* Function:__RME_Pgt_Pgdir_Unmap *******************************************
Description : Unmap a page directory from the page table.
Input       : struct RME_Cap_Pgt* Pgt_Parent - The parent page table to unmap from.
              rme_ptr_t Pos - The position in the page table.
              struct RME_Cap_Pgt* Pgt_Child - The child page table to unmap.
Output      : None.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Pgdir_Unmap(struct RME_Cap_Pgt* Pgt_Parent, rme_ptr_t Pos,
                                  struct RME_Cap_Pgt* Pgt_Child)
{
    rme_ptr_t* Parent_Table;
    rme_ptr_t* Child_Table;
    rme_ptr_t Temp;

    /* Are we trying to unmap the kernel space on the top level? */
    if(((Pgt_Parent->Base&RME_PGT_TOP)!=0)&&(Pos>=256))
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Parent_Table=RME_CAP_GETOBJ(Pgt_Parent,rme_ptr_t*);

    /* Make sure that there is something */
    Temp=Parent_Table[Pos];
    if(Temp==0)
        return RME_ERR_HAL_FAIL;

    /* Is this a page? We cannot unmap pages like this */
    if((RME_PGT_SZORD(Pgt_Parent->Order)==RME_PGT_SIZE_4K)||((Temp&RME_X64_MMU_PDE_SUP)!=0))
        return RME_ERR_HAL_FAIL;

    /* Is this child table mapped here? - check that in the future */

    Child_Table=(rme_ptr_t*)Temp;
    rme_ptr_t* Expected_Child_Table = RME_CAP_GETOBJ(Pgt_Child, rme_ptr_t*);
    if (Child_Table != Expected_Child_Table)
        return RME_ERR_HAL_FAIL;
    /* Try to unmap it. Use CAS just in case */
    if(RME_COMP_SWAP(&(Parent_Table[Pos]),Temp,0)==0)
        return RME_ERR_HAL_FAIL;

    return 0;
}
/* End Function:__RME_Pgt_Pgdir_Unmap **************************************/

/* Function:__RME_Pgt_Lookup **************************************************
Description : Lookup a page entry in a page directory.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page directory to lookup.
              rme_ptr_t Pos - The position to look up.
Output      : rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Lookup(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Pos, rme_ptr_t* Paddr, rme_ptr_t* Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t Temp;

    /* Check if the position is within the range of this page table */
    if((Pos>>RME_PGT_NMORD(Pgt_Op->Order))!=0)
        return RME_ERR_HAL_FAIL;

    /* Get the table */
    Table=RME_CAP_GETOBJ(Pgt_Op,rme_ptr_t*);
    /* Get the position requested - atomic read */
    Temp=Table[Pos];

    /* Start lookup - is this a terminal page, or? */
    if(RME_PGT_SZORD(Pgt_Op->Order)==RME_PGT_SIZE_4K)
    {
        if((Temp&RME_X64_MMU_P)==0)
            return RME_ERR_HAL_FAIL;
    }
    else
    {
        if(((Temp&RME_X64_MMU_P)==0)||((Temp&RME_X64_MMU_PDE_SUP)==0))
            return RME_ERR_HAL_FAIL;
    }

    /* This is a page. Return the physical address and flags */
    if(Paddr!=0)
        *Paddr=RME_X64_MMU_ADDR(Temp);

    if(Flags!=0)
        *Flags=RME_X64_PGFLG_NAT2RME(Temp);

    return 0;
}
/* End Function:__RME_Pgt_Lookup *******************************************/

/* Function:__RME_Pgt_Walk **************************************************
Description : Walking function for the page table. This function just does page
              table lookups. The page table that is being walked must be the top-
              level page table. The output values are optional; only pass in pointers
              when you need that value.
              Walking kernel page tables is prohibited.
Input       : struct RME_Cap_Pgt* Pgt_Op - The page table to walk.
              rme_ptr_t Vaddr - The virtual address to look up.
Output      : rme_ptr_t* Pgt - The pointer to the page table level.
              rme_ptr_t* Map_Vaddr - The virtual address that starts mapping.
              rme_ptr_t* Paddr - The physical address of the page.
              rme_ptr_t* Size_Order - The size order of the page.
              rme_ptr_t* Num_Order - The entry order of the page.
              rme_ptr_t* Flags - The RME standard flags of the page.
Return      : rme_ptr_t - If successful, 0; else RME_ERR_HAL_FAIL.
******************************************************************************/
rme_ptr_t __RME_Pgt_Walk(struct RME_Cap_Pgt* Pgt_Op, rme_ptr_t Vaddr, rme_ptr_t* Pgt,
                           rme_ptr_t* Map_Vaddr, rme_ptr_t* Paddr, rme_ptr_t* Size_Order, rme_ptr_t* Num_Order, rme_ptr_t* Flags)
{
    rme_ptr_t* Table;
    rme_ptr_t Pos;
    rme_ptr_t Temp;
    rme_ptr_t Size_Cnt;
    /* Accumulates the flag information about each level - these bits are ANDed */
    rme_ptr_t Flags_Accum;
    /* No execute bit - this bit is ORed */
    rme_ptr_t No_Execute;

    /* Check if this is the top-level page table */
    if(((Pgt_Op->Base)&RME_PGT_TOP)==0)
        return RME_ERR_HAL_FAIL;

    /* Are we attempting a kernel or non-canonical lookup? If yes, stop immediately */
    if(Vaddr>=0x7FFFFFFFFFFFULL)
        return RME_ERR_HAL_FAIL;

    /* Get the table and start lookup */
    Table=RME_CAP_GETOBJ(Pgt_Op, rme_ptr_t*);

    /* Do lookup recursively */
    Size_Cnt=RME_PGT_SIZE_512G;
    Flags_Accum=0xFFF;
    No_Execute=0;
    while(1)
    {
        /* Calculate where is the entry - always 0 to 512*/
        Pos=(Vaddr>>Size_Cnt)&0x1FF;
        /* Atomic read */
        Temp=Table[Pos];
        /* Find the position of the entry - Is there a page, a directory, or nothing? */
        if((Temp&RME_X64_MMU_P)==0)
            return RME_ERR_HAL_FAIL;
        if(((Temp&RME_X64_MMU_PDE_SUP)!=0)||(Size_Cnt==RME_PGT_SIZE_4K))
        {
            /* This is a page - we found it */
            if(Pgt!=0)
                *Pgt=(rme_ptr_t)Table;
            if(Map_Vaddr!=0)
                *Map_Vaddr=RME_ROUND_DOWN(Vaddr,Size_Cnt);
            if(Paddr!=0)
                *Paddr=RME_X64_MMU_ADDR(Temp);
            if(Size_Order!=0)
                *Size_Order=Size_Cnt;
            if(Num_Order!=0)
                *Num_Order=RME_PGT_NUM_512;
            if(Flags!=0)
                *Flags=RME_X64_PGFLG_NAT2RME(No_Execute|(Temp&Flags_Accum));

            break;
        }
        else
        {
            /* This is a directory, we goto that directory to continue walking */
            Flags_Accum&=Temp;
            No_Execute|=Temp&RME_X64_MMU_NX;
            Table=(rme_ptr_t*)RME_X64_PA2VA(RME_X64_MMU_ADDR(Temp));
        }

        /* The size order always decreases by 512 */
        Size_Cnt-=RME_PGT_SIZE_512B;
    }

    return 0;
}

/*Function:__RME_Svc_Param_Get**************************************************
Description:Get system call Params from Regs.
********************************************************************************/
void __RME_Svc_Param_Get(struct RME_Reg_Struct* Reg,rme_ptr_t* Svc,rme_ptr_t* Cid,rme_ptr_t* Param)
{
    *Svc=(Reg->RDI)>>32;
    *Cid=(Reg->RDI)&0xFFFFFFFF;
    Param[0]=Reg->RSI;
    Param[1]=Reg->RDX;
    Param[2]=Reg->R8;
}

/*End Function:__RME_Svc_Param_Get***************************************************/

/*Function:__RME_Svc_Retval_Set**************************************************
Description:Set system call return values.
********************************************************************************/
void __RME_Svc_Retval_Set(struct RME_Reg_Struct* Reg,rme_ret_t Retval)
{
    Reg->RAX=(rme_ptr_t)Retval;
}
/*End Function:__RME_Svc_Retval_Set********************************************/

/*Function:__RME_Inv_Retval_Set**************************************************
Description:Set invocation return values.
********************************************************************************/
void __RME_Inv_Retval_Set(struct RME_Reg_Struct* Reg,rme_ret_t Retval)
{
    Reg->RDI=(rme_ptr_t)Retval;
}
/*End Function:__RME_Inv_Retval_Set********************************************/

/*Function:__RME_List_Crt**************************************************
Description:Create a doubly linked list. 
********************************************************************************/
void __RME_List_Crt(struct RME_List* Head)
{
    Head->Next=Head;
    Head->Prev=Head;
}
/*End Function:__RME_List_Crt********************************************/

/*Function:__RME_List_Ins**************************************************
Description:Insert into a doubly linked list.
********************************************************************************/
void __RME_List_Ins(struct RME_List* New,struct RME_List* Prev,struct RME_List* Next)
{
    New->Prev=Prev;
    Prev->Next=New;
    New->Next=Next;
    Next->Prev=New;
}
/*End Function:__RME_List_Ins********************************************/

/*Function:__RME_List_Del**************************************************
Description:Delete a doubly linked list.
********************************************************************************/
void __RME_List_Del(struct RME_List* Prev,struct RME_List* Next)
{
    Prev->Next = Next;
    Next->Prev = Prev;
}
/*End Function:__RME_List_Del********************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
