/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : if_can.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Implement the function interfaces of if_can
 ********************************************************************************************/
#ifndef _IF_CAN_H
#define _IF_CAN_H

#include "common.h"
/***************can channel*****************/
typedef enum
{
    E_CAN_CH_1 = 0U,
    E_CAN_CH_2,
    E_CAN_CH_3,
    E_CAN_CH_4,
    E_CAN_CH_5,
    E_CAN_CH_MAX
} E_CAN_CH;

/***************can mode*****************/
typedef enum
{
    E_CAN_MSG_NORMAL_MODE = 0U,
    E_CAN_MSG_FD_MODE,
    E_CAN_MSG_MODE_MAX
} E_CAN_MSG_MODE;

/****************can msg*********************/
typedef struct
{
    uint32 id_u32;
    uint8 len_u8;
    uint8 data_u8[64U];
    E_CAN_MSG_MODE can_mode;
} S_CAN_MSG;

/****************can interface**************/
extern BOOL if_can_init(void);
extern BOOL if_can_send(E_CAN_CH ch, S_CAN_MSG *pMsg);
extern BOOL if_can_get(E_CAN_CH ch, S_CAN_MSG *pMsg);

#endif
/*******************************************end file****************************************/
