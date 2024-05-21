/***************************************************************************
文件名：vcp_app_layer.c
模    块：主控插件与通信插件通信协议——应用层解析与组包
详    述：
作    者：jiaqx,20231129
***************************************************************************/
#include "vcp_app_layer.h"
#include "vcp_time_manage.h"
#include "devlib_eth.h"
#include "support_libPub.h"
#include "vcp_safe_layer.h"
#include "vcp_app_layer_process.h"
typedef struct
{
    uint16 timeajust;              /* 时钟同步 */
    uint16 applytimeajust;         /* 申请时钟同步  有应答 */
    uint16 getpluginfo;            /* 获取插件信息 */
    uint16 setchlinfo;             /* 设置通道配置信息 */
    uint16 getchlinfo;             /* 设置通道配置信息 */
    uint16 setrouter;              /* 设置数据转发路由表 */
    uint16 getrouter;              /* 查询数据转发路由表 */
    uint16 exp;                    /* 收到外部数据发送给主控 */

} S_MSG_NO_TEPY;

typedef struct
{
    S_MSG_NO_TEPY CAN1;            /* 应用层CAN1序列号 */
    S_MSG_NO_TEPY CAN2;            /* 应用层CAN2序列号 */
    S_MSG_NO_TEPY ETH;            /* 应用层ETH序列号 */
} S_APPLAYER_MSG_NO_U16;

static S_APPLAYER_MSG_NO_U16 s_applayer_mag_no_u16;


/****************************************************************************
* 函数名: round_circle_process
* 说    明: 应用层轮训数据——请求方数据处理
* 参    数: uint8 *pApp 应用层数据
*       uint16 applayer_len 应用层数据长度
* 返回值: 无
* 备    注: 仅实现系内以太网数据处理
 ****************************************************************************/
static void round_circle_process( uint8 *pApp, uint16 applayer_len)
{
    uint8 *pAppData = NULL;
    uint16 applayer_datalen = 0U;
    S_APP_LAYER_HEADER s_app_layer_heander;

    /* 1.参数合法性检查 */
    if( pApp == NULL)
    {
        MY_Printf("round_circle_process pApp == NULL err !!!\r\n");
        return ;
    }

    if( (applayer_len < sizeof(S_APP_LAYER_HEADER) ) || ( applayer_len > SAFE_LAYER_PLOADLEN ) )
    {
        MY_Printf("round_circle_process applayer_len is err !!!\r\n");
        return ;
    }

    /* 2.解析应用数据 */
    /* 2.1 解析应用层帧头 */
    support_memcpy(&s_app_layer_heander, (S_APP_LAYER_HEADER *)pApp, sizeof(S_APP_LAYER_HEADER));

    /* 2.1 解析应用层负载数据 */
    applayer_datalen = applayer_len - sizeof(S_APP_LAYER_HEADER);
    pAppData = &pApp[sizeof(S_APP_LAYER_HEADER)];

    /* 3.分类进行处理 */
    /* 3.1 检查报文类型 */
    if(ROUND_CIRCLE_TYPE != s_app_layer_heander.msg_type )
    {
        MY_Printf("round_circle_process msg_type is err %d !!!\r\n",s_app_layer_heander.msg_type);
        return;
    }

    /* 3.2 数据处理 */
    switch( s_app_layer_heander.msg_sub_type )
    {
        /* 2.1 时钟同步——有应答 */
        case TIME_SET_LOCAL:
            time_set_local_process( &pAppData[0], applayer_datalen);
            break;

        /* 2.2 获取插件信息 */
        case COMM_PLUG_INFO:
            comm_plug_info_process();
            break;

        /* 2.3 配置通道配置信息 */
        case SET_CHANNL_INFO:
              /* 暂无 */
            break;

        /* 2.4 查询通道配置信息 */
        case GET_CHANNL_INFO:
            /* 暂无 */
             break;

        /* 2.5 设置数据转发路由表 */
        case SET_CONTIUE_INFO:
              /* 暂无 */
             break;

        /* 2.6 查询数据转发路由表 */
        case GET_CONTIUE_INFO:
             /* 暂无 */
             break;

        /* 2.6 异常防护 */
        default:
            MY_Printf("round_circle_process TIMER_PULSE_TYPE msg_sub_type err !\r\n");
            break;
    }
}

