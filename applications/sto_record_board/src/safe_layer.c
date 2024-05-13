/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */

#include "safe_layer.h"

#define DBG_TAG "SafeLayer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "crc.h"
#include "type.h"
#include "app_layer.h"
#include "vcp_time_manage.h"
#include "eth_thread.h"

uint8_t ETH0_To_STO_DataBuf[SAFE_LAYER_PLOADLEN];
uint8_t ETH1_To_STO_DataBuf[SAFE_LAYER_PLOADLEN];
static uint32_t safelayer_ETH0_msg_no_u32 = 0U, safelayer_ETH1_msg_no_u32 = 0U;

/****************************************************************************
 * 函数名: rx_safe_layer_check
 * 说明:校核安全层数据
 * 参数: uint8_t *pBuf 安全层数据
 * uint8_t from_chl 来源信道
 * 返回值: 安全层校验合法标识
 ****************************************************************************/
rt_err_t rx_safe_layer_check(S_DATA_HANDLE * data_handle, uint8_t *pBuf, uint8_t from_chl)
{
    sint8_t tmp_adr = 0;
    rt_err_t ret = -RT_ERROR;
    uint32_t recv_crc_u32 = 0U;
    r_safe_layer *pSafe_layer = NULL;
    if (pBuf != NULL)
    {
        pSafe_layer = (r_safe_layer *) &pBuf[0];

        /* 主控插件发送 或 通信1子板发向主控 的数据 */
        if ((pSafe_layer->src_adr == Safe_ZK_I_ADR) || (pSafe_layer->src_adr == Safe_ZK_II_ADR) ||
                        ((pSafe_layer->src_adr == Safe_TX1_I_C_ADR && pSafe_layer->des_adr == Safe_ZK_I_ADR) || (pSafe_layer->src_adr == Safe_TX1_II_C_ADR && pSafe_layer->des_adr == Safe_ZK_II_ADR)))
        {
            /*判断安全层数据长度合法  1480负载最大长度+4CRC32+4CRC32+安全层头*/
            if (pSafe_layer->lenth <= SAFE_LAYER_PLOADLEN + 4 + 4 + sizeof(r_safe_layer))
            {
                recv_crc_u32 = *(uint32_t *) ((uint8_t *) &pBuf[0] + pSafe_layer->lenth - 4 - 4);
                /*校验安全层第一次CRC值*/
                if (recv_crc_u32 == crc32_create(&pBuf[0], pSafe_layer->lenth - 4 - 4, (uint32_t) 0x5A5A5A5A))
                {
                    recv_crc_u32 = *(uint32_t *) ((uint8_t *) &pBuf[0] + pSafe_layer->lenth - 4);
                    /*校验安全层第二次CRC值*/
                    if (recv_crc_u32 == generate_CRC32(&pBuf[0], pSafe_layer->lenth - 4 - 4, (uint32_t) 0x5A5A5A5A))
                    {
                        /* 把来源的通道号暂存到预留 字段 */
                        pSafe_layer->res = from_chl;
                        /*把源地址和目标地址进行交换，方便处理后回传 到VCP平台*/
                        tmp_adr = pSafe_layer->src_adr;
                        pSafe_layer->src_adr = pSafe_layer->des_adr;
                        pSafe_layer->des_adr = tmp_adr;
                        app_layer_check(data_handle, &pBuf[sizeof(r_safe_layer)], pBuf);
                        ret = RT_EOK;
                    }
                    else
                    {
                        LOG_E("safe_layer_check CRC2 err !");
#if 0   //TODO(mingzhao)
                        set_CrcErr_state(from_chl);
#endif
                    }
                }
                else
                {
//                    rt_kprintf("src %x, id %x, %x, %x, %x\r\n", pBuf[0], pBuf[4], pBuf[5], pBuf[6], pBuf[7]);
//                    LOG_HEX("data", 8, pBuf, pSafe_layer->lenth);


                    //目的地址 源地址 6 6
//                    LOG_E("dst %x %x %x %x %x %x", eth_buf[0], eth_buf[1], eth_buf[2], eth_buf[3], eth_buf[4], eth_buf[5]);
//                    LOG_E("src %x %x %x %x %x %x", eth_buf[6], eth_buf[7], eth_buf[8], eth_buf[9], eth_buf[10], eth_buf[11]);

//                    rt_kprintf("safe_layer_check CRC1 err ! src %x, des %x, len %d", pSafe_layer->src_adr, pSafe_layer->des_adr, pSafe_layer->lenth);
//                    LOG_E("safe_layer_check CRC1 err ! src %x, des %x, len %d", pSafe_layer->src_adr, pSafe_layer->des_adr, pSafe_layer->lenth);
//                    LOG_E("safe_layer_check CRC1 err !");
#if 0   //TODO(mingzhao)
                    set_CrcErr_state(from_chl);
#endif
                }
            }
            else
            {
                LOG_E("safe_layer_check len err 0x%x !", pSafe_layer->lenth);
            }
        }
        else
        {
//            LOG_E("safe_layer_check addr err!==>%02x\r\n", pSafe_layer->src_adr);   //TODO(mingzhao)
        }
    }
    else
    {
        LOG_E("safe_layer_check pBuf == NULL err !\r\n");
    }
    return ret;
}

