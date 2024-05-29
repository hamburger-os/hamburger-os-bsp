/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : app_main.c
 **@author: Created By jiaqx
 **@date  : 2023.11.02
 **@brief : app_main function
 ********************************************************************************************/

#include "support_timer.h"
#include "devlib_can.h"
#include "support_libPub.h"

#include "devlib_eth.h"
#include "vcp_safe_layer.h"
#include "vcp_app_layer.h"
#include "list_manage.h"
#include "send_mainctl_protocol.h"
#include "send_external_protocol.h"
#include "vcp_app_layer_process.h"

/*******************************************************************************************
 ** @brief: send_pulse_data_proc
 ** @param: null
 *******************************************************************************************/
static void send_pulse_data_proc( uint8* pdata, uint16 data_len)
{
    /* 暂定应用层提供，后续又安全层提供 相对应接口 */
    uint8 sta_sendBuf[1400] = {0U};

    /* 1.参数合法性检查 */
    if( pdata == NULL )
    {
        MY_Printf("send_pulse_data_proc pdata is NULL !!!\r\n");
        return ;
    }

    uint8 *pSafeTx = &sta_sendBuf[0U];
    uint8 *pAppTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER)];
    uint8 *pAppDataTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER) + sizeof(S_APP_LAYER_HEADER)];
    uint16 safelayer_len_Tx = 0U, applayer_len_Tx = 0U, applayer_datalen_Tx = 0U;
    S_SAFE_LAYER_HEADER s_safe_layer_header;

    /* 3.应用数据赋值 */
    pAppDataTx = (uint8 *)pdata;
    applayer_datalen_Tx = data_len;
    /* 5.应用层封包 */
    vcp_app_layer_pakage(pAppTx, pAppDataTx, applayer_datalen_Tx, ROUND_PULSE_MODE, RX_EXPDATA_MAINCTL, IN_ETH_DEV);
    applayer_len_Tx = sizeof(S_APP_LAYER_HEADER) + applayer_datalen_Tx;

    /* 5.安全层封包 */
    vcp_safe_layer_pakage(pSafeTx, pAppTx, applayer_len_Tx, IN_ETH_DEV);
    support_memcpy(&s_safe_layer_header, (S_SAFE_LAYER_HEADER *)pSafeTx, sizeof(S_SAFE_LAYER_HEADER));
    safelayer_len_Tx = s_safe_layer_header.lenth;

    devLib_eth_sendData(E_ETH_ZK, pSafeTx, safelayer_len_Tx);

}

/*******************************************************************************************
 ** @brief: mainctl_msg_data_proc
 ** @param: null
 *******************************************************************************************/
static void mainctl_msg_data_proc( void )
{
    /* 1.开辟空间 */
    uint8 s_msg_data[1500] = {0};
    uint16 msg_datalen = 0 ;

    /* 2.组织应用数据*/
    /* 2.1 组织CAN队列数据 */

    for( E_CAN_TYPE channel_num = 0; channel_num < E_CAN_TYPE_MAX; channel_num++ )
    {
        mainctl_msg_data_pakage(&s_msg_data[0U],&msg_datalen, channel_num,E_CHANNEL_CAN);
    }

//    /* 2.2 组织ETH队列数据 */
//    for( E_ETH_TYPE channel_num = 0; channel_num < E_ETH_TYPE_MAX; channel_num++ )
//    {
//        mainctl_msg_data_pakage(&s_msg_data[0U],&msg_datalen, channel_num,E_CHANNEL_ETH);
//    }

#if 0
    MY_Printf("send_mainctl_data_proc eth data len is legal, len:%d !!!\r\n",msg_datalen);
    MY_Printf("data:");
    for(uint16 i=0;i<msg_datalen;i++)
    {

        MY_Printf("%.2x ",s_msg_data[i]);
    }
    MY_Printf("\r\n");
#endif

    /* 3.判断增加最大长度判断 */
    if((msg_datalen < 14U) || (msg_datalen > 1300))
    {
        return;
    }

    /* 4.发送周期数据——经安全层与应用层打包 */
    send_pulse_data_proc(&s_msg_data[0U], msg_datalen);

}

/*******************************************************************************************
 ** @brief: get_send_mainctl_data_flg
 ** @param: null
 *******************************************************************************************/
static BOOL get_send_mainctl_data_flg()
{
    BOOL ret = FALSE;

    static S_TIMER_INFO timer_50ms = { FALSE, 0U };
    static S_TIMER_INFO timer_delay_1s = { FALSE, 0U };

    /* 1.获取与主控同步状态 */
    if ( E_CLOCKSYNC_NONE == get_ClockSyncState() )
    {
        //MY_Printf("get_ClockSyncState is E_CLOCKSYNC_NONE !!! \r\n");
        return ret;
    }

    /* 2.同步完成后等待1s，等待主控完全启动 */
    if ( FALSE == support_timer_timeoutMN(&timer_delay_1s, 1000U) )
    {
        //MY_Printf("timer_delay_1s < 1000 !!! \r\n");
        return ret;
    }
    /* 2.1 维护时间变量 */
    if (timer_delay_1s.timer > 1000U)
    {
        MY_Printf("ClockSync is ok !!! \r\n");
        timer_delay_1s.init_flag = TRUE;
        timer_delay_1s.timer = 1000U;
    }

    /* 3.周期数据周期50ms */
    if ( FALSE == support_timer_timeoutM(&timer_50ms, 50U) )
    {
        return ret;
    }

    /* 4.条件均满足 则启动发送 */
    ret = TRUE;

    return ret;


}


/*******************************************************************************************
 ** @brief: send_mainctl_data_proc
 ** @param: null
 *******************************************************************************************/
static void send_mainctl_data_proc( void )
{
    /* 1.发送主控数据 */
    if ( TRUE == get_send_mainctl_data_flg() )
    {
        mainctl_msg_data_proc();
    }

    /* 2.清空队列数据 */
    if(E_CLOCKSYNC_NONE == get_ClockSyncState())
    {
        mainctl_msg_data_clean();
    }

}


/*******************************************************************************************
 ** @brief: send_external_data_proc
 ** @param: null
 *******************************************************************************************/
static void send_external_data_proc( void )
{

    /* 1.发送CAN队列中的数据 */
    for( E_CAN_TYPE channel_num = 0; channel_num < E_CAN_TYPE_MAX; channel_num++ )
    {
        send_external_list_data(channel_num,E_CHANNEL_CAN);
    }

    /* 2.发送ETH队列中的数据 */
    for( E_ETH_TYPE channel_num = 0; channel_num < E_ETH_TYPE_MAX; channel_num++ )
    {
        send_external_list_data(channel_num,E_CHANNEL_ETH);
    }

}

/*******************************************************************************************
 ** @brief: app_send_data
 ** @param: null
 *******************************************************************************************/
extern void app_send_data( void )
{
    uint32 timer_tick = 0U;
    uint32 timer_tick_diff = 0U;
    timer_tick = support_timer_getTick();

    /* 1.发送数据至主控 */
    send_mainctl_data_proc();

    /* 2.发送数据至外部*/
    send_external_data_proc();

    /* 3.超时判断 */
    timer_tick_diff = support_timer_getTick() - timer_tick;
    if( timer_tick_diff > 45U )
    {
        MY_Printf("app_send_data is timeout !!! \r\n");
        MY_Printf("timer_tick_diff: %d !!!\r\n",timer_tick_diff);
    }


}


