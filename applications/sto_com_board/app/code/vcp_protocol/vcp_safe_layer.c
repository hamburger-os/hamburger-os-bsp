/***************************************************************************
文件名：vcp_safe_layer.c
模    块：主控插件与通信插件通信协议——安全层解析与组包
详    述：
作    者：jiaqx,20231129
***************************************************************************/

#include "vcp_safe_layer.h"
#include "vcp_app_layer.h"
#include "vcp_time_manage.h"
#include "app.h"
#include "vcp_crc.h"

#include "support_gpio.h"
#include "support_libPub.h"
typedef struct
{
    uint32 CAN1;      /* 安全层CAN1序列号 */
    uint32 CAN2;      /* 安全层CAN2序列号 */
    uint32 ETH;       /* 安全层ETH序列号 */
} S_SAFELAYER_MSG_NO_U32;

static S_SAFELAYER_MSG_NO_U32 safelayer_msg_no_u32 = {0,0,0};

/****************************************************************************
* 函数名:  vcp_safe_layer_check
* 说    明: 安全层数据校验
* 参    数: uint8 *pBuf 安全层数据
        uint8 from_chl 系内通道编号(1,2,3)
* 返回值: sint8 安全层校验合法性--结果标识
* 备    注: 仅实现系内以太网数据检查
 ****************************************************************************/
extern sint8 vcp_safe_layer_check(uint8 *pBuf , uint8 from_chl)
{
    sint8 ret = ERROR;
    uint32 recv_crc_u32 = 0U;
    S_SAFE_LAYER_HEADER s_safe_layer_header ;

    /* 1.参数合法性检查*/
    /* 1.1 指针参数合法性检查 */
    if( NULL == pBuf )
    {
        MY_Printf("vcp_safe_layer_check pBuf == NULL err !!!\r\n");
        return ret;
    }
    /* 1.2 系内通道合法性检查 */
    if( (from_chl!=IN_ETH_DEV) && (from_chl!=IN_FDCAN1_DEV) && (from_chl!=IN_FDCAN2_DEV))
    {
        MY_Printf("vcp_safe_layer_check from_chl is  err !!!\r\n");
        return ret;
    }

    /* 2. 解析VCP安全层帧头数据 */
    support_memcpy(&s_safe_layer_header, pBuf, sizeof(S_SAFE_LAYER_HEADER) );

    /* 3. 安全层校验 */
    /* 3.1 源地址校验*/
    if((s_safe_layer_header.src_adr != Safe_ZK_I_ADR) || (s_safe_layer_header.src_adr != Safe_ZK_II_ADR))
    {
        MY_Printf("vcp_safe_layer_check addr err!==>%.2x\r\n",s_safe_layer_header.src_adr);
        return ret;
    }

    /* 3.2 数据长度校验 */
    /* 判断安全层数据长度合法 */
    if(s_safe_layer_header.lenth >= COMM_MAX_PAYLOAD_LEN )
    {
        MY_Printf("vcp_safe_layer_check LEN err 0x%x !\r\n", s_safe_layer_header.lenth);
        return ret;
    }

    /* 3.3 安全层第一次CRC校验 */
    recv_crc_u32 = *(uint32 *)((uint8 *)&pBuf[0] + s_safe_layer_header.lenth - 4U- 4U);
    if(recv_crc_u32 != crc32_create(&pBuf[0], s_safe_layer_header.lenth - 4U- 4U, (uint32)0x5A5A5A5A))
    {

        MY_Printf("vcp_safe_layer_check CRC1 err !\r\n");
        MY_Printf("s_safe_layer_header.len:%d !\r\n",s_safe_layer_header.lenth);
        return ret;
    }

    /* 3.4 安全层第二次CRC校验 */
    recv_crc_u32 = *(uint32 *)((uint8 *)&pBuf[0] + s_safe_layer_header.lenth - 4U);
    if(recv_crc_u32 != generate_CRC32(&pBuf[0], s_safe_layer_header.lenth - 4U - 4U, (uint32)0x5A5A5A5A))
    {
        MY_Printf("vcp_safe_layer_check CRC2 err !\r\n");
        return ret;
    }

    /* 3.5 安全层校验通过 */
    ret = OK;
    return ret;
}


