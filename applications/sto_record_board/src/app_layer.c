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

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "AppLayer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/****************************************************************************
* 函数名: tx_data_export
* 说  明: 把业务数据内的 通道数据发送至目的通道(外部道道 )
* 参数:  uint8_t *pbuf主控发来的应用层业务数据中的通道数据中转发数据
         uint8_t  data_len 转发数据长度
                 uint8_t des_chl 需要转发的通道号
* 返回值: 无
 ****************************************************************************/
static void tx_data_to_export(S_DATA_HANDLE * data_handle, uint8_t des_chl , uint8_t *pbuf , uint16_t  data_len)
{
    if (pbuf != NULL)
    {
        //printf("tx_data_to_export---des_chl:%d!\r\n", des_chl);
        switch (des_chl)
        {
//           case EXP_FDCAN_3_DEV:
//           case EXP_FDCAN_4_DEV:
//           case EXP_FDCAN_5_DEV:
//        tx_data_to_exp_can(des_chl,pbuf,data_len);
//           break;
//           case EXP_KSZ8851_I_DEV:
//             Tx_Data_to_exp_1net(pbuf,data_len);
//           break;
//           case EXP_KSZ8851_II_DEV:
//             Tx_Data_to_exp_2net(pbuf,data_len);
//           break;
//           case RS422_DEV:
//            tx_data_to_rs422_I(pbuf,data_len);
//           break;
//           case HDLC_DEV:
//            tx_data_to_rs422_II(pbuf,data_len);
//           break;
//           case MVB_DEV:
//            tx_data_to_rs422_II(pbuf,data_len);
//           break;
        /* 外部硬CAN */
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
//            Protocol_STO_2_ExpCAN(des_chl, pbuf, data_len);
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
            LOG_I("tx_data_export des_chl  = %x err !\r\n", des_chl);
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
            //printf("mainctl_2_export chnal_index  = %x !\r\n" , chnal_index);
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
                    pthis = (h_exp_chanl *) &pbuf[step_pos];
                    step_pos = step_pos + sizeof(h_exp_chanl);
                    tx_data_to_export(data_handle, chnal_index, &pbuf[step_pos], pthis->data_len);
                    step_pos = step_pos + pthis->data_len;
                    break;
                default:
                    LOG_W("mainctl_2_export chnal_index  = %x err !\r\n", chnal_index);
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
#if 0 //TODO(mingzhao)
        //printf("app_layer_check  msg_sub_type %d-%d!\r\n",pApp_layer->msg_type,pApp_layer->msg_sub_type);
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
                get_comm_plug_info(p_safe_layer, pApp_layer);
            }
                break;
            case SET_CHANNL_INFO:/*23轮循模式时 设置通道配置信息，即为配置信息 通道数目+通道1参数+通道2参数+N参数 有应答 2字节OK或ERR*/
            {
                /*把通道配置信息设置到本地*/
                set_exp_chanl_refer(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer) + 1],
                        pBuf[sizeof(r_app_layer)]);
                /*把通道配置信息存储到本地*/
                save_exp_chanl_refer(&pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);

            }
                break;
            case GET_CHANNL_INFO:/*24轮循模式时 查询通道配置信息，通道数目+通道号+通道号+N 有应答 通道数目+通道1参数+通道2参数+N参数*/
            {
                get_exp_chanl_refer(p_safe_layer, pApp_layer, &pBuf[sizeof(r_app_layer) + 1],
                        pBuf[sizeof(r_app_layer)]);

            }
                break;
            case SET_CONTIUE_INFO:/*27轮循模式时 设置数据转发路由表，路由表数目1字节+路由表1(4字节)+N 有应答 2字节OK或ERR*/
            {
//                      app_chl_router_save(p_safe_layer ,pApp_layer , &pBuf[sizeof(r_app_layer)+1] , pBuf[sizeof(r_app_layer)] );
            }
                break;
            case GET_CONTIUE_INFO: /*29轮循模式时 查询数据转发路由表，2字节保留 有应答  路由表数目1字节+路由表1(4字节)+N*/
            {
//                      app_chl_router_get(p_safe_layer , pApp_layer);
            }
                break;
            default:
                ret = ER;
                printf("app_layer_check ROUND_CIRCLE_TYPE msg_sub_type err !\r\n");
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
            default:
                ret = ER;
                printf("app_layer_check ROUND_CIRCLE_ACK_TYPE msg_sub_type err !\r\n");
                break;
            }
        }
        /*周期模式*/
        else
#endif

        if (pApp_layer->msg_type == ROUND_PULSE_MODE)
        {
//            LOG_I("msg_sub_type %d!\r\n", pApp_layer->msg_sub_type);
            switch (pApp_layer->msg_sub_type)
            {
#if 0 //TODO(mingzhao)
                case IAP_PAKAGE:/* 5周期模式时 IAP数据包  业务数据为IAP数据包  无应答*/
                {
                    ;
                }
                break;
#endif
                case RX_MAINCTLDATA_EXP: /*33周期模式时 收到主控数据发送到外部通道  通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N  无应答*/
                {
                    mainctl_2_export(data_handle, &pBuf[sizeof(r_app_layer) + 1], pBuf[sizeof(r_app_layer)]);
                }
                break;
                default:
                    ret = -RT_ERROR;
                    //LOG_E("app_layer_check TIMER_PULSE_TYPE msg_sub_type err !\r\n");
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

