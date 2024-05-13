/***************************************************************************
文件名：vcp_app_layer_process.c
模    块：主控插件与通信插件通信协议——应用层数据处理
详    述：
作    者：jiaqx,20231129
***************************************************************************/
#include "vcp_app_layer_process.h"

#include "vcp_app_layer.h"
#include "vcp_safe_layer.h"
#include "vcp_time_manage.h"

#include "support_libPub.h"
#include "devlib_eth.h"
#include "support_timer.h"
/* 暂时没有使用 */
///* 时钟同步标志： 1-已进行时钟同步 */
static E_CLOCKSYNC_STATE  InCAN1_diff_flag = E_CLOCKSYNC_NONE, InCAN2_diff_flag = E_CLOCKSYNC_NONE, InETH_diff_flag = E_CLOCKSYNC_NONE;
//uint8  InCAN1_2_MainCtl_en = 0, InCAN2_2_MainCtl_en = 0, InETH_2_MainCtl_en = 0;
//uint32 InCAN1_SendDelay_timer = 0, InCAN2_SendDelay_timer = 0, InETH_SendDelay_timer = 0;

static InETH_RoundPluseState = E_ROUNDPLSE_NONE;

/*插件通道信息*/
typedef struct comm_chanl_info {
    uint8   chanl_index;
    uint8   chanl_sta;
    uint32  Tx_datalen;
  uint32  Rx_datalen;
 } S_CHANL_INFO;
/*插件通道信系电压、电流*/
typedef struct comm_chanl_power {
    uint16   plug_power;
    uint16   plug_current;
 } S_CHANL_POWER;
/*插件信息固定参数*/
typedef struct comm_plug_info {
    uint32 ctl_cpu_soft_ver;
    uint32 comm_cpu_soft_ver;
    uint32 ctl_self_A;
    uint32 ctl_self_B;
    uint8  chl_self_all; /*0x55 正常 0xAA异常*/
    uint8  plug_id[8];
    uint32 open_Times;
    uint32 open_long_Time;
    uint8  tempr_num;
    uint16 plug_tempr;
    uint8  power_channls;
    uint16 power_vot;
    uint16 power_curret;
    uint8  chl_num;
 } S_PLUG_INFO ;
/***********************插件信息end********************/


 /* 安全层业务 */
 extern void set_ClockSyncState( E_CLOCKSYNC_STATE e_clocksync_state)
 {
     InETH_diff_flag = e_clocksync_state;
 }

 extern E_CLOCKSYNC_STATE get_ClockSyncState( void )
 {
     return InETH_diff_flag;
 }



 /* 周期业务标志 */
 extern void set_RoundPluseState( E_ROUNDPLSE_STATE e_roundplse_state)
 {
     InETH_RoundPluseState = e_roundplse_state;
 }

 extern E_ROUNDPLSE_STATE get_RoundPluseState( void )
 {
     return InETH_RoundPluseState;
 }



static uint8 sta_sendBuf[1400U]={0};

