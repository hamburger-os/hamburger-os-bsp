/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_mvb.h"

#define DBG_TAG "if_mvb"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "linklib/inc/if_gpio.h"

#pragma pack(1)
typedef struct
{
    uint16_t tx_port[10];
    uint16_t rx_port[10];
    uint8_t tx_num;
    uint8_t rx_num;
} SWOS2_MVB_PORT_INFO;
#pragma pack()

typedef struct
{
    rt_device_t dev;
    E_SLOT_ID id; /* 板子id */
    SWOS2_MVB_PORT_INFO port_info;
} S_MVB_DEV;

static S_MVB_DEV s_mvb_dev;

BOOL if_mvb_init(void)
{
    S_MVB_DEV *p_mvb_dev = &s_mvb_dev;

    rt_memset(p_mvb_dev, 0, sizeof(S_MVB_DEV));

    p_mvb_dev->id = if_gpio_getSlotId();

    if(p_mvb_dev->id != E_SLOT_ID_3 && p_mvb_dev->id != E_SLOT_ID_7)
    {
        return FALSE;
    }

    p_mvb_dev->dev = rt_device_find("mvb");
    if(RT_NULL == p_mvb_dev->dev)
    {
        return FALSE;
    }

    if(rt_device_open(p_mvb_dev->dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("mvb open error");
        return FALSE;
    }

    if(rt_device_control(p_mvb_dev->dev, RT_DEVICE_CTRL_BLK_GETGEOME, (void *)&p_mvb_dev->port_info) != RT_EOK)
    {
        LOG_E("mvb get port info error");
        return FALSE;
    }

#ifdef DUMP_MVB_CH_INFO
    for(int i = 0; i < p_mvb_dev->port_info.rx_num; i++)
    {
        LOG_I("rx port: %x", p_mvb_dev->port_info.rx_port[i]);
    }

    for(int i = 0; i < p_mvb_dev->port_info.tx_num; i++)
    {
        LOG_I("tx port: %x", p_mvb_dev->port_info.tx_port[i]);
    }
#endif
    return TRUE;
}

BOOL if_mvb_send(E_MVB_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_MVB_CH_MAX)
    {
        return FALSE;
    }

    S_MVB_DEV *p_mvb_dev = &s_mvb_dev;

    if (rt_device_write(p_mvb_dev->dev, (p_mvb_dev->port_info.tx_port[1] & 0x0FFF), pdata, len) == len)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint16 if_mvb_get(E_MVB_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_MVB_CH_MAX)
    {
        return FALSE;
    }

    S_MVB_DEV *p_mvb_dev = &s_mvb_dev;
    rt_size_t read_size = 0;

    read_size = rt_device_read(p_mvb_dev->dev, (p_mvb_dev->port_info.rx_port[1] & 0x0FFF), pdata, len);
    return read_size;
}
