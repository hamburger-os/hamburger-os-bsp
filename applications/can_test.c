/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     lvhan       the first version
 */

#include <stdlib.h>
#include "board.h"
#include <rtthread.h>

#define DBG_TAG "app.can"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct _can_test
{
    rt_device_t              can_dev;
    rt_thread_t              can_thread;
    rt_thread_t              can_thread_tx;
    struct rt_semaphore      rx_sem;
    struct rt_messagequeue   *tx_mq;
    int                      thread_is_run;
};
static struct _can_test can_test = {
    .thread_is_run = 0,
};

/* 接收数据回调函数 */
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&can_test.rx_sem);

    return RT_EOK;
}

static void can_tx_thread_entry(void *parameter)
{
    struct _can_test *ptest = (struct _can_test *)parameter;

    rt_err_t result = RT_EOK;
    struct rt_can_msg rxmsg = {0};

    LOG_D("thread tx startup...");
    while (ptest->thread_is_run)
    {
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
        rxmsg.hdr_index = -1;
#else
        rxmsg.hdr = -1;
#endif
        /* 阻塞等待接收信号量 */
        result = rt_mq_recv(ptest->tx_mq, &rxmsg, sizeof(rxmsg), 1000);
        if (result == RT_EOK)
        {
            /* echo写回 */
            if (rt_device_write(ptest->can_dev, 0, &rxmsg, sizeof(rxmsg)) == sizeof(rxmsg))
            {
                LOG_D("write %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
                LOG_HEX("write", 16, rxmsg.data, 8);
            }
            else
            {
                LOG_E("write %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            }
        }
    }

    rt_sem_detach(&ptest->rx_sem);
    rt_mq_delete(ptest->tx_mq);
    rt_device_close(ptest->can_dev);
    LOG_D("thread exited!");
}

static void can_thread_entry(void *parameter)
{
    struct _can_test *ptest = (struct _can_test *)parameter;

    rt_err_t result = RT_EOK;
    struct rt_can_msg rxmsg = {0};

    LOG_D("thread startup...");
    while (ptest->thread_is_run)
    {
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
        rxmsg.hdr_index = -1;
#else
        rxmsg.hdr = -1;
#endif
        /* 阻塞等待接收信号量 */
        result = rt_sem_take(&ptest->rx_sem, 1000);
        if (result == RT_EOK)
        {
            /* 从 CAN 读取一帧数据 */
            if (rt_device_read(ptest->can_dev, 0, &rxmsg, sizeof(rxmsg)) == sizeof(rxmsg))
            {
                LOG_D("read %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
                LOG_HEX("read", 16, rxmsg.data, 8);
            }
            else
            {
                LOG_E("read %x %d %d %d", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            }
            /* echo发送至消息队列 */
            result = rt_mq_send(ptest->tx_mq, &rxmsg, sizeof(rxmsg));
            if (result != RT_EOK)
            {
                LOG_E("mq send error!");
            }
        }
    }

    rt_sem_detach(&ptest->rx_sem);
    rt_mq_delete(ptest->tx_mq);
    rt_device_close(ptest->can_dev);
    LOG_D("thread exited!");
}

static void can_echo_test(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: cantest [cmd]\n");
        rt_kprintf("       cantest --probe [dev_name]\n");
        rt_kprintf("       cantest --baud [baud, unit:k, e.g 500]\n");
        rt_kprintf("       cantest --start\n");
        rt_kprintf("       cantest --write [data]\n");
        rt_kprintf("       cantest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--probe") == 0)
        {
            /* 查找 CAN 设备 */
            can_test.can_dev = rt_device_find(argv[2]);
            if (can_test.can_dev != NULL)
            {
                /* 设置 CAN 通信的默认波特率 */
                if (rt_device_control(can_test.can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud) != RT_EOK)
                {
                    LOG_E("set baud error!");
                }
                /* 设置 CAN 的工作模式为正常工作模式 */
                if (rt_device_control(can_test.can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL) != RT_EOK)
                {
                    LOG_E("set mode error!");
                }
            }
            else
            {
                LOG_E("find '%s' error!", argv[2]);
            }
        }
        else if (rt_strcmp(argv[1], "--baud") == 0)
        {
            if (can_test.can_dev != NULL)
            {
                uint32_t baud = strtoul(argv[2], NULL, 10) * 1000;
                /* 设置 CAN 通信的波特率 */
                if (rt_device_control(can_test.can_dev, RT_CAN_CMD_SET_BAUD, (void *)baud) != RT_EOK)
                {
                    LOG_E("baud %d error!", baud);
                }
            }
            else
            {
                LOG_E("device does not exist!");
            }
        }
        else if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (can_test.can_dev != NULL)
            {
                /* 以中断接收及发送方式打开 CAN 设备 */
                if (rt_device_open(can_test.can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
                {
                    LOG_E("open '%s' error!", can_test.can_dev->parent.name);
                }
                /* 设置接收回调函数 */
                rt_device_set_rx_indicate(can_test.can_dev, can_rx_call);
                /* 初始化 CAN 接收信号量 */
                rt_sem_init(&can_test.rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
                /* 初始化消息队列 */
                can_test.tx_mq = rt_mq_create("tx_mq", sizeof(struct rt_can_msg), sizeof(struct rt_can_msg)*128, RT_IPC_FLAG_FIFO);
            }
            else
            {
                LOG_E("device does not exist!");
            }
            if (can_test.can_thread == RT_NULL)
            {
                /* 创建 app 线程 */
                can_test.can_thread = rt_thread_create("can", can_thread_entry, &can_test, 2048, 26, 10);
                /* 创建成功则启动线程 */
                if (can_test.can_thread != RT_NULL)
                {
                    rt_thread_startup(can_test.can_thread);
                    can_test.thread_is_run = 1;
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
            if (can_test.can_thread_tx == RT_NULL)
            {
                /* 创建 app 线程 */
                can_test.can_thread_tx = rt_thread_create("can_tx", can_tx_thread_entry, &can_test, 2048, 27, 10);
                /* 创建成功则启动线程 */
                if (can_test.can_thread_tx != RT_NULL)
                {
                    rt_thread_startup(can_test.can_thread_tx);
                    can_test.thread_is_run = 1;
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
        else if (rt_strcmp(argv[1], "--write") == 0)
        {
            if (can_test.can_dev != RT_NULL)
            {
                struct rt_can_msg txmsg = {0};

                txmsg.id = 0x78;              /* ID 为 0x78 */
                txmsg.ide = RT_CAN_STDID;     /* 标准格式 */
                txmsg.rtr = RT_CAN_DTR;       /* 数据帧 */
                txmsg.len = 8;                /* 数据长度为 8 */
                /* 待发送的 8 字节数据 */
                txmsg.data[0] = 0x00;
                txmsg.data[1] = 0x11;
                txmsg.data[2] = 0x22;
                txmsg.data[3] = 0x33;
                txmsg.data[4] = 0x44;
                txmsg.data[5] = 0x55;
                txmsg.data[6] = 0x66;
                txmsg.data[7] = 0x77;
                /* 发送一帧 CAN 数据 */
                if (rt_device_write(can_test.can_dev, 0, &txmsg, sizeof(txmsg)) == 0)
                {
                    LOG_E("write error.");
                }
                else
                {
                    LOG_D("write %x %d %d %d", txmsg.id, txmsg.ide, txmsg.rtr, txmsg.len);
                    LOG_HEX("write", 16, txmsg.data, 8);
                }
            }
            else
            {
                LOG_E("device does not exist!");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            can_test.can_thread = RT_NULL;
            can_test.can_thread_tx = RT_NULL;
            can_test.thread_is_run = 0;
        }
        else
        {
            rt_kprintf("Usage: cantest [cmd]\n");
            rt_kprintf("       cantest --probe [dev_name]\n");
            rt_kprintf("       cantest --baud [baud, unit:k, e.g 500]\n");
            rt_kprintf("       cantest --start\n");
            rt_kprintf("       cantest --write [data]\n");
            rt_kprintf("       cantest --stop\n");
        }
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(can_echo_test, cantest, can echo test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