/* 像主控发送时钟同步信息 */
static void applyDiff_toSTO( void )
{
    uint8 pbuf[4] = {0,0,0,0};

    uint8 *pSafeTx = &sta_sendBuf[0U];
    uint8 *pAppTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER)];
    uint8 *pAppDataTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER) + sizeof(S_APP_LAYER_HEADER)];
    uint16 safelayer_len_Tx = 0U, applayer_len_Tx = 0U, applayer_datalen_Tx = 0U;

    S_SAFE_LAYER_HEADER s_safe_layer_header;

    applayer_datalen_Tx = 4U;
    support_memcpy(pAppDataTx, &pbuf[0], applayer_datalen_Tx);

    vcp_app_layer_pakage(pAppTx, pAppDataTx, applayer_datalen_Tx, ROUND_CIRCLE_TYPE, APPLY_TIME_SET, IN_ETH_DEV);
    applayer_len_Tx = sizeof(S_APP_LAYER_HEADER) + applayer_datalen_Tx;

    vcp_safe_layer_pakage(pSafeTx, pAppTx, applayer_len_Tx, IN_ETH_DEV);
    support_memcpy(&s_safe_layer_header, (S_SAFE_LAYER_HEADER *)pSafeTx, sizeof(S_SAFE_LAYER_HEADER));
    safelayer_len_Tx = s_safe_layer_header.lenth;

    MY_Printf("↓\r\n");
    MY_Printf("applyDiff_toSTO len:%d !!!\r\n", safelayer_len_Tx);
    MY_Printf("pSafeTx data:");
    for(uint16 i=0; i<safelayer_len_Tx; i++)
    {
        MY_Printf("%.2x ",pSafeTx[i]);
    }
    MY_Printf("\r\n");
    MY_Printf("↑\r\n");

    devLib_eth_sendData(E_ETH_ZK, pSafeTx, safelayer_len_Tx);
}

/* 安全层业务 */
/* 1. 时钟申请业务
 * 2. 接收安全层消息并解析处理。
 * 3. 提供周期数据发送接口
 * 4. 提供周期数据发送接口
 * 5. 与主控进行相应的数据交互。
 * */
extern void tast_ClockSync( void )
{
    static S_TIMER_INFO ClockSync_timer_5s = { FALSE, 0U };

    if ( TRUE == support_timer_timeoutM(&ClockSync_timer_5s, 5000U))
    {
        if(E_CLOCKSYNC_NONE == get_ClockSyncState() )
        {
            applyDiff_toSTO();
        }

    }

}



/* 时钟同步程序处理 */
extern void time_set_local_process( uint8 *pAppData ,uint16 applayer_datalen )
{
    uint32 Times = 0;

    /* 应用数据 */
    uint8 *pSafeTx = &sta_sendBuf[0U];
    uint8 *pAppTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER)];
    uint8 *pAppDataTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER) + sizeof(S_APP_LAYER_HEADER)];
    uint16 safelayer_len_Tx = 0U, applayer_len_Tx = 0U, applayer_datalen_Tx = 0U;

    S_SAFE_LAYER_HEADER s_safe_layer_header;
    /* 1. 参数合法性检查 */
    if( NULL == pAppData)
    {
        MY_Printf("app_clock_adjust pBuf == NULL err !!!\r\n");
        return ;
    }

    if( ( 0U == applayer_datalen) || ( applayer_datalen > APP_LAYER_PLOADLEN ) )
    {
        MY_Printf("app_clock_adjust datalen is err !!!\r\n");
        return ;
    }

    /* 2.解析数据 */
    Times = *(uint32 *)pAppData;

    /* 3.数据处理 */
    /* 3.1 设置安全层ETH通道时钟信号 */
    SetTime_ETH_diff( Times );

    /* 3.设置系统时钟同步标志位 */
    set_ClockSyncState(E_CLOCKSYNC_OK);

    /* 4.更新发送计时标志位（后续优化）*/
    /* InETH_SendDelay_timer = HAL_GetTick(); */

    /* 5.回传数据 */
    /* 5.1 组织应用数据 */
    /* 5.1.1 获取同步后的时钟信号 */
    Times = GetTime_ETH_diff();

    /* 5.1.2 VCP应用层数据 */

    applayer_datalen_Tx = sizeof(Times);
    support_memcpy(pAppDataTx, &Times, applayer_datalen_Tx);

    /* 5.2 VCP应用层封包 */

    vcp_app_layer_pakage(pAppTx, pAppDataTx, applayer_datalen_Tx, ROUND_CIRCLE_ACK_TYPE, TIME_SET_LOCAL, IN_ETH_DEV);
    applayer_len_Tx = sizeof(S_APP_LAYER_HEADER) + applayer_datalen_Tx;