/****************************************************************************
* 函数名: round_circle_ack_process
* 说    明: 应用层轮训数据——应答方数据处理
* 参    数: uint8 *pApp 应用层数据
*       uint16 applayer_len 应用层数据长度
* 返回值: 无
* 备    注: 仅实现系内以太网数据处理
 ****************************************************************************/
static void round_circle_ack_process( uint8 *pBuf )
{
//    r_app_layer *pApp_layer = NULL;
//
//    /* 1.参数合法性检查 */
//    if( pBuf == NULL)
//    {
//        MY_Printf("round_circle_process pBuf == NULL err !\r\n");
//        return ;
//    }
//
//    /* 2.识别子类型*/
//    pApp_layer = (r_app_layer *)pBuf;
//    switch( pApp_layer->msg_sub_type )
//    {
//        /* 2.1 时钟同步申请——无应答 */
//        case APPLY_TIME_SET:
//            /* 时钟同步应答处理
//             * 1. 设置同步时钟
//             *  */
//            break;
//
//        /* 2.2 异常防护 */
//        default:
//            MY_Printf("round_circle_ack_process TIMER_PULSE_TYPE msg_sub_type err !\r\n");
//            break;
//    }
}

extern void rec_msg_mainctl_proc( uint8 *pdata );
/****************************************************************************
* 函数名: round_pluse_process
* 说    明: 应用层周期数据处理
* 参    数: uint8 *pBuf 应用层数据
*       uint16 applayer_len 应用层数据长度
* 返回值: 无
* 备    注: 仅实现系内以太网数据处理
 ****************************************************************************/
static void round_pluse_process( uint8 *pApp  ,uint16 applayer_len)
{
    uint8 *pAppData = NULL;
    uint16 applayer_datalen = 0U;
    S_APP_LAYER_HEADER s_app_layer_heander;

    /* 1.参数合法性检查 */
    if( pApp == NULL)
    {
        MY_Printf("round_pluse_process pApp == NULL err !!!\r\n");
        return ;
    }

    if( (applayer_len < sizeof(S_APP_LAYER_HEADER) ) || ( applayer_len > SAFE_LAYER_PLOADLEN ) )
    {
        MY_Printf("round_pluse_process applayer_len is err !!!\r\n");
        return ;
    }

    /* 2.解析应用数据 */
    /* 2.1 解析应用层帧头 */
    support_memcpy(&s_app_layer_heander, (S_APP_LAYER_HEADER *)pApp, sizeof(S_APP_LAYER_HEADER));

    /* 2.1 解析应用层负载数据 */
    applayer_datalen = applayer_len - sizeof(S_APP_LAYER_HEADER);
    pAppData = &pApp[sizeof(S_APP_LAYER_HEADER)];

    /* 3.分类进行处理 */
    /* 3.1 检查报文类型 */
    if(ROUND_PULSE_MODE != s_app_layer_heander.msg_type )
    {
        MY_Printf("round_pluse_process msg_type is err %d !!!\r\n",s_app_layer_heander.msg_sub_type);
        return;
    }

    /* 2.识别子类型*/
    switch(s_app_layer_heander.msg_sub_type)
    {
        /* 2.1 IAP数据包*/
        case IAP_PAKAGE: break;

        /* 2.2 外部通道数据包*/
        case RX_MAINCTLDATA_EXP:
            rec_msg_mainctl_proc(pAppData);
            set_RoundPluseState(E_ROUNDPLSE_OK);
#if 0
            MY_Printf("↓\r\n");
            MY_Printf("round_pluse_process len:%d !!!\r\n", applayer_datalen);
            MY_Printf("pSafeTx data:\r\n");
            for(uint16 i=0; i<applayer_datalen; i++)
            {
                MY_Printf("%.2x ",pAppData[i]);
                if( 0U == ( (i+1) % 20 ) )
                    MY_Printf("\r\n");
            }
            MY_Printf("\r\n");
            MY_Printf("↑\r\n");
#endif
            break;

        /* 2.3 异常防护 */
        default:
            MY_Printf("vcp_app_layer_round_pluse_process TIMER_PULSE_TYPE msg_sub_type err !\r\n");
            break;
    }

}

