/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-29     zm       the first version
 */
#include "send_data_by_eth.h"

#define DBG_TAG "send2zk"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "data_handle.h"
#include "app_layer.h"
#include "safe_layer.h"
#include "Common.h"
#include "eth_thread.h"

static r_exp_chanl VmCAN_Exp_Rx;
static t_exp_I_II_eth sto_ETH;
uint32_t VM_CAN_RxCnt = 0U;

/****************************************************************************
* 函数名: app_chl_router_init
* 说  明: 初始化外部通道独立缓存数据
* 参数: 无
* 返回值: 无
 ****************************************************************************/
static void exp_port_rxbuf_init(void)
{
    memset((void *)&VmCAN_Exp_Rx , 0x00 , sizeof(r_exp_chanl));
}

static void tx_data_pack_mainctl(uint8_t target_chl,uint8_t *pbuf , uint16_t  data_len)
{
    r_app_layer pApp;
    r_safe_layer safe_layer;
    /*周期触发*/
    pApp.msg_type = ROUND_PULSE_MODE;
    /*外部数据发往内部数据*/
    pApp.msg_sub_type = RX_EXPDATA_MAINCTL;

    safe_layer.src_adr = Safe_RECORD_ADR;
    /* 主控地址 */
    if(target_chl == ETH_CH_INEX_1)
    {
        safe_layer.des_adr = Safe_ZK_I_ADR;
    }
    else if(target_chl == ETH_CH_INEX_2)
    {
        safe_layer.des_adr = Safe_ZK_II_ADR;
    }
    else
    {
        LOG_E("target ch err %d", target_chl);
        return;
    }

    if (pbuf != NULL)
    {
        if (target_chl == ETH_CH_INEX_1 || target_chl == ETH_CH_INEX_2)
        {
            safe_layer.res = target_chl;

            add_applayer_pakage_tx((uint8_t *) &safe_layer, &pApp, pbuf, data_len);
        }
        else
        {
            LOG_E("tx_data_pack_mainctl target_chl  = %x err !\r\n", target_chl);
        }
    }
    else
    {
        LOG_E("tx_data_pack_mainctl *pbuf = NULL err !\r\n");
    }
}

/****************************************************************************
* 函数名:  pack_save_loal_buf_tx_mainctl
* 说  明:  5毫秒内组包 发送出去；把每个外部通信通道独立缓存外部通道数据组成组包格式，发送至系内以太网。
* 参  数: 无
* 返回值: 无
 ****************************************************************************/
void pack_save_loal_buf_tx_mainctl(void)
{
    /*统计通道数量 I系  II系 */
    uint8_t ch_count = 0;
    /*统计送至 I系的总字节数  */
    sto_ETH.data_len = 1; /*预留第一个字节为 存放组包的通道数量 */

    if (VmCAN_Exp_Rx.data_len > 0)
    {
        /* 拷贝要发送数据 */
        /* 7+exp_1eth.data_len 通道格式的头部为7个字节+数据域长度 */
        memcpy(&sto_ETH.buf[sto_ETH.data_len], &VmCAN_Exp_Rx, VmCAN_Exp_Rx.data_len + 7U);
        sto_ETH.data_len += VmCAN_Exp_Rx.data_len + 7U;
        ch_count++;
    }

    /* 发送到STO系以太网 */
    if ((sto_ETH.data_len > 1U) && (sto_ETH.data_len <= APP_LAYER_PLOADLEN))
    {
        /*通道数量 放帧的前位*/
        sto_ETH.buf[0] = ch_count;
        /*把通道组合后一大包发送至系内以太网I*/
        tx_data_pack_mainctl(ETH_CH_INEX_1, (uint8_t *) sto_ETH.buf, sto_ETH.data_len);
        tx_data_pack_mainctl(ETH_CH_INEX_2, (uint8_t *) sto_ETH.buf, sto_ETH.data_len);
    }
    else
    {
        LOG_E("tx_data_pack_mainctl pack_save_loal_buf_tx_mainctl Len err !sto_ETH.data_len:%d.\r\n",
                sto_ETH.data_len);
    }

    /*清空接收暂存每一个通道的缓存和总统计*/
    exp_port_rxbuf_init();
}

/****************************************************************************
* 函数名:  long_pakage_tx_mainctl
* 说  明:  接收到单包大于缓存的时侯处理机制  原则上不能出现此情况 只送上去有效缓存数据，超出部分舍去
* 参数:     s_comm_node *p_s_node 接收外部通信通道数据块
* 返回值: 无
 ****************************************************************************/
void  long_pakage_tx_mainctl(s_comm_node *p_s_node)
{
    r_exp_chanl pThis;
    t_exp_I_II_eth LI_ETH;
    pThis.channl_index = p_s_node->dev;
    pThis.time_print = p_s_node->tick;
    pThis.data_len =  EXPORT_DATA_PLOADLEN - 8;/*8 = 1字节 通道数量 + 7个字节帧头*/

    /*通道数量 放帧的前位*/
    sto_ETH.buf[0] = 1;
    LI_ETH.data_len = 1;
    /*7 通道格式的头部为7个字节*/
    memcpy(&LI_ETH.buf[LI_ETH.data_len], (uint8_t *)&pThis ,7U);
    LI_ETH.data_len = LI_ETH.data_len + 7U;
    /*DATA域*/
    memcpy(&LI_ETH.buf[LI_ETH.data_len], p_s_node->buf , pThis.data_len);
    LI_ETH.data_len = LI_ETH.data_len + pThis.data_len;

    tx_data_pack_mainctl(ETH_CH_INEX_1,(uint8_t *)LI_ETH.buf , LI_ETH.data_len);
    tx_data_pack_mainctl(ETH_CH_INEX_2,(uint8_t *)LI_ETH.buf , LI_ETH.data_len);
}

