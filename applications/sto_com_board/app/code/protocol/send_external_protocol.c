/***************************************************************************
文件名：vcp_app_layer.c
模    块：主控插件与通信插件通信协议——应用层解析与组包
详    述：
作    者：jiaqx,20231129
***************************************************************************/

#include "support_libPub.h"
#include "list_manage.h"
#include "send_external_protocol.h"
#include "receive_mainctl_protocol.h"
#include "devlib_can.h"
#include "devlib_eth.h"
#include "devlib_hdlc.h"
#include "devlib_mvb.h"
#include "devlib_rs485.h"

/*******************************************************************************************
 ** @brief: STO_DMI_canProtocol
 ** @param: e_can_type *p_can_frame_data
 *******************************************************************************************/
static void STO_DMI_canProtocol( E_CAN_TYPE e_can_type, S_CAN_FRAME_DATA *p_can_frame_data )
{
    uint8 pri_u8 = 0U, no_u8 = 0U;
    uint8 ucCAN_ID  = 0x62U;

    /* 1.参数合法性检查 */
    if(( NULL == p_can_frame_data ) || ( 0U == p_can_frame_data->length_u8 ) )
    {
        return ;
    }

    /* 2.通道有效性检查*/
    if(( E_TX2CAN1 != e_can_type ) && ( E_TX2CAN2 != e_can_type ))
    {
        return ;
    }

    /* 3.修正优先级与序号 */
    pri_u8 = p_can_frame_data->priority_u8;
    no_u8 = p_can_frame_data->no_u8;
    switch ( pri_u8 )
    {
    /* 3.1 修正主控插件CPUA自检信息*/
        case 0x63u:
        {
            p_can_frame_data->priority_u8 = ucCAN_ID;
            switch ( no_u8 )
            {
                case 0U :p_can_frame_data->no_u8 = 0U;break;
                case 1U :p_can_frame_data->no_u8 = 1U;break;
                case 2U :p_can_frame_data->no_u8 = 3U;p_can_frame_data->priority_u8 = ucCAN_ID + 2U;break;
                case 3U :p_can_frame_data->no_u8 = 5U;break;
                default :support_memset((uint8*)p_can_frame_data,0x00,sizeof(S_CAN_FRAME_DATA));break;

            }
        }
        break;

        /* 3.2 修正主控插件CPUB自检信息*/
        case 0x73u:
        {
            p_can_frame_data->priority_u8 = ucCAN_ID;
            switch ( no_u8 )
            {
                case 0U :p_can_frame_data->no_u8 = 2U;break;
                case 1U :p_can_frame_data->no_u8 = 3U;break;
                case 2U :p_can_frame_data->no_u8 = 4U;p_can_frame_data->priority_u8 = ucCAN_ID + 2U;break;
                default :support_memset((uint8*)p_can_frame_data,0x00,sizeof(S_CAN_FRAME_DATA));break;
            }
        }
        break;

        /* 3.3 修正通信插件自检信息*/
        case 0x50U:
        {
            p_can_frame_data->priority_u8 = ucCAN_ID;
            switch ( no_u8 )
            {
                case 0U :p_can_frame_data->no_u8 = 4U;break;
                default :support_memset((uint8*)p_can_frame_data,0x00,sizeof(S_CAN_FRAME_DATA));break;
            }
        }
        break;

        /* 3.4 修正显示器自检信息*/
        case 0x90U:
        case 0x92U:
        {
            p_can_frame_data->priority_u8 = ucCAN_ID;
            switch ( no_u8 )
            {
                case 0U :p_can_frame_data->no_u8 = 5U;break;
                default :support_memset((uint8*)p_can_frame_data,0x00,sizeof(S_CAN_FRAME_DATA));break;
            }
        }
        break;

        /* 3.5 修正记录插件自检信息*/
        case 0xA0U:
        {
            p_can_frame_data->priority_u8 = ucCAN_ID;
            switch ( no_u8 )
            {
                case 0U :p_can_frame_data->no_u8 = 7U;break;
                default :support_memset((uint8*)p_can_frame_data,0x00,sizeof(S_CAN_FRAME_DATA));break;
            }
        }
        break;

        /* 3.6 其他优先级均不变*/
        default:
            break;
    }

}

