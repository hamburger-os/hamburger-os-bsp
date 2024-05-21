/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_eth.h
 **@author: Created By jiaqx
 **@date  : 2023.11.07
 **@brief : Manage devlib eth
 ********************************************************************************************/
#ifndef DEVLIB_MVB_H
#define DEVLIB_MVB_H

#include "support_mvb.h"
typedef enum
{
    E_MVB_EX1 = 0U,
    E_MVB_MAX
}E_MVB_CLASS;


extern BOOL devlib_mvb_sendData(E_MVB_CLASS e_mvb_cls, S_MVB_FRAME *pframe);
extern BOOL devlib_mvb_getData(E_MVB_CLASS e_mvb_cls, S_MVB_FRAME *pframe);

#endif
/**************************************end file*********************************************/

