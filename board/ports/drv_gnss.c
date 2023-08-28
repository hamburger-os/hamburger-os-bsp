/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-11     lvhan       the first version
 */

#include "board.h"
#include <ipc/ringbuffer.h>
#include "lwgps/lwgps.h"

#define DBG_TAG "gnss"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* gnss dirver class */
struct gnss_driver
{
    rt_device_t     dev;

    char *          uart_name;
    rt_device_t     uart_dev;
    rt_thread_t     uart_thread;
    rt_mq_t         uart_mq;
    uint8_t         buffer[BSP_GNSS_BUFFER_SIZE];
    uint8_t         nmea[BSP_GNSS_BUFFER_SIZE];
    struct rt_ringbuffer * rb;
#ifdef BSP_GNSS_USING_POWER_PIN
    rt_base_t       power_pin;
#endif
    lwgps_t hgps;
    rt_mutex_t mutex;

    int isThreadRun;
};
static struct gnss_driver gnss_dev = {
    .uart_name = BSP_GNSS_DEVNAME,
};

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

static void gnss_power_set(rt_base_t onoff)
{
    switch(onoff)
    {
    case 0:
        rt_pin_write(gnss_dev.power_pin, PIN_HIGH);
        break;
    case 1:
        rt_pin_write(gnss_dev.power_pin, PIN_LOW);
        break;
    default:
        break;
    }
}

static rt_size_t uart_buffer_2_nmea(struct rt_ringbuffer * rb, uint8_t *nmea)
{
    rt_size_t i = 0, index = 0;
    rt_size_t buffer_len = 0, nmea_len = 0;
    rt_memset(nmea, 0, BSP_GNSS_BUFFER_SIZE);

    buffer_len = rt_ringbuffer_data_len(rb);
    rt_ringbuffer_get(rb, nmea, buffer_len);
//    LOG_HEX("buffer", 16, nmea, buffer_len);

    for (i = 0; i < buffer_len; i++)
    {
        if (rt_memcmp(&nmea[i], "\r\n", 2) == 0)
        {
            index = i + 2;
        }
    }
    nmea_len = index;
//    LOG_HEX("cur   ", 16, &nmea[index], buffer_len - nmea_len);
    rt_ringbuffer_put(rb, &nmea[index], buffer_len - nmea_len);
    rt_memset(&nmea[index], 0, buffer_len - nmea_len);
    LOG_HEX("nmea  ", 16, nmea, nmea_len);

    return nmea_len;
}

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(gnss_dev.uart_mq, &msg, sizeof(msg));
    return result;
}

