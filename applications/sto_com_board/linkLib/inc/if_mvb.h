/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : if_mvb.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Implement the function interfaces of if_mvb
 ********************************************************************************************/
#ifndef _IF_MVB_H
#define _IF_MVB_H

#include "common.h"

/***************mvb channel*****************/
typedef enum
{
    E_MVB_CH_1 = 0U,
    E_MVB_CH_MAX
} E_MVB_CH;

extern BOOL if_mvb_init(void);
extern BOOL if_mvb_send(E_MVB_CH ch, uint8 *pdata, uint16 len);
extern uint16 if_mvb_get(E_MVB_CH ch, uint8 *pdata, uint16 len);

#endif
/*******************************************end file****************************************/