/*******************************************************************************************
 ** @brief: get_e_can_class
 ** @param: e_can_type
 *******************************************************************************************/
static E_CAN_CLASS get_e_can_class( E_CAN_TYPE e_can_type )
{
    E_CAN_CLASS e_can_class = E_CAN_MAX;
    switch(e_can_type)
    {
    case E_TX1CAN1: e_can_class = E_CAN_EX1;break;
    case E_TX1CAN2: e_can_class = E_CAN_EX2;break;
    case E_TX1CAN3: e_can_class = E_CAN_EX1;break;
    case E_TX1CAN4: e_can_class = E_CAN_EX2;break;
    case E_TX1CAN5: e_can_class = E_CAN_EX3;break;
    case E_TX2CAN1: e_can_class = E_CAN_EX1;break;
    case E_TX2CAN2: e_can_class = E_CAN_EX2;break;
    case E_TX2CAN3: e_can_class = E_CAN_EX1;break;
    case E_TX2CAN4: e_can_class = E_CAN_EX2;break;
    case E_TX2CAN5: e_can_class = E_CAN_EX3;break;

    case E_TX1VMCAN1:
    case E_TX1VMCAN2:
    case E_TX2VMCAN1:
    case E_TX2VMCAN2:
        break;
    default:
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
        break;
    }
    return e_can_class;

}


/*******************************************************************************************
 ** @brief: get_e_eth_class
 ** @param: e_eth_type
 *******************************************************************************************/
static E_ETH_CLASS get_e_eth_class( E_ETH_TYPE e_eth_type )
{
    E_ETH_CLASS e_eth_class = E_ETH_MAX;
    switch( e_eth_type )
    {
    case E_TX1_ETH1: e_eth_class = E_ETH_EX1;break;
    case E_TX1_ETH2: e_eth_class = E_ETH_EX2;break;
    case E_TX1_ETH3: e_eth_class = E_ETH_EX1;break;
    case E_TX1_ETH4: e_eth_class = E_ETH_EX2;break;
    case E_TX2_ETH1: e_eth_class = E_ETH_EX1;break;
    case E_TX2_ETH2: e_eth_class = E_ETH_EX1;break;
    default:
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
        break;
    }
    return e_eth_class;

}

/*******************************************************************************************
 ** @brief: get_e_rs485_class
 ** @param: e_eth_type
 *******************************************************************************************/
static E_HDLC_CLASS get_e_hdlc_class( E_ETH_TYPE e_eth_type )
{
    E_HDLC_CLASS e_hdlc_class = E_HDLC_MAX;

    switch(e_eth_type)
    {
    case E_TX2_HDLC: e_hdlc_class = E_HDLC_EX1;break;

    default:
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
        break;
    }

    return e_hdlc_class;

}

/*******************************************************************************************
 ** @brief: get_e_rs485_class
 ** @param: e_eth_type
 *******************************************************************************************/
static E_MVB_CLASS get_e_mvb_class( E_ETH_TYPE e_eth_type )
{
    E_MVB_CLASS e_mvb_class = E_MVB_MAX;

    switch(e_eth_type)
    {
    case E_TX2_MVB: e_mvb_class = E_MVB_EX1;break;

    default:
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
        break;
    }

    return e_mvb_class;

}

/*******************************************************************************************
 ** @brief: get_e_rs485_class
 ** @param: e_eth_type
 *******************************************************************************************/
static E_RS485_CLASS get_e_rs485_class( E_ETH_TYPE e_eth_type )
{
    E_RS485_CLASS e_rs485_class = E_RS485_MAX;

    switch(e_eth_type)
    {
    case E_TX1_RS485A: e_rs485_class = E_RS485_EX1;break;
    case E_TX1_RS485B: e_rs485_class = E_RS485_EX1;break;
    case E_TX2_RS485A: e_rs485_class = E_RS485_EX1;break;
    default:
        MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
        break;
    }

    return e_rs485_class;

}

