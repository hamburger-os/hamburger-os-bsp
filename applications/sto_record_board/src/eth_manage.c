/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-30     zm       the first version
 */
#include "eth_manage.h"

#include <rtthread.h>
#include <rtdevice.h>

#include "data_handle.h"

#define DBG_TAG "ETHManage"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_device_t eth_dev[ETH_MANAGE_CHANNEL_MAX];

void ETHManageTX(ETHManageChannel channel, const void *buffer, rt_uint16_t size)
{
    rt_uint16_t r_size = 0;

    r_size = rt_device_write(eth_dev[channel], 0, (void *)buffer, size);
    if(r_size != size)
    {
        LOG_D("eth %d tx error", channel);
    }
}

void ETHManageSetRXCallback(ETHManageChannel ch, rt_err_t (*rx_ind)(rt_device_t dev,rt_size_t size))
{
    if(rx_ind != NULL)
    {
        rt_device_set_rx_indicate(eth_dev[ch], rx_ind);
    }
}

static void ETHManageChannelInit(char *device_name, ETHManageChannel ch)
{
    eth_dev[ch] = rt_device_find(device_name);
    if (eth_dev[ch] == RT_NULL)
    {
        LOG_E("%s find NULL.", device_name);
    }

    if(rt_device_open(eth_dev[ch], RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s open fail.", device_name);
    }

    LOG_I("%s open successful", device_name);
}

#define ETH_MANAGE_THREAD_STACK_SIZE       2048
#define ETH_MANAGE_THREAD_PRIORITY         20
#define ETH_MANAGE_THREAD_TIMESLICE        5

static uint8_t Eth1_Rxbuf[1500];

static uint8_t Eth2_Txbuf[1500];
static uint8_t Eth2_Rxbuf[1500];

const uint8_t RecordBoard_ETHMAC1[6]= {0xfc,0x3f,0xab,0x23,0x00,0x39};
const uint8_t RecordBoard_ETHMAC2[6]= {0xfc,0x3f,0xab,0x00,0x23,0x00};
const uint8_t RecordBoard_ETHMAC3[6]= {0xfc,0x3f,0xab,0x0f,0x00,0x23};

static rt_err_t ETHRXChannel1Callback(rt_device_t dev, rt_size_t size)
{
    if(size != 0)
    {
        if(dev == eth_dev[ETHManageChannel1])
        {
            LOG_I("e0:");
            LOG_I("rx size %d", size);
            rt_device_read(dev, 0, (void *)Eth1_Rxbuf, size);
            LOG_I("des mac %x %x %x %x %x %x ",
                    Eth1_Rxbuf[0], Eth1_Rxbuf[1], Eth1_Rxbuf[2], Eth1_Rxbuf[3], Eth1_Rxbuf[4], Eth1_Rxbuf[5]);
            LOG_I("src mac %x %x %x %x %x %x ",
                    Eth1_Rxbuf[6], Eth1_Rxbuf[7], Eth1_Rxbuf[8], Eth1_Rxbuf[9], Eth1_Rxbuf[10], Eth1_Rxbuf[11]);
            LOG_I("recv data %x %x %x %x %x %x ",
                    Eth1_Rxbuf[74], Eth1_Rxbuf[75], Eth1_Rxbuf[76], Eth1_Rxbuf[77], Eth1_Rxbuf[79], Eth1_Rxbuf[79]);
        }
    }
}

static void ETHManageTestThreadEntry(void *arg)
{
    ETHManageChannelInit("e1", ETHManageChannel2);

    ETHManageSetRXCallback(ETHManageChannel2, ETHRXChannel1Callback);

    memcpy(Eth2_Txbuf,RecordBoard_ETHMAC1,sizeof(RecordBoard_ETHMAC1));
    memcpy(&Eth2_Txbuf[6],RecordBoard_ETHMAC2,sizeof(RecordBoard_ETHMAC2));
    memset(&Eth2_Txbuf[12],0xaa,1024);
    Eth2_Txbuf[60] = 0;
    while(1)
    {
        ETHManageTX(ETHManageChannel2, (const void *)Eth2_Txbuf, 80);
        Eth2_Txbuf[60]++;
        if(Eth2_Txbuf[60] >= 255)
        {
            Eth2_Txbuf[60] = 0;
        }
//        rt_thread_mdelay(10);
        rt_thread_mdelay(1000);
    }
}

void ETHManageTestThreadInit(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("eth_manage_test", ETHManageTestThreadEntry, RT_NULL,
            ETH_MANAGE_THREAD_STACK_SIZE, ETH_MANAGE_THREAD_PRIORITY, ETH_MANAGE_THREAD_TIMESLICE);
    if(tid != NULL)
    {
        rt_thread_startup(tid);
    }
}

