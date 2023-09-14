/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_hdlc.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Manage support HDLC 
********************************************************************************************/
#ifndef _SUPPORT_HDLC_H
#define _SUPPORT_HDLC_H

#include "common.h"

/*****************hdlc id**********************/
typedef enum
{
   E_HDLC_ID_1 = 0U,
   E_HDLC_ID_MAX
}E_HDLC_ID;

/*****************hdlc state*******************/
typedef enum 
{
   E_HDLC_OK = 0U,
   E_HDLC_ERR
}E_HDLC_STATE;

/******************hdlc frame******************/
#define HDLC_FRAME_SIZE_MAX    ( 256U )      /* HDLC单帧最大值  */

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
  uint16 len;          
  uint8 data_u8[HDLC_FRAME_SIZE_MAX];        
}S_HDLC_FRAME;

#pragma pack()

extern E_HDLC_STATE support_hdlc_init( E_HDLC_ID id );
extern E_HDLC_STATE support_hdlc_sendData( E_HDLC_ID id, S_HDLC_FRAME *pframe );
extern E_HDLC_STATE supprot_hdlc_getData( E_HDLC_ID id, S_HDLC_FRAME *pframe );

#endif
/**************************************end file*********************************************/

