/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_rs485.h
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support rs485
 ********************************************************************************************/
#ifndef _SUPPORT_RS485_H
#define _SUPPORT_RS485_H

#include "common.h"

/*****************RS485 id**********************/
typedef enum
{
    E_RS485_ID_1 = 0U,
    E_RS485_ID_MAX
} E_RS485_ID;

/*****************RS485 state*******************/
typedef enum
{
    E_RS485_OK = 0U,
    E_RS485_ERR
} E_RS485_STATE;

/******************RS485 frame******************/
#define RS485_FRAME_SIZE_MAX    ( 256U )      /* RS485单帧最大值  */

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    uint16 len;
    uint8 data_u8[RS485_FRAME_SIZE_MAX];
} S_RS485_FRAME;

#pragma pack()

extern E_RS485_STATE support_rs485_init(E_RS485_ID id);
extern E_RS485_STATE support_rs485_sendData(E_RS485_ID id, S_RS485_FRAME *pframe);
extern E_RS485_STATE supprot_rs485_getData(E_RS485_ID id, S_RS485_FRAME *pframe);

#endif
/**************************************end file*********************************************/

