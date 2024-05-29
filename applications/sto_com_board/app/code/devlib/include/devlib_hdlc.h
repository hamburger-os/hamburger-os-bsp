/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_eth.h
 **@author: Created By jiaqx
 **@date  : 2023.11.07
 **@brief : Manage devlib eth
 ********************************************************************************************/
#ifndef DEVLIB_HDLC_H
#define DEVLIB_HDLC_H

#include "support_hdlc.h"
typedef enum
{
    E_HDLC_EX1 = 0U,
    E_HDLC_MAX
}E_HDLC_CLASS;


extern BOOL devlib_hdlc_sendData(E_HDLC_CLASS e_hdlc_cls, S_HDLC_FRAME *pframe);
extern BOOL devlib_hdlc_getData(E_HDLC_CLASS e_hdlc_cls, S_HDLC_FRAME *pframe);

#endif
/**************************************end file*********************************************/

