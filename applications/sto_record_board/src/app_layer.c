/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */

#include "app_layer.h"

#define DBG_TAG "AppLayer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "safe_layer.h"
#include "Common.h"
#include "vcp_time_manage.h"
#include "eth_thread.h"
#include "board_info.h"
#include "data_handle.h"

static uint16_t  applayer_timeajust_ETH0_no_u16 = 0U,
                                 applayer_timeajust_ETH1_no_u16 = 0U,
                                 applayer_applytimeajust_ETH0_no_u16 = 0U,
                                 applayer_applytimeajust_ETH1_no_u16 = 0U,
                                 applayer_getpluginfo_ETH0_no_u16 = 0U,
                                 applayer_getpluginfo_ETH1_no_u16 = 0U,
                                 applayer_setchlinfo_ETH0_no_u16 = 0U,
                                 applayer_setchlinfo_ETH1_no_u16 = 0U,
                                 applayer_getchlinfo_ETH0_no_u16 = 0U,
                                 applayer_getchlinfo_ETH1_no_u16 = 0U,
                                 applayer_setrouter_ETH0_no_u16 = 0U,
                                 applayer_setrouter_ETH1_no_u16 = 0U,
                                 applayer_getrouter_ETH0_no_u16 = 0U,
                                 applayer_getrouter_ETH1_no_u16 = 0U,
                                 applayer_exp_ETH0_no_u16 = 0U,
                                 applayer_exp_ETH1_no_u16 = 0U;

uint8_t  InETH0_2_MainCtl_en = 0, InETH1_2_MainCtl_en = 0;
uint8_t  InETH0_diff_flag = 0, InETH1_diff_flag = 0;
uint32_t InETH0_SendDelay_timer = 0, InETH1_SendDelay_timer = 0;

uint32_t g_u32_exp_tx_CmpTime = 0u;


/****************************************************************************
* 函数名: app_clock_adjust
* 说明:与平台校正时钟同步
* 参数:   uint8_t *p_safe_layer 安全层数据
*         r_app_layer *pApp_layer  应用层数据
*         uint8_t *pbuf 应用层业数据
* 返回值: 应用层处理结果标识
 ****************************************************************************/
static void app_clock_adjust(uint8_t *p_safe_layer, r_app_layer *pApp_layer, uint8_t *pBuf)
{
    uint32_t Times = 0;
    r_safe_layer *pRx_safe = NULL;
    Times = *(uint32_t *) pBuf;

    if (p_safe_layer != NULL)
    {
        pRx_safe = (r_safe_layer *) p_safe_layer;

        if (pRx_safe->res == ETH_CH_INEX_1)
        {
            SetTime_ETH0_diff(Times);
            Times = GetTime_ETH0_diff();
            InETH0_diff_flag = 1;
            InETH0_SendDelay_timer = rt_tick_get();

        }
        else if (pRx_safe->res == ETH_CH_INEX_2)
        {
            SetTime_ETH1_diff(Times);
            Times = GetTime_ETH1_diff();
            InETH1_diff_flag = 1;
            InETH1_SendDelay_timer = rt_tick_get();
        }
        else
        {
            LOG_E("app_clock_adjust chl %d err !\r\n", pRx_safe->res);
        }
    }

    /*应用层封装*/
    if (pApp_layer->msg_type == ROUND_CIRCLE_TYPE) //对于轮询报文发起方 填3  应答方填 5
        pApp_layer->msg_type = ROUND_CIRCLE_ACK_TYPE;

    add_applayer_pakage_tx(p_safe_layer, pApp_layer, (uint8_t *) &Times, sizeof(Times));
}

/****************************************************************************
* 函数名: app_clock_applyadjust
* 说明:   处理申请的平台校正时钟同步
* 参数:   uint8_t *p_safe_layer 安全层数据
*         r_app_layer *pApp_layer  应用层数据
*         uint8_t *pbuf 应用层业数据
* 返回值: 应用层处理结果标识
 ****************************************************************************/