/****************************************************************************
* 函数名: vcp_app_layer_process
* 说    明:应用层数据处理
* 参    数: uint8 *pApp 应用层数据
*       uint16 applayer_len 应用层数据长度
* 返回值: 应用层处理结果标识
* 备    注: 仅实现系内以太网数据处理
 ****************************************************************************/
extern void vcp_app_layer_process(uint8 *pApp , uint16 applayer_len )
{
    S_APP_LAYER_HEADER s_app_layer_heander;

    /* 1. 参数合法性检查 */
    if( pApp == NULL )
    {
        MY_Printf("vcp_app_layer_process pApp == NULL err !!!\r\n");
        return ;
    }

    if (0U == applayer_len)
    {
        MY_Printf("vcp_app_layer_process applayer_len == 0U !!!\r\n");
        return ;
    }
#if 0
    if( 0U != applayer_len )
    {

        MY_Printf("vcp_app_layer_process eth data len is legal, len:%d !!!\r\n",applayer_len);
        MY_Printf("data:");
        for(uint16 i=0;i<applayer_len;i++)
        {
            MY_Printf("%.2x ",pApp[i]);
        }
        MY_Printf("\r\n");
    }
#endif

    /* 2.获取应用层帧头 */
    support_memcpy(&s_app_layer_heander, (S_APP_LAYER_HEADER *)pApp, sizeof(S_APP_LAYER_HEADER));

    /* 2.识别VCP应用层报文类型 */
    switch( s_app_layer_heander.msg_type )
    {
        /*2.1 轮循模式  请求方*/
        case ROUND_CIRCLE_TYPE:
            round_circle_process(pApp, applayer_len);
            break;

        /*2.2 轮循模式  应答方*/
        case ROUND_CIRCLE_ACK_TYPE:
            round_circle_ack_process( pApp );
            break;

        /*2.3 周期模式  有应答模式*/
        case ROUND_PULSE_MODE:
            round_pluse_process( pApp , applayer_len);
            break;

        /*2.4 异常防护处理 */
        default:
            MY_Printf("app_layer_check msg_type err msg_tepe = %d !!!\r\n",s_app_layer_heander.msg_type);
            break;
    }

}

