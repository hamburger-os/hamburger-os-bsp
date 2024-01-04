/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-24     myshow       the first version
 */
#include <rtthread.h>

#include <TinyFrame.h>

typedef struct
{
    rt_device_t serial;
    struct rt_semaphore rx_sem;
    char uart_name[PKG_TINYFRAME_UART_NAME_MAX_LEN];
    int enable_rx;
    int enable_rx_cb;
    void (*rx_cb)(uint8_t data);
} TinyFramUserData;

static void tf_thread_entry(void *parameter)
{
    TinyFrame *tf = (TinyFrame *)parameter;
    TinyFramUserData *tfu = (TinyFramUserData *)tf->userdata;

    while (1)
    {
        char ch;
        while (rt_device_read(tfu->serial, -1, &ch, 1) != 1)
        {
            rt_sem_take(&tfu->rx_sem, RT_WAITING_FOREVER);
        }
        // rt_kprintf("%c", ch);

        if (tfu->enable_rx)
        {
            TF_Accept(tf, &ch, 1);
        }
        if (tfu->enable_rx_cb && tfu->rx_cb != NULL)
        {
            tfu->rx_cb(ch);
        }
        // TF_Tick(tf);
    }
}

static int _tf_uart_init(TinyFrame *tf, TinyFramUserData *tfu, char *uart_name, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{

    tf->userdata = tfu;
    /* init sem */

    char buf[PKG_TINYFRAME_UART_NAME_MAX_LEN + 6] = {0};
    char sem_name[] = "_rx_sem";
    rt_memcpy(buf, uart_name, strlen(uart_name) + 1);
    strcat(buf, sem_name);
    rt_kprintf("sem name:%s\n", buf);
    rt_sem_init(&tfu->rx_sem, buf, 0, RT_IPC_FLAG_FIFO);

    /* init uart */
    int uart_len = strlen(uart_name) + 1;

    RT_ASSERT(uart_len < PKG_TINYFRAME_UART_NAME_MAX_LEN);
    rt_memcpy(tfu->uart_name, uart_name, uart_len);
    tfu->serial = rt_device_find(tfu->uart_name);

    RT_ASSERT(tfu->serial != RT_NULL);

    rt_device_open(tfu->serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);

    rt_device_set_rx_indicate(tfu->serial, rx_ind);

    /* creat thread */
    rt_thread_t thread = rt_thread_create("serial", tf_thread_entry, tf, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    return 0;
}
