/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-24     zm       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "mcp2517fd test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define MCP2517FD_THREAD_STACK_SIZE       2048
#define MCP2517FD_THREAD_PRIORITY         19
#define MCP2517FD_THREAD_TIMESLICE        5

typedef struct {
    rt_device_t dev;
    const char *device_name;
} s_mcp2517fd_test;

static s_mcp2517fd_test mcp2517fd_test[] = {
    {
        .device_name = "mcp2517fd1",
    },
    {
        .device_name = "mcp2517fd2",
    },
};

static rt_thread_t mcp2517fd_thread = RT_NULL;

static void MCP2517FDTestOpen(s_mcp2517fd_test *mcp2517_device)
{
    mcp2517_device->dev = rt_device_find(mcp2517_device->device_name);
    if (RT_NULL == mcp2517_device->dev)
    {
        LOG_E("%s find NULL", mcp2517_device->device_name);
        return;
    }

    if(rt_device_open(mcp2517_device->dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s open fail", mcp2517_device->device_name);
        return;
    }

    LOG_I("%s open successful", mcp2517_device->device_name);
}

static void MCP2517FDTestThreadEntry(void *arg)
{
    uint8_t i = 0;
    struct rt_can_msg tx_msg = {0};
    struct rt_can_msg rx_msg = {0};
    rt_size_t size = 0;

//    for(i = 0; i < sizeof(mcp2517fd_test) / sizeof(s_mcp2517fd_test); i++)
//    {
//        MCP2517FDTestOpen(&mcp2517fd_test[i]);
//    }

    MCP2517FDTestOpen(&mcp2517fd_test[0]);

    tx_msg.id = 0x00031001;
    tx_msg.ide = RT_CAN_EXTID;
    tx_msg.rtr = RT_CAN_DTR;
    tx_msg.len = sizeof(tx_msg.data);
    memset(tx_msg.data, 0x11, sizeof(tx_msg.data));

    while(1)
    {
#if 0
        size = rt_device_write(mcp2517fd_test[0].dev, 0, &tx_msg, sizeof(tx_msg));
#else
        size = rt_device_read(mcp2517fd_test[0].dev, 0, &rx_msg, sizeof(struct rt_can_msg));
        if(sizeof(struct rt_can_msg) == size)
        {
            LOG_I("rx size %d, data %x %x %x %x %x %x %x %x len = %d",
                        size, rx_msg.data[0],rx_msg.data[1], rx_msg.data[2],rx_msg.data[3],
                        rx_msg.data[4],rx_msg.data[5], rx_msg.data[6],rx_msg.data[7], rx_msg.len);
        }
        memset(&rx_msg, 0x00, sizeof(struct rt_can_msg));
#endif
        rt_thread_mdelay(100);
    }
}

static void MCP2517FDTest(int argc, char **argv)
{

    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: mcp2517fdtest [cmd]\n");
        rt_kprintf("       mcp2517fdtest --start\n");
        rt_kprintf("       mcp2517fdtest --stop\n");
    }
    else
    {
        if(rt_strcmp(argv[1], "--start") == 0)
        {
            if (mcp2517fd_thread == RT_NULL)
            {
                mcp2517fd_thread = rt_thread_create("mcp2517fd test", MCP2517FDTestThreadEntry, RT_NULL,
                                                    MCP2517FD_THREAD_STACK_SIZE, MCP2517FD_THREAD_PRIORITY, MCP2517FD_THREAD_TIMESLICE);
                if(mcp2517fd_thread != NULL)
                {
                    rt_thread_startup(mcp2517fd_thread);
                }
                else
                {
                    LOG_E("thread create error!");
                }
            }
            else
            {
                LOG_W("thread already exists!");
            }
        }
        else if(rt_strcmp(argv[1], "--stop") == 0)
        {
            if(mcp2517fd_thread != RT_NULL)
            {
                if(rt_thread_delete(mcp2517fd_thread) == RT_EOK)
                {
                    LOG_I("thread delete ok!");
                }
                else
                {
                    LOG_E("thread delete error!");
                }
            }
            else
            {
                LOG_W("thread already delete!");
            }
            mcp2517fd_thread = RT_NULL;
        }
        else
        {
            rt_kprintf("Usage: mcp2517fdtest [cmd]\n");
            rt_kprintf("       mcp2517fdtest --start\n");
            rt_kprintf("       mcp2517fdtest --stop\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(MCP2517FDTest, mcp2517fdtest, macp2517fd test);
#endif /* RT_USING_FINSH */
