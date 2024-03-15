/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-22     lvhan       the first version
 */

#include "board.h"
#include <rtthread.h>

#define DBG_TAG "app.uart"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define UART_MAX_LEN 2048

struct _uart_test
{
    rt_device_t                 uart_dev;
    rt_thread_t                 uart_thread;
    rt_mq_t                     uart_mq;
    int                         thread_is_run;
    rt_uint32_t                 parity_bit;
    rt_uint32_t                 baud;
};
static struct _uart_test uart_test = {
    .thread_is_run = 0,
};

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

/* 接收数据回调函数 */
static rt_err_t uart_rx_call(rt_device_t dev, rt_size_t size)
{
    rt_err_t result = RT_EOK;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(uart_test.uart_mq, &msg, sizeof(msg));
#ifdef ULOG_USING_ISR_LOG
    if (result != RT_EOK)
    {
        LOG_E("uart rx mq send error %d", result);
    }
#endif
    return result;
}

static void uart_thread_entry(void *parameter)
{
    struct _uart_test *ptest = (struct _uart_test *)parameter;

    struct rx_msg msg;
    rt_size_t length_r, length_w;
    rt_err_t result = RT_EOK;
    uint8_t buffer[UART_MAX_LEN];

    while (ptest->thread_is_run)
    {
        rt_memset(&msg, 0, sizeof(msg));
        rt_memset(buffer, 0, sizeof(buffer));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(ptest->uart_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        //当正确接受消息队列代表驱动送回接收信号
        if (result == RT_EOK)
        {
            msg.size = (msg.size > UART_MAX_LEN)?(UART_MAX_LEN):(msg.size);
            /* 从串口读取数据 */
            length_r = rt_device_read(ptest->uart_dev, 0, buffer, UART_MAX_LEN);

            LOG_D("rd : (%d %d)", msg.size, length_r);
            LOG_HEX("rd", 16, buffer, length_r);
            /* echo 回显测试 */
            if (length_r > 0)
            {
                length_w = rt_device_write(ptest->uart_dev, 0, buffer, length_r);
                if (length_w != length_r)
                {
                    LOG_E("wr error %d/%d.", length_w, length_r);
                }
                else
                {
                    LOG_D("wr : (%d %d)", length_w, length_r);
                    LOG_HEX("wr", 16, buffer, length_w);
                }
            }
        }
    }
}

static void uart_echo_test(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: uarttest [cmd]\n");
        rt_kprintf("       uarttest --probe [dev_name]\n");
        rt_kprintf("       uarttest --baud [baud, e.g 115200]\n");
        rt_kprintf("       uarttest --parity [parity bit, e.g odd, even, none]\n");
        rt_kprintf("       uarttest --start\n");
        rt_kprintf("       uarttest --write [data]\n");
        rt_kprintf("       uarttest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--probe") == 0)
        {
            /* 查找 uart 设备 */
            uart_test.uart_dev = rt_device_find(argv[2]);
            uart_test.baud = 115200;
            uart_test.parity_bit = PARITY_NONE;
            if (uart_test.uart_dev != NULL)
            {
                struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
                /* step2：修改串口配置参数 */
                config.baud_rate = 115200;        // 修改波特率为 115200
                config.data_bits = DATA_BITS_8;             // 数据位 8
                config.stop_bits = STOP_BITS_1;             // 停止位 1
                config.parity    = PARITY_NONE;             // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
                config.rx_bufsz  = UART_MAX_LEN;
                config.tx_bufsz  = UART_MAX_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
                config.bufsz     = UART_MAX_LEN;
#endif

                /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
                rt_device_control(uart_test.uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
            }
            else
            {
                LOG_E("find '%s' error!", argv[2]);
            }
        }
        else if (rt_strcmp(argv[1], "--baud") == 0)
        {
            if (uart_test.uart_dev != NULL)
            {
                uart_test.baud = strtoul(argv[2], NULL, 10);

                struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
                /* step2：修改串口配置参数 */
                config.baud_rate = uart_test.baud;
                config.data_bits = DATA_BITS_8;             // 数据位 8
                config.stop_bits = STOP_BITS_1;             // 停止位 1
                config.parity    = uart_test.parity_bit;             // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
                config.rx_bufsz  = UART_MAX_LEN;
                config.tx_bufsz  = UART_MAX_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
                config.bufsz     = UART_MAX_LEN;
#endif

                /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
                rt_device_control(uart_test.uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
            }
            else
            {
                LOG_E("device does not exist!");
            }
        }
        else if ((rt_strcmp(argv[1], "--parity") == 0) && ((rt_strcmp(argv[2], "none") == 0) || (rt_strcmp(argv[2], "odd") == 0) || (rt_strcmp(argv[2], "even") == 0)))
        {
            if (uart_test.uart_dev != NULL)
            {
                if(rt_strcmp(argv[2], "odd") == 0)
                {
                    uart_test.parity_bit = PARITY_ODD;
                }
                else if(rt_strcmp(argv[2], "even") == 0)
                {
                    uart_test.parity_bit = PARITY_EVEN;
                }
                else
                {
                    uart_test.parity_bit = PARITY_NONE;
                }

                struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
                /* step2：修改串口配置参数 */
                config.baud_rate = uart_test.baud;
                config.data_bits = DATA_BITS_8;             // 数据位 8
                config.stop_bits = STOP_BITS_1;             // 停止位 1
                config.parity    = uart_test.parity_bit;    // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
                config.rx_bufsz  = UART_MAX_LEN;
                config.tx_bufsz  = UART_MAX_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
                config.bufsz     = UART_MAX_LEN;
#endif

                /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
                rt_device_control(uart_test.uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
            }
            else
            {
                LOG_E("device does not exist!");
            }
        }
        else if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (uart_test.uart_dev != NULL)
            {
                /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
                if (rt_device_open(uart_test.uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
                if (rt_device_open(uart_test.uart_dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
#endif
                {
                    LOG_E("open '%s' error!", argv[2]);
                }
                /* 设置接收回调函数 */
                rt_device_set_rx_indicate(uart_test.uart_dev, uart_rx_call);
                /* 初始化 uart 接收消息队列 */
                if (uart_test.uart_mq == RT_NULL)
                {
                    uart_test.uart_mq = rt_mq_create("uart", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);
                }
            }
            else
            {
                LOG_E("device does not exist!");
            }
            if (uart_test.uart_thread == RT_NULL)
            {
                /* 创建 app 线程 */
                uart_test.uart_thread = rt_thread_create("uart", uart_thread_entry, &uart_test, 4096, 26, 10);
                /* 创建成功则启动线程 */
                if (uart_test.uart_thread != RT_NULL)
                {
                    rt_thread_startup(uart_test.uart_thread);
                    uart_test.thread_is_run = 1;
                }
                else
                {
                    LOG_E("thread create error!");
                }
            }
        }
        else if (rt_strcmp(argv[1], "--write") == 0)
        {
            if (uart_test.uart_dev != RT_NULL)
            {
                int data_len = rt_strlen(argv[2]);
                int write_len = 0;
                write_len = rt_device_write(uart_test.uart_dev, 0, argv[2], data_len);
                if (write_len != data_len)
                {
                    LOG_E("write error %d/%d\n", write_len, data_len);
                }
                else
                {
                    LOG_D("write : (%d)%s\n", write_len, argv[2]);
                }
            }
            else
            {
                LOG_E("device does not exist!");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            rt_device_close(uart_test.uart_dev);
        }
        else
        {
            rt_kprintf("Usage: uarttest [cmd]\n");
            rt_kprintf("       uarttest --probe [dev_name]\n");
            rt_kprintf("       uarttest --baud [baud, e.g 115200]\n");
            rt_kprintf("       uarttest --parity [parity bit, e.g odd, even, none]\n");
            rt_kprintf("       uarttest --start\n");
            rt_kprintf("       uarttest --write [data]\n");
            rt_kprintf("       uarttest --stop\n");
        }
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(uart_echo_test, uarttest, uart echo test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
