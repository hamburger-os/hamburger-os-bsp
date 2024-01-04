/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-26     lvhan       the first version
 */

#include "board.h"
#include <ipc/ringbuffer.h>

#define DBG_TAG "lora"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define UART_MAX_LEN 256

/* lora dirver class */
struct lora_driver
{
    rt_device_t     dev;

    char *          uart_name;
    rt_device_t     uart_dev;
    rt_thread_t     uart_thread;
    rt_mq_t         uart_mq;
    uint8_t         buffer[BSP_LORA_BUFFER_SIZE];
    struct rt_ringbuffer * rb;
#ifdef BSP_LORA_USING_POWER_PIN
    rt_base_t       power_pin;
#endif

    rt_mutex_t mutex;
    struct rt_completion rx_completion;

    int isThreadRun;
};
static struct lora_driver lora_dev = {
    .uart_name = BSP_LORA_DEVNAME,
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

    result = rt_mq_send(lora_dev.uart_mq, &msg, sizeof(msg));
    LOG_D("rx : (%d)", msg.size);
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
    struct lora_driver *lora = (struct lora_driver *)parameter;

    struct rx_msg msg;
    rt_size_t length_r;
    rt_err_t result = RT_EOK;
    uint8_t buffer[UART_MAX_LEN];

    LOG_D("thread startup...");
    while (lora->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        rt_memset(buffer, 0, sizeof(buffer));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(lora->uart_mq, &msg, sizeof(msg), 100);
        //当正确接受消息队列代表驱动送回接收信号
        if (result == RT_EOK)
        {
            /* 从串口读取数据 */
            length_r = rt_device_read(lora->uart_dev, 0, buffer, msg.size);
            if (length_r > 0)
            {
                rt_ringbuffer_put(lora->rb, buffer, length_r);
            }
        }
        //当消息队列超时说明已经持续一段时间未收到消息
        //这个超时时间取决于波特率和协议的最大长度
        //例如波特率9600，协议最大长度96，这时设置为100
        else if (result == -RT_ETIMEOUT)
        {
            length_r = rt_ringbuffer_data_len(lora->rb);
            if (length_r > 0)
            {
                LOG_D("read : (%d)", length_r);
                rt_completion_done(&lora->rx_completion);
            }
        }
    }

    rt_mq_delete(lora->uart_mq);
    rt_device_close(lora->uart_dev);
    LOG_D("thread exited!");
}

static int8_t lora_AT_cmd(const char *cmd)
{
    rt_size_t length_r;
    rt_memset(lora_dev.buffer, 0, BSP_LORA_BUFFER_SIZE);

    rt_thread_mdelay(20);
    LOG_D("write : (%d)%s", rt_strlen(cmd), (char *)cmd);
    rt_device_write(lora_dev.uart_dev, 0, cmd, rt_strlen(cmd));
    rt_thread_mdelay(20);

    if (rt_completion_wait(&lora_dev.rx_completion, UART_MAX_LEN) != RT_EOK)
    {
        LOG_E("rx comple timeout!");
    }
    length_r = rt_ringbuffer_data_len(lora_dev.rb);
    rt_ringbuffer_get(lora_dev.rb, lora_dev.buffer, length_r);

    length_r = rt_device_read(lora_dev.uart_dev, 0, lora_dev.buffer, UART_MAX_LEN);
    LOG_D("read : (%d)%s", length_r, (char *)lora_dev.buffer);
    LOG_HEX("rd", 16, lora_dev.buffer, length_r);

    return 0;
}

static int rt_lora_init(void)
{
#ifdef BSP_LORA_USING_POWER_PIN
    lora_dev.power_pin = rt_pin_get(BSP_LORA_POWER_PIN);
    rt_pin_mode(lora_dev.power_pin, PIN_MODE_OUTPUT);
    rt_pin_write(lora_dev.power_pin, PIN_LOW);
#endif

    /* step1：查找 uart 设备 */
    lora_dev.uart_dev = rt_device_find(lora_dev.uart_name);
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BSP_LORA_RATE;           // 修改波特率
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
    rt_device_control(lora_dev.uart_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(lora_dev.uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
    if (rt_device_open(lora_dev.uart_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
    {
        LOG_E("open '%s' error!", lora_dev.uart_name);
    }

    /* 初始化完成量 */
    rt_completion_init(&lora_dev.rx_completion);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(lora_dev.uart_dev, uart_rx_call);
    /* 初始化 uart 接收消息队列 */
    lora_dev.uart_mq = rt_mq_create("lora", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);
    /* 创建环形缓冲区 */
    lora_dev.rb = rt_ringbuffer_create(UART_MAX_LEN);
    /* 创建 app 线程 */
    lora_dev.uart_thread = rt_thread_create("lora", uart_thread_entry, &lora_dev, 2048, 14, 10);
    /* 创建成功则启动线程 */
    if (lora_dev.uart_thread != RT_NULL)
    {
        rt_thread_startup(lora_dev.uart_thread);
        lora_dev.isThreadRun = 1;
    }
    else
    {
        LOG_E("thread create error!");
    }

    rt_thread_mdelay(1000);
    lora_AT_cmd("...\r");

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_COMPONENT_EXPORT(rt_lora_init);