static void app_clock_applyadjust(uint8_t *p_safe_layer, r_app_layer *pApp_layer, uint8_t *pBuf)
{
    uint32_t Times = 0;
    r_safe_layer *pRx_safe = NULL;

    Times = *(uint32_t *) pBuf;
//    serial_num = *((uint32_t *) pBuf + 1);

    if (p_safe_layer != NULL)
    {
        pRx_safe = (r_safe_layer *) p_safe_layer;

        if (pRx_safe->res == ETH_CH_INEX_1)
        {
            SetTime_ETH0_diff(Times);
            Times = GetTime_ETH0_diff();
            InETH0_diff_flag = 1;
            InETH0_SendDelay_timer = rt_tick_get();
//            LOG_I("clock eth1 applyadjust %d", InETH0_SendDelay_timer);
        }
        else if (pRx_safe->res == ETH_CH_INEX_2)
        {
            SetTime_ETH1_diff(Times);
            Times = GetTime_ETH1_diff();
            InETH1_diff_flag = 1;
            InETH1_SendDelay_timer = rt_tick_get();
//            LOG_I("clock eth2 applyadjust %d", InETH1_SendDelay_timer);
        }
        else
        {
            LOG_E("app_clock_applyadjust chl %d err !\r\n", pRx_safe->res);
        }
    }
}

/****************************************************************************
* 函数名: channel_canframe_proc
* 说  明: 处理通道数据
* 参数:
* uint8_t des_chl 通道号
* uint8_t *pbuf 应用层业务数据中的通道数据
* uint8_t  data_len 数据长度
* 返回值: 无
 ****************************************************************************/
static void channel_canframe_proc(S_DATA_HANDLE * data_handle, uint8_t des_chl , uint8_t *pbuf , uint16_t  data_len)
{
    if (pbuf != NULL)
    {
        switch (des_chl)
        {
        /* 外部硬CAN */
        case DATA_CHANNEL_TX1CAN1:
        case DATA_CHANNEL_TX1CAN2:
        case DATA_CHANNEL_TX1VMCAN1:
        case DATA_CHANNEL_TX1VMCAN2:
            ETHToCanDataHandleByTX1(data_handle, pbuf, data_len);
            break;
        case DATA_CHANNEL_TX2CAN2:
        case DATA_CHANNEL_TX2VMCAN1:
        case DATA_CHANNEL_TX2VMCAN2:
//            ETHToCanDataHandleByTX2(data_handle, pbuf, data_len);
//            break;
        case DATA_CHANNEL_TX1CAN3:
        case DATA_CHANNEL_TX1CAN4:
        case DATA_CHANNEL_TX2CAN1:
        case DATA_CHANNEL_TX2CAN3:
        case DATA_CHANNEL_TX2CAN4:
        case DATA_CHANNEL_TX1CAN5:
        case DATA_CHANNEL_TX2CAN5:
            ETHToCanDataHandle(data_handle, pbuf, data_len);
            break;
#if 0 //TODO(mingzhao)
            /* 内部虚拟CAN */
        case DATA_CHANNEL_TX1VMCAN1:
        case DATA_CHANNEL_TX1VMCAN2:
        case DATA_CHANNEL_TX2VMCAN1:
        case DATA_CHANNEL_TX2VMCAN2:
            break;

            /* 外部ETH1 */
        case DATA_CHANNEL_TX1ETH1:
            Protocol_STO_2_LKJ15(pbuf, data_len);
            break;
            /* 外部HDLC */
        case DATA_CHANNEL_TX2HDLC:
            Protocol_STO_2_TCMS_Data(pbuf, data_len);
            break;
            /* 外部RS485 */
        case DATA_CHANNEL_TX1RS485a:
        case DATA_CHANNEL_TX1RS485b:
        case DATA_CHANNEL_TX2RS485a:
            Protocol_STO_2_RS485_Data(des_chl, pbuf, data_len);
            break;
            /* 外部ETH3 */
        case DATA_CHANNEL_TX1ETH3:
            Protocol_STO_2_ETH1_PD(pbuf, data_len);
#endif
        default:
            LOG_D("tx_data_export des_chl  = %x err !\r\n", des_chl);
            break;
        }
    }
    else
    {
        LOG_E("tx_data_export pbuf  = NULL err !\r\n");
    }
}

