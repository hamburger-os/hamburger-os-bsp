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

#ifdef PKG_FFFEFRAME_USING_EXAMPLE

#include "fffeFrame.h"
#include "drv_tca9555.h"

#define DBG_TAG "fffeEX"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FFFE_RX_MAX_LEN 1024

typedef struct
{
    char *devname;
    rt_device_t serial;
    struct rt_messagequeue *rx_mq;
    struct rt_messagequeue *process_mq;
    int isThreadRun;
} fffeFrameUserData;

static fffeFrameUserData userdata = {
    .devname = PKG_FFFEFRAME_EXAMPLE_DEVNAME,
    .isThreadRun = 1,
};

/* 用户自定义协议 */
enum
{
    ID_CMD_WRITE_LINE = 0x1,
    ID_ACK_WRITE_LINE,
    ID_CMD_READ_LINE,
    ID_ACK_READ_LINE
};

typedef struct __attribute__ ((packed))
{
    uint8_t line;       //电缆号
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
} CMD_WRITE_LINE;

typedef struct __attribute__ ((packed))
{
    uint8_t line;       //电缆号
} ACK_WRITE_LINE;

typedef struct __attribute__ ((packed))
{
    uint8_t line;       //电缆号
} CMD_READ_LINE;

typedef struct __attribute__ ((packed))
{
    uint8_t line;       //电缆号
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
} ACK_READ_LINE;

#define PROCESS_MAX_LEN     128//定义1条协议的最大长度
struct process_msg
{
    fffeFrameHead head;
    uint8_t buffer[PROCESS_MAX_LEN];
    fffeFrameEnd end;
};

/* 协议处理线程 */
static void process_thread_entry(void *parameter)
{
    fffeFrame *frame = (fffeFrame *)parameter;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

    //测试一下cmd能否发出
    CMD_WRITE_LINE cmd1 = {
            .line = 6,
            .data1 = 0xffff,
            .data2 = 0xffff,
            .data3 = 0xffff,
            .data4 = 0xffff,
    };
    fffeFrame_cmd(frame, ID_CMD_WRITE_LINE, (uint8_t *)&cmd1, sizeof(cmd1));

    CMD_READ_LINE cmd2 = {
            .line = 6,
    };
    fffeFrame_cmd(frame, ID_CMD_READ_LINE, (uint8_t *)&cmd2, sizeof(cmd2));

    struct process_msg msg;
    rt_err_t result;

    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->process_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            LOG_D("proces: %d %x", msg.head.len, msg.head.cmd);
            LOG_HEX("    proces", 16, msg.buffer, fffeFrame_get_buffer_len(&msg.head));

            switch(msg.head.cmd)
            {
            case ID_CMD_WRITE_LINE:
            {
                LOG_D("   cmd: ID_CMD_WRITE_LINE");
                CMD_WRITE_LINE *cmd = (CMD_WRITE_LINE *)msg.buffer;
                /* 用户在这里进行数据处理,并给出ack */
                rt_tca9555_fast_mode(cmd->line, 0x20, 0x0);
                rt_tca9555_fast_write(cmd->line, 0x20, cmd->data1);
                rt_tca9555_fast_mode(cmd->line, 0x21, 0x0);
                rt_tca9555_fast_write(cmd->line, 0x21, cmd->data2);
                rt_tca9555_fast_mode(cmd->line, 0x22, 0x0);
                rt_tca9555_fast_write(cmd->line, 0x22, cmd->data3);
                rt_tca9555_fast_mode(cmd->line, 0x23, 0x0);
                rt_tca9555_fast_write(cmd->line, 0x23, cmd->data4);
                ACK_WRITE_LINE ack = {
                        .line = cmd->line,
                };
                fffeFrame_ack(frame, FF_ACK_OK, msg.head.sn, msg.head.cmd, (uint8_t *)&ack, sizeof(ack));
            }
                break;
            case ID_CMD_READ_LINE:
            {
                LOG_D("   cmd: ID_CMD_READ_LINE");
                CMD_READ_LINE *cmd = (CMD_READ_LINE *)msg.buffer;
                /* 用户在这里进行数据处理,并给出ack */
                ACK_READ_LINE ack = {
                        .line = cmd->line,
                };
                rt_tca9555_fast_mode(cmd->line, 0x20, 0xffff);
                rt_tca9555_fast_read(cmd->line, 0x20, &ack.data1);
                rt_tca9555_fast_mode(cmd->line, 0x21, 0xffff);
                rt_tca9555_fast_read(cmd->line, 0x21, &ack.data2);
                rt_tca9555_fast_mode(cmd->line, 0x22, 0xffff);
                rt_tca9555_fast_read(cmd->line, 0x22, &ack.data3);
                rt_tca9555_fast_mode(cmd->line, 0x23, 0xffff);
                rt_tca9555_fast_read(cmd->line, 0x23, &ack.data4);
                fffeFrame_ack(frame, FF_ACK_OK, msg.head.sn, msg.head.cmd, (uint8_t *)&ack, sizeof(ack));
            }
                break;
            default:
                break;
            }
        }
    }
}
/* ------------- */

