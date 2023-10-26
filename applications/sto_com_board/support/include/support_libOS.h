/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_libOS.h
 **@author: Created By Chengt
 **@date  : 2022.04.07
 **@brief : Implement the function interfaces of support_libOS
 ********************************************************************************************/
#ifndef SUPPORT_LIBOS_H
#define SUPPORT_LIBOS_H

#include "common.h"

typedef enum
{
    TASK01_ID = 0U,
    TASK02_ID,
    TASK03_ID,
    TASK04_ID
} E_SYS_TASK_ID;

typedef void (*task_func)(void);

extern void support_osRunning(task_func p_init);
extern BOOL support_osRegisterFunc(E_SYS_TASK_ID task_id, task_func p_func);

extern BOOL support_SemCreate(uint8 *sem_id);
extern BOOL support_SemPend(uint8 sem_id);
extern BOOL support_SemPost(uint8 sem_id);

#endif

/**************************************end file*********************************************/
