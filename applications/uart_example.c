/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-05     lvhan       the first version
 */
#include "board.h"
#include <rtthread.h>

#define DBG_TAG "app.uart"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DEV_NAME BSP_UART_EXAMPLE_DEVNAME

#define BAUD_CONFIG         BAUD_RATE_115200    //波特率
#define PARITY_CONFIG       PARITY_NONE         //0无校验1奇校验2偶校验
#define DATA_BITS_CONFIG    DATA_BITS_8         //数据位
#define STOP_BITS_CONFIG    STOP_BITS_1         //停止位

#define BUFFER_LEN 256

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

static rt_device_t  uart_dev = RT_NULL;
static rt_thread_t  uart_thread = RT_NULL;
static rt_mq_t      uart_mq = RT_NULL;
static int thread_is_run = 0;

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(uart_mq, &msg, sizeof(msg));
#ifdef ULOG_USING_ISR_LOG
    if (result != RT_EOK)
    {
        LOG_E("uart input mq send error %d", result);
    }
#endif
    return result;
}

static void uart_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_size_t length_r, length_w;
    uint8_t rx_buffer[BUFFER_LEN];

    /* step1：查找串口设备 */
    uart_dev = rt_device_find(DEV_NAME);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_CONFIG;         // 修改波特率为 115200
    config.data_bits = DATA_BITS_CONFIG;    // 数据位 8
    config.stop_bits = STOP_BITS_CONFIG;    // 停止位 1
    config.parity    = PARITY_CONFIG;       // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
    config.rx_bufsz  = BUFFER_LEN;
    config.tx_bufsz  = BUFFER_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
    config.bufsz     = BUFFER_LEN;
#endif

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
    if (rt_device_open(uart_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
    {
        goto cleanup;
    }
    LOG_D("open uart '%s' sucessful!", DEV_NAME);

    /* 初始化消息队列 */
    uart_mq = rt_mq_create("uart", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(uart_dev, uart_input);

    while (thread_is_run)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(uart_mq, &msg, sizeof(msg), 1000);
        if (result == RT_EOK)
        {
            if (msg.size > 0)
            {
                rt_memset(rx_buffer, 0, BUFFER_LEN);
                /* 从串口读取数据 */
                length_r = rt_device_read(uart_dev, 0, rx_buffer, msg.size);
                if (length_r > 0)
                {
                    LOG_D("read : (%d)%s", length_r, rx_buffer);
                    /* echo 回显测试 */
                    length_w = rt_device_write(uart_dev, 0, rx_buffer, length_r);
                    if (length_w != length_r)
                    {
                        LOG_E("write error %d/%d.", length_w, length_r);
                        goto cleanup;
                    }
                    else
                    {
                        LOG_D("write : (%d)%s", length_w, rx_buffer);
                    }
                }
            }
        }
    }

cleanup:
    uart_thread = RT_NULL;
    rt_device_close(uart_dev);
    LOG_D("thread exited!");
}

static void uart_echo_test(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: uartechotest [cmd]\n");
        rt_kprintf("       uartechotest --start\n");
        rt_kprintf("       uartechotest --write [data]\n");
        rt_kprintf("       uartechotest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (uart_thread == RT_NULL)
            {
                /* 创建 app 线程 */
                uart_thread = rt_thread_create("uart", uart_thread_entry, NULL, 2048, 26, 10);
                /* 创建成功则启动线程 */
                if (uart_thread != RT_NULL)
                {
                    rt_thread_startup(uart_thread);
                    thread_is_run = 1;
                }
                else
                {
                    rt_kprintf("rt_thread_create error %d!\n", uart_thread);
                }
            }
            else
            {
                rt_kprintf("thread already exists!\n");
            }
        }
        else if (rt_strcmp(argv[1], "--write") == 0)
        {
            if (uart_dev != RT_NULL)
            {
                int data_len = rt_strlen(argv[2]);
                int write_len = 0;
                write_len = rt_device_write(uart_dev, 0, argv[2], data_len);
                if (write_len != data_len)
                {
                    rt_kprintf("write error %d/%d\n", write_len, data_len);
                }
                else
                {
                    rt_kprintf("write : (%d)%s\n", write_len, argv[2]);
                }
            }
            else
            {
                rt_kprintf("device does not exist!\n");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            uart_thread = RT_NULL;
            thread_is_run = 0;
        }
        else
        {
            rt_kprintf("cmd does not exist!\n");
            rt_kprintf("Usage: uartechotest [cmd]\n");
            rt_kprintf("       uartechotest --start\n");
            rt_kprintf("       uartechotest --write [data]\n");
            rt_kprintf("       uartechotest --stop\n");
        }
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(uart_echo_test, uartechotest, uart echo test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
