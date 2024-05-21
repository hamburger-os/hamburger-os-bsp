/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : devlib_eth.h
 **@author: Created By jiaqx
 **@date  : 2023.11.07
 **@brief : Manage devlib eth
 ********************************************************************************************/
#ifndef DEVLIB_CAN_H
#define DEVLIB_CAN_H

#include "support_can.h"

typedef enum
{
    E_CAN_ZK1,
    E_CAN_ZK2,
    E_CAN_EX1,
    E_CAN_EX2,
    E_CAN_EX3,
    E_CAN_MAX
}E_CAN_CLASS;

/******************can frame******************/
/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    /*******************************************
     **  11bit ID=优先级(8bit)+帧号(3bit)
     ********************************************/
    uint8 priority_u8;
    uint8 no_u8;
    uint8 length_u8;
    uint8 data_u8[8U];
} S_CAN_FRAME_DATA;

#pragma pack()

extern BOOL devLib_can_sendData( E_CAN_CLASS can_cls, S_CAN_FRAME_DATA *p_can_frame_data );
extern BOOL devLib_can_getData( E_CAN_CLASS can_cls, S_CAN_FRAME_DATA *p_can_frame_data );

#endif
/**************************************end file*********************************************/