static int16_t ff_init(fffeFrame *frame);
static int16_t ff_close(fffeFrame *frame);
static int16_t ff_write(fffeFrame *frame, const uint8_t *buffer, uint16_t size);
static int16_t ff_read(fffeFrame *frame, uint8_t *buffer, uint16_t size);
static int16_t ff_rx_indicate(fffeFrame *frame, fffeFrameHead *head, uint8_t *buffer, fffeFrameEnd *end);

static fffeFrame frame = {
    .peer = FF_SLAVE,
    .user_data = &userdata,
    .maxlen = FFFE_RX_MAX_LEN,

    .init = ff_init,
    .close = ff_close,
    .write = ff_write,
    .read = ff_read,
    .rx_indicate = ff_rx_indicate,
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
    fffeFrame *frame = (fffeFrame *)parameter;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

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
            fffeFrame_accept(frame, msg.size);
        }
    }
}

static int16_t ff_init(fffeFrame *frame)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;
    /* step1：查找串口设备 */
    puserdata->serial = rt_device_find(puserdata->devname);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_115200;        // 修改波特率为 115200
    config.data_bits = DATA_BITS_8;             // 数据位 8
    config.stop_bits = STOP_BITS_1;             // 停止位 1
    config.parity    = PARITY_NONE;             // 无奇偶校验位
    config.rx_bufsz  = 128;
    config.tx_bufsz  = 128;

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
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, frame, FFFE_RX_MAX_LEN*2, 26, 10);
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
    thread = rt_thread_create("process", process_thread_entry, frame, FFFE_RX_MAX_LEN*2, 27, 10);
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
static int16_t ff_close(fffeFrame *frame)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

    rt_mq_delete(puserdata->rx_mq);
    rt_device_close(puserdata->serial);
    puserdata->isThreadRun = 0;

    return RT_EOK;
}
static int16_t ff_write(fffeFrame *frame, const uint8_t *buffer, uint16_t size)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;
    return rt_device_write(puserdata->serial, 0, buffer, size);
}
static int16_t ff_read(fffeFrame *frame, uint8_t *buffer, uint16_t size)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;
    return rt_device_read(puserdata->serial, 0, buffer, size);
}
static int16_t ff_rx_indicate(fffeFrame *frame, fffeFrameHead *head, uint8_t *buffer, fffeFrameEnd *end)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;
    LOG_D("rxdata: %x %d %x %d %x %x", head->start, head->len, head->cmd, head->sn, end->check, end->end);
    /* TODO：用户的协议处理就在这里
            建议协议处理放置在单独的线程进行
            此处的例子使用消息队列通知线程执行 */
    rt_err_t result;
    struct process_msg msg;
    rt_memcpy(&msg.head, head, sizeof(fffeFrameHead));
    rt_memcpy(msg.buffer, buffer, fffeFrame_get_buffer_len(head));
    rt_memcpy(&msg.end, end, sizeof(fffeFrameEnd));

    result = rt_mq_send(puserdata->process_mq, &msg, sizeof(msg));
    if (result != RT_EOK)
    {
        LOG_E("process mq send error %d", result);
    }

    return result;
}

static int ff_uart_init(void)
{
    int ret = 0;
    ret = fffeFrame_init(&frame);

    return ret;
}
INIT_APP_EXPORT(ff_uart_init);
#endif
