/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : if_rs485.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Implement the function interfaces of if_rs485
 ********************************************************************************************/
#ifndef _IF_RS485_H
#define _IF_RS485_H

#include "common.h"
/***************rs485 channel*****************/
typedef enum
{
    E_RS485_CH_1 = 0U,
    E_RS485_CH_MAX
} E_RS485_CH;

extern BOOL if_rs485_init(void);
extern BOOL if_rs485_send(E_RS485_CH ch, uint8 *pdata, uint16 len);
extern uint16 if_rs485_get(E_RS485_CH ch, uint8 *pdata, uint16 len);

#endif
/*******************************************end file****************************************/