/*******************************************************************************************
 **  Check_exp_2_mainctl_SendFlag    检查发送标志
 **  输入参数：无
 **  输出参数：
 *******************************************************************************************/
void Check_exp_2_mainctl_SendFlag(void)
{
    if ((InETH0_diff_flag == 1) && (Common_BeTimeOutMN(&InETH0_SendDelay_timer, 1000)))
    {
        InETH0_2_MainCtl_en = 1;
    }

    if ((InETH1_diff_flag == 1) && (Common_BeTimeOutMN(&InETH1_SendDelay_timer, 1000)))
    {
        InETH1_2_MainCtl_en = 1;
    }
}

/****************************************************************************
* 函数名:  export_2_mainctl
* 说  明:  外部来的数据接收按 协议组好帧暂存至独立通道缓存
* 参数:    s_comm_node *p_s_node 接收外部通信通道数据块
* 返回值: 无
 ****************************************************************************/

void export_2_mainctl( s_comm_node *p_s_node)
{
    S_APP_INETH_PACK *p_can_PACK;

    if(InETH0_2_MainCtl_en != 1 && InETH1_2_MainCtl_en != 1 ) /*20220825 和主控同步后再添加通道数据 */
    {
        return;
    }
    if(p_s_node != NULL)
    {
        /*总的统计合法于一包数据的长度*/
        if( p_s_node->len < LIMIT_EXPORT_DATA_PLOADLEN)
        {
            switch (p_s_node->dev)
            {
                /* 向主控发数据 */
                case DATA_CHANNEL_TX2ZK:
                    VmCAN_Exp_Rx.channl_index = p_s_node->dev;

                    p_can_PACK = (S_APP_INETH_PACK*) VmCAN_Exp_Rx.buf;
                    VM_CAN_RxCnt++;
                    p_can_PACK->frameAllNum = VM_CAN_RxCnt;
                    /* 计算总帧数和总长度 */
                    p_can_PACK->frameNum++; /* 总帧数 */
                    memcpy(p_can_PACK->data + p_can_PACK->datalen, p_s_node->buf, p_s_node->len);
                    p_can_PACK->datalen += p_s_node->len;
                    VmCAN_Exp_Rx.data_len = p_can_PACK->datalen + 7;

                    VmCAN_Exp_Rx.time_print = rt_tick_get();
                    pack_save_loal_buf_tx_mainctl();

                    break;
                default:
                    LOG_E("tx_data_export des_chl  = %x err", p_s_node->dev);
                    break;
            }
        }
    }
    else
    {
        /*printf("export_2_mainctl app_chl_router_find p_s_node= %x pThis = %x err !\r\n" , p_s_node , pThis);*/
    }
}

/*******************************************************************************************
**  Protocol_ExpCAN_2_STO    CAN协议转STO以太网协议
**  输入参数：   pbuf            --      转发数据
**                          data_len    --      转发数据长度
**  输出参数：
*******************************************************************************************/
void Protocol_ExpCAN_2_STO(S_ETH_CAN_BUF *pbuf, uint16_t  data_len)
{
    s_comm_node comm_node;

    /* 1.将CAN信息帧转成网络信息帧 */
    comm_node.dev = pbuf->chl;
    memcpy(comm_node.buf, &pbuf->frame_data, data_len);
    comm_node.len = data_len;

    /* 2.网络信息帧处理 */
    export_2_mainctl(&comm_node);
}

void linklayer_sendFrame( uint8_t chl, uint8_t pri, uint8_t no, uint8_t *pdata, uint8_t dataLen )
{
    S_ETH_CAN_BUF eth_canBuf;
    S_ETH_CAN_FRAME canFrame;
    uint16_t len = 0U;

    /* 1.参数合法性检查 */
    if(( NULL == pdata ) || ( dataLen > 64U ))
    {
        return;
    }

    /* 1.初始化 */
    memset(( uint8_t *)&eth_canBuf, 0U, sizeof( S_ETH_CAN_BUF ));
    memset(( uint8_t *)&canFrame, 0U, sizeof( S_ETH_CAN_FRAME ));

    /* 2.组织CAN帧 */
    canFrame.ID = (( uint32_t )pri << 3U )+ no;
    canFrame.len = dataLen;

    memcpy( &canFrame.Data[0U], pdata, dataLen );

    /* 3.组织ETH_CAN帧 */
    eth_canBuf.chl = chl;
    eth_canBuf.FDFormat = 0U;
    len = canFrame.len + 5U;

    memcpy(&eth_canBuf.frame_data, &canFrame, len );

    /* 3.协议转换并转发数据到STO ETH */
    Protocol_ExpCAN_2_STO( &eth_canBuf, len );
}
