/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_hdlc.h"

#define DBG_TAG "if_hdlc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "if_gpio.h"

#define SWOS2_HDLC_SIZE_MQ_NUM   (20)
//#define SWOS2_HDLC_MSG_MQ_NUM    (20)
#define SWOS2_HDLC_MSG_MQ_NUM    (300)
#define SWOS2_HDLC_FRAME_BUF_SIZE   (256)

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    uint16_t len;
    uint8_t buf[SWOS2_HDLC_FRAME_BUF_SIZE];
} S_SWOS2_HDLC_FRAME;

#pragma pack()

typedef struct
{
    E_SLOT_ID id; /* 板子id */
    rt_device_t dev;
    rt_mq_t size_mq;
    rt_mq_t msg_mq;
} S_SWOS2_HDLC_DEV;

static S_SWOS2_HDLC_DEV swos2_hdlc_dev;

static rt_err_t swos2_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;
    S_SWOS2_HDLC_DEV *p_dev = &swos2_hdlc_dev;
    uint32_t read_size;

    read_size = size;
    result = rt_mq_send(p_dev->size_mq, &read_size, sizeof(uint32_t));
    if (result != RT_EOK)
    {
        LOG_E("swos2 hdlc rx mq send error %d", result);
    }
    return result;
}

static void swos2_hdlc_rx_thread_entry(void *param)
{
    S_SWOS2_HDLC_DEV *dev = (S_SWOS2_HDLC_DEV *)param;

    if(RT_NULL == dev)
    {
        return;
    }

    uint32_t read_size = 0;
    rt_err_t result;
    S_SWOS2_HDLC_FRAME hdlc_rx_msg;

    while(1)
    {
        if(RT_EOK == rt_mq_recv(dev->size_mq, &read_size, sizeof(uint32_t), 0))
        {
            if(read_size > 0)
            {
                rt_memset(hdlc_rx_msg.buf, 0, SWOS2_HDLC_FRAME_BUF_SIZE);
                hdlc_rx_msg.len = rt_device_read(dev->dev, 0, hdlc_rx_msg.buf, read_size);
                if(hdlc_rx_msg.len == read_size)
                {
//                    rt_kprintf("read %x, len %d\r\n", hdlc_rx_msg.buf[0], read_size);
                    result = rt_mq_send(dev->msg_mq, &hdlc_rx_msg, sizeof(S_SWOS2_HDLC_FRAME));
                    if (result != RT_EOK)
                    {
                        LOG_E("msg mq send error %d", result);
                    }
                }
            }
        }
        rt_thread_mdelay(1);
    }
}

BOOL if_hdlc_init(void)
{
    rt_thread_t tid;

    memset(&swos2_hdlc_dev, 0, sizeof(S_SWOS2_HDLC_DEV));

    /* 1.识别板子类型 */
    swos2_hdlc_dev.id = if_gpio_getSlotId();
    if(swos2_hdlc_dev.id != E_SLOT_ID_3 && swos2_hdlc_dev.id != E_SLOT_ID_7)
    {
        return FALSE;
    }

    /* 2.查找并打开设备 */
    swos2_hdlc_dev.dev = rt_device_find("z8523l16");
    if(RT_NULL == swos2_hdlc_dev.dev)
    {
        LOG_E("find z8523l16 error");
        return FALSE;
    }

    if(rt_device_open(swos2_hdlc_dev.dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("hdlc open error");
        return FALSE;
    }

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(swos2_hdlc_dev.dev, swos2_rx_ind);

    /* 3.创建消息队列 */
    swos2_hdlc_dev.size_mq = rt_mq_create("hdlc size mq", sizeof(uint32_t), SWOS2_HDLC_SIZE_MQ_NUM, RT_IPC_FLAG_FIFO);
    if (RT_NULL == swos2_hdlc_dev.size_mq)
    {
        LOG_E("hdlc size mq null");
        return FALSE;
    }

    swos2_hdlc_dev.msg_mq = rt_mq_create("hdlc msg mq", sizeof(S_SWOS2_HDLC_FRAME), SWOS2_HDLC_MSG_MQ_NUM, RT_IPC_FLAG_FIFO);
    if (RT_NULL == swos2_hdlc_dev.msg_mq)
    {
        LOG_E("hdlc msg mq null");
        return FALSE;
    }

    /* 4.创建，启动接收线程 */
//    tid = rt_thread_create("if hdlc rx", swos2_hdlc_rx_thread_entry, &swos2_hdlc_dev, 2048, 12, 5);
    tid = rt_thread_create("if hdlc rx", swos2_hdlc_rx_thread_entry, &swos2_hdlc_dev, 2048, 20, 5);
    if (tid == RT_NULL)
    {
        LOG_E("sw_hdlc_rx thread create fail!");
        return FALSE;
    }
    else
    {
        rt_thread_startup(tid);
    }

    return TRUE;
}

BOOL if_hdlc_send(E_HDLC_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_HDLC_CH_MAX)
    {
        return FALSE;
    }

    uint32_t size = 0;
    S_SWOS2_HDLC_DEV *dev = &swos2_hdlc_dev;

    size = rt_device_write(dev->dev, 0, (const void*)pdata, len);
    if(size != len)
    {
        return FALSE;
    }

    return TRUE;
}

uint16 if_hdlc_get(E_HDLC_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_HDLC_CH_MAX)
    {
        return FALSE;
    }

    rt_err_t ret;
    S_SWOS2_HDLC_DEV *dev = &swos2_hdlc_dev;
    S_SWOS2_HDLC_FRAME rx_msg;

    ret = rt_mq_recv(dev->msg_mq, (void *)&rx_msg, sizeof(S_SWOS2_HDLC_FRAME), RT_WAITING_NO);
    if (RT_EOK == ret)
    {
        rt_memcpy(pdata, &rx_msg.buf, rx_msg.len);
        return rx_msg.len;
    }
    else
    {
        return 0;
    }

    return 0;
}