/****************************************************************************
* 函数名: mainctl_2_export
* 说  明: 主控发来的数据发到外部通道
* 参数:  uint8_t *pbuf主控发来的应用层业务数据中的通道数据
         uint8_t chnal_num 通道数据 对应的通道数量
* 返回值: 无
 ****************************************************************************/
static void mainctl_2_export(S_DATA_HANDLE * data_handle, uint8_t *pbuf, uint8_t chl_num)
{
    uint8_t i = 0U;
    uint8_t chnal_index = 0U; /*取帧中通道号临时变量*/
    uint16_t step_pos = 0U; /*通道数据块偏移临时变量*/
    h_exp_chanl *pthis;

    if (pbuf != NULL)
    {
        for (i = 0; i < chl_num; i++)
        {
            chnal_index = pbuf[step_pos];
            /* 判断通道编号 */
            switch (chnal_index)
            {
                case DATA_CHANNEL_TX1CAN1:
                case DATA_CHANNEL_TX1CAN2:
                case DATA_CHANNEL_TX2CAN1:
                case DATA_CHANNEL_TX2CAN2:
                case DATA_CHANNEL_TX1CAN3:
                case DATA_CHANNEL_TX1CAN4:
                case DATA_CHANNEL_TX2CAN3:
                case DATA_CHANNEL_TX2CAN4:
                case DATA_CHANNEL_TX1CAN5:
                case DATA_CHANNEL_TX2CAN5:
                case DATA_CHANNEL_TX1VMCAN1:
                case DATA_CHANNEL_TX1VMCAN2:
                case DATA_CHANNEL_TX2VMCAN1:
                case DATA_CHANNEL_TX2VMCAN2:
//                    LOG_I("mainctl_2_export chnal_index  = %x !" , chnal_index);
                    pthis = (h_exp_chanl *) &pbuf[step_pos];
                    step_pos = step_pos + sizeof(h_exp_chanl);
                    channel_canframe_proc(data_handle, chnal_index, &pbuf[step_pos], pthis->data_len);
                    step_pos = step_pos + pthis->data_len;
                    break;
                case 0x32:
                case 0x33:
                case 0x50:
                case 0x81:
                case 0x82:
                case 0x92:
                    break;
                default:
                    LOG_D("mainctl_2_export chnal_index err = %x !", chnal_index);
                    break;
            }
        }
    }
    else
    {
        LOG_E("mainctl_2_export pbuf  = NULL err !\r\n");
    }
}

/****************************************************************************
 * 函数名: app_layer_check
 * 说明:分析处理应用层数据
 * 参数: uint8_t *pBuf 应用层数据
 *       uint8_t *p_safe_layer 收到的安全层数据
 * 返回值: 应用层处理结果标识
 ****************************************************************************/
