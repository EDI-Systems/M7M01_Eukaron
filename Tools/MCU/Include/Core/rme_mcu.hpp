/******************************************************************************
Filename    : rme_mcu.hpp
Author      : pry
Date        : 08/04/2017
Licence     : LGPL v3+; see COPYING for details.
Description : The header of the mcu tool.
******************************************************************************/

/* Defines *******************************************************************/
#ifdef __HDR_DEFS__
#ifndef __RME_MCU_HPP_TYPES__
#define __RME_MCU_HPP_TYPES__
/*****************************************************************************/
typedef char                                s8_t;
typedef short                               s16_t;
typedef int                                 s32_t;
typedef long long                           s64_t;
typedef unsigned char                       u8_t;
typedef unsigned short                      u16_t;
typedef unsigned int                        u32_t;
typedef unsigned long long                  u64_t;
/* Make things compatible in 32-bit or 64-bit environments */
typedef s64_t                               ret_t;
typedef u64_t                               ptr_t;
/*****************************************************************************/
/* __RME_MCU_HPP_TYPES__ */
#endif
/* __HDR_DEFS__ */
#endif

namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_MCU_HPP_DEFS__
#define __RME_MCU_HPP_DEFS__
/*****************************************************************************/
/* EXTERN definition */
#define EXTERN                              extern

/* Power of 2 macros */
#define ALIGN_POW(X,POW)                    (((X)>>(POW))<<(POW))
#define POW2(POW)                           (((ptr_t)1)<<(POW))

/* The alignment value used when printing macros */
#define MACRO_ALIGNMENT                     (56)
/* The code generator author name */
#define CODE_AUTHOR                         ("The A7M project generator.")

/* Generic kernel object sizes */
#define CAPTBL_SIZE(NUM,BITS)               ((BITS)/8*8*(NUM))
#define PROC_SIZE(BITS)                     ((BITS)/8*8)
#define SIG_SIZE(BITS)                      ((BITS)/8*4)

/* Interrupt flag area size (in bytes) */
#define KERNEL_INTF_SIZE                    (1024)
/* Entry point slot size (in words) */
#define ENTRY_SLOT_SIZE                     (8)

/* Kerneo object size rounding macro */
#define KOTBL_ROUND(SIZE)                   (SIZE)
/* Compute the total capability table size when given the macros */
#define CAPTBL_TOTAL(NUM,CAPACITY,BITS)     (((NUM)/(CAPACITY))*KOTBL_ROUND(CAPTBL_SIZE(CAPACITY,(BITS)))+ \
                                             KOTBL_ROUND(CAPTBL_SIZE((NUM)%(CAPACITY),(BITS))))
/*****************************************************************************/
/* __RME_MCU_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_MCU_HPP_CLASSES__
#define __RME_MCU_HPP_CLASSES__
/*****************************************************************************/
/* The application instance class */
class Main
{
private:
    void Alloc_Code(void);
    void Check_Code(void);
    void Alloc_Data(void);
    void Check_Device(void);

    void Check_Kobj(void);
    void Alloc_Loc(void);
    void Alloc_RVM_Pgtbl(std::unique_ptr<class Proc>& Proc,
                         std::unique_ptr<class Pgtbl>& Pgtbl);
    void Alloc_RVM(void);
    void Alloc_Macro_Pgtbl(std::unique_ptr<class Proc>& Proc,
                           std::unique_ptr<class Pgtbl>& Pgtbl);
    void Alloc_Macro(void);
    void Backprop_RVM(void);

public:
    std::unique_ptr<std::string> Input;
    std::unique_ptr<std::string> Output;
    std::unique_ptr<std::string> Format;

    std::unique_ptr<class Fsys> Fsys;
    std::unique_ptr<class Proj> Proj;
    std::unique_ptr<class Chip> Chip;
    std::unique_ptr<class Plat> Plat;
    
    Main(int argc, char* argv[]);

    void Parse(void);
    void Alloc_Mem(void);
    void Alloc_Captbl(void);
};
/*****************************************************************************/
/* __RME_MCU_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
