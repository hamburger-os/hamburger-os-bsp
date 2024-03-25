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

#define DBG_TAG "EthThread"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "data_handle.h"
#include "safe_layer.h"
#include "record_ota.h"

#define ETH0_THREAD_PRIORITY         20//22//14
#define ETH0_THREAD_STACK_SIZE       (1024 * 10)
#define ETH0_THREAD_TIMESLICE        5

// #define ETH_RX_BUF_MAX_NUM 1500

typedef struct {
    rt_device_t dev;
    rt_mq_t size_mq;
    uint8_t rx_buf[ETH_RX_BUF_MAX_NUM];
    uint32_t rx_szie;
    uint32_t rx_size_mq_err;
    uint8_t err_log_cnt;
    uint32_t is_update;
} S_ETH_THREAD;

uint8_t record_mac_i[]  = {0x4c, 0x53, 0x57, 0x00, 0x03, 0x01};
uint8_t record_mac_ii[] = {0x4c, 0x53, 0x57, 0x00, 0x03, 0x10};
uint8_t zk_mac_i[] = {0x4c, 0x53, 0x57, 0x00, 0x01, 0x02};
uint8_t zk_mac_ii[] = {0x4c, 0x53, 0x57, 0x00, 0x01, 0x0a};

extern S_DATA_HANDLE eth0_can_data_handle;
extern S_DATA_HANDLE eth1_can_data_handle;

static S_ETH_THREAD eth0_thread;
static S_ETH_THREAD eth1_thread;

static uint8_t eth0_send_buf[COMM_RBUF_SIZE];
static uint8_t eth1_send_buf[COMM_RBUF_SIZE];

