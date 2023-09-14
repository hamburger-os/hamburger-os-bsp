/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : if_os.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Implement the function interfaces of if_os
********************************************************************************************/
#ifndef _IF_OS_H
#define _IF_OS_H

#include "common.h"

/***************rs485 channel*****************/
typedef struct 
{
  void *sem;
}OS_EVENT;


extern sint32 if_OSSemCreate( OS_EVENT *psem );
extern void if_OSSemPend( OS_EVENT *psem, sint32 timeout, sint8 *perr );
extern void if_OSSemPost( OS_EVENT *psem );

extern sint32 if_OSTaskCreate( const char *task_name,
                        void (*entry)(void *param),
                        void *param,
                        uint32 stack_size,
                        uint8 priority);

extern void if_OSTimeDly( uint32 ticks );

#endif

/*******************************************end file*******************************************************/