//    MY_Printf    MY_Printf("---------------------\r\n");
//    MY_Printf("applayer_len_Tx len:%d !!!\r\n", applayer_len_Tx);
//    MY_Printf("pAppTx data:");
//    for(uint16 i=0; i<applayer_len_Tx; i++)
//    {
//        MY_Printf("%.2x ",pAppTx[i]);
//    }
//    MY_Printf("\r\n");("---------------------\r\n");
    MY_Printf("applayer_len_Tx len:%d !!!\r\n", applayer_len_Tx);
    MY_Printf("pAppTx data:");
    for(uint16 i=0; i<applayer_len_Tx; i++)
    {
        MY_Printf("%.2x ",pAppTx[i]);
    }
    MY_Printf("\r\n");


    /* 5.3 VCP安全层封包 */
    vcp_safe_layer_pakage(pSafeTx, pAppTx, applayer_len_Tx, IN_ETH_DEV);
    support_memcpy(&s_safe_layer_header, (S_SAFE_LAYER_HEADER *)pSafeTx, sizeof(S_SAFE_LAYER_HEADER));
    safelayer_len_Tx = s_safe_layer_header.lenth;

    MY_Printf("↓\r\n");
    MY_Printf("time_set_local len:%d !!!\r\n", safelayer_len_Tx);
    MY_Printf("pSafeTx data:");
    for(uint16 i=0; i<safelayer_len_Tx; i++)
    {
        MY_Printf("%.2x ",pSafeTx[i]);
    }
    MY_Printf("\r\n");
    MY_Printf("↑\r\n");
    /* 6. 发送数据 */
    devLib_eth_sendData(E_ETH_ZK, pSafeTx, safelayer_len_Tx);
    MY_Printf("AAA\r\n");

}


/* 查询板卡状态处理 */
/* 查询状态需要和主控对应，板卡自身状态异常，会导致主控宕机 ，暂时屏蔽减少宕机次数 */
extern void comm_plug_info_process( void )
{
#if 0
    uint8 *pSafeTx = &sta_sendBuf[0U];
    uint8 *pAppTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER)];
    uint8 *pAppDataTx = &sta_sendBuf[sizeof(S_SAFE_LAYER_HEADER) + sizeof(S_APP_LAYER_HEADER)];
    uint16 safelayer_len_Tx = 0U, applayer_len_Tx = 0U, applayer_datalen_Tx = 0U;

    S_SAFE_LAYER_HEADER s_safe_layer_header;

    S_PLUG_INFO s_plug_info;
    S_CHANL_INFO  s_chanl_info[10];
    uint8 i = 0U;
    uint32 u32_NowTime = 0 , u32_diff_time = 0;
    /* 组织通信插件数据 暂定为固定值 */
    s_plug_info.ctl_cpu_soft_ver = 0;
    s_plug_info.comm_cpu_soft_ver = 0x01010101;
    s_plug_info.ctl_self_A = 0;
    s_plug_info.ctl_self_B = 0;
    s_plug_info.chl_self_all = 0x55; /* 0x55正常 0xAA异常 */
    support_memset(s_plug_info.plug_id , 0x00 , sizeof(s_plug_info.plug_id));
    s_plug_info.plug_id[0] = 0x06;
    s_plug_info.open_Times = 1;
    //GetTime(&u32_NowTime);
    u32_NowTime = support_timer_getTick();
    s_plug_info.open_long_Time = u32_NowTime;

    /*采集温度通道数量*/
    s_plug_info.tempr_num = 1;
    s_plug_info.plug_tempr = 30;
    /*功率采集通道数量*/
    s_plug_info.power_channls = 1;
    s_plug_info.power_vot = 24;
    s_plug_info.power_curret = 1;
    /*通道信息数量*/
    s_plug_info.chl_num = 1;

    applayer_datalen_Tx = sizeof(S_PLUG_INFO);
    support_memcpy(&pAppDataTx[0], &s_plug_info, applayer_datalen_Tx);

    for( i = 0 ; i < s_plug_info.chl_num ; i++)
    {
        s_chanl_info[0].chanl_index = 1;
        s_chanl_info[0].chanl_sta = 0x55;
        s_chanl_info[0].Tx_datalen = 0;
        s_chanl_info[0].Rx_datalen = 0;

        applayer_datalen_Tx = applayer_datalen_Tx + i*sizeof(S_CHANL_INFO);
        support_memcpy(&pAppDataTx[applayer_datalen_Tx] , &s_chanl_info[i] , sizeof(S_CHANL_INFO));
    }

    vcp_app_layer_pakage(pAppTx, pAppDataTx, applayer_datalen_Tx, ROUND_CIRCLE_ACK_TYPE, COMM_PLUG_INFO, IN_ETH_DEV);
    applayer_len_Tx = sizeof(S_APP_LAYER_HEADER) + applayer_datalen_Tx;

    vcp_safe_layer_pakage(pSafeTx, pAppTx, applayer_len_Tx, IN_ETH_DEV);
    support_memcpy(&s_safe_layer_header, (S_SAFE_LAYER_HEADER *)pSafeTx, sizeof(S_SAFE_LAYER_HEADER));
    safelayer_len_Tx = s_safe_layer_header.lenth;

    MY_Printf("↓\r\n");
    MY_Printf("comm_plug_info len:%d !!!\r\n", safelayer_len_Tx);
    MY_Printf("pSafeTx data:");
    for(uint16 i=0; i<safelayer_len_Tx; i++)
    {
        MY_Printf("%.2x ",pSafeTx[i]);
    }
    MY_Printf("\r\n");
    MY_Printf("↑\r\n");

    devLib_eth_sendData(E_ETH_ZK, pSafeTx, safelayer_len_Tx);

