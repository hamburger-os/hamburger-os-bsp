/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_can.h"

#define DBG_TAG "if_can"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>
#include <stdio.h>

#include "if_gpio.h"

#define CAN_RX_THREAD_PRIORITY         20
#define CAN_RX_THREAD_STACK_SIZE       (1024 * 4)
#define CAN_RX_THREAD_TIMESLICE        5

#define CAN_MQ_NUM          (256)

typedef struct
{
    rt_uint16_t channel;
    rt_uint16_t len;
} S_CAN_LEN_INFO;

typedef struct
{
    const char* dev_name;
    rt_device_t can_dev;
    rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size);
    rt_mq_t can_mq;
    S_CAN_LEN_INFO info;
} S_CAN_INFO;

typedef struct
{
    E_SLOT_ID id; /* 板子id */
    S_CAN_INFO info[E_CAN_CH_MAX];
    uint8_t ch_num;

    struct rt_mailbox mailbox;
} S_CAN_DEV;

static S_CAN_DEV can_dev;

static char can_mb_pool[1024];

/* 接收数据回调函数 */
static rt_err_t swos2_can_ch1_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后邮件 */
    S_CAN_LEN_INFO *info = &can_dev.info[E_CAN_CH_1].info;

    if(size != 0)
    {
        info->channel = E_CAN_CH_1;
        info->len = size;
        rt_mb_send(&can_dev.mailbox, (rt_uint32_t)info);
    }
    return RT_EOK;
}

static rt_err_t swos2_can_ch2_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后邮件 */
    S_CAN_LEN_INFO *info = &can_dev.info[E_CAN_CH_2].info;

    if(size != 0)
    {
        info->channel = E_CAN_CH_2;
        info->len = size;
        rt_mb_send(&can_dev.mailbox, (rt_uint32_t)info);
    }
    return RT_EOK;
}

static rt_err_t swos2_can_ch3_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后邮件 */
    S_CAN_LEN_INFO *info = &can_dev.info[E_CAN_CH_3].info;

    if(size != 0)
    {
        info->channel = E_CAN_CH_3;
        info->len = size;
        rt_mb_send(&can_dev.mailbox, (rt_uint32_t)info);
    }
    return RT_EOK;
}

static rt_err_t swos2_can_ch4_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后邮件 */
    S_CAN_LEN_INFO *info = &can_dev.info[E_CAN_CH_4].info;

    if(size != 0)
    {
        info->channel = E_CAN_CH_4;
        info->len = size;
        rt_mb_send(&can_dev.mailbox, (rt_uint32_t)info);
    }
    return RT_EOK;
}

static rt_err_t swos2_can_ch5_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后邮件 */
    S_CAN_LEN_INFO *info = &can_dev.info[E_CAN_CH_5].info;

    if(size != 0)
    {
        info->channel = E_CAN_CH_5;
        info->len = size;
        rt_mb_send(&can_dev.mailbox, (rt_uint32_t)info);
    }
    return RT_EOK;
}

static void CanRxThreadEntry(void *parameter)
{
    S_CAN_DEV *p_can_dev = &can_dev;
    rt_err_t ret = RT_EOK;
    S_CAN_LEN_INFO *info = RT_NULL;
    rt_uint32_t mbval;
    rt_size_t read_size = 0;

    struct rt_can_msg rxmsg = { 0 };

    while (1)
    {
        ret = rt_mb_recv(&p_can_dev->mailbox, &mbval, RT_WAITING_FOREVER);
        if (RT_EOK == ret)
        {
            info = (S_CAN_LEN_INFO *) mbval;
            RT_ASSERT(info != RT_NULL);

            if ((info->channel + 1) > can_dev.ch_num)
            {
                LOG_E("ch %d > can_dev.ch_num %d", info->channel, can_dev.ch_num);
                continue;
            }

            read_size = rt_device_read(can_dev.info[info->channel].can_dev, 0, &rxmsg, sizeof(rxmsg));
            if (read_size == sizeof(rxmsg))
            {
#if 1
                //LOG_D("read %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
                //LOG_HEX("read", 16, rxmsg.data, 8);
                //rt_kprintf("read %x \r\n", rxmsg.id);
                ret = rt_mq_send(can_dev.info[info->channel].can_mq, (const void *) &rxmsg, sizeof(rxmsg));
                if (ret != RT_EOK)
                {
                    LOG_E("can mq send error %d", info->channel);
                    continue;
                }
#else

                if (rt_device_write(can_dev.info[info->channel].can_dev, 0, &rxmsg, sizeof(rxmsg)) != sizeof(rxmsg))
                {
                    LOG_E("can data send error %d", info->channel);
                    continue;
                }
#endif

            }
            else
            {
                LOG_E("ch %d read %x %d %d %d %d", info->channel, rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len, read_size);
            }
            rt_memset(&rxmsg, 0, sizeof(rxmsg));
        }
    }
}