rt_err_t app_layer_check(S_DATA_HANDLE * data_handle, uint8_t *pBuf, uint8_t *p_safe_layer)
{
    rt_err_t ret = RT_EOK;
    r_app_layer *pApp_layer = NULL;
    pApp_layer = (r_app_layer *) pBuf;

    if (pBuf != NULL)
    {
//        LOG_I("pApp_layer %d, %d, %d time %d", pApp_layer->msg_sub_type, pApp_layer->msg_type, pApp_layer->serial_num, rt_tick_get());
        /*轮循模式，请求*/
        if (pApp_layer->msg_type == ROUND_CIRCLE_TYPE)
        {
            switch (pApp_layer->msg_sub_type)
            {
                case TIME_SET_LOCAL:/*3 轮循模式时 时钟同步，主控下发时钟通信插件同步 业务数据为4个字节 有应答*/
                {
                    app_clock_adjust(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer)]);
                }
                    break;
                case COMM_PLUG_INFO:/*30 轮循模式时 主控获取插件信息，周期+平态工作状态 有应答*/
                {
                    get_comm_plug_info(p_safe_layer, pApp_layer); //TODO(mingzhao) plug info
                }
                    break;
                case SET_CHANNL_INFO:/*23轮循模式时 设置通道配置信息，即为配置信息 通道数目+通道1参数+通道2参数+N参数 有应答 2字节OK或ERR*/
                {
    //                /*把通道配置信息设置到本地*/       //TODO(mingzhao) SET_CHANNL_INFO
    //                set_exp_chanl_refer(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer) + 1],
    //                pBuf[sizeof(r_app_layer)]);
    //                /*把通道配置信息存储到本地*/
    //                save_exp_chanl_refer(&pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);
    //                LOG_I("SET_CHANNL_INFO NO");  //TODO(mingzhao )
                }
                    break;
                case GET_CHANNL_INFO:/*24轮循模式时 查询通道配置信息，通道数目+通道号+通道号+N 有应答 通道数目+通道1参数+通道2参数+N参数*/
                {
    //                get_exp_chanl_refer(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);       //TODO(mingzhao) GET_CHANNL_INFO
    //                LOG_I("SET_CHANNL_INFO NO");  //TODO(mingzhao )

                }
                    break;
                case SET_CONTIUE_INFO:/*27轮循模式时 设置数据转发路由表，路由表数目1字节+路由表1(4字节)+N 有应答 2字节OK或ERR*/
                {
    //                app_chl_router_save(p_safe_layer ,pApp_layer , &pBuf[sizeof(r_app_layer)+1] , pBuf[sizeof(r_app_layer)] );
                }
                    break;
                case GET_CONTIUE_INFO: /*29轮循模式时 查询数据转发路由表，2字节保留 有应答  路由表数目1字节+路由表1(4字节)+N*/
                {
    //                app_chl_router_get(p_safe_layer , pApp_layer);
                }
                    break;
                case APPLY_TIME_SET:
                    break;
                default:
                    ret = -RT_ERROR;
                    LOG_E("app_layer_check ROUND_CIRCLE_TYPE msg_sub_type err %d !", pApp_layer->msg_sub_type);
                    break;
            }
        }
        /*轮循模式，应答*/
        else if (pApp_layer->msg_type == ROUND_CIRCLE_ACK_TYPE)
        {
            switch (pApp_layer->msg_sub_type)
            {
                case APPLY_TIME_SET:/*6 轮循模式时 时钟同步申请，主控下发申请的时钟通信插件同步 业务数据为4个字节 无应答*/
                {
                    app_clock_applyadjust(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer)]);
                }
                    break;

                case COMM_PLUG_INFO:/* 轮循模式时 主控获取插件信息 */
                {
    //                    LOG_I("ROUND_CIRCLE_ACK_TYPE COMM_PLUG_INFO src %d dst %d " , pSafe_layer->des_adr , pSafe_layer->src_adr);
                    break;
                }
                case TIME_SET_LOCAL:
                    break;
                default:
                    ret = -RT_ERROR;
                    LOG_E("app_layer_check ROUND_CIRCLE_ACK_TYPE msg_sub_type err %d !", pApp_layer->msg_sub_type);
                    break;
            }
        }
        /*周期模式*/
        else if (pApp_layer->msg_type == ROUND_PULSE_MODE)
        {
            switch (pApp_layer->msg_sub_type)
            {
                case IAP_PAKAGE:/* 5周期模式时 IAP数据包  业务数据为IAP数据包  无应答*/
                {

                }
                    break;
                case RX_MAINCTLDATA_EXP: /*33周期模式时 收到主控数据发送到外部通道  通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N  无应答*/
                {
                    mainctl_2_export(data_handle, &pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);
                }
                    break;
                case RX_EXPDATA_MAINCTL: /* 34 周期模式时 收到外部数据发送给主控     通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N 无应答*/
                {
                    /* 解析通信插件转发的数据 */
                    mainctl_2_export(data_handle, &pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);
                    break;
                }
                    break;
                default:
                    ret = -RT_ERROR;
                    LOG_E("app_layer_check TIMER_PULSE_TYPE msg_sub_type err !\r\n");
                    break;
            }
        }
        else
        {
            ret = -RT_ERROR;
//            LOG_E("app_layer_check msg_type err !\r\n"); //TODO(mingzhao)
        }
    }
    else
    {
        ret = -RT_ERROR;
        LOG_E("app_layer_check pBuf == NULL err !\r\n");
    }
    return ret;
}


