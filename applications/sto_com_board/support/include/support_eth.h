/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_eth.h
 **@author: Created By Chengt
 **@date  : 2019.12.13
 **@brief : Manage support ETH
 ********************************************************************************************/
#ifndef _SUPPORT_ETH_H
#define _SUPPORT_ETH_H

#include "common.h"
/****************eth type*********************/
typedef enum
{
    E_ETH_ID_1 = 0U,
    E_ETH_ID_2,
    E_ETH_ID_3,
    E_ETH_ID_MAX
} E_ETH_ID;

/*****************eth state*******************/
typedef enum
{
    E_ETH_OK = 0U,
    E_ETH_ERR
} E_ETH_STATE;

/******************eth frame******************/
#define ETH_FRAME_SIZE_MAX    ( 1400u )      /* ETH单帧最大值  */

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    uint16 len;
    uint8 data_u8[ETH_FRAME_SIZE_MAX];
} S_ETH_FRAME;

#pragma pack()

extern E_ETH_STATE support_eth_init(E_ETH_ID id);
extern E_ETH_STATE support_eth_sendData(E_ETH_ID id, uint8 *pbuf, uint16 len);
extern E_ETH_STATE supprot_eth_getData(E_ETH_ID id, S_ETH_FRAME *pframe);
extern void supprot_eth_proc(E_ETH_ID type);
#endif
/**************************************end file*********************************************/