/*******************************************************************************************
 ** @brief: protocol_can_sendData
 ** @param: e_eth_type
 *******************************************************************************************/
static void protocol_can_sendData( E_CAN_CLASS can_cls, S_CAN_FRAME_DATA *p_can_frame_data )
{
    /* 1.参数合法性检查 */
    if(( NULL == p_can_frame_data ) || ( 0U == p_can_frame_data->length_u8 ) )
    {
        //MY_Printf("protocol_can_sendData formal parameter is illegal !!!");
        return ;
    }

    /* 2.双CAN与单CAN进行绑定 */
    switch ( can_cls )
    {
        case E_CAN_EX1:
        case E_CAN_EX2:
            devLib_can_sendData( E_CAN_EX1, p_can_frame_data);
            devLib_can_sendData( E_CAN_EX2, p_can_frame_data);
            break;
        case E_CAN_EX3:
            devLib_can_sendData( E_CAN_EX3, p_can_frame_data);
        default:
            break;
    }

}

/*******************************************************************************************
 ** @brief: send_can_list_data
 ** @param: e_eth_type
 *******************************************************************************************/
static void send_can_list_data( E_CAN_TYPE e_can_type)
{
    S_CAN_FRAME_DATA s_can_frame_data = {0};
    E_CAN_CLASS e_can_class = E_CAN_MAX;

    /* 1.获取队列数据 */
    while( TRUE == getListData_CANtype_mainctl(e_can_type, &s_can_frame_data) )
    {
        /* 1.1 获取硬件资源  */
        e_can_class = get_e_can_class( e_can_type );

        /* 1.2 CAN帧优先级修改 */
        STO_DMI_canProtocol(e_can_type,&s_can_frame_data);

        /* 1.3 发送数据*/
        protocol_can_sendData( e_can_class, &s_can_frame_data);
    }

}


/*******************************************************************************************
 ** @brief: send_eth_list_data
 ** @param: e_eth_type
 *******************************************************************************************/
static void send_eth_list_data(E_ETH_TYPE e_eth_type)
{
    S_ETH_FRAME s_eth_frame = {0};
    E_ETH_CLASS e_eth_class = E_ETH_MAX;

    /* 1.获取队列数据 */
    while( TRUE == getListData_ETHtype_mainctl(e_eth_type, &s_eth_frame) )
    {
        /* 1.1 检查长度单帧长度合法性 */
        if (s_eth_frame.len > ETH_FRAME_SIZE_MAX)
        {
            MY_Printf("send_eth_list_data s_eth_frame.len > ETH_FRAME_SIZE_MAX !!!\r\n");
            break ;
        }
        /* 1.2 获取硬件资源 */
        e_eth_class = get_e_eth_class( e_eth_type );

        /* 1.3 发送数据 */
        devLib_eth_sendData(e_eth_class,(uint8*)&s_eth_frame.data_u8,s_eth_frame.len);

    }

}

/*******************************************************************************************
 ** @brief: send_hdlc_list_data
 ** @param: e_eth_type
 *******************************************************************************************/
static void send_hdlc_list_data(E_ETH_TYPE e_eth_type)
{
    S_ETH_FRAME s_eth_frame = {0};
    S_HDLC_FRAME* p_hdlc_frame = {0};
    E_HDLC_CLASS e_hdlc_class = E_HDLC_MAX;

    /* 1.获取队列数据 */
    while( TRUE == getListData_ETHtype_mainctl(e_eth_type, &s_eth_frame) )
    {
        /* 1.1 检查长度单帧长度合法性 */
        if (s_eth_frame.len > HDLC_FRAME_SIZE_MAX)
        {
            MY_Printf("send_hdlc_list_data s_eth_frame.len > HDLC_FRAME_SIZE_MAX !!!\r\n");
            break ;
        }

        /* 1.2 数据格式转换 */
        else
        {
            p_hdlc_frame = (S_HDLC_FRAME*)&s_eth_frame;
        }

        /* 1.3 获取硬件资源 */
        e_hdlc_class = get_e_hdlc_class( e_eth_type );

        /* 1.4 发送数据 */
        devlib_hdlc_sendData(e_hdlc_class,p_hdlc_frame);
    }

}