/****************************************************************************
 * 函数名: get_applayer_pakage_tx_flag
 * 说明:       获取发送数据标志，时钟同步之前，不反馈除时钟同步之外的命令的应答
 * 参数:   uint8_t *pSafe 安全层数据
 *         r_app_layer *pApp  应用层数据
 * 返回值: 无
 ****************************************************************************/
static uint8_t get_applayer_pakage_tx_flag(uint8_t *pSafe, r_app_layer *pApp)
{
    r_app_layer app_layer;
    r_safe_layer *pRx_safe = NULL;
//    uint8_t Buf[APP_LAYER_PLOADLEN + 4];
    uint8_t send_flag = 0;

    pRx_safe = (r_safe_layer *) pSafe;
    app_layer.msg_type = pApp->msg_type;
    app_layer.msg_sub_type = pApp->msg_sub_type;

    /* 时钟同步命令允许应答 */
    if ((app_layer.msg_type == ROUND_CIRCLE_TYPE) || (app_layer.msg_type == ROUND_CIRCLE_ACK_TYPE))
    {
//        LOG_I("app_layer.msg_type %d app_layer.msg_sub_type %d", app_layer.msg_type, app_layer.msg_sub_type);
        if ((app_layer.msg_sub_type == TIME_SET_LOCAL) || (app_layer.msg_sub_type == APPLY_TIME_SET)
                || (app_layer.msg_sub_type == COMM_PLUG_INFO))
            send_flag = 1;
    }
    /* 其他命令，在收到各自通道时钟同步之前，不允许应答 */
    else
    {
        switch (pRx_safe->res)
        {
            case ETH_CH_INEX_1:
                if (InETH0_2_MainCtl_en)
                {
                    send_flag = 1;
                }
                break;
            case ETH_CH_INEX_2:
                if (InETH1_2_MainCtl_en)
                {
                    send_flag = 1;
                }
                break;
            default:
                break;
        }
    }
    return send_flag;
}

/****************************************************************************
 * 函数名: add_applayer_pakage_tx
* 说明:封装APP 层
* 参数:   uint8_t *pSafe 安全层数据
*         r_app_layer *pApp  应用层数据
*         uint8_t *pbuf 应用层业数据
*         uint8_t app_datalen  应用层业数据长度
* 返回值: 无
 ****************************************************************************/
