/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-27     zm       the first version
 */
#include "sto_record_board.h"
#include "led_ctrl.h"
#include "eth_manage.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "STORecordBoard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_thread record_board_main;

#define MAIN_THREAD_PRIORITY         25
#define MAIN_THREAD_STACK_SIZE       512
#define MAIN_THREAD_TIMESLICE        5

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t main_thread_stack[MAIN_THREAD_STACK_SIZE];

#if 0
rt_device_t eth_dev[3];
uint8_t Eth1_Rxbuf[1500];
static rt_err_t ETHRXCallback(rt_device_t dev, rt_size_t size)
{
    rt_device_read(dev, 0, (void *)Eth1_Rxbuf, 80);
    if(dev == eth_dev[0])
    {
        LOG_I("e0:");
    }
    else if(dev == eth_dev[1])
    {
        LOG_I("e1:");
    }
    LOG_I("des mac %x %x %x %x %x %x ",
            Eth1_Rxbuf[0], Eth1_Rxbuf[1], Eth1_Rxbuf[2], Eth1_Rxbuf[3], Eth1_Rxbuf[4], Eth1_Rxbuf[5]);
    LOG_I("src mac2 %x %x %x %x %x %x ",
            Eth1_Rxbuf[6], Eth1_Rxbuf[7], Eth1_Rxbuf[8], Eth1_Rxbuf[9], Eth1_Rxbuf[10], Eth1_Rxbuf[11]);
}

static void ETHDeviceInit(char *device_name, rt_uint8_t eth_id)
{
    eth_dev[eth_id] = rt_device_find(device_name);
    if (eth_dev[eth_id] == RT_NULL)
    {
        LOG_E("%s find NULL.", device_name);
    }

    if(rt_device_open(eth_dev[eth_id], RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s open fail.", device_name);
    }

    LOG_I("%s open successful", device_name);

    rt_device_set_rx_indicate(eth_dev[eth_id], ETHRXCallback);
}
uint8_t Eth1_Txbuf[1500];
const uint8_t RecordBoard_ETHMAC1[6]= {0xfc,0x3f,0xab,0x23,0x00,0x39};
const uint8_t RecordBoard_ETHMAC2[6]= {0xfc,0x3f,0xab,0x00,0x23,0x00};
const uint8_t RecordBoard_ETHMAC3[6]= {0xfc,0x3f,0xab,0x0f,0x00,0x23};
static void *STORecordBoardThreadEntry(void *parameter)
{
    LedCtrlInit();
    ETHDeviceInit("e0", 0);
    memset(Eth1_Txbuf, 0, 1500);
    ETHDeviceInit("e1", 1);
    while (1)
    {
        memcpy(Eth1_Txbuf,RecordBoard_ETHMAC2,sizeof(RecordBoard_ETHMAC2));
        memcpy(&Eth1_Txbuf[6],RecordBoard_ETHMAC1,sizeof(RecordBoard_ETHMAC1));
        memset(&Eth1_Txbuf[12],0xcc,80);
        rt_device_write(eth_dev[0], 0, (void *)Eth1_Txbuf, 1024);
        rt_thread_mdelay(500);
    }
}
#endif
static void *STORecordBoardThreadEntry(void *parameter)
{
    LedCtrlInit();
    ETHManageInit();
    ETHManageTestThreadInit();
    while (1)
    {
        rt_thread_mdelay(500);
    }
}
static void STORecordBoardInit(void)
{
    rt_thread_init(&record_board_main, "main thread", STORecordBoardThreadEntry,
    RT_NULL, main_thread_stack, MAIN_THREAD_STACK_SIZE, MAIN_THREAD_PRIORITY, MAIN_THREAD_TIMESLICE);
    /* 启动线程 */
    rt_thread_startup(&record_board_main);
}

INIT_APP_EXPORT(STORecordBoardInit);