int linke_eth_send(ETH_CH_INEX ch, uint8_t *pdata, uint16_t len)
{
    uint8_t send_len = 0;

    if(ETH_CH_INEX_1 == ch)
    {
        memset(eth0_send_buf, 0, sizeof(eth0_send_buf));

        memcpy(eth0_send_buf, zk_mac_i, 6u);   /* 目的I系主控 */
        memcpy(&eth0_send_buf[6], record_mac_i, 6u);   /* 源地址 */

        /* 长度 */
        eth0_send_buf[12] = (uint8_t)(len >> 8);
        eth0_send_buf[13] = (uint8_t)len;

        /* 数据 */
        memcpy(&eth0_send_buf[14], pdata, len);
        send_len = len + 14;
        
        if(rt_device_write(eth0_thread.dev, 0, eth0_send_buf, send_len) != send_len)
        {
            return 0;
        }
        else
        {
//            LOG_HEX("eth1", 16, eth0_send_buf, send_len);
            return 1;
        }
    }
    else if(ETH_CH_INEX_2 == ch)
    {
        memset(eth1_send_buf, 0, sizeof(eth1_send_buf));

        memcpy(eth1_send_buf, zk_mac_ii, 6u);   /* 目的II系主控 */
        memcpy(&eth1_send_buf[6], record_mac_ii, 6u);   /* 源地址 */

        /* 长度 */
        eth1_send_buf[12] = (uint8_t)(len >> 8);
        eth1_send_buf[13] = (uint8_t)len;

        /* 数据 */
        memcpy(&eth1_send_buf[14], pdata, len);
        send_len = len + 14;

        if(rt_device_write(eth1_thread.dev, 0, eth1_send_buf, send_len) != send_len)
        {
            return 0;
        }
        else
        {
//            LOG_HEX("eth2", 16, eth1_send_buf, send_len);
            return 1;
        }
    }
    else
    {
        return 0;
    }
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

static rt_err_t ETH0RXChannel1Callback(rt_device_t dev, rt_size_t size)
{
    uint32_t rx_size;
    rt_err_t ret = RT_EOK;

    rx_size = size;
    if(rx_size != 0)
    {
        if(eth0_thread.is_update == 0)
        {
            ret = rt_mq_send(eth0_thread.size_mq, (const void *)&rx_size, sizeof(uint32_t));
            if(ret != RT_EOK)
            {
                eth0_thread.rx_size_mq_err++;
                return ret;
            }
            else
            {
                eth0_thread.rx_size_mq_err = 0;
            }
        }
    }
    return ret;
}

static rt_err_t ETH1RXChannel1Callback(rt_device_t dev, rt_size_t size)
{
    uint32_t rx_size;
    rt_err_t ret = RT_EOK;

    rx_size = size;
    if(rx_size != 0)
    {
        if(eth1_thread.is_update == 0)
        {
            ret = rt_mq_send(eth1_thread.size_mq, (const void *)&rx_size, sizeof(uint32_t));
            if(ret != RT_EOK)
            {
                eth1_thread.rx_size_mq_err++;
                return ret;
            }
            else
            {
                eth1_thread.rx_size_mq_err = 0;
            }
        }
    }
    return ret;
}

static void ETH0ThreadEntry(void *parameter)
{
    rt_err_t ret = RT_EOK;

    while(1)
    {
        if(RecordOTAGetMode() == RecordOTAModeNormal)
        {
            if(eth0_thread.is_update != 0)
            {
                eth0_thread.is_update = 0;
            }

            if(eth1_thread.is_update != 0)
            {
                eth1_thread.is_update = 0;
            }
            ret = rt_mq_recv(eth0_thread.size_mq, (void *)&eth0_thread.rx_szie, sizeof(uint32_t), RT_WAITING_NO);
            if(RT_EOK == ret && eth0_can_data_handle.can_data_mq != RT_NULL)
            {
                rt_device_read(eth0_thread.dev, 0, (void *)eth0_thread.rx_buf, eth0_thread.rx_szie);
                rx_safe_layer_check(&eth0_can_data_handle, eth0_thread.rx_buf + 14, ETH_CH_INEX_1);  //14 = sizeof(eth_frame_t)
            }

            ret = rt_mq_recv(eth1_thread.size_mq, (void *)&eth1_thread.rx_szie, sizeof(uint32_t), RT_WAITING_NO);
            if(RT_EOK == ret && eth1_can_data_handle.can_data_mq != RT_NULL)
            {
                rt_device_read(eth1_thread.dev, 0, (void *)eth1_thread.rx_buf, eth1_thread.rx_szie);
                rx_safe_layer_check(&eth1_can_data_handle, eth1_thread.rx_buf + 14, ETH_CH_INEX_2);  //14 = sizeof(eth_frame_t)
            }
        }
        else
        {
            if(eth0_thread.is_update != 1)
            {
                eth0_thread.is_update = 1;
            }

            if(eth1_thread.is_update != 1)
            {
                eth1_thread.is_update = 1;
            }
        }

        if(eth0_thread.rx_size_mq_err != 0 && eth0_thread.err_log_cnt < 5)
        {
            eth0_thread.err_log_cnt++;
            LOG_E("e0 rx mq err %d", eth0_thread.rx_size_mq_err);
        }

        if(eth1_thread.rx_size_mq_err != 0 && eth1_thread.err_log_cnt < 5)
        {
            LOG_E("e1 rx mq err %d", eth1_thread.rx_size_mq_err);
        }
        rt_thread_mdelay(1);
    }
}

rt_err_t ETHInit(void)
{
    eth0_thread.size_mq = rt_mq_create("e0 queue", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
    if(RT_NULL == eth0_thread.size_mq)
    {
        LOG_E("eth0 create mq failed");
        return -RT_EEMPTY;
    }

    eth1_thread.size_mq = rt_mq_create("e1 queue", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
    if(RT_NULL == eth1_thread.size_mq)
    {
        LOG_E("eth1 create mq failed");
        return -RT_EEMPTY;
    }

    ETHChannelInit(&eth0_thread, "e0");
    ETHChannelSetRXCallback(&eth0_thread, ETH0RXChannel1Callback);

    ETHChannelInit(&eth1_thread, "e1");
    ETHChannelSetRXCallback(&eth1_thread, ETH1RXChannel1Callback);

    return RT_EOK;
}

int ETHThreadInit(void)
{
    rt_thread_t tid1;

    tid1 = rt_thread_create("eth_thread",
                            ETH0ThreadEntry, RT_NULL,
                            ETH0_THREAD_STACK_SIZE,
                            ETH0_THREAD_PRIORITY, ETH0_THREAD_TIMESLICE);

    if(tid1 != NULL)
    {
        rt_thread_startup(tid1);
        return RT_EOK;
    }

    return -RT_ERROR;
}

