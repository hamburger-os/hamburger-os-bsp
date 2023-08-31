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
    struct rt_semaphore rx_sem;
    uint8_t thread_is_run;
} s_mcp2517fd_test;

static s_mcp2517fd_test mcp2517fd_test[] = {
    {
        .device_name = "mcp2517fd1",
        .thread_is_run = 1,
    },
    {
        .device_name = "mcp2517fd2",
        .thread_is_run = 1,
    },
};

static rt_thread_t mcp2517fd_thread = RT_NULL;

/* 接收数据回调函数 */
static rt_err_t mcp2517fd_can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&mcp2517fd_test[0].rx_sem);

    return RT_EOK;
}

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

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(mcp2517_device->dev, mcp2517fd_can_rx_call);

    /* 初始化 CAN 接收信号量 */
    rt_sem_init(&mcp2517_device->rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
}

static void MCP2517FDTestThreadEntry(void *arg)
{
    uint8_t i = 0;
    rt_err_t result = RT_EOK;
    struct rt_can_msg rxmsg = {0};

//    for(i = 0; i < sizeof(mcp2517fd_test) / sizeof(s_mcp2517fd_test); i++)
//    {
//        MCP2517FDTestOpen(&mcp2517fd_test[i]);
//    }

    MCP2517FDTestOpen(&mcp2517fd_test[0]);

    while(mcp2517fd_test[0].thread_is_run)
    {
        /* 阻塞等待接收信号量 */
        result = rt_sem_take(&mcp2517fd_test[0].rx_sem, 1000);
        if (result == RT_EOK)
        {
            /* 从 CAN 读取一帧数据 */
            if (rt_device_read(mcp2517fd_test[0].dev, 0, &rxmsg, sizeof(struct rt_can_msg)) == sizeof(rxmsg))
            {
                LOG_D("read %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            }
            else
            {
                LOG_E("read %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            }
            /* echo写回 */
            if (rt_device_write(mcp2517fd_test[0].dev, 0, &rxmsg, sizeof(rxmsg)) == sizeof(rxmsg))
            {
                LOG_D("write %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
                LOG_HEX("write", 16, rxmsg.data, 8);
            }
            else
            {
                LOG_E("write %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            }
        }
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
                    mcp2517fd_test[0].thread_is_run = 1;
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
            mcp2517fd_test[0].thread_is_run = 0;
            rt_sem_detach(&mcp2517fd_test[0].rx_sem);
            rt_device_close(mcp2517fd_test[0].dev);
            LOG_D("mcp2157fd test stop");
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
