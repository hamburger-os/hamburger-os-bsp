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
#include "safe_layer.h"

#define DBG_TAG "EthThread"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define ETH0_THREAD_PRIORITY         10
#define ETH0_THREAD_STACK_SIZE       (1024 * 2)
#define ETH0_THREAD_TIMESLICE        5

#define ETH1_THREAD_PRIORITY         10
#define ETH1_THREAD_STACK_SIZE       (1024 * 2)
#define ETH1_THREAD_TIMESLICE        5

#define ETH_RX_BUF_MAX_NUM 1500

typedef struct {
    rt_device_t dev;
    rt_mq_t size_mq;
    uint8_t rx_buf[ETH_RX_BUF_MAX_NUM];
    uint32_t rx_szie;
} S_ETH_THREAD;

extern S_DATA_HANDLE eth0_can_data_handle;
extern S_DATA_HANDLE eth1_can_data_handle;

static S_ETH_THREAD eth0_thread;
static S_ETH_THREAD eth1_thread;

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

static rt_err_t ETH0RXChannel1Callback(rt_device_t dev, rt_size_t size)
{
    uint32_t rx_size;
    rt_err_t ret = RT_EOK;

    rx_size = size;
    if(rx_size != 0)
    {
        ret = rt_mq_send(eth0_thread.size_mq, (const void *)&rx_size, sizeof(uint32_t));
        if(ret != RT_EOK)
        {
            LOG_E("can data mq send error");
            return ret;
        }
    }
    return ret;
}

static void *ETH0ThreadEntry(void *parameter)
{
    rt_err_t ret = RT_EOK;

    ETHChannelInit(&eth0_thread, "e0");
    ETHChannelSetRXCallback(&eth0_thread, ETH0RXChannel1Callback);

    eth0_thread.size_mq = rt_mq_create("e0 queue", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
//    eth0_thread.size_mq = rt_mq_create("e0 queue", sizeof(uint32_t), 600, RT_IPC_FLAG_FIFO);
    if(RT_NULL == eth0_thread.size_mq)
    {
        LOG_E("eth0 create mq failed");
        return ;
    }

    while(1)
    {
//        ret = rt_mq_recv(eth0_thread.size_mq, (void *)&eth0_thread.rx_szie, sizeof(uint32_t), 1000);
        ret = rt_mq_recv(eth0_thread.size_mq, (void *)&eth0_thread.rx_szie, sizeof(uint32_t), RT_WAITING_FOREVER);
        if(RT_EOK == ret)
        {
            rt_device_read(eth0_thread.dev, 0, (void *)eth0_thread.rx_buf, eth0_thread.rx_szie);
            rx_safe_layer_check(&eth0_can_data_handle, eth0_thread.rx_buf + 14, IN_ETH_DEV);  //14 = sizeof(eth_frame_t)
        }
        rt_thread_mdelay(5);
    }
}

//static rt_err_t ETH1RXChannel1Callback(rt_device_t dev, rt_size_t size)
//{
//    uint32_t rx_size;
//    rt_err_t ret = RT_EOK;
//
//    rx_size = size;
//    if(rx_size != 0)
//    {
//        ret = rt_mq_send(eth1_thread.size_mq, (const void *)&rx_size, sizeof(uint32_t));
//        if(ret != RT_EOK)
//        {
//            LOG_E("can data mq send error");
//            return ret;
//        }
//    }
//    return ret;
//}
//
//static void *ETH1ThreadEntry(void *parameter)
//{
//    rt_err_t ret = RT_EOK;
//
//    ETHChannelInit(&eth1_thread, "e1");
//    ETHChannelSetRXCallback(&eth1_thread, ETH1RXChannel1Callback);
//
//    eth1_thread.size_mq = rt_mq_create("e1 queue", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
//    if(RT_NULL == eth1_thread.size_mq)
//    {
//        LOG_E("eth1 create mq failed");
//        return ;
//    }
//
//    while(1)
//    {
//        ret = rt_mq_recv(eth1_thread.size_mq, (void *)&eth1_thread.rx_szie, sizeof(uint32_t), 1000);
//        if(RT_EOK == ret)
//        {
//            rt_device_read(eth1_thread.dev, 0, (void *)eth1_thread.rx_buf, eth1_thread.rx_szie);
//            rx_safe_layer_check(&eth1_can_data_handle, eth1_thread.rx_buf + 14, IN_ETH_DEV);
//        }
//
//        rt_thread_mdelay(5);
//    }
//}

int ETHThreadInit(void)
{
    rt_thread_t tid1, tid2;

    tid1 = rt_thread_create("eth0_thread",
                            ETH0ThreadEntry, RT_NULL,
                            ETH0_THREAD_STACK_SIZE,
                            ETH0_THREAD_PRIORITY, ETH0_THREAD_TIMESLICE);

//    tid2 = rt_thread_create("eth1_thread",
//                            ETH1ThreadEntry, RT_NULL,
//                            ETH1_THREAD_STACK_SIZE,
//                            ETH1_THREAD_PRIORITY, ETH1_THREAD_TIMESLICE);

    if(tid1 != NULL)
    {
        rt_thread_startup(tid1);
        return RT_EOK;
    }

//    if(tid2 != NULL)
//    {
//        rt_thread_startup(tid2);
//        return RT_EOK;
//    }
    return -RT_ERROR;
}