/****************************************************************************
* 函数名: get_vcp_safe_layer_des_adr
* 说    明: 组织安全层数据——目的地址
* 参    数: 无
* 返回值: uint8 安全层数据——目的地址
* 备    注：目前I/II系主控插件地址相同 此处仅返回I系地址
 ****************************************************************************/
static uint8 get_vcp_safe_layer_des_adr( void )
{
    return Safe_ZK_I_ADR;
}

/****************************************************************************
* 函数名: get_vcp_safe_layer_src_adr
* 说    明: 组织安全层数据——源地址
* 参    数: 无
* 返回值: uint8 安全层数据——源地址
* 备    注：目前I/II系通信插件地址相同，此处仅返回I系地址
 ****************************************************************************/
static uint8 get_vcp_safe_layer_src_adr( void )
{
    E_BOARD_ID BoardId = ID_NONE;
    uint8 safelayer_srcadr = 0U;

    /* 1. 获取板卡位置信息 */
    BoardId = support_gpio_getBoardId();

    /* 2.获取板子位置信息 */
    switch ( BoardId )
    {
        case ID_TX1_Load: safelayer_srcadr = Safe_TX1_I_L_ADR;break;
        case ID_TX1_Child:safelayer_srcadr = Safe_TX1_I_C_ADR;break;
        case ID_TX2_Load:safelayer_srcadr = Safe_TX2_I_L_ADR;break;
        case ID_TX2_Child:safelayer_srcadr = Safe_TX2_I_C_ADR;break;
        default:
            MY_Printf("get_vcp_safe_layer_src_adr BoardId is err !!!\r\n");
            break;
    }
    return safelayer_srcadr;
}

/****************************************************************************
* 函数名: get_vcp_safe_layer_sig_pos
* 说    明: 组织安全层数据——标识位
* 参    数: r_app_layer *pApp 应用层数据帧头指针
* 返回值: uint8 安全层数据——标识位
* 备    注：无
 ****************************************************************************/
static uint8 get_vcp_safe_layer_sig_pos( uint8 *pApp )
{
    uint8 sig_pos = 0x03U;
    S_APP_LAYER_HEADER s_app_layer_header = {0};
    /* 1. 参数合法性检查 */
    if( pApp == NULL )
    {
        MY_Printf("get_vcp_safe_layer_sig_pos pApp is NULL !!!\r\n");
        return sig_pos;
    }

    /* 2.数据处理 */
//    MY_Printf("---------------------\r\n");
//    MY_Printf("pApp len:%d !!!\r\n", sizeof(S_APP_LAYER_HEADER));
//    MY_Printf("pApp data:");
//    for(uint16 i=0; i<sizeof(S_APP_LAYER_HEADER); i++)
//    {
//        MY_Printf("%.2x ",pApp[i]);
//    }
//    MY_Printf("\r\n");
//    MY_Printf("---------------------\r\n");
    support_memcpy(&s_app_layer_header, &pApp[0], sizeof(S_APP_LAYER_HEADER));

//    MY_Printf("msg_sub_type send_chl_2 is %.2x !!!\r\n",s_app_layer_header.msg_type);
//
//    MY_Printf("---------------------\r\n");
//    MY_Printf("msg_sub_type send_chl_2 is %.2x !!!\r\n",s_app_layer_header.msg_sub_type);
//    MY_Printf("msg_sub_type send_chl_2 is %.2x !!!\r\n",s_app_layer_header.serial_num);
    /* 2.1 时间同步类 标识符设置为0x05 */
    if((s_app_layer_header.msg_type == ROUND_CIRCLE_TYPE) || (s_app_layer_header.msg_type == ROUND_CIRCLE_ACK_TYPE))
    {
        if((s_app_layer_header.msg_sub_type == TIME_SET_LOCAL) || (s_app_layer_header.msg_sub_type == APPLY_TIME_SET))
        {
            sig_pos = 0x05U;

        }
    }

    /* 2.2 非时间同步类 标识符设置为0x03 */
    else
    {
        sig_pos =0x03U;
    }

    return sig_pos;
}

/****************************************************************************
* 函数名: get_vcp_safe_layer_res
* 说    明: 组织安全层数据——预留位
* 参    数: 无
* 返回值: uint8 安全层数据——预留位
* 备    注：无
 ****************************************************************************/
static uint8 get_vcp_safe_layer_res( void )
{
    return 0x00U;
}

