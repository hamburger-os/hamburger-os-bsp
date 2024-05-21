/***************************************************************************
#include <text_app/protocol/include/app_list_manage.h>
文件名：vcp_time_manage.c
模  块：安全层所需时间管理模块
详  述：
***************************************************************************/
#include "list_manage.h"
#include "support_libList.h"
#include "devlib_can.h"
#include "devlib_eth.h"

/************************************* 主控数据接收队列 *****************************************/
/* TX_REC_CAN_LIST */
static S_ArrayList *g_TX_CAN_LIST[TX_CAN_CHANNEL_NUM] = { NULL };

/* TX_REC_ETH_LIST */
static S_ArrayList *g_TX_ETH_LIST[TX_ETH_CHANNEL_NUM] = { NULL };

/* TX_REC_HDLC_LIST */
static S_ArrayList *g_TX_HDLC_LIST = NULL;

/* TX_REC_MVB_LIST */
static S_ArrayList *g_TX_MVB_LIST = NULL;

/* TX_REC_485_LIST */
static S_ArrayList *g_TX_RS485_LIST[TX_RS485_CHANNEL_NUM] = { NULL };


/************************************* 外部数据接收队列 *****************************************/
/* TX_REC_CAN_LIST */
static S_ArrayList *g_EX_CAN_LIST[TX_CAN_CHANNEL_NUM] = { NULL };

/* TX_REC_ETH_LIST */
static S_ArrayList *g_EX_ETH_LIST[TX_ETH_CHANNEL_NUM] = { NULL };

/* TX_REC_HDLC_LIST */
static S_ArrayList *g_EX_HDLC_LIST = NULL;

/* TX_REC_MVB_LIST */
static S_ArrayList *g_EX_MVB_LIST = NULL;

/* TX_REC_485_LIST */
static S_ArrayList *g_EX_RS485_LIST[TX_RS485_CHANNEL_NUM] = { NULL };



/****************************************************************************
* 函数名: ZK_List_Init
* 说    明: 初始化主控消息队列，用于存放内网解析完成后的数据
* 参    数: 无
* 返回值: 无
* 备    注: 无
 ****************************************************************************/
