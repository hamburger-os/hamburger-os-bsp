/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_timer.h
**@author: Created By Chengt
**@date  : 2019.12.13
**@brief : Manage support TIMER 
********************************************************************************************/
#ifndef _SUPPORT_TIMER_H
#define _SUPPORT_TIMER_H

#include "common.h"

typedef struct
{
  BOOL  init_flag;   
  uint32 timer;         
}S_TIMER_INFO;

extern BOOL   support_timer_init( void );
extern uint32 support_timer_getTick( void );
extern void   support_timer_delayms( uint32 ms );
extern BOOL   support_timer_timeoutM( S_TIMER_INFO *timer_info, uint32 ms );
extern BOOL   support_timer_timeoutMN( S_TIMER_INFO *timer_info, uint32 ms );
#endif
/**************************************end file*********************************************/
