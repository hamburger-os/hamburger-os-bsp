/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_mvb.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Manage support MVB 
********************************************************************************************/
#ifndef _SUPPORT_MVB_H
#define _SUPPORT_MVB_H

#include "common.h"

/*****************MVB id**********************/
typedef enum
{
   E_MVB_ID_1 = 0U,
   E_MVB_ID_MAX
}E_MVB_ID;

/*****************MVB state*******************/
typedef enum 
{
   E_MVB_OK = 0U,
   E_MVB_ERR
}E_MVB_STATE;

/******************MVB frame******************/
#define MVB_FRAME_SIZE_MAX    ( 32U )      /* MVB单帧最大值  */

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
  uint16 len;          
  uint8 data_u8[MVB_FRAME_SIZE_MAX];        
}S_MVB_FRAME;

#pragma pack()

extern E_MVB_STATE support_mvb_init( E_MVB_ID id );
extern E_MVB_STATE support_mvb_sendData( E_MVB_ID id, S_MVB_FRAME *pframe );
extern E_MVB_STATE supprot_mvb_getData( E_MVB_ID id, S_MVB_FRAME *pframe );

#endif
/**************************************end file*********************************************/

