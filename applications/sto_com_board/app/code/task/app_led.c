/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/
#include "support_gpio.h"
#include "support_timer.h"
#include "support_eth.h"
/*******************************************************************************************
 ** @brief: app_led
 ** @param: null
 *******************************************************************************************/
extern void app_led(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    if ( TRUE == support_timer_timeoutM(&timer, 50U))
    {
        support_gpio_toggle(LED1_ID);
    }

 }
