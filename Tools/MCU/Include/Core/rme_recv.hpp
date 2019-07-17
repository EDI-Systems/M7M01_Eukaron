/******************************************************************************
Filename    : rme_recv.hpp
Author      : pry
Date        : 16/07/2019
Licence     : LGPL v3+; see COPYING for details.
Description : The header for the receive class.
******************************************************************************/

/* Defines *******************************************************************/
namespace rme_mcu
{
#ifdef __HDR_DEFS__
#ifndef __RME_RECV_HPP_DEFS__
#define __RME_RECV_HPP_DEFS__
/*****************************************************************************/

/*****************************************************************************/
/* __RME_RECV_HPP_DEFS__ */
#endif
/* __HDR_DEFS__ */
#endif
/* End Defines ***************************************************************/

/* Classes *******************************************************************/
#ifdef __HDR_CLASSES__
#ifndef __RME_RECV_HPP_CLASSES__
#define __RME_RECV_HPP_CLASSES__

/* Use defines in these headers */
#define __HDR_DEFS__
#undef __HDR_DEFS__

/*****************************************************************************/
/* Receive endpoint information */
class Recv:public Kobj
{
public:
    Recv(xml_node_t* Node);
    ~Recv(void);
};
/*****************************************************************************/
/* __RME_RECV_HPP_CLASSES__ */
#endif
/* __HDR_CLASSES__ */
#endif
}
/* End C++ Classes ***********************************************************/

/* End Of File ***************************************************************/

/* Copyright (C) Evo-Devo Instrum. All rights reserved ***********************/