/****************************************************************************
* 函数名: app_add_safelayer_pakage_tx
* 说明:把APP的数据封装成安全层并发到相应的通道
* 参数: uint8_t *pSafe 收到的安全层信息
*      uint8_t *pApp  应用层据
*      uint16_t app_len 应用层数据长度
* 返回值: 无
****************************************************************************/
void app_add_safelayer_pakage_tx(uint8_t *pSafe, uint8_t *pApp, uint16_t app_len)
{
    uint32_t cacu_crc_u32 = 0U;
    r_safe_layer safe_layer;
    r_safe_layer *pRx_safe = NULL;
    r_app_layer *pRx_app = NULL;

    memset(ETH0_To_STO_DataBuf, 0x00, SAFE_LAYER_PLOADLEN);
    memset(ETH1_To_STO_DataBuf, 0x00, SAFE_LAYER_PLOADLEN);

    if ((pSafe != NULL) && (pApp != NULL) && (app_len < SAFE_LAYER_PLOADLEN))
    {
        pRx_safe = (r_safe_layer *) pSafe;
        pRx_app = (r_app_layer *) pApp;

        /*安全层信息*/
        safe_layer.des_adr = pRx_safe->des_adr;/*目标地址 预留此处的目的支持区分I系或II系  目前固定为03*/
        safe_layer.src_adr = Safe_RECORD_ADR;

        /* 时钟同步申请，安全层序列号填0 */
        if ((pRx_app->msg_type == ROUND_CIRCLE_TYPE) && (pRx_app->msg_sub_type == APPLY_TIME_SET))
        {
            safe_layer.serial_num = 0;
            if (pRx_safe->res == ETH_CH_INEX_1)
            {
                safelayer_ETH0_msg_no_u32 = 0;
            }
            else if (pRx_safe->res == ETH_CH_INEX_2)
            {
                safelayer_ETH1_msg_no_u32 = 0;
            }
            else
            {
                LOG_E("clock pRx_safe->res err !\r\n");
                return;
            }
        }
        else
        {
            if (pRx_safe->res == ETH_CH_INEX_1)
            {
                safe_layer.serial_num = safelayer_ETH0_msg_no_u32;/*序列号*/
//                LOG_I("eth1 safe num %d", safe_layer.serial_num);
                safelayer_ETH0_msg_no_u32 = count_msg_no(safelayer_ETH0_msg_no_u32);/*安全层的序列号每发出一帧则增一*/
            }
            else if (pRx_safe->res == ETH_CH_INEX_2)
            {
                safe_layer.serial_num = safelayer_ETH1_msg_no_u32;/*序列号*/
//                LOG_I("eth2 safe num %d", safe_layer.serial_num);
                safelayer_ETH1_msg_no_u32 = count_msg_no(safelayer_ETH1_msg_no_u32);/*安全层的序列号每发出一帧则增一*/
            }
            else
            {
                LOG_E("pRx_safe->res err !\r\n");
                return;
            }
        }

        if ((pRx_app->msg_type == ROUND_CIRCLE_TYPE) || (pRx_app->msg_type == ROUND_CIRCLE_ACK_TYPE))
        {
            if ((pRx_app->msg_sub_type == TIME_SET_LOCAL) || (pRx_app->msg_sub_type == APPLY_TIME_SET))
            {
                safe_layer.sig_pos = 0x05;/*标识位 除时间同步不用时间戳 05不用时间戳 */
            }
        }
        else
        {
            safe_layer.sig_pos = 0x03;/*标识位 其它都要用时间戳 3用时间戳  */
        }

        safe_layer.res = 0x00; /*预留*/
        if (pRx_safe->res == ETH_CH_INEX_1)
        {
            safe_layer.time_print = GetTime_ETH0_diff();/*时间戳*/
        }
        else if (pRx_safe->res == ETH_CH_INEX_2)
        {
            safe_layer.time_print = GetTime_ETH1_diff();/*时间戳*/
        }
        else
        {
            LOG_E("app_add_safelayer_pakage_tx CHL %d err !\r\n", pRx_safe->res);
            return;
        }

        safe_layer.lenth = sizeof(r_safe_layer) + app_len + 4 + 4;

//        LOG_I("msg type %d sub type %d sin %d time %d num %d len %d", pRx_app->msg_type, pRx_app->msg_sub_type, safe_layer.sig_pos, safe_layer.time_print, safe_layer.serial_num, safe_layer.lenth);

        if (pRx_safe->res == ETH_CH_INEX_1)
        {
            memcpy(ETH0_To_STO_DataBuf, &safe_layer, sizeof(r_safe_layer)); //len = 14

            memcpy(&ETH0_To_STO_DataBuf[sizeof(r_safe_layer)], &pApp[0], app_len); //buf[14]

            cacu_crc_u32 = crc32_create(&ETH0_To_STO_DataBuf[0], safe_layer.lenth - 4 - 4, (uint32_t) 0x5A5A5A5A);
            memcpy(&ETH0_To_STO_DataBuf[safe_layer.lenth - 4 - 4], (uint8_t *) &cacu_crc_u32, sizeof(cacu_crc_u32));

            cacu_crc_u32 = generate_CRC32(&ETH0_To_STO_DataBuf[0], safe_layer.lenth - 4 - 4, (uint32_t) 0x5A5A5A5A);
            memcpy(&ETH0_To_STO_DataBuf[safe_layer.lenth - 4], (uint8_t *) &cacu_crc_u32, sizeof(cacu_crc_u32));
        }
        else if (pRx_safe->res == ETH_CH_INEX_2)
        {
            memcpy(ETH1_To_STO_DataBuf, &safe_layer, sizeof(r_safe_layer)); //len = 14

            memcpy(&ETH1_To_STO_DataBuf[sizeof(r_safe_layer)], &pApp[0], app_len); //buf[14]

            cacu_crc_u32 = crc32_create(&ETH1_To_STO_DataBuf[0], safe_layer.lenth - 4 - 4, (uint32_t) 0x5A5A5A5A);
            memcpy(&ETH1_To_STO_DataBuf[safe_layer.lenth - 4 - 4], (uint8_t *) &cacu_crc_u32, sizeof(cacu_crc_u32));

            cacu_crc_u32 = generate_CRC32(&ETH1_To_STO_DataBuf[0], safe_layer.lenth - 4 - 4, (uint32_t) 0x5A5A5A5A);
            memcpy(&ETH1_To_STO_DataBuf[safe_layer.lenth - 4], (uint8_t *) &cacu_crc_u32, sizeof(cacu_crc_u32));
        }
        else
        {
            LOG_E("ch err %d", pRx_safe->res);
            return;
        }

        if ((pRx_app->msg_type == ROUND_CIRCLE_TYPE) || (pRx_app->msg_type == ROUND_CIRCLE_ACK_TYPE)
                || (pRx_app->msg_type == ROUND_PULSE_MODE))
        {
            /*此处借用一下预留字节传送一下通道号*/
            switch (pRx_safe->res)
            {
                case ETH_CH_INEX_1:
                    if(linke_eth_send(ETH_CH_INEX_1, ETH0_To_STO_DataBuf, safe_layer.lenth) != 1)
                    {
                        LOG_E("safe eth1 send error");
                    }
                    break;
                case ETH_CH_INEX_2:
                    if(linke_eth_send(ETH_CH_INEX_2, ETH1_To_STO_DataBuf, safe_layer.lenth) != 1)
                    {
                        LOG_E("safe eth2 send error");
                    }
                    break;
                default:
                    LOG_E("ch err not send %d", pRx_safe->res);
                    break;
            }
        }
        else
        {
            
        }
    }
    else
    {
        LOG_E("app_add_safelayer_pakage_tx pSafe == NULL or pApp == NULL) err !\r\n");
    }
}