/****************************************************************************
* 函数名: vcp_safe_layer_serial_num_reset
* 说    明: 清除安全层数据——序列号
* 参    数: uint8 send_chl 系内通道编号(1,2,3)
* 返回值: 无
* 备    注：无
 ****************************************************************************/
static void vcp_safe_layer_serial_num_reset( uint8 send_chl )
{
    /* 1.清除序列号  */
    switch(send_chl)
    {
        /* 1.1 内部以太网序列号 */
        case IN_ETH_DEV:
            safelayer_msg_no_u32.ETH = 0U;
            break;

        /* 1.2 内部CAN1序列号 */
        case IN_FDCAN1_DEV:
            safelayer_msg_no_u32.CAN1 = 0U;
            break;

        /* 1.3 内部CAN2序列号 */
        case IN_FDCAN2_DEV:
            safelayer_msg_no_u32.CAN2 = 0U;
            break;

        /* 2.4 异常错误防护*/
        default:
            MY_Printf("vcp_safe_layer_serial_num_reset send_chl is err !\r\n");
            break;
    }
}


/****************************************************************************
* 函数名: get_vcp_safe_layer_serial_num
* 说    明: 组织安全层数据——序列号
* 参    数: r_app_layer *pApp 应用层数据帧头指针
*         uint8 send_chl 系内通道编号(1,2,3)
* 返回值: uint32 安全层数据——序列号
* 备    注：无
 ****************************************************************************/
