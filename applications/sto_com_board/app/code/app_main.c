/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/
#include "support_init.h"
#include "support_libOS.h"
#include "support_libPub.h"

#include "support_can.h"
#include "support_eth.h"
#include "support_hdlc.h"
#include "support_mvb.h"
#include "support_rs485.h"
#include "support_timer.h"
#include "support_gpio.h"
#include "receive_external_protocol.h"
#include "support_libList.h"


static void cantestsend(void)
{
    S_ArrayList * pList = NULL;
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_CAN_FRAME_DATA s_can_frame_data = {0};
    uint8 num = 0U;
    uint8* testdata = NULL;
    if ( TRUE == support_timer_timeoutM(&timer, 2000U))
    {
        for (uint8 j = 0; j < 8; ++j)
        {
            s_can_frame_data.priority_u8 = 0x60U;
            s_can_frame_data.no_u8++;
            s_can_frame_data.length_u8 = 8U;

            for (uint8 i = 0; i < 8; ++i)
            {
                s_can_frame_data.data_u8[i] = num++;
            }
#if 0
            rec_msg_external_proc(E_EX_CAN_EX1, (uint8*)&s_can_frame_data);
#else
            pList = get_ZK_list( DATA_CHANNEL_TX2CAN1 );
            if( NULL != pList )
            {
                //MY_Printf("pList !!!\r\n");
                support_arraylistAdd( pList, (uint8 *)&s_can_frame_data, sizeof( S_CAN_FRAME_DATA ));
            }
#endif
#if 0
            uint8 var = 0U;
            MY_Printf("test data , len:%d !!!\r\n",sizeof(S_CAN_FRAME_DATA));
            MY_Printf("data:");

            testdata = (uint8 *)&s_can_frame_data;
            for( var=0; var<sizeof(S_CAN_FRAME_DATA); var++)
            {

                MY_Printf("%.2x ",testdata[var]);
            }
            MY_Printf("\r\n");
#endif
        }
    }

}





extern void app_led(void);
extern void app_init(void);
extern void app_receive_data(void);
extern void app_send_data(void);
extern void tast_ClockSync( void );
/*******************************************************************************************
 ** @brief: app_main_init
 ** @param: null
 *******************************************************************************************/
static void app_main_init(void)
{
    MY_Printf("\r\n+++++++++++++++++app Init.....>>>>>>>>>>>>>>\r\n");

    E_BOARD_ID BoardId = 0U;
    BoardId = support_gpio_getBoardId();
    MY_Printf("BoardId:%d\r\n", BoardId);

    /* 1.应用层初始化 */
    app_init();

    /* 2.注册业务函数 */
    /* 2.1 数据接收业务 */
    support_osRegisterFunc(TASK01_ID, app_receive_data);

    /* 2.2 数据发送业务  */
    support_osRegisterFunc(TASK01_ID, app_send_data);

    /* 2.3 指示灯管理业务 */
    support_osRegisterFunc(TASK03_ID, app_led);
    //support_osRegisterFunc(TASK03_ID, cantestsend);

    /* 2.4 安全通信协议—时钟同步业务 */
    support_osRegisterFunc(TASK01_ID, tast_ClockSync);

    MY_Printf("\r\n+++++++++++++++++app Init Success.....>>>>>>>>>>>>>>\r\n");

}


/*******************************************************************************************
 ** @brief: com_app_main_init
 ** @param: null
 *******************************************************************************************/
extern void com_app_main_init(void)
{
    /* 1.支持层初始化 */
    support_init();

    /* 2.通信任务初始化 */
    app_main_init();

}


#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#include "vcp_app_layer_process.h"

extern void set_ClockSyncState( E_CLOCKSYNC_STATE e_clocksync_state);
void synctest(void)
{
    set_ClockSyncState(E_CLOCKSYNC_OK);
}


MSH_CMD_EXPORT_ALIAS(synctest, synchtest, sync test);
#endif /* RT_USING_FINSH */