static void ZK_List_Init( void )
{
    uint8 num = 0;

    /* 1.创建CAN通信通道队列 */
    for( num = 0U; num < TX_CAN_CHANNEL_NUM; num++ )
    {
        g_TX_CAN_LIST[num] = support_arraylistCreate("TX_CAN_LIST", sizeof( S_CAN_FRAME_DATA ), 120U );
        if( NULL == g_TX_CAN_LIST[num])
        {
            MY_Printf("g_TX_CAN_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

    /* 2.创建ETH通信通道队列 */
    for( num = 0U; num < TX_ETH_CHANNEL_NUM; num++ )
    {
        g_TX_ETH_LIST[num] = support_arraylistCreate("TX_ETH_LIST", sizeof( S_ETH_FRAME ), 5U );
        if( NULL == g_TX_ETH_LIST[num])
        {
            MY_Printf("g_TX_ETH_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

    /* 3.创建HDLC通信通道队列 */
    g_TX_HDLC_LIST = support_arraylistCreate("TX_HDLC_LIST", sizeof( S_ETH_FRAME ), 5U );
    if( NULL == g_TX_HDLC_LIST)
    {
        MY_Printf("g_TX_HDLC_LIST arraylistCreate is err !!!\r\n");
    }

    /* 4.创建MVB通信通道队列 */
    g_TX_MVB_LIST = support_arraylistCreate("TX_MVB_LIST", sizeof( S_ETH_FRAME ), 5U );
    if( NULL == g_TX_MVB_LIST)
    {
        MY_Printf("g_TX_MVB_LIST arraylistCreate is err !!!\r\n");
    }

    /* 5.创建RS485通信通道队列  */
    for( num = 0U; num < TX_RS485_CHANNEL_NUM; num++ )
    {
        g_TX_RS485_LIST[num] = support_arraylistCreate("TX_RS485_LIST", sizeof( S_ETH_FRAME ), 5U );
        if( NULL == g_TX_RS485_LIST[num])
        {
            MY_Printf("g_TX_RS485_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

}

/****************************************************************************
* 函数名: ZK_List_Init
* 说    明: 初始化主控消息队列，用于存放内网解析完成后的数据
* 参    数: 无
* 返回值: 无
* 备    注: 无
 ****************************************************************************/
static void EX_List_Init( void )
{
    uint8 num = 0;

    /* 1.创建CAN通信通道队列 */
    for( num = 0U; num < TX_CAN_CHANNEL_NUM; num++ )
    {
        g_EX_CAN_LIST[num] = support_arraylistCreate("EX_CAN_LIST", sizeof( S_CAN_FRAME_DATA ), 120U );
        if( NULL == g_EX_CAN_LIST[num])
        {
            MY_Printf("g_EX_CAN_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

    /* 2.创建ETH通信通道队列 */
    for( num = 0U; num < TX_ETH_CHANNEL_NUM; num++ )
    {
        g_EX_ETH_LIST[num] = support_arraylistCreate("EX_ETH_LIST", sizeof( S_ETH_FRAME ), 5U );
        if( NULL == g_EX_ETH_LIST[num])
        {
            MY_Printf("g_EX_ETH_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

    /* 3.创建HDLC通信通道队列 */
    g_EX_HDLC_LIST = support_arraylistCreate("EX_HDLC_LIST", sizeof( S_ETH_FRAME ), 5U );
    if( NULL == g_EX_HDLC_LIST)
    {
        MY_Printf("g_EX_HDLC_LIST arraylistCreate is err !!!\r\n");
    }

    /* 4.创建MVB通信通道队列 */
    g_EX_MVB_LIST = support_arraylistCreate("EX_MVB_LIST", sizeof( S_ETH_FRAME ), 5U );
    if( NULL == g_EX_MVB_LIST)
    {
        MY_Printf("EX_MVB_LIST arraylistCreate is err !!!\r\n");
    }

    /* 5.创建RS485通信通道队列  */
    for( num = 0U; num < TX_RS485_CHANNEL_NUM; num++ )
    {
        g_EX_RS485_LIST[num] = support_arraylistCreate("EX_RS485_LIST", sizeof( S_ETH_FRAME ), 5U );
        if( NULL == g_EX_RS485_LIST[num])
        {
            MY_Printf("g_EX_RS485_LIST[%d] arraylistCreate is err !!!\r\n",num);
        }
    }

}

/****************************************************************************
* 函数名: list_manage_init
* 说    明: 初始化应用所使用的队列消息
* 参    数: 无
* 返回值: 无
* 备    注: 无
 ****************************************************************************/
extern void list_manage_init( void )
{
    /* 1.接收队列初始化 */
    ZK_List_Init();

    /* 2.发送缓冲初始化 */
    EX_List_Init();

    /* 3.输出提示信息 */
    MY_Printf("list_manage_init OK!!!\r\n");
}

/****************************************************************************
* 函数名: get_ZK_list_Init
* 说    明: 获取主控消息队列句柄指针
* 参    数: uint8 通道ID
* 返回值: S_ArrayList* 主控消息句柄指针
 ****************************************************************************/
extern S_ArrayList *get_ZK_list( uint8 channelId )
{
    S_ArrayList *p_List = NULL;

    /* 1. 检查参数合法性 */
    if ((channelId < DATA_CHANNEL_TX1CAN1) ||(channelId > DATA_CHANNEL_TX2RS485a))
    {
        MY_Printf("get_ZK_list channelId is err, channelId:%.2x !!!\r\n",channelId);
        return p_List;
    }

    /* 2. 数据处理 */
    switch( channelId )
    {
    /* TX1_CAN_LIST */
    case DATA_CHANNEL_TX1CAN1: p_List = g_TX_CAN_LIST[0U];break;
    case DATA_CHANNEL_TX1CAN2: p_List = g_TX_CAN_LIST[1U];break;
    case DATA_CHANNEL_TX1CAN3: p_List = g_TX_CAN_LIST[2U];break;
    case DATA_CHANNEL_TX1CAN4: p_List = g_TX_CAN_LIST[3U];break;
    case DATA_CHANNEL_TX1CAN5: p_List = g_TX_CAN_LIST[4U];break;
    case DATA_CHANNEL_TX1VMCAN1: p_List = g_TX_CAN_LIST[5U];break;
    case DATA_CHANNEL_TX1VMCAN2: p_List = g_TX_CAN_LIST[6U];break;

    /* TX2_CAN_LIST */
    case DATA_CHANNEL_TX2CAN1: p_List = g_TX_CAN_LIST[7U];break;
    case DATA_CHANNEL_TX2CAN2: p_List = g_TX_CAN_LIST[8U];break;
    case DATA_CHANNEL_TX2CAN3: p_List = g_TX_CAN_LIST[9U];break;
    case DATA_CHANNEL_TX2CAN4: p_List = g_TX_CAN_LIST[10U];break;
    case DATA_CHANNEL_TX2CAN5: p_List = g_TX_CAN_LIST[11U];break;
    case DATA_CHANNEL_TX2VMCAN1: p_List = g_TX_CAN_LIST[12U];break;
    case DATA_CHANNEL_TX2VMCAN2: p_List = g_TX_CAN_LIST[13U];break;

    /* TX1_ETH_LIST */
    case DATA_CHANNEL_TX1ETH1: p_List = g_TX_ETH_LIST[0U];break;
    case DATA_CHANNEL_TX1ETH2: p_List = g_TX_ETH_LIST[1U];break;
    case DATA_CHANNEL_TX1ETH3: p_List = g_TX_ETH_LIST[2U];break;
    case DATA_CHANNEL_TX1ETH4: p_List = g_TX_ETH_LIST[3U];break;

    /* TX2_ETH_LIST */
    case DATA_CHANNEL_TX2ETH1: p_List = g_TX_ETH_LIST[4U];break;
    case DATA_CHANNEL_TX2ETH2: p_List = g_TX_ETH_LIST[5U];break;

    /* TX_HDLC_LIST */
    case DATA_CHANNEL_TX2HDLC: p_List = g_TX_HDLC_LIST;break;

    /* TX_MVB_LIST */
    case DATA_CHANNEL_TX2MVB: p_List = g_TX_MVB_LIST;break;

    /* TX_RS485_LIST */
    case DATA_CHANNEL_TX1RS485a: p_List = g_TX_RS485_LIST[0U];break;
    case DATA_CHANNEL_TX1RS485b: p_List = g_TX_RS485_LIST[1U];break;
    case DATA_CHANNEL_TX2RS485a: p_List = g_TX_RS485_LIST[2U];break;

    default:break;
    }

    return p_List;
}

/****************************************************************************
* 函数名: get_EX_list_Init
* 说    明: 获取主控消息队列句柄指针
* 参    数: uint8 通道ID
* 返回值: S_ArrayList* 主控消息句柄指针
 ****************************************************************************/
extern S_ArrayList *get_EX_list( uint8 channelId )
{
    S_ArrayList *p_List = NULL;

    /* 1. 检查参数合法性 */
    if ((channelId < DATA_CHANNEL_TX1CAN1) ||(channelId > DATA_CHANNEL_TX2RS485a))
    {
        MY_Printf("get_EX_list channelId is err, channelId:%.2x !!!\r\n",channelId);
        return p_List;
    }

    /* 2. 数据处理 */
    switch( channelId )
    {
    /* TX1_CAN_LIST */
    case DATA_CHANNEL_TX1CAN1: p_List = g_EX_CAN_LIST[0U];
    if(p_List == NULL )
    {
        MY_Printf("g_EX_CAN_LIST[0U] is NULL!!!\r\n");
    }
    break;
    case DATA_CHANNEL_TX1CAN2: p_List = g_EX_CAN_LIST[1U];break;
    case DATA_CHANNEL_TX1CAN3: p_List = g_EX_CAN_LIST[2U];break;
    case DATA_CHANNEL_TX1CAN4: p_List = g_EX_CAN_LIST[3U];break;
    case DATA_CHANNEL_TX1CAN5: p_List = g_EX_CAN_LIST[4U];break;
    case DATA_CHANNEL_TX1VMCAN1: p_List = g_EX_CAN_LIST[5U];break;
    case DATA_CHANNEL_TX1VMCAN2: p_List = g_EX_CAN_LIST[6U];break;

    /* TX2_CAN_LIST */
    case DATA_CHANNEL_TX2CAN1: p_List = g_EX_CAN_LIST[7U];break;
    case DATA_CHANNEL_TX2CAN2: p_List = g_EX_CAN_LIST[8U];break;
    case DATA_CHANNEL_TX2CAN3: p_List = g_EX_CAN_LIST[9U];break;
    case DATA_CHANNEL_TX2CAN4: p_List = g_EX_CAN_LIST[10U];break;
    case DATA_CHANNEL_TX2CAN5: p_List = g_EX_CAN_LIST[11U];break;
    case DATA_CHANNEL_TX2VMCAN1: p_List = g_EX_CAN_LIST[12U];break;
    case DATA_CHANNEL_TX2VMCAN2: p_List = g_EX_CAN_LIST[13U];break;

    /* TX1_ETH_LIST */
    case DATA_CHANNEL_TX1ETH1: p_List = g_EX_ETH_LIST[0U];break;
    case DATA_CHANNEL_TX1ETH2: p_List = g_EX_ETH_LIST[1U];break;
    case DATA_CHANNEL_TX1ETH3: p_List = g_EX_ETH_LIST[2U];break;
    case DATA_CHANNEL_TX1ETH4: p_List = g_EX_ETH_LIST[3U];break;

    /* TX2_ETH_LIST */
    case DATA_CHANNEL_TX2ETH1: p_List = g_EX_ETH_LIST[4U];break;
    case DATA_CHANNEL_TX2ETH2: p_List = g_EX_ETH_LIST[5U];break;

    /* TX_HDLC_LIST */
    case DATA_CHANNEL_TX2HDLC: p_List = g_EX_HDLC_LIST;break;

    /* TX_MVB_LIST */
    case DATA_CHANNEL_TX2MVB: p_List = g_EX_MVB_LIST;break;

    /* TX_RS485_LIST */
    case DATA_CHANNEL_TX1RS485a: p_List = g_EX_RS485_LIST[0U];break;
    case DATA_CHANNEL_TX1RS485b: p_List = g_EX_RS485_LIST[1U];break;
    case DATA_CHANNEL_TX2RS485a: p_List = g_TX_RS485_LIST[2U];break;

    default:break;
    }

    return p_List;
}


/*******************************************************************************************
 ** @brief: get_channelIdByCanType
 ** @param: type
 *******************************************************************************************/
extern uint8 get_channelIdByCanType( E_CAN_TYPE type )
{
    uint8 channel_id = 0U;
    switch( type )
    {
    case E_TX1CAN1: channel_id = DATA_CHANNEL_TX1CAN1;break;
    case E_TX1CAN2: channel_id = DATA_CHANNEL_TX1CAN2;break;
    case E_TX1CAN3: channel_id = DATA_CHANNEL_TX1CAN3;break;
    case E_TX1CAN4: channel_id = DATA_CHANNEL_TX1CAN4;break;
    case E_TX1CAN5: channel_id = DATA_CHANNEL_TX1CAN5;break;
    case E_TX1VMCAN1: channel_id = DATA_CHANNEL_TX1VMCAN1;break;
    case E_TX1VMCAN2: channel_id = DATA_CHANNEL_TX1VMCAN2;break;

    case E_TX2CAN1: channel_id = DATA_CHANNEL_TX2CAN1;break;
    case E_TX2CAN2: channel_id = DATA_CHANNEL_TX2CAN2;break;
    case E_TX2CAN3: channel_id = DATA_CHANNEL_TX2CAN3;break;
    case E_TX2CAN4: channel_id = DATA_CHANNEL_TX2CAN4;break;
    case E_TX2CAN5: channel_id = DATA_CHANNEL_TX2CAN5;break;
    case E_TX2VMCAN1: channel_id = DATA_CHANNEL_TX2VMCAN1;break;
    case E_TX2VMCAN2: channel_id = DATA_CHANNEL_TX2VMCAN2;break;

    default:break;
    }
    return channel_id;
}

/*******************************************************************************************
 ** @brief: get_channelIdByEthType
 ** @param: type
 *******************************************************************************************/
extern uint8 get_channelIdByEthType( E_ETH_TYPE type )
{
    uint8 channel_id = 0U;
    switch( type )
    {
    case E_TX1_ETH1: channel_id = DATA_CHANNEL_TX1ETH1;break;
    case E_TX1_ETH2: channel_id = DATA_CHANNEL_TX1ETH2;break;
    case E_TX1_ETH3: channel_id = DATA_CHANNEL_TX1ETH3;break;
    case E_TX1_ETH4: channel_id = DATA_CHANNEL_TX1ETH4;break;

    case E_TX2_ETH1: channel_id = DATA_CHANNEL_TX2ETH1;break;
    case E_TX2_ETH2: channel_id = DATA_CHANNEL_TX2ETH2;break;

    case E_TX2_HDLC: channel_id = DATA_CHANNEL_TX2HDLC;break;

    case E_TX2_MVB : channel_id = DATA_CHANNEL_TX2MVB;break;

    case E_TX1_RS485A:channel_id = DATA_CHANNEL_TX1RS485a;break;
    case E_TX1_RS485B:channel_id = DATA_CHANNEL_TX1RS485b;break;
    case E_TX2_RS485A:channel_id = DATA_CHANNEL_TX2RS485a;break;

    default:break;
    }
    return channel_id;
}