#endif
}


/****************************************************************************
* 函数名: get_applayer_pakage_tx_flag
* 说明:		获取发送数据标志，时钟同步之前，不反馈除时钟同步之外的命令的应答
* 参数:   uint8 *pSafe 安全层数据
*         r_app_layer *pApp  应用层数据
* 返回值: 无
 ****************************************************************************/
//static uint8 get_applayer_pakage_tx_flag(uint8 *pSafe , r_app_layer *pApp)
//{
//	 r_app_layer app_layer;
//	 r_safe_layer *pRx_safe = NULL;
//	 uint8 Buf[APP_LAYER_PLOADLEN+4];
//	 uint8 send_flag = 0;
//
//	 pRx_safe = (r_safe_layer *)pSafe;
//	 app_layer.msg_type = pApp->msg_type;
//	 app_layer.msg_sub_type = pApp->msg_sub_type;
//
//#ifdef	STO_V2
//	 /* 时钟同步命令允许应答 */
//	 if((app_layer.msg_type == ROUND_CIRCLE_TYPE) || (app_layer.msg_type == ROUND_CIRCLE_ACK_TYPE))
//	 {
//		 if((app_layer.msg_sub_type == TIME_SET_LOCAL) || (app_layer.msg_sub_type == APPLY_TIME_SET) \
//				 || (app_layer.msg_sub_type == COMM_PLUG_INFO))
//				send_flag = 1;
//	 }
//	 /* 其他命令，在收到各自通道时钟同步之前，不允许应答 */
//	 else
//	 {
//		 switch(pRx_safe->res)
//		 {
//			 case IN_FDCAN1_DEV:
//				 if(InCAN1_2_MainCtl_en)
//					 send_flag = 1;
//				 break;
//			 case IN_FDCAN2_DEV:
//				 if(InCAN2_2_MainCtl_en)
//					 send_flag = 1;
//				 break;
//			 case IN_ETH_DEV:
//				 if(InETH_2_MainCtl_en)
//					 send_flag = 1;
//				 break;
//			 default:
//				 break;
//		 }
//	 }
//#else
//	 send_flag = 1;
//#endif
//
//	 return send_flag;
//}