/* ---------------------------------------VCP_APP应用层封包相关函数--------------------------------------------------------- */

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN1_circle_ack
* 说    明: 组织应用层数据——序列号——CAN1——轮训模式_请求方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN1——轮训模式_请求方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN1_circle( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 申请时钟同步类型 */
        case APPLY_TIME_SET:/*6 轮询模式，申请时钟同步*/
            serial_num = s_applayer_mag_no_u16.CAN1.timeajust;
            s_applayer_mag_no_u16.CAN1.timeajust = count_msg_no16( s_applayer_mag_no_u16.CAN1.timeajust );
            break;

        /* 1.2 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN1_circle_ack
* 说    明: 组织应用层数据——序列号——CAN1——轮训模式_回应方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN1——轮训模式_回应方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN1_circle_ack( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 时钟同步——有应答 */
        case TIME_SET_LOCAL:
            serial_num = s_applayer_mag_no_u16.CAN1.applytimeajust;
            s_applayer_mag_no_u16.CAN1.applytimeajust = count_msg_no16( s_applayer_mag_no_u16.CAN1.applytimeajust );
            break;

        /* 1.2 获取插件信息 */
        case COMM_PLUG_INFO:
            serial_num = s_applayer_mag_no_u16.CAN1.getpluginfo;
            s_applayer_mag_no_u16.ETH.getpluginfo = count_msg_no16( s_applayer_mag_no_u16.CAN1.getpluginfo );
            break;

        /* 1.3 配置通道配置信息 */
        case SET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.CAN1.setchlinfo;
            s_applayer_mag_no_u16.CAN1.setchlinfo = count_msg_no16( s_applayer_mag_no_u16.CAN1.setchlinfo );
            break;

        /* 1.4 查询通道配置信息 */
        case GET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.CAN1.getchlinfo;
            s_applayer_mag_no_u16.CAN1.getchlinfo = count_msg_no16( s_applayer_mag_no_u16.CAN1.getchlinfo );
            break;

        /* 1.5 设置数据转发路由表 */
        case SET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.CAN1.setrouter;
            s_applayer_mag_no_u16.CAN1.setrouter = count_msg_no16( s_applayer_mag_no_u16.CAN1.setrouter );
            break;

        /* 1.6 查询数据转发路由表 */
        case GET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.CAN1.getrouter;
            s_applayer_mag_no_u16.CAN1.getrouter = count_msg_no16( s_applayer_mag_no_u16.CAN1.getrouter );
            break;

        /* 1.7 异常防护 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN1_pluse
* 说    明: 组织应用层数据——序列号——CAN1——周期模式
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN1——周期模式
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN1_pluse(uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 IAP数据包*/
        case IAP_PAKAGE: break;

        /* 1.2 外部数据类型 */
        case RX_EXPDATA_MAINCTL:
            serial_num = s_applayer_mag_no_u16.CAN1.exp;
            s_applayer_mag_no_u16.CAN1.exp = count_msg_no16( s_applayer_mag_no_u16.CAN1.exp );
            break;

        /* 1.3 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN1
* 说    明: 组织应用层数据——序列号——CAN1
* 参    数: uint8 msg_type 待封装的报文类型
*       uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN1
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN1(uint8 msg_type, uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1. 数据处理 */
    switch( msg_type )
    {
        /*1.1 轮循模式  请求方*/
        case ROUND_CIRCLE_TYPE:
            serial_num = get_applayer_msg_no_CAN1_circle( msg_sub_type );
            break;

        /*1.2 轮循模式  应答方*/
        case ROUND_CIRCLE_ACK_TYPE:
            serial_num = get_applayer_msg_no_CAN1_circle_ack( msg_sub_type );
            break;

        /*1.3 周期模式  有应答模式*/
        case ROUND_PULSE_MODE:
            serial_num = get_applayer_msg_no_CAN1_pluse( msg_sub_type );
            break;

        /*1.4 异常防护处理 */
        default:
            MY_Printf("get_applayer_msg_no_ETH msg_type err !\r\n");
            break;
    }
    return serial_num;

}