static int CanRxThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("if can rx", CanRxThreadEntry, RT_NULL,
    CAN_RX_THREAD_STACK_SIZE,
    CAN_RX_THREAD_PRIORITY, CAN_RX_THREAD_TIMESLICE);

    if (tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

static BOOL swos2_can_cfg(S_CAN_DEV *p_can_dev)
{
    if (NULL == p_can_dev)
    {
        return FALSE;
    }

    switch (p_can_dev->id)
    {
    case E_SLOT_ID_1:
    case E_SLOT_ID_5:
        p_can_dev->info[E_CAN_CH_1].dev_name = "can1";
        p_can_dev->info[E_CAN_CH_1].rx_ind = swos2_can_ch1_rx_call;
        p_can_dev->info[E_CAN_CH_2].dev_name = "can2";
        p_can_dev->info[E_CAN_CH_2].rx_ind = swos2_can_ch2_rx_call;
        p_can_dev->info[E_CAN_CH_3].dev_name = "mcp2517fd1";
        p_can_dev->info[E_CAN_CH_3].rx_ind = swos2_can_ch3_rx_call;
        p_can_dev->info[E_CAN_CH_4].dev_name = "mcp2517fd2";
        p_can_dev->info[E_CAN_CH_4].rx_ind = swos2_can_ch4_rx_call;
        p_can_dev->ch_num = 4;
        break;
    case E_SLOT_ID_3:
    case E_SLOT_ID_7:
        p_can_dev->info[E_CAN_CH_1].dev_name = "can1";
        p_can_dev->info[E_CAN_CH_1].rx_ind = swos2_can_ch1_rx_call;
        p_can_dev->info[E_CAN_CH_2].dev_name = "can2";
        p_can_dev->info[E_CAN_CH_2].rx_ind = swos2_can_ch2_rx_call;
        p_can_dev->info[E_CAN_CH_3].dev_name = "mcp2517fd1";
        p_can_dev->info[E_CAN_CH_3].rx_ind = swos2_can_ch3_rx_call;
        p_can_dev->info[E_CAN_CH_4].dev_name = "mcp2517fd2";
        p_can_dev->info[E_CAN_CH_4].rx_ind = swos2_can_ch4_rx_call;
        p_can_dev->ch_num = 4;
        break;
    case E_SLOT_ID_2:
    case E_SLOT_ID_4:
    case E_SLOT_ID_6:
    case E_SLOT_ID_8:
        p_can_dev->info[E_CAN_CH_1].dev_name = "can1";
        p_can_dev->info[E_CAN_CH_1].rx_ind = swos2_can_ch1_rx_call;
        p_can_dev->info[E_CAN_CH_2].dev_name = "can2";
        p_can_dev->info[E_CAN_CH_2].rx_ind = swos2_can_ch2_rx_call;
        p_can_dev->info[E_CAN_CH_3].dev_name = "mcp2517fd1";
        p_can_dev->info[E_CAN_CH_3].rx_ind = swos2_can_ch3_rx_call;
        p_can_dev->info[E_CAN_CH_4].dev_name = "mcp2517fd2";
        p_can_dev->info[E_CAN_CH_4].rx_ind = swos2_can_ch4_rx_call;
        p_can_dev->info[E_CAN_CH_5].dev_name = "mcp2517fd3";
        p_can_dev->info[E_CAN_CH_5].rx_ind = swos2_can_ch5_rx_call;
        p_can_dev->ch_num = 5;
        break;
    default:
        LOG_E("can cfg slot id error");
        return FALSE;
    }
    return TRUE;
}

static BOOL swos2_can_init(S_CAN_DEV *p_can_dev)
{
    uint8_t i;
    uint8_t can_mq_name[20];

    if (NULL == p_can_dev)
    {
        return FALSE;
    }

    if (rt_mb_init(&p_can_dev->mailbox, "can mbt", &can_mb_pool[0], sizeof(can_mb_pool) / 4, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("can mb init error");
        return FALSE;
    }

    for (i = 0; i < p_can_dev->ch_num; i++)
    {
        p_can_dev->info[i].can_dev = rt_device_find(p_can_dev->info[i].dev_name);
        if (p_can_dev->info[i].can_dev != NULL)
        {
            /* 设置 CAN 通信的默认波特率 */
            if (rt_device_control(p_can_dev->info[i].can_dev, RT_CAN_CMD_SET_BAUD, (void *) CAN500kBaud) != RT_EOK)
            {
                LOG_E("set baud error!");
                return FALSE;
            }
            /* 设置 CAN 的工作模式为正常工作模式 */
            if (rt_device_control(p_can_dev->info[i].can_dev, RT_CAN_CMD_SET_MODE,
                    (void *) RT_CAN_MODE_NORMAL) != RT_EOK)
            {
                LOG_E("set mode error!");
                return FALSE;
            }

            /* 以中断接收及发送方式打开 CAN 设备 */
            if (rt_device_open(p_can_dev->info[i].can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
            {
                LOG_E("open '%s' error!", p_can_dev->info[i].dev_name);
            }

            /* 设置接收回调函数 */
            rt_device_set_rx_indicate(p_can_dev->info[i].can_dev, p_can_dev->info[i].rx_ind);

            /* 初始化消息队列 */
            sprintf((char *)can_mq_name, "can %d mq", i);
            p_can_dev->info[i].can_mq = rt_mq_create((const char *)can_mq_name, sizeof(struct rt_can_msg), CAN_MQ_NUM, RT_IPC_FLAG_FIFO);
            if (RT_NULL == p_can_dev->info[i].can_mq)
            {
                LOG_E("can %d mq null", i);
                return FALSE;
            }
        }
        else
        {
            LOG_E("find '%s' error!", p_can_dev->info[i].dev_name);
            return FALSE;
        }
    }

    if (CanRxThreadInit() != RT_EOK)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL if_can_init(void)
{
    BOOL result;

    S_CAN_DEV *p_can_dev = &can_dev;

    memset(p_can_dev, 0, sizeof(S_CAN_DEV));

    p_can_dev->id = if_gpio_getSlotId();

    result = swos2_can_cfg(p_can_dev);
    if (result != TRUE)
    {
        LOG_E("swos2_can_cfg error");
        return result;
    }

    result = swos2_can_init(p_can_dev);
    if (result != TRUE)
    {
        LOG_E("swos2_can_init error");
        return result;
    }
    return TRUE;
}

BOOL if_can_send(E_CAN_CH ch, S_CAN_MSG *pMsg)
{
    struct rt_can_msg rxmsg = { 0 };

    if(NULL == pMsg)
    {
        return FALSE;
    }

    if(ch >= E_CAN_CH_MAX)
    {
        return FALSE;
    }

    rxmsg.id = pMsg->id_u32;
    rxmsg.len = pMsg->len_u8;
    rxmsg.fd_frame = pMsg->can_mode;
    memcpy(rxmsg.data, pMsg->data_u8, sizeof(pMsg->data_u8));

    if (rt_device_write(can_dev.info[ch].can_dev, 0, &rxmsg, sizeof(rxmsg)) == sizeof(rxmsg))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL if_can_get(E_CAN_CH ch, S_CAN_MSG *pMsg)
{
    rt_err_t ret = RT_EOK;
    struct rt_can_msg rx_msg;

    if (NULL == pMsg)
    {
        return FALSE;
    }

    if(ch >= E_CAN_CH_MAX)
    {
        return FALSE;
    }

    ret = rt_mq_recv(can_dev.info[ch].can_mq, (void *) &rx_msg, sizeof(struct rt_can_msg), RT_WAITING_NO);
    if (RT_EOK == ret)
    {
        pMsg->id_u32 = rx_msg.id;
        pMsg->len_u8 = rx_msg.len;
        memcpy(pMsg->data_u8, rx_msg.data, sizeof(pMsg->data_u8));

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
