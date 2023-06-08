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
#include <rtdevice.h>

#ifdef PKG_HDLC7c7eFRAME_USING_EXAMPLE

#include "Hdlc7c7eFrame.h"

#define DBG_TAG "hdlcEX"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

typedef struct
{
    char *devname;
    rt_device_t serial;
    struct rt_messagequeue *rx_mq;
    struct rt_messagequeue *process_mq;
    int isThreadRun;
} Hdlc7c7eFrameUserData;

static Hdlc7c7eFrameUserData userdata = {
    .devname = PKG_HDLC7c7eFRAME_EXAMPLE_DEVNAME,
    .isThreadRun = 1,
};

/* 用户自定义协议 */
enum
{
    ID_STATION_POLLING = 0x1,
    ID_CONTROLLER_ACK = 0x81,
};

typedef struct __attribute__ ((packed))
{
    uint8_t hook;           //车钩操作
    uint16_t reserve;
} TYPE_STATION_POLLING;

typedef struct __attribute__ ((packed))
{
    uint32_t distance_h;        //测距-远
    uint32_t distance_l;        //测距-近
    uint8_t logo;               //标识牌
    uint16_t pressure_1;        //风压
    uint16_t pressure_2;        //风压
    uint8_t hook;               //车钩状态
    uint16_t reserve;
} TYPE_CONTROLLER_ACK;

struct process_msg
{
    Hdlc7c7eFrameHead head;
    uint8_t buffer[HDLC7c7eFRAME_MAX_LEN];
    Hdlc7c7eFrameEnd end;
};

/* 协议处理线程 */
static void process_thread_entry(void *parameter)
{
    Hdlc7c7eFrame *frame = (Hdlc7c7eFrame *)parameter;
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;

    //测试一下cmd能否发出
    TYPE_STATION_POLLING cmd1 = {
        .hook = 0,
        .reserve = 0x7c7e,
    };
    Hdlc7c7eFrame_type(frame, 123, ID_STATION_POLLING, (uint8_t *)&cmd1, sizeof(cmd1));

    TYPE_STATION_POLLING cmd2 = {
        .hook = 1,
        .reserve = 0x7c7d,
    };
    Hdlc7c7eFrame_type(frame, 1, ID_STATION_POLLING, (uint8_t *)&cmd2, sizeof(cmd2));

    TYPE_CONTROLLER_ACK cmd3 = {
        .distance_h = 123456,
        .distance_l = 123,
        .logo = 0,
        .pressure_1 = 456,
        .pressure_2 = 789,
        .hook = 1,
        .reserve = 0,
    };
    Hdlc7c7eFrame_type(frame, 1, ID_CONTROLLER_ACK, (uint8_t *)&cmd3, sizeof(cmd3));

    struct process_msg msg;
    rt_err_t result;

    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->process_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            LOG_D("proces: %d %x", msg.head.len, msg.head.type);
            LOG_HEX("    proces", 16, msg.buffer, Hdlc7c7eFrame_get_buffer_len(&msg.head));

            switch(msg.head.type)
            {
            case ID_STATION_POLLING:
            {
                LOG_D("   cmd: ID_STATION_POLLING");
                TYPE_STATION_POLLING *type = (TYPE_STATION_POLLING *)msg.buffer;
                /* 用户在这里进行数据处理,并给出ack */
                TYPE_CONTROLLER_ACK ack = {
                    .distance_h = 123456,
                    .distance_l = 123,
                    .logo = 0,
                    .pressure_1 = 456,
                    .pressure_2 = 789,
                    .hook = type->hook,
                    .reserve = 0x7c7e,
                };
                Hdlc7c7eFrame_ack(frame, 1, HDLC_ACK_OK, msg.head.id, msg.head.type, (uint8_t *)&ack, sizeof(ack));
            }
                break;
            default:
                break;
            }
        }
    }
}
/* ------------- */

static int8_t hdlc_init(Hdlc7c7eFrame *frame);
static int8_t hdlc_close(Hdlc7c7eFrame *frame);
static int8_t hdlc_write(Hdlc7c7eFrame *frame, const uint8_t *buffer, uint8_t size);
static int8_t hdlc_read(Hdlc7c7eFrame *frame, uint8_t *buffer, uint8_t size);
static int8_t hdlc_rx_indicate(Hdlc7c7eFrame *frame, Hdlc7c7eFrameHead *head, uint8_t *buffer, Hdlc7c7eFrameEnd *end);