/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN2_circle_ack
* 说    明: 组织应用层数据——序列号——CAN2——轮训模式_请求方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN2——轮训模式_请求方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN2_circle( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 申请时钟同步类型 */
        case APPLY_TIME_SET:/*6 轮询模式，申请时钟同步*/
            serial_num = s_applayer_mag_no_u16.CAN2.timeajust;
            s_applayer_mag_no_u16.CAN2.timeajust = count_msg_no16( s_applayer_mag_no_u16.CAN2.timeajust );
            break;

        /* 1.2 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN2_circle_ack
* 说    明: 组织应用层数据——序列号——CAN2——轮训模式_回应方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN2——轮训模式_回应方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN2_circle_ack( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 时钟同步——有应答 */
        case TIME_SET_LOCAL:
            serial_num = s_applayer_mag_no_u16.CAN2.applytimeajust;
            s_applayer_mag_no_u16.CAN2.applytimeajust = count_msg_no16( s_applayer_mag_no_u16.CAN2.applytimeajust );
            break;

        /* 1.2 获取插件信息 */
        case COMM_PLUG_INFO:
            serial_num = s_applayer_mag_no_u16.CAN2.getpluginfo;
            s_applayer_mag_no_u16.CAN2.getpluginfo = count_msg_no16( s_applayer_mag_no_u16.CAN2.getpluginfo );
            break;

        /* 1.3 配置通道配置信息 */
        case SET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.CAN2.setchlinfo;
            s_applayer_mag_no_u16.CAN2.setchlinfo = count_msg_no16( s_applayer_mag_no_u16.CAN2.setchlinfo );
            break;

        /* 1.4 查询通道配置信息 */
        case GET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.CAN2.getchlinfo;
            s_applayer_mag_no_u16.CAN2.getchlinfo = count_msg_no16( s_applayer_mag_no_u16.CAN2.getchlinfo );
            break;

        /* 1.5 设置数据转发路由表 */
        case SET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.CAN2.setrouter;
            s_applayer_mag_no_u16.ETH.setrouter = count_msg_no16( s_applayer_mag_no_u16.CAN2.setrouter );
            break;

        /* 1.6 查询数据转发路由表 */
        case GET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.CAN2.getrouter;
            s_applayer_mag_no_u16.CAN2.getrouter = count_msg_no16( s_applayer_mag_no_u16.CAN2.getrouter );
            break;

        /* 1.7 异常防护 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_ETH_pluse
* 说    明: 组织应用层数据——序列号——CAN2——周期模式
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN2——周期模式
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN2_pluse(uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 IAP数据包*/
        case IAP_PAKAGE: break;

        /* 1.2 外部数据类型 */
        case RX_EXPDATA_MAINCTL:
            serial_num = s_applayer_mag_no_u16.CAN2.exp;
            s_applayer_mag_no_u16.CAN2.exp = count_msg_no16( s_applayer_mag_no_u16.CAN2.exp );
            break;

        /* 1.3 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_CAN2
* 说    明: 组织应用层数据——序列号——CAN2
* 参    数: uint8 msg_type 待封装的报文类型
*       uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——CAN2
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_CAN2(uint8 msg_type, uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1. 数据处理 */
    switch( msg_type )
    {
        /*1.1 轮循模式  请求方*/
        case ROUND_CIRCLE_TYPE:
            serial_num = get_applayer_msg_no_CAN2_circle( msg_sub_type );
            break;

        /*1.2 轮循模式  应答方*/
        case ROUND_CIRCLE_ACK_TYPE:
            serial_num = get_applayer_msg_no_CAN2_circle_ack( msg_sub_type );
            break;

        /*1.3 周期模式  有应答模式*/
        case ROUND_PULSE_MODE:
            serial_num = get_applayer_msg_no_CAN2_pluse( msg_sub_type );
            break;

        /*1.4 异常防护处理 */
        default:
            MY_Printf("get_applayer_msg_no_ETH msg_type err !\r\n");
            break;
    }
    return serial_num;

}


