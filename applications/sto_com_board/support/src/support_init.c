/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_test_app.c
 **@author: Created By Chengt
 **@date  : 2023.09.06
 **@brief : test support function
 ********************************************************************************************/
#include <stdio.h>

#include "support_init.h"
#include "support_timer.h"
#include "support_gpio.h"
#include "support_can.h"
#include "support_eth.h"
/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/

/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/

/*******************************************************************************************
 ** @brief: support_init
 ** @param: null
 *******************************************************************************************/
extern int support_init(void)
{
    E_BOARD_ID board_id;

    MY_Printf("\r\n+++++++++++++++++support Init.....>>>>>>>>>>>>>>\r\n");

    if(IO_OK != support_gpio_init(LED_NONE, IO_LOW))
    {
        MY_Printf("support_gpio_init error\r\n");
        return -1;
    }
    else
    {
        MY_Printf("gpio init ok\r\n");
    }

    board_id = support_gpio_getBoardId();
    if(ID_NONE == board_id)
    {
        MY_Printf("support_gpio_getBoardId error\r\n");
        return -1;
    }
    else
    {
        MY_Printf("get board id ok\r\n");
    }

    if(support_timer_init() != TRUE)
    {
        MY_Printf("support_timer_init error\r\n");
        return -1;
    }
    else
    {
        MY_Printf("timer init ok\r\n");
    }

    if(support_can_init(E_CAN_ID_MAX) != E_CAN_OK)
    {
        MY_Printf("support_can_init error\r\n");
        return -1;
    }
    else
    {
        MY_Printf("can init ok\r\n");
    }

    if(support_eth_init(E_ETH_ID_MAX) != E_ETH_OK)
    {
        MY_Printf("support_eth_init error\r\n");
        return -1;
    }
    else
    {
        MY_Printf("eth init ok\r\n");
    }

    return 0;
}
/**************************************end file*********************************************/
