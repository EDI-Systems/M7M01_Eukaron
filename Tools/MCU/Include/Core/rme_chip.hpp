/******************************************************************************
Filename    : rme_chip.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the chip class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_CHIP_HPP_DEFS__
#define __RME_CHIP_HPP_DEFS__
/*****************************************************************************/
/* Option types */
#define OPTION_RANGE        (0)
#define OPTION_SELECT       (1)
/*****************************************************************************/
/* __RME_CHIP_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_CHIP_HPP_CLASSES__
#define __RME_CHIP_HPP_CLASSES__
/*****************************************************************************/
/* The option information */
class Option
{
public:
    /* Name*/
    std::unique_ptr<std::string> Name;
    /* Type of the option, either range or select */
    ptr_t Type;
    /* Macro of the option */
    std::unique_ptr<std::string> Macro;
    /* Range of the option */
    std::vector<std::unique_ptr<std::string>> Range;

    Option(xml_node_t* Node);
    ~Option(void){};
};

/* Chip information */
class Chip
{
public:
    /* The name of the chip class */
	std::unique_ptr<std::string> Chip_Class;
    /* Compatible chip list */
	std::unique_ptr<std::string> Chip_Compat;
    /* The vendor */
    std::unique_ptr<std::string> Vendor;
    /* The platform */
	std::unique_ptr<std::string> Plat;
    /* The number of CPU cores */
	ptr_t Cores;
    /* The number of MPU regions */
    ptr_t Regions;

    /* The platform-specific attributes to be passed to the platform-specific generator */
    std::vector<std::unique_ptr<class Raw>> Attr;
    /* Memory information */
    std::vector<std::unique_ptr<class Mem>> Code;
    std::vector<std::unique_ptr<class Mem>> Data;
    std::vector<std::unique_ptr<class Mem>> Device;
    /* Raw option information */
	std::vector<std::unique_ptr<class Option>> Option;
    /* Interrupt vector information */
	std::vector<std::unique_ptr<class Vect>> Vect;

    Chip(xml_node_t* Node);
    ~Chip(void){};
};
/*****************************************************************************/
/* __RME_CHIP_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End Classes ***************************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
