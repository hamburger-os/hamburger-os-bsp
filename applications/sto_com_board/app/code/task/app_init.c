/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/
#include "support_gpio.h"
#include "list_manage.h"
/*******************************************************************************************
 ** @brief: app_led
 ** @param: null
 *******************************************************************************************/
static void BoardId_check( void )
{
    E_BOARD_ID BoardId = 0U;
    BoardId = support_gpio_getBoardId();

    switch ( BoardId )
    {
    case ID_TX1_Load :MY_Printf("This is TX1_Load!!!\r\n");break;
    case ID_TX1_Child:MY_Printf("This is TX1_Child!!!\r\n");break;
    case ID_TX2_Load :MY_Printf("This is TX2_Load!!!\r\n");break;
    case ID_TX2_Child:MY_Printf("This is TX2_Child!!!\r\n");break;
    default:MY_Printf("BoardId is err !!!\r\n");break;
    }

}

/*******************************************************************************************
 ** @brief: app_led
 ** @param: null
 *******************************************************************************************/
extern void app_init(void)
{
    /* 1.板卡类型检查 */
    BoardId_check();

    /* 2.接收队列初始化 */
    list_manage_init();

}
