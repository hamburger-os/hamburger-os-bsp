/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_eth.h"

#include <rtthread.h>

#if RT_VER_NUM >= 0x40100
#include <unistd.h>
#else
#include <dfs_posix.h>
#endif /* RT_VER_NUM >= 0x40100 */
#include "netif/ethernetif.h"
#include "netif/ethernet.h"

#define DBG_TAG "if_eth"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#define SW_ETH_RX_BUF_MAX_NUM (1500U)

typedef struct
{
    rt_uint16_t channel;
    rt_uint16_t len;
} S_ETH_LEN_INFO;

typedef struct
{
    rt_device_t dev;
    const char *name;
    S_ETH_LEN_INFO eth_info;
    uint8_t rx_buf[SW_ETH_RX_BUF_MAX_NUM];
} S_SWOS2_ETH;

typedef struct
{
    S_SWOS2_ETH dev[E_ETH_CH_MAX];
    struct rt_mailbox mailbox;
} S_SWOS2_ETH_DEV;

static char eth_mb_pool[1024];

static S_SWOS2_ETH_DEV swos2_eth_dev;

static rt_err_t sw_eth1_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_err_t ret = RT_EOK;
    S_ETH_LEN_INFO *info = &swos2_eth_dev.dev[E_ETH_CH_1].eth_info;

    if(size != 0)
    {
        info->channel = E_ETH_CH_1;
        info->len = size;
        rt_mb_send(&swos2_eth_dev.mailbox, (rt_uint32_t)info);
    }
    return ret;
}

static rt_err_t sw_eth2_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_err_t ret = RT_EOK;
    S_ETH_LEN_INFO *info = &swos2_eth_dev.dev[E_ETH_CH_2].eth_info;
//    if(size != 0)
//    {
//        info->channel = E_ETH_CH_2;
//        info->len = size;
//        rt_mb_send(&swos2_eth_dev.mailbox, (rt_uint32_t)info);
//    }
    return ret;
}

static void sw_eth_set_rx_callback(S_SWOS2_ETH *eth, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{
    if(rx_ind != NULL && eth != NULL)
    {
        rt_device_set_rx_indicate(eth->dev, rx_ind);
    }
}

static void sw_ethrx_thread_entry(void *param)
{
    S_ETH_LEN_INFO *info = RT_NULL;
    rt_uint32_t mbval;
    rt_err_t ret;

    while (1)
    {
        ret = rt_mb_recv(&swos2_eth_dev.mailbox, &mbval, RT_WAITING_FOREVER);
        if (RT_EOK == ret)
        {
            info = (S_ETH_LEN_INFO *) mbval;
            RT_ASSERT(info != RT_NULL);
            LOG_I("ETH:%d,size = %d", info->channel, info->len);

            if(rt_device_read(swos2_eth_dev.dev[info->channel].dev, 0, (void *)swos2_eth_dev.dev[info->channel].rx_buf, info->len) != info->len)
            {
                LOG_E("eth %d rcv error", info->channel);
            }
            else
            {
                LOG_HEX("read", 16, swos2_eth_dev.dev[info->channel].rx_buf, info->len);
//                LOG_I("ch %d, len %d", info->channel, info->len);
            }

            info = RT_NULL;
        }
        else
        {
            LOG_E("mb rcv error %d", ret);
        }

        rt_thread_mdelay(5);
    }
}

BOOL if_eth_init(void)
{
    rt_thread_t tid;


    if (rt_mb_init(&swos2_eth_dev.mailbox, "eth_rx_t_mb", &eth_mb_pool[0], sizeof(eth_mb_pool) / 4, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("eth mb init error");
        return FALSE;
    }

    tid = rt_thread_create("sw_eth_rx", sw_ethrx_thread_entry, RT_NULL, 2048, 12, 5);
    if (tid == RT_NULL)
    {
        LOG_E("sw_eth_rx thread create fail!");
        return FALSE;
    }

    rt_thread_startup(tid);

    swos2_eth_dev.dev[E_ETH_CH_1].dev = rt_device_find("e");
    if (RT_NULL != swos2_eth_dev.dev[E_ETH_CH_1].dev)
    {
        if(rt_device_open(swos2_eth_dev.dev[E_ETH_CH_1].dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
        {
            LOG_E("e open error.");
            return FALSE;
        }
        else
        {
            sw_eth_set_rx_callback(&swos2_eth_dev.dev[E_ETH_CH_1], sw_eth1_rx_callback);
        }
    }
    else
    {
        LOG_E("can not find e if!");
        return FALSE;
    }

    swos2_eth_dev.dev[E_ETH_CH_2].dev = rt_device_find("e0");
    if (RT_NULL != swos2_eth_dev.dev[E_ETH_CH_2].dev)
    {
        if(rt_device_open(swos2_eth_dev.dev[E_ETH_CH_2].dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
        {
            LOG_E("e0 open error.");
            return FALSE;
        }
        else
        {
            sw_eth_set_rx_callback(&swos2_eth_dev.dev[E_ETH_CH_2], sw_eth2_rx_callback);
        }
    }
    else
    {
        LOG_E("can not find e0 if!");
        return FALSE;
    }

    return TRUE;
}

BOOL if_eth_send(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_ETH_CH_MAX)
    {
        return FALSE;
    }

    if(rt_device_write( swos2_eth_dev.dev[ch].dev, 0, (const void *)pdata, len) != len)
    {
        return FALSE;
    }
    return TRUE;
}

uint16 if_eth_get(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
//    rt_device_read
    return TRUE;
}