/*******************************************************************************************
 ** @brief: send_mvb_list_data
 ** @param: e_eth_type
 *******************************************************************************************/
static void send_mvb_list_data( E_ETH_TYPE e_eth_type)
{
    S_ETH_FRAME s_eth_frame = {0};
    S_MVB_FRAME* p_mvb_frame = NULL;
    E_MVB_CLASS e_mvb_class = E_MVB_MAX;

    /* 1.获取队列数据 */
    while( TRUE == getListData_ETHtype_mainctl(e_eth_type, &s_eth_frame) )
    {
        /* 1.1 检查长度单帧长度合法性 */
        if (s_eth_frame.len > MVB_FRAME_SIZE_MAX)
        {
            MY_Printf("send_mvb_list_data s_eth_frame.len > MVB_FRAME_SIZE_MAX !!!\r\n");
            break ;
        }

        /* 1.2 数据格式转换 */
        else
        {
            p_mvb_frame = (S_MVB_FRAME*)&s_eth_frame;
        }

        /* 1.3 获取硬件资源 */
        e_mvb_class = get_e_mvb_class( e_eth_type );

        /* 1.4 发送数据 */
        devlib_mvb_sendData(e_mvb_class,p_mvb_frame);
    }

}

/*******************************************************************************************
 ** @brief: send_rs485_list_data
 ** @param: e_eth_type
 *******************************************************************************************/
static void send_rs485_list_data( E_ETH_TYPE e_eth_type)
{
    S_ETH_FRAME s_eth_frame = {0};
    S_RS485_FRAME* p_rs485_frame = NULL;
    E_RS485_CLASS e_rs485_class = E_RS485_MAX;

    /* 1.获取队列数据 */
    while( TRUE == getListData_ETHtype_mainctl(e_eth_type, &s_eth_frame) )
    {

        /* 1.1 检查长度单帧长度合法性 */
        if (s_eth_frame.len > RS485_FRAME_SIZE_MAX)
        {
            MY_Printf("send_mvb_list_data s_eth_frame.len > MVB_FRAME_SIZE_MAX !!!\r\n");
            break ;
        }

        /* 1.2 数据格式转换 */
        else
        {
            p_rs485_frame = (S_RS485_FRAME*)&s_eth_frame;
        }

        /* 1.3 获取硬件资源 */
        e_rs485_class = get_e_rs485_class( e_eth_type );

        /* 1.4 发送数据 */
        devLib_rs485_sendData(e_rs485_class,p_rs485_frame);

    }

}


/*******************************************************************************************
 ** @brief: send_external_list_data
 ** @param: channel_type_num e_channel_type
 *******************************************************************************************/
extern void send_external_list_data( uint8 channel_type_num, E_CHANNEL_TYPE e_channel_type)
{

    /* 1.CAN通道组织通道数据 */
    if (E_CHANNEL_CAN == e_channel_type)
    {
        send_can_list_data(channel_type_num);
    }

    /* 2.ETH通道组织通道数据 */
    else
    {
        switch( channel_type_num )
        {
        case E_TX1_ETH1:
        case E_TX1_ETH2:
        case E_TX1_ETH3:
        case E_TX1_ETH4:
        case E_TX2_ETH1:
        case E_TX2_ETH2:
            send_eth_list_data( channel_type_num );
            break;
        case E_TX2_HDLC:
            send_hdlc_list_data( channel_type_num );
            break;
        case E_TX2_MVB:
            send_mvb_list_data( channel_type_num );
            break;
        case E_TX1_RS485A:
        case E_TX1_RS485B:
        case E_TX2_RS485A:
            send_rs485_list_data( channel_type_num );
            break;
        default:
            MY_Printf("channel_data_pakage channel_type is err !!!\r\n");
            break;
        }
    }

}





