/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_timer.c
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : Manage support layer timer
 ********************************************************************************************/
#include "if_os.h"
#include "if_timer.h"
#include "support_timer.h"
/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/

/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/

/*****************************************************************************************
 *        Local definitions
 *****************************************************************************************/
#define APP_PERIOD_VALUE ( 100U )
/*******************************************************************************************
 ** @brief: support_timer_init
 ** @param: null
 *******************************************************************************************/
extern BOOL support_timer_init(void)
{
    MY_Printf("support_timer_init==>OK!!!\n");
    return TRUE;
}

/*******************************************************************************************
 ** @brief: support_timer_getTick
 ** @param: null
 *******************************************************************************************/
extern uint32 support_timer_getTick(void)
{
    uint32 tick = 0U;

    tick = if_timer_getTicks();

    return tick;
}

/*******************************************************************************************
 ** @brief: support_timer_delayms
 ** @param: Waiting time in ms.
 *******************************************************************************************/
extern void support_timer_delayms(uint32 ms)
{
    if_OSTimeDly(ms);
}
/*******************************************************************************************
 ** @brief: support_timer_timeoutM
 ** @param: *timer_info, ms
 *******************************************************************************************/
extern BOOL support_timer_timeoutM(S_TIMER_INFO *timer_info, uint32 ms)
{
    uint32 curtv = 0U;
    BOOL status = FALSE;
    /* get the tick */
    curtv = support_timer_getTick();
    /* init the timer */
    if ( FALSE == timer_info->init_flag)
    {
        timer_info->init_flag = TRUE;
        timer_info->timer = curtv;
    }
    /* check the conditon about the timeout */
    if ((curtv - timer_info->timer) >= ms)
    {
        timer_info->timer = curtv;
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }
    return status;
}

/*******************************************************************************************
 ** @brief: support_timer_timeoutMN
 ** @param: *timer_info, ms
 *******************************************************************************************/
extern BOOL support_timer_timeoutMN(S_TIMER_INFO *timer_info, uint32 ms)
{
    uint32 curtv = 0U;
    BOOL status = FALSE;

    /* get tick */
    curtv = support_timer_getTick();

    /* init the timer */
    if ( FALSE == timer_info->init_flag)
    {
        timer_info->init_flag = TRUE;
        timer_info->timer = curtv;
    }
    /* check the condtion for timeout */
    if ((curtv - timer_info->timer) >= ms)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }
    return status;
}

/**************************************end file*********************************************/
