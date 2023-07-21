/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */
#include "eth_thread.h"

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "data_handle.h"

#define DBG_TAG "EthThread"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define ETH_THREAD_PRIORITY         18
#define ETH_THREAD_STACK_SIZE       2048
#define ETH_THREAD_TIMESLICE        5

#define ETH_RX_BUF_MAX_NUM 1500

typedef struct {
    rt_device_t dev;
    rt_mq_t size_mq;
    uint8_t rx_buf[ETH_RX_BUF_MAX_NUM];
    uint32_t rx_szie;
} S_ETH_THREAD;

static S_ETH_THREAD eth0_thread;
S_DATA_HANDLE eth_can_data_handle;

#define ETH_READ_IN_Callback 1

static rt_err_t ETHRXChannel1Callback(rt_device_t dev, rt_size_t size)
{
    uint32_t rx_size;
    rt_err_t ret = RT_EOK;

    if(size != 0)
    {
#if ETH_READ_IN_Callback
        eth0_thread.rx_szie = size;
        rt_device_read(eth0_thread.dev, 0, (void *)eth0_thread.rx_buf, eth0_thread.rx_szie);
        LOG_I("rx size %d", eth0_thread.rx_szie);
        LOG_I("des mac %x %x %x %x %x %x ",
                eth0_thread.rx_buf[0], eth0_thread.rx_buf[1], eth0_thread.rx_buf[2], eth0_thread.rx_buf[3], eth0_thread.rx_buf[4], eth0_thread.rx_buf[5]);
        LOG_I("src mac %x %x %x %x %x %x ",
                eth0_thread.rx_buf[6], eth0_thread.rx_buf[7], eth0_thread.rx_buf[8], eth0_thread.rx_buf[9], eth0_thread.rx_buf[10], eth0_thread.rx_buf[11]);
        LOG_I(" %d ",
                eth0_thread.rx_buf[60]);
#else
        ret = rt_mq_send(eth0_thread.size_mq, (const void *)&rx_size, sizeof(uint32_t));
        if(ret != RT_EOK)
        {
            LOG_E("can data mq send error\r\n");
            return ret;
        }
#endif
    }
    return ret;
}

static void ETHChannelSetRXCallback(S_ETH_THREAD *p_thread, rt_err_t (*rx_ind)(rt_device_t dev,rt_size_t size))
{
    if(rx_ind != NULL && p_thread != NULL)
    {
        rt_device_set_rx_indicate(p_thread->dev, rx_ind);
    }
}

static void ETHChannelInit(S_ETH_THREAD *p_thread, char *device_name)
{
    p_thread->dev = rt_device_find(device_name);
    if (RT_NULL == p_thread->dev)
    {
        LOG_E("%s find NULL.", device_name);
    }

    if(rt_device_open(p_thread->dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s open fail.", device_name);
    }

    LOG_I("%s open successful", device_name);
}

static void *ETHThreadEntry(void *parameter)
{
    rt_err_t ret = RT_EOK;

    DataHandleInit(&eth_can_data_handle);

    ETHChannelInit(&eth0_thread, "e0");
    ETHChannelSetRXCallback(&eth0_thread, ETHRXChannel1Callback);

    eth0_thread.size_mq = rt_mq_create("e0 queue", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
    if(RT_NULL == eth0_thread.size_mq)
    {
        LOG_E("eth0 create mq failed");
        return ;
    }

    while(1)
    {
#if ETH_READ_IN_Callback

#else
        ret = rt_mq_recv(eth0_thread.size_mq, (void *)&eth0_thread.rx_szie, sizeof(uint32_t), 1000);
        if(RT_EOK == ret)
        {
            rt_device_read(eth0_thread.dev, 0, (void *)eth0_thread.rx_buf, eth0_thread.rx_szie);
#if 1
            LOG_I("rx size %d", eth0_thread.rx_szie);
            LOG_I("des mac %x %x %x %x %x %x ",
                    eth0_thread.rx_buf[0], eth0_thread.rx_buf[1], eth0_thread.rx_buf[2], eth0_thread.rx_buf[3], eth0_thread.rx_buf[4], eth0_thread.rx_buf[5]);
            LOG_I("src mac %x %x %x %x %x %x ",
                    eth0_thread.rx_buf[6], eth0_thread.rx_buf[7], eth0_thread.rx_buf[8], eth0_thread.rx_buf[9], eth0_thread.rx_buf[10], eth0_thread.rx_buf[11]);
            LOG_I(" %d ",
                    eth0_thread.rx_buf[60]);
#endif
//            ETHToCanDataHandle(&eth_can_data_handle, eth0_thread.rx_buf, (uint16_t)eth0_thread.rx_szie);
        }
#endif
        rt_thread_mdelay(10);
    }
}

int ETHThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("eth_thread",
                            ETHThreadEntry, RT_NULL,
                            ETH_THREAD_STACK_SIZE,
                            ETH_THREAD_PRIORITY, ETH_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