static uint32 get_vcp_safe_layer_serial_num(uint8 *pApp, uint8 send_chl)
{
    uint32 serial_num = 0U;
    S_APP_LAYER_HEADER s_app_layer_header;
    /* 1.参数合法性检查 */
    if( (send_chl != IN_ETH_DEV) && (send_chl != IN_FDCAN1_DEV) && (send_chl != IN_FDCAN2_DEV))
    {
        MY_Printf("vcp_safe_layer_pakage send_chl is  err !!!\r\n");
        return serial_num;
    }

    /* 2.数据处理 */
    support_memcpy(&s_app_layer_header, (S_APP_LAYER_HEADER *)pApp, sizeof(S_APP_LAYER_HEADER));

    /* 2.1 时钟同步申请类  */
    if((s_app_layer_header.msg_type == ROUND_CIRCLE_TYPE) || (s_app_layer_header.msg_type == ROUND_CIRCLE_ACK_TYPE))
    {
        /* 2.1.1 指定序列号为0 */
        serial_num = 0U;

        /* 2.2.2 清零序列号 */
        /* 屏蔽20231226*/
        //vcp_safe_layer_serial_num_reset( send_chl );

    }

    /* 2.2 其他情况 */
    else
    {
        switch(send_chl)
        {
            /* 2.1.1 内部以太网序列号 */
            case IN_ETH_DEV:
                serial_num = safelayer_msg_no_u32.ETH;
                safelayer_msg_no_u32.ETH = count_msg_no(serial_num);
                break;

            /* 2.1.2 内部CAN1序列号 */
            case IN_FDCAN1_DEV:
                serial_num = safelayer_msg_no_u32.CAN1;
                safelayer_msg_no_u32.CAN1 = count_msg_no(serial_num);
                break;

            /* 2.1.3 内部CAN2序列号 */
            case IN_FDCAN2_DEV:
                serial_num = safelayer_msg_no_u32.CAN2;
                safelayer_msg_no_u32.CAN2 = count_msg_no(serial_num);
                break;

            /* 2.2.4 异常错误防护*/
            default:
                MY_Printf("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
                break;
        }
    }
    return serial_num;
}

/****************************************************************************
* 函数名: get_vcp_safe_layer_time_print
* 说    明: 组织安全层数据——时间戳
* 参    数: uint8 send_chl 系内通道编号(1,2,3)
* 返回值: uint32 安全层数据——时间戳
* 备    注：无
 ****************************************************************************/
static uint32 get_vcp_safe_layer_time_print( uint8 send_chl)
{
    uint32 time_print = 0U;
    /* 1.组织时间戳  */
    switch(send_chl)
    {
        /* 1.1 内部以太网序列号 */
	    case IN_ETH_DEV:
	        time_print = GetTime_ETH_diff();
	        break;

	    /* 1.2 内部CAN1序列号 */
	    case IN_FDCAN1_DEV:
	        time_print = GetTime_CAN1_diff();
	        break;
	 
	    /* 1.3 内部CAN2序列号 */
	    case IN_FDCAN2_DEV:
            time_print = GetTime_CAN2_diff();
            break;
	    break;
	 
	    /* 2.4 异常错误防护*/
	    default:
	        MY_Printf("get_vcp_safe_layer_serial_num send_chl is err !!!\r\n");
	        break;
        }
    return time_print;
}

/****************************************************************************
* 函数名: get_vcp_safe_layer_lenth
* 说    明: 组织安全层数据——安全层长度
* 参    数: uint32 app_len 应用层长度
* 返回值: uint32 安全层长度
* 备    注：无
 ****************************************************************************/
static uint32 get_vcp_safe_layer_lenth(uint32 app_len)
{
    return (uint32)(sizeof(S_SAFE_LAYER_HEADER) + app_len + 4U + 4U);
}

/****************************************************************************
* 函数名: vcp_safe_layer_pakage
* 说    明: 把VCP_APP的数据封装成安全层
* 参    数: uint8 *pSafe 封装完成的安全层数据
*         uint8 *pApp 待封装的应用层数据
*         uint16 app_len 待封装的应员层长度
*         send_chl 需要封装的信道(1,2,3)
* 返回值: 无
 ****************************************************************************/
extern void vcp_safe_layer_pakage(uint8 *pSafe , uint8 *pApp , uint16 app_len, uint8 send_chl)
{
    S_SAFE_LAYER_HEADER s_safe_layer_header= {0};
    uint16 safe_layer_data_len = 0U;
    uint32 cacu_crc_u32 = 0U;
    /* 1.参数合法性检查 */
    if((pSafe == NULL) || (pApp == NULL) || (app_len > APP_LAYER_PLOADLEN))
    {
        MY_Printf("vcp_safe_layer_pakage parameter is illegal !!!\r\n");
        return;
    }

    if( (send_chl != IN_ETH_DEV) && (send_chl != IN_FDCAN1_DEV) && (send_chl != IN_FDCAN2_DEV))
    {
        MY_Printf("vcp_safe_layer_pakage send_chl is  err !!!\r\n");
        return ;
    }

    /* 2.组织安全层帧头数据 */
    /* 2.1 目的地址 */
    s_safe_layer_header.des_adr = get_vcp_safe_layer_des_adr();

    /* 2.2 源地址 */
    s_safe_layer_header.src_adr = get_vcp_safe_layer_src_adr();

    /* 2.3 标识符*/
    s_safe_layer_header.sig_pos = get_vcp_safe_layer_sig_pos( pApp );

    /* 2.4 预留位置 */
    s_safe_layer_header.res = get_vcp_safe_layer_res();

    /* 2.5 序列号 */
    s_safe_layer_header.serial_num = get_vcp_safe_layer_serial_num( pApp, send_chl );

    /* 2.6 时间戳 */
    s_safe_layer_header.time_print = get_vcp_safe_layer_time_print( send_chl );

    /* 2.7 数据长度 */
    s_safe_layer_header.lenth = get_vcp_safe_layer_lenth( app_len );

    /* 3. 组织安全层数据 */
    /* 3.1 帧头数据 */
    support_memcpy(&pSafe[0], &s_safe_layer_header, sizeof(S_SAFE_LAYER_HEADER));
    safe_layer_data_len += sizeof(S_SAFE_LAYER_HEADER);

    /* 3.2 应用层数据 */
    support_memcpy(&pSafe[safe_layer_data_len], &pApp[0], app_len);
    safe_layer_data_len += app_len;

    /* 3.3 帧尾数据 */
    /* 3.3.1 第一层CRC数据 */
    cacu_crc_u32 = crc32_create(&pSafe[0], s_safe_layer_header.lenth - 4U -4U , (uint32)0x5A5A5A5A);
    support_memcpy(&pSafe[safe_layer_data_len] , (uint8 *)&cacu_crc_u32 , sizeof(cacu_crc_u32));
    safe_layer_data_len += 4U;

    /* 3.3.1 第二层CRC数据 */
    cacu_crc_u32 = generate_CRC32(&pSafe[0], s_safe_layer_header.lenth - 4U - 4U, (uint32)0x5A5A5A5A);
    support_memcpy(&pSafe[safe_layer_data_len] , (uint8 *)&cacu_crc_u32 , sizeof(cacu_crc_u32));
    safe_layer_data_len += 4U;

}
