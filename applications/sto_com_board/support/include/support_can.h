/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_can.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support CAN
 ********************************************************************************************/
#ifndef _SUPPORT_CAN_H
#define _SUPPORT_CAN_H

#include "common.h"

/*****************can id**********************/
typedef enum
{
    E_CAN_ID_1 = 0U,
    E_CAN_ID_2,
    E_CAN_ID_3,
    E_CAN_ID_4,
    E_CAN_ID_5,
    E_CAN_ID_MAX
} E_CAN_ID;

/*****************can state*******************/
typedef enum
{
    E_CAN_OK = 0U,
    E_CAN_ERR
} E_CAN_STATE;

/***************can mode*****************/
typedef enum
{
    E_CAN_NORMAL_MODE = 0U,
    E_CAN_FD_MODE,
    E_CAN_MODE_MAX
} E_CAN_MODE;

/******************can frame******************/
/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    /*******************************************
     **  11bit ID=优先级(8bit)+帧号(3bit)
     ********************************************/
    E_CAN_MODE can_mode;
    uint8 priority_u8;
    uint8 no_u8;
    uint8 length_u8;
    uint8 data_u8[8U];
} S_CAN_FRAME;

#pragma pack()

extern E_CAN_STATE support_can_init(E_CAN_ID id);
extern E_CAN_STATE support_can_sendData(E_CAN_ID id, S_CAN_FRAME *p_can_frame);
extern E_CAN_STATE supprot_can_getData(E_CAN_ID id, S_CAN_FRAME *p_can_frame);

#endif
/**************************************end file*********************************************/