static void gnss_thread_entry(void *parameter)
{
    rt_err_t result;
    struct rx_msg msg;
    rt_size_t length_r, length_w;
    struct gnss_driver *pgnss = (struct gnss_driver *)parameter;

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BSP_GNSS_DEFAULT_RATE;
    config.data_bits = DATA_BITS_8;     // 数据位 8
    config.stop_bits = STOP_BITS_1;     // 停止位 1
    config.parity    = PARITY_NONE;     // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
    config.rx_bufsz  = BSP_GNSS_BUFFER_SIZE;
    config.tx_bufsz  = BSP_GNSS_BUFFER_SIZE;
#endif
#ifdef RT_USING_SERIAL_V1
    config.bufsz     = BSP_GNSS_BUFFER_SIZE;
#endif

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(pgnss->uart_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(pgnss->uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
    if (rt_device_open(pgnss->uart_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
    {
        LOG_E("can not open uart '%s'!", pgnss->uart_name);
        return;
    }

    /* 初始化互斥 */
    pgnss->mutex = rt_mutex_create("gnss", RT_IPC_FLAG_PRIO);
    /* 初始化环形缓冲区 */
    pgnss->rb = rt_ringbuffer_create(BSP_GNSS_BUFFER_SIZE);
    /* 初始化消息队列 */
    pgnss->uart_mq = rt_mq_create("gnss", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(pgnss->uart_dev, uart_input);

    LOG_D("startup...");
    while(pgnss->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(pgnss->uart_mq, &msg, sizeof(msg), 1000);
        if (result == RT_EOK)
        {
            if (msg.size > 0)
            {
                rt_memset(pgnss->buffer, 0, sizeof(pgnss->buffer));
                /* 从串口读取数据 */
                length_r = rt_device_read(pgnss->uart_dev, 0, pgnss->buffer, msg.size);
                if (length_r > 0)
                {
                    LOG_D("read  : (%d)", length_r);
                    if (rt_ringbuffer_put(pgnss->rb, pgnss->buffer, length_r) != length_r)
                    {
                        LOG_E("ringbuffer put error.");
                    }
                    length_w = uart_buffer_2_nmea(pgnss->rb, pgnss->nmea);

                    /* Process all input data */
                    rt_mutex_take(pgnss->mutex, RT_WAITING_FOREVER);
                    lwgps_process(&pgnss->hgps, pgnss->nmea, length_w);
                    rt_mutex_release(pgnss->mutex);

                    /* Print messages */
                    LOG_D("[%d %d] %d/%d/%d %d:%d:%d %d.%06d %d.%06d %d.%03d %d.%03d %d.%03d"
                            , pgnss->hgps.is_valid, pgnss->hgps.sats_in_use
                            , pgnss->hgps.year, pgnss->hgps.month, pgnss->hgps.date
                            , pgnss->hgps.hours, pgnss->hgps.minutes, pgnss->hgps.seconds
                            , (int)pgnss->hgps.latitude, (int)((pgnss->hgps.latitude - (int)pgnss->hgps.latitude)*1000000)
                            , (int)pgnss->hgps.longitude, (int)((pgnss->hgps.longitude - (int)pgnss->hgps.longitude)*1000000)
                            , (int)pgnss->hgps.altitude, (int)((pgnss->hgps.altitude - (int)pgnss->hgps.altitude)*1000)
                            , (int)pgnss->hgps.speed, (int)((pgnss->hgps.speed - (int)pgnss->hgps.speed)*1000)
                            , (int)pgnss->hgps.variation, (int)((pgnss->hgps.variation - (int)pgnss->hgps.variation)*1000));
                }
            }
        }
    }

    //关闭串口
    rt_device_close(pgnss->uart_dev);
    //清除互斥
    rt_mutex_delete(pgnss->mutex);
    //清除环形缓冲区
    rt_ringbuffer_destroy(pgnss->rb);
    //清除消息队列
    rt_mq_delete(pgnss->uart_mq);

    LOG_D("end.");
}

static rt_err_t  gnss_init   (rt_device_t dev)
{
    gnss_dev.dev = dev;

#ifdef BSP_GNSS_USING_POWER_PIN
    gnss_dev.power_pin = rt_pin_get(BSP_GNSS_POWER_PIN);
    rt_pin_mode(gnss_dev.power_pin, PIN_MODE_OUTPUT);
    gnss_power_set(0);
#endif

    /* Init GPS */
    lwgps_init(&gnss_dev.hgps);

    /* step1：查找串口设备 */
    gnss_dev.uart_dev = rt_device_find(gnss_dev.uart_name);
    if (gnss_dev.uart_dev == NULL)
    {
        LOG_E("can not find uart '%s'!", gnss_dev.uart_name);
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t  gnss_open   (rt_device_t dev, rt_uint16_t oflag)
{
#ifdef BSP_GNSS_USING_POWER_PIN
    gnss_power_set(1);
#endif

    /* 创建 gnss 线程 */
    gnss_dev.uart_thread = rt_thread_create("gnss", gnss_thread_entry, &gnss_dev, 2048, 14, 10);
    /* 创建成功则启动线程 */
    if (gnss_dev.uart_thread != RT_NULL)
    {
        gnss_dev.isThreadRun = 1;
        rt_thread_startup(gnss_dev.uart_thread);
    }
    else
    {
        LOG_E("can not creat thread!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t  gnss_close  (rt_device_t dev)
{
#ifdef BSP_GNSS_USING_POWER_PIN
    gnss_power_set(0);
#endif

    //结束线程
    gnss_dev.isThreadRun = 0;

    return RT_EOK;
}

static rt_size_t gnss_read   (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_mutex_take(gnss_dev.mutex, RT_WAITING_FOREVER);
//    lwgps_t *pdata = (lwgps_t *)buffer;
//    *pdata = gnss_dev.hgps;
    rt_memcpy(buffer, &gnss_dev.hgps, size);
    rt_mutex_release(gnss_dev.mutex);

    return 0;
}

static int rt_gnss_init(void)
{
    //注册设备
    rt_device_t gnss_dev = rt_malloc(sizeof(struct rt_device));
    if (gnss_dev)
    {
        gnss_dev->type = RT_Device_Class_Char;
        gnss_dev->init = gnss_init;
        gnss_dev->open = gnss_open;
        gnss_dev->close = gnss_close;
        gnss_dev->read = gnss_read;
        gnss_dev->write = NULL;
        gnss_dev->control = NULL;
        if (rt_device_register(gnss_dev, "gnss", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
        {
            LOG_I("device created successfully!");
        }
        else
        {
            LOG_E("device created failed!");
            return -RT_ERROR;
        }
    }
    else
    {
        LOG_E("no memory for create device");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_gnss_init);