/****************************************************************************
* 函数名:  get_applayer_msg_no_ETH_circle_ack
* 说    明: 组织应用层数据——序列号——ETH——轮训模式_请求方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——ETH——轮训模式_请求方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_ETH_circle( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 申请时钟同步类型 */
        case APPLY_TIME_SET:/*6 轮询模式，申请时钟同步*/
            serial_num = s_applayer_mag_no_u16.ETH.timeajust;
            s_applayer_mag_no_u16.ETH.timeajust = count_msg_no16( s_applayer_mag_no_u16.ETH.timeajust );
            break;

        /* 1.2 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_ETH_circle_ack
* 说    明: 组织应用层数据——序列号——ETH——轮训模式_回应方
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——ETH——轮训模式_回应方
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_ETH_circle_ack( uint8 msg_sub_type )
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 时钟同步——有应答 */
        case TIME_SET_LOCAL:
            serial_num = s_applayer_mag_no_u16.ETH.applytimeajust;
            s_applayer_mag_no_u16.ETH.applytimeajust = count_msg_no16( s_applayer_mag_no_u16.ETH.applytimeajust );
            break;

        /* 1.2 获取插件信息 */
        case COMM_PLUG_INFO:
            serial_num = s_applayer_mag_no_u16.ETH.getpluginfo;
            s_applayer_mag_no_u16.ETH.getpluginfo = count_msg_no16( s_applayer_mag_no_u16.ETH.getpluginfo );
            break;

        /* 1.3 配置通道配置信息 */
        case SET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.ETH.setchlinfo;
            s_applayer_mag_no_u16.ETH.setchlinfo = count_msg_no16( s_applayer_mag_no_u16.ETH.setchlinfo );
            break;

        /* 1.4 查询通道配置信息 */
        case GET_CHANNL_INFO:
            serial_num = s_applayer_mag_no_u16.ETH.getchlinfo;
            s_applayer_mag_no_u16.ETH.getchlinfo = count_msg_no16( s_applayer_mag_no_u16.ETH.getchlinfo );
            break;

        /* 1.5 设置数据转发路由表 */
        case SET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.ETH.setrouter;
            s_applayer_mag_no_u16.ETH.setrouter = count_msg_no16( s_applayer_mag_no_u16.ETH.setrouter );
            break;

        /* 1.6 查询数据转发路由表 */
        case GET_CONTIUE_INFO:
            serial_num = s_applayer_mag_no_u16.ETH.getrouter;
            s_applayer_mag_no_u16.ETH.getrouter = count_msg_no16( s_applayer_mag_no_u16.ETH.getrouter );
            break;

        /* 1.7 异常防护 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_ETH_pluse
* 说    明: 组织应用层数据——序列号——ETH——周期模式
* 参    数: uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——ETH——周期模式
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_ETH_pluse(uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1.识别应用层帧子类型*/
    switch( msg_sub_type )
    {
        /* 1.1 IAP数据包*/
        case IAP_PAKAGE: break;

        /* 1.2 外部数据类型 */
        case RX_EXPDATA_MAINCTL:
            serial_num = s_applayer_mag_no_u16.ETH.exp;
            s_applayer_mag_no_u16.ETH.exp = count_msg_no16( s_applayer_mag_no_u16.ETH.exp );
            break;

        /* 1.3 异常防护处理 */
        default:
            MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
            break;
    }

    return serial_num;
}

/****************************************************************************
* 函数名:  get_applayer_msg_no_ETH
* 说    明: 组织应用层数据——序列号——ETH
* 参    数: uint8 msg_type 待封装的报文类型
*       uint8 msg_sub_type 待封装的报文子类型
* 返回值: uint16 serial_num 应用层数据——序列号——ETH
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no_ETH(uint8 msg_type, uint8 msg_sub_type)
{
    uint16 serial_num = 0U;

    /* 1. 数据处理 */
    switch( msg_type )
    {
        /*1.1 轮循模式  请求方*/
        case ROUND_CIRCLE_TYPE:
            serial_num = get_applayer_msg_no_ETH_circle( msg_sub_type );
            break;

        /*1.2 轮循模式  应答方*/
        case ROUND_CIRCLE_ACK_TYPE:
            serial_num = get_applayer_msg_no_ETH_circle_ack( msg_sub_type );
            break;

        /*1.3 周期模式  有应答模式*/
        case ROUND_PULSE_MODE:
            serial_num = get_applayer_msg_no_ETH_pluse( msg_sub_type );
            break;

        /*1.4 异常防护处理 */
        default:
            MY_Printf("get_applayer_msg_no_ETH msg_type err !\r\n");
            break;
    }
    return serial_num;

}

/****************************************************************************
* 函数名:  get_applayer_msg_no
* 说    明: 组织应用层数据——序列号
* 参    数: uint8 msg_type 待封装的报文类型
*       uint8 msg_sub_type 待封装的报文子类型
*       uint8 send_chl 需要封装的信道(1,2,3)
* 返回值: uint16 serial_num 应用层数据——序列号
* 备    注: 无
 ****************************************************************************/
