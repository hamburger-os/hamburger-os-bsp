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

#include <rtthread.h>
#include <rtdevice.h>

#include "crc.h"
#include "type.h"
#include "app_layer.h"

#define DBG_TAG "SafeLayer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/****************************************************************************
 * 函数名: rx_safe_layer_check
 * 说明:校核安全层数据
 * 参数: uint8_t *pBuf 安全层数据
 uint8_t from_chl 来源信道
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

        if ((pSafe_layer->src_adr == Safe_ZK_I_ADR) || (pSafe_layer->src_adr == Safe_ZK_II_ADR)) /* 主控插件发送的数据 */
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
//                        if(pBuf[0] == 0x47 && pBuf[1] == 3)
//                        {
//                            LOG_I("rcv 0x47 ok");
//                        }
//                        else if(pBuf[0] == 0x48 && pBuf[1] == 3)
//                        {
//                            LOG_I("rcv 0x48 ok");
//                        }
                        /*把来源的通道号暂存到预留 字段*/
                        pSafe_layer->res = from_chl;
                        /*把源地址和目标地址进行交换，方便处理后回传 到VCP平台*/
                        tmp_adr = pSafe_layer->src_adr;
                        pSafe_layer->src_adr = pSafe_layer->des_adr;
                        pSafe_layer->des_adr = tmp_adr;
                        app_layer_check(data_handle, &pBuf[sizeof(r_safe_layer)], pBuf);
                        ret = RT_EOK;
//                        LOG_I("CRC Check OK");
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
                    LOG_E("safe_layer_check CRC1 err ! src %x, des %x, len %d", pSafe_layer->src_adr, pSafe_layer->des_adr, pSafe_layer->lenth);
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