void add_applayer_pakage_tx(uint8_t *pSafe, r_app_layer *pApp, uint8_t *pbuf, uint16_t app_datalen)
{
    r_app_layer app_layer;
    r_safe_layer *pRx_safe = NULL;
    uint8_t Buf[APP_LAYER_PLOADLEN + 4];
    uint8_t send_flag = 0; /* 时钟同步之前，不反馈除时钟同步之外的命令的应答 */

    if ((pApp != NULL) && (pSafe != NULL) && (app_datalen < APP_LAYER_PLOADLEN))
    {
        pRx_safe = (r_safe_layer *) pSafe;
        app_layer.msg_type = pApp->msg_type;
        app_layer.msg_sub_type = pApp->msg_sub_type;

        send_flag = get_applayer_pakage_tx_flag(pSafe, pApp);

        if (send_flag)
        {
            /*轮循模式，请求*/
            if (app_layer.msg_type == ROUND_CIRCLE_TYPE)
            {
//                LOG_I("ROUND_CIRCLE_TYPE msg_sub_type, res %d", app_layer.msg_sub_type, pRx_safe->res);
                switch (app_layer.msg_sub_type)
                {
                case APPLY_TIME_SET:/*6 轮询模式，申请时钟同步*/
                {
//                    LOG_I("zk app no %d", pApp->serial_num);
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_applytimeajust_ETH0_no_u16;/*序列号*/
                        applayer_applytimeajust_ETH0_no_u16 = count_msg_no16(applayer_applytimeajust_ETH0_no_u16);
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_applytimeajust_ETH1_no_u16;/*序列号*/
                        applayer_applytimeajust_ETH1_no_u16 = count_msg_no16(applayer_applytimeajust_ETH1_no_u16);
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                default:
                    LOG_E("ROUND_CIRCLE_TYPE app_layer_ser_num err !\r\n");
                    break;
                }
            }
            /*轮循模式，应答*/
            else if (app_layer.msg_type == ROUND_CIRCLE_ACK_TYPE)
            {
                switch (app_layer.msg_sub_type)
                {
                case TIME_SET_LOCAL:/*3 轮循模式时 时钟同步*/
                {
//                    LOG_I("2-zk app no %d", pApp->serial_num);
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        applayer_timeajust_ETH0_no_u16 = pApp->serial_num;
                        app_layer.serial_num = applayer_timeajust_ETH0_no_u16;/*序列号*/
                        applayer_timeajust_ETH0_no_u16 = count_msg_no16(applayer_timeajust_ETH0_no_u16);
//                        LOG_I("TIME_SET_LOCAL eth1 sub_type %d %d", app_layer.serial_num, applayer_timeajust_ETH0_no_u16);
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        applayer_timeajust_ETH1_no_u16 = pApp->serial_num;
                        app_layer.serial_num = applayer_timeajust_ETH1_no_u16;/*序列号*/
                        applayer_timeajust_ETH1_no_u16 = count_msg_no16(applayer_timeajust_ETH1_no_u16);
//                        LOG_I("TIME_SET_LOCAL eth2 sub_type %d %d", app_layer.serial_num, applayer_timeajust_ETH1_no_u16);
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                case COMM_PLUG_INFO:/*30 轮循模式时*/
                {
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_getpluginfo_ETH0_no_u16;/*序列号*/
                        applayer_getpluginfo_ETH0_no_u16 = count_msg_no16(applayer_getpluginfo_ETH0_no_u16);/* 应用层的序列号每发出一帧则增一 */
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_getpluginfo_ETH1_no_u16;/*序列号*/
                        applayer_getpluginfo_ETH1_no_u16 = count_msg_no16(applayer_getpluginfo_ETH1_no_u16);/* 应用层的序列号每发出一帧则增一 */
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                case SET_CHANNL_INFO:/*23轮循模式时 设置通道配置信息*/
                {
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_setchlinfo_ETH0_no_u16;/*序列号*/
                        applayer_setchlinfo_ETH0_no_u16 = count_msg_no16(applayer_setchlinfo_ETH0_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_setchlinfo_ETH1_no_u16;/*序列号*/
                        applayer_setchlinfo_ETH1_no_u16 = count_msg_no16(applayer_setchlinfo_ETH1_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                case GET_CHANNL_INFO:/*24轮循模式时 查询通道配置信息*/
                {
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_getchlinfo_ETH0_no_u16;/*序列号*/
                        applayer_getchlinfo_ETH0_no_u16 = count_msg_no16(applayer_getchlinfo_ETH0_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_getchlinfo_ETH1_no_u16;/*序列号*/
                        applayer_getchlinfo_ETH1_no_u16 = count_msg_no16(applayer_getchlinfo_ETH1_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                case SET_CONTIUE_INFO:/*27轮循模式时 设置数据转发路由表*/
                {
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_setrouter_ETH0_no_u16;/*序列号*/
                        applayer_setrouter_ETH0_no_u16 = count_msg_no16(applayer_setrouter_ETH0_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_setrouter_ETH1_no_u16;/*序列号*/
                        applayer_setrouter_ETH1_no_u16 = count_msg_no16(applayer_setrouter_ETH1_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                case GET_CONTIUE_INFO: /*29轮循模式时 查询数据转发路由表*/
                {
                    if (pRx_safe->res == ETH_CH_INEX_1)
                    {
                        app_layer.serial_num = applayer_getrouter_ETH0_no_u16;/*序列号*/
                        applayer_getrouter_ETH0_no_u16 = count_msg_no16(applayer_getrouter_ETH0_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else if (pRx_safe->res == ETH_CH_INEX_2)
                    {
                        app_layer.serial_num = applayer_getrouter_ETH1_no_u16;/*序列号*/
                        applayer_getrouter_ETH1_no_u16 = count_msg_no16(applayer_getrouter_ETH1_no_u16);/*应用层的序列号每发出一帧则增一*/
                    }
                    else
                    {
                        ;
                    }
                }
                    break;
                default:
                    LOG_E("ROUND_CIRCLE_ACK_TYPE app_layer_ser_num err !\r\n");
                    break;
                }
            }
            /*周期模式*/
            else if (app_layer.msg_type == ROUND_PULSE_MODE)
            {
//                LOG_I("ROUND_PULSE_MODE app_layer.msg_sub_type%d, res %d", app_layer.msg_sub_type, pRx_safe->res);
                switch (app_layer.msg_sub_type)
                {
                    case IAP_PAKAGE:/* 5周期模式时 IAP数据包  业务数据为IAP数据包*/
                    {

                    }
                    break;
                    case RX_EXPDATA_MAINCTL: /*34周期模式时*/
                    {
                        if (pRx_safe->res == ETH_CH_INEX_1)
                        {
                            app_layer.serial_num = applayer_exp_ETH0_no_u16;/*序列号*/
//                            LOG_I("eth1 app num %d", app_layer.serial_num);
                            applayer_exp_ETH0_no_u16 = count_msg_no16(applayer_exp_ETH0_no_u16);/*应用层的序列号每发出一帧则增一*/
                        }
                        else if (pRx_safe->res == ETH_CH_INEX_2)
                        {
                            app_layer.serial_num = applayer_exp_ETH1_no_u16;/*序列号*/
//                            LOG_I("eth2 app num %d", app_layer.serial_num);
                            applayer_exp_ETH1_no_u16 = count_msg_no16(applayer_exp_ETH1_no_u16);/*应用层的序列号每发出一帧则增一*/
                        }
                        else
                        {
                            ;
                        }
                    }
                        break;
                    default:
                        LOG_E("ROUND_PULSE_MODE app_layer_ser_num err !\r\n");
                        break;
                }
            }
            else
            {
                LOG_E("app_layer.msg_type app_layer_ser_num err !\r\n");
            }
            
            /*应用层头*/
            memcpy(&Buf[0], (uint8_t *) &app_layer, sizeof(r_app_layer));
            /*应用层数据域*/
            memcpy(&Buf[sizeof(r_app_layer)], &pbuf[0], app_datalen);
            /*调安全层封装并发送走*/
            app_add_safelayer_pakage_tx(pSafe, Buf, sizeof(r_app_layer) + app_datalen);
        }
        else
        {

        }
    }
    else
    {
        LOG_E("add_applayer_pakage_tx  pApp == NULL err !\r\n");
    }
}

/*******************************************************************************************
 **  ApplyDiffToSto    向STO发送时钟同步请求
 **  输入参数：  chl  --  发送通道
 **  输出参数：
 *******************************************************************************************/
void ApplyDiffToSto(uint8_t chl)
{
    uint8_t pbuf[4] = { 0, 0, 0, 0 };
    r_app_layer pApp;
    r_safe_layer safe_layer;

    /* 轮询模式，请求帧 */
    pApp.msg_type = ROUND_CIRCLE_TYPE;
    /*外部数据发往内部数据*/
    pApp.msg_sub_type = APPLY_TIME_SET;

    safe_layer.src_adr = Safe_RECORD_ADR;
    /* 主控地址 */
    if(chl == ETH_CH_INEX_1)
    {
        safe_layer.des_adr = Safe_ZK_I_ADR;
    }
    else if(chl == ETH_CH_INEX_2)
    {
        safe_layer.des_adr = Safe_ZK_II_ADR;
    }
    else
    {
        LOG_E("ApplyDiffToSto chl err %d", chl);
    }

    safe_layer.res = chl;

    add_applayer_pakage_tx((uint8_t *) &safe_layer, &pApp, pbuf, 4);
}