static uint16 get_applayer_msg_no(uint8 msg_type, uint8 msg_sub_type, uint8 send_chl)
{
    uint16 serial_num = 0U;

    /* 1.参数合法性检查 */
    if( (send_chl != IN_ETH_DEV) && (send_chl != IN_FDCAN1_DEV) && (send_chl != IN_FDCAN2_DEV))
    {
        MY_Printf("get_applayer_msg_no send_chl is err !!!\r\n");
        return serial_num;
    }

    /* 2.数据处理  */
    switch( send_chl )
    {
        /* 2.1.1 内部以太网序列号 */
        case IN_ETH_DEV:
            serial_num = get_applayer_msg_no_ETH(msg_type, msg_sub_type);
            break;

        /* 2.1.2 内部CAN1序列号 */
        case IN_FDCAN1_DEV:
            serial_num = get_applayer_msg_no_CAN1(msg_type, msg_sub_type);
            break;

        /* 2.1.3 内部CAN2序列号 */
        case IN_FDCAN2_DEV:
            serial_num = get_applayer_msg_no_CAN2(msg_type, msg_sub_type);
            break;

        /* 2.2.4 异常错误防护*/
        default:
            MY_Printf("get_applayer_msg_no send_chl is err !!!\r\n");
            break;
    }
    return serial_num;
}

/****************************************************************************
* 函数名:  vcp_app_layer_pakage
* 说    明: 把VCP_APP的数据封装成安全层
* 参    数: uint8 *pApp 封装完成的应用层数据
*       uint8 *pAppData 待封装的应用层负载数据
*       uint16 applayer_datalen 待封装的应用层负载长度
*       uint8 msg_type 待封装的报文类型
*       uint8 msg_sub_type 待封装的报文子类型
*       uint8 send_chl 需要封装的信道(1,2,3)
* 返回值: 无
* 备    注: 无
 ****************************************************************************/
extern void vcp_app_layer_pakage( uint8 *pApp, uint8 *pAppData, uint16 applayer_datalen,
                                  uint8 msg_type, uint8 msg_sub_type, uint8 send_chl)
{
    S_APP_LAYER_HEADER s_app_layer_header ;
    uint16 app_layer_len = 0U;
    uint16 serial_num = 0U;

    /* 1.参数合法性检查 */
    /* 1.1 指针参数合法性检查 */
    if((pApp == NULL) || (pAppData == NULL) )
    {
        MY_Printf("vcp_app_layer_pakage pApp or pAppData is NULL !!!\r\n");
        return ;
    }
    /* 1.2 应用数据长度合法性检查 */
    if((applayer_datalen >= APP_LAYER_PLOADLEN))
    {
        MY_Printf("vcp_app_layer_pakage app_datalen is err !!!\r\n");
        return ;
    }

    /* 1.3 通道号合法性检查 */
    if( (send_chl != IN_ETH_DEV) && (send_chl != IN_FDCAN1_DEV) && (send_chl != IN_FDCAN2_DEV))
    {
        MY_Printf("vcp_app_layer_pakage send_chl is err !!!\r\n");
        return ;
    }

    /* 2.获取序列号 */
    serial_num = get_applayer_msg_no(msg_type, msg_sub_type, send_chl);

    /* 3.组织应用层帧头数据 */
    s_app_layer_header.msg_type = msg_type;
    s_app_layer_header.msg_sub_type = msg_sub_type;
    s_app_layer_header.serial_num = serial_num;

    //MY_Printf("msg_sub_type send_chl_1 is %d !!!\r\n",msg_sub_type);
    /* 4.组织应用层数据 */
    support_memcpy(&pApp[0], &s_app_layer_header, sizeof(s_app_layer_header));
    app_layer_len = sizeof(s_app_layer_header);

    support_memcpy(&pApp[app_layer_len], &pAppData[0], applayer_datalen);
    app_layer_len = sizeof(s_app_layer_header) + applayer_datalen;

}

