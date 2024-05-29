/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_eth.h
 **@author: Created By jiaqx
 **@date  : 2023.11.07
 **@brief : Manage devlib eth
 ********************************************************************************************/
#ifndef DEVLIB_RS485_H
#define DEVLIB_RS485_H

#include "support_rs485.h"

typedef enum
{
    E_RS485_EX1 = 0U,
    E_RS485_MAX
}E_RS485_CLASS;


extern BOOL devLib_rs485_sendData( E_RS485_CLASS e_rs485_cls, S_RS485_FRAME *pframe );
extern BOOL devLib_rs485_getData( E_RS485_CLASS e_rs485_cls, S_RS485_FRAME *pframe );

#endif
/**************************************end file*********************************************/

