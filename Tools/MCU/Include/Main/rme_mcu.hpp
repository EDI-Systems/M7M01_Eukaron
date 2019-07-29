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
/* Power of 2 macros */
#define POW2(POW)                           (((ptr_t)1)<<(POW))
#define ROUND_DOWN(X,POW)                   (((X)>>(POW))<<(POW))
#define ROUND_UP(X,POW)                     ROUND_DOWN((X)+POW2(POW)-1,POW)

/* The code generator author name */
#define CODE_AUTHOR                         "The RME project generator."
/* The license for the generator */
#define CODE_LICENSE                        "LGPL v3+; see COPYING for details."
/* Generator macro alignment */
#define MACRO_ALIGN                         (56)

/* Interrupt flag area size (in bytes), fixed across all architectures */
#define KERNEL_INTF_SIZE                    (1024)
/* Entry point slot size (in words), fixed across all architectures */
#define ENTRY_SLOT_SIZE                     (8)
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
    void Alloc_Cap(void);
    void Link_Cap(void);
    void Alloc_Obj(void);

    void Gen_RME(void);
    void Gen_RVM(void);
    void Gen_Proc(void);
    void Gen_Proj(void);
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
