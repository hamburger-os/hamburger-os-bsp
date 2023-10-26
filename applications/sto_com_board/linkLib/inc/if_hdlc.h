/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : if_hdlc.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Implement the function interfaces of if_hdlc
 ********************************************************************************************/
#ifndef _IF_HDLC_H
#define _IF_HDLC_H

#include "common.h"

/***************hdlc channel*****************/
typedef enum
{
    E_HDLC_CH_1 = 0U,
    E_HDLC_CH_MAX
} E_HDLC_CH;

extern BOOL if_hdlc_init(void);
extern BOOL if_hdlc_send(E_HDLC_CH ch, uint8 *pdata, uint16 len);
extern uint16 if_hdlc_get(E_HDLC_CH ch, uint8 *pdata, uint16 len);

#endif
/*******************************************end file****************************************/