static Hdlc7c7eFrame frame = {
    .peer = HDLC_SLAVE,
    .addr = 123,
    .user_data = &userdata,

    .init = hdlc_init,
    .close = hdlc_close,
    .write = hdlc_write,
    .read = hdlc_read,
    .rx_indicate = hdlc_rx_indicate,
};

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(userdata.rx_mq, &msg, sizeof(msg));
    return result;
}

static void serial_thread_entry(void *parameter)
{
    Hdlc7c7eFrame *frame = (Hdlc7c7eFrame *)parameter;
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;

    struct rx_msg msg;
    rt_err_t result;

    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            /* 从串口读取数据 */
            Hdlc7c7eFrame_accept(frame, msg.size);
        }
    }
}

static int8_t hdlc_init(Hdlc7c7eFrame *frame)
{
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;
    /* step1：查找串口设备 */
    puserdata->serial = rt_device_find(puserdata->devname);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_9600;          // 修改波特率为 9600
    config.data_bits = DATA_BITS_8;             // 数据位 8
    config.stop_bits = STOP_BITS_1;             // 停止位 1
    config.parity    = PARITY_NONE;             // 无奇偶校验位
    config.rx_bufsz  = HDLC7c7eFRAME_MAX_LEN;
    config.tx_bufsz  = HDLC7c7eFRAME_MAX_LEN;

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(puserdata->serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
    if (rt_device_open(puserdata->serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* 初始化消息队列 */
    puserdata->rx_mq = rt_mq_create("ffferx", sizeof(struct rx_msg), 4, RT_IPC_FLAG_FIFO);

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(puserdata->serial, uart_input);

    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, frame, 2048, 26, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        return -RT_ERROR;
    }

    /* 初始化消息队列 */
    puserdata->process_mq = rt_mq_create("process", sizeof(struct process_msg), 4, RT_IPC_FLAG_FIFO);

    /* 创建 协议数据处理 线程 */
    thread = rt_thread_create("process", process_thread_entry, frame, 2048, 27, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
static int8_t hdlc_close(Hdlc7c7eFrame *frame)
{
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;

    rt_mq_delete(puserdata->rx_mq);
    rt_device_close(puserdata->serial);
    puserdata->isThreadRun = 0;

    return RT_EOK;
}
static int8_t hdlc_write(Hdlc7c7eFrame *frame, const uint8_t *buffer, uint8_t size)
{
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;
    return rt_device_write(puserdata->serial, 0, buffer, size);
}
static int8_t hdlc_read(Hdlc7c7eFrame *frame, uint8_t *buffer, uint8_t size)
{
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;
    return rt_device_read(puserdata->serial, 0, buffer, size);
}
static int8_t hdlc_rx_indicate(Hdlc7c7eFrame *frame, Hdlc7c7eFrameHead *head, uint8_t *buffer, Hdlc7c7eFrameEnd *end)
{
    Hdlc7c7eFrameUserData *puserdata = (Hdlc7c7eFrameUserData *)frame->user_data;
    LOG_D("rxdata: %x %d %x %d %x %x", head->start, head->len, head->type, head->id, end->sum, end->end);
    /* TODO：用户的协议处理就在这里
            建议协议处理放置在单独的线程进行
            此处的例子使用消息队列通知线程执行 */
    rt_err_t result;
    struct process_msg msg;
    rt_memcpy(&msg.head, head, sizeof(Hdlc7c7eFrameHead));
    rt_memcpy(msg.buffer, buffer, Hdlc7c7eFrame_get_buffer_len(head));
    rt_memcpy(&msg.end, end, sizeof(Hdlc7c7eFrameEnd));

    result = rt_mq_send(puserdata->process_mq, &msg, sizeof(msg));
    if (result != RT_EOK)
    {
        LOG_E("process mq send error %d", result);
    }

    return result;
}

static int hdlc_uart_init(void)
{
    int ret = 0;
    ret = Hdlc7c7eFrame_init(&frame);

    return ret;
}
INIT_APP_EXPORT(hdlc_uart_init);
#endif
