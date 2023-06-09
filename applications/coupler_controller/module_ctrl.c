/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-26     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "fffeFrame.h"
#include "coupler_controller.h"

#define DBG_TAG "module"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define DEBUG_LED   LED_CAN2

#define FFFE_RX_MAX_LEN 2048

/* 用户自定义协议 */
enum
{
    ID_CMD_OPEN = 0x1,
    ID_ACK_OPEN_ERR = 0x4001,
    ID_ACK_OPEN = 0x8001,
    ID_CMD_READ = 0x2,
    ID_ACK_READ_ERR = 0x4002,
    ID_ACK_READ = 0x8002,
};

typedef struct __attribute__ ((packed))
{
    uint8_t function;//1：开启模块；0：关闭模块
} CMD_OPENDef;

typedef struct __attribute__ ((packed))
{
    uint8_t logo;               //标识牌
    uint8_t islogo;             //标识牌有效位
    uint32_t distance_h;        //测距-远
    uint8_t isdistance_h;       //测距-远有效位
    uint32_t distance_l;        //测距-近
    uint8_t isdistance_l;       //测距-近有效位
    uint8_t out_hook;           //摘钩状态
    uint8_t isout_hook;         //摘钩状态有效位
} ACK_OPENDef;

typedef struct __attribute__ ((packed))
{
    uint8_t function;//1：读所有数据
} CMD_READDef;

typedef struct __attribute__ ((packed))
{
    uint8_t logo;               //标识牌
    uint8_t islogo;             //标识牌有效位
    uint32_t distance_h;        //测距-远
    uint8_t isdistance_h;       //测距-远有效位
    uint32_t distance_l;        //测距-近
    uint8_t isdistance_l;       //测距-近有效位
    uint8_t out_hook;           //摘钩状态
    uint8_t isout_hook;         //摘钩状态有效位
} ACK_READDef;

#define PROCESS_MAX_LEN     FFFE_RX_MAX_LEN//定义1条协议的最大长度
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
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    LOG_I("module ctrl startup...");

#if 0
    //测试协议是否可以发出
    CMD_OPENDef cmd1 = {
        .function = 1,
    };
    fffeFrame_cmd(frame, ID_CMD_OPEN, (uint8_t *)&cmd1, sizeof(cmd1));
    rt_thread_delay(200);

    CMD_OPENDef cmd2 = {
        .function = 0,
    };
    fffeFrame_cmd(frame, ID_CMD_OPEN, (uint8_t *)&cmd2, sizeof(cmd2));
    rt_thread_delay(200);

    CMD_READDef cmd3 = {
        .function = 1,
    };
    fffeFrame_cmd(frame, ID_CMD_READ, (uint8_t *)&cmd3, sizeof(cmd3));
    rt_thread_delay(200);

    ACK_OPENDef ack1 = {
        .logo = 1,
        .islogo = 1,
        .distance_h = 0xfdfeff,
        .isdistance_h = 1,
        .distance_l = 0xfdfeff,
        .isdistance_l = 1,
        .out_hook = 0,
        .isout_hook = 1,
    };
    fffeFrame_ack(frame, FF_ACK_OK, 1, ID_CMD_OPEN, (uint8_t *)&ack1, sizeof(ack1));
    rt_thread_delay(200);

    ACK_READDef ack2 = {
        .logo = 1,
        .islogo = 0,
        .distance_h = 0xfdfeff,
        .isdistance_h = 0,
        .distance_l = 0xfdfeff,
        .isdistance_l = 0,
        .out_hook = 0,
        .isout_hook = 0,
    };
    fffeFrame_ack(frame, FF_ACK_OK, 1, ID_CMD_READ, (uint8_t *)&ack2, sizeof(ack2));
    rt_thread_delay(200);
#endif

    rt_err_t result;
    struct process_msg msg = {0};
    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->process_module_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            coupler_controller_led_toggle(DEBUG_LED);
            LOG_D("proces: %d %x", msg.head.len, msg.head.cmd);
            LOG_HEX("    proces", 16, msg.buffer, fffeFrame_get_buffer_len(&msg.head));

            switch(msg.head.cmd)
            {
            case ID_ACK_OPEN_ERR:
            {
                LOG_E("   cmd: ID_ACK_OPEN_ERR");
            }
                break;
            case ID_ACK_OPEN:
            {
                LOG_D("   cmd: ID_ACK_OPEN");
                ACK_OPENDef *data = (ACK_OPENDef *)msg.buffer;
                LOG_I("  cmd: ID_ACK_OPEN %d %d %d %d", data->islogo, data->isdistance_h, data->isdistance_l, data->isout_hook);
                if (data->islogo == 0 && data->isdistance_h == 0 && data->isdistance_l == 0 && data->isout_hook == 0)
                {
                    puserdata->isopen = 0;
                    LOG_I("   cmd: ID_ACK_OPEN Image module off");
                }
                else
                {
                    puserdata->isopen = 1;
                    LOG_I("   cmd: ID_ACK_OPEN Image module on");
                    //启动读
                    CMD_READDef cmd = {
                        .function = 1,
                    };
                    fffeFrame_cmd(frame, ID_CMD_READ, (uint8_t *)&cmd, sizeof(cmd));
                }
            }
                break;
            case ID_ACK_READ_ERR:
            {
                LOG_E("   cmd: ID_ACK_READ_ERR");
            }
                break;
            case ID_ACK_READ:
            {
                LOG_D("   cmd: ID_ACK_READ");
                ACK_READDef *data = (ACK_READDef *)msg.buffer;
                LOG_I("   cmd: ID_ACK_READ %d(%d), %d(%d), %d(%d), %d(%d)"
                        , data->logo, data->islogo
                        , data->distance_h, data->isdistance_h
                        , data->distance_l, data->isdistance_l
                        , data->out_hook, data->isout_hook);
                if (data->islogo)
                {
                    puserdata->logo = data->logo;
                }
                if (data->isdistance_h)
                {
                    puserdata->distance_h = data->distance_h;
                }
                if (data->isdistance_l)
                {
                    puserdata->distance_l = data->distance_l;
                }
                if (data->isout_hook)
                {
                    puserdata->out_hook = data->out_hook;
                }
                if (puserdata->isopen)
                {
                    //重复读
                    CMD_READDef cmd = {
                        .function = 1,
                    };
                    fffeFrame_cmd(frame, ID_CMD_READ, (uint8_t *)&cmd, sizeof(cmd));

                    puserdata->timeout = rt_tick_get_millisecond() - puserdata->timeout;
                    if (puserdata->timeout > 0)
                    {
                        LOG_W("ID_CMD_READ timeout %d", puserdata->timeout);
                    }
                    puserdata->timeout = rt_tick_get_millisecond();
                }
            }
                break;
            default:
                break;
            }
        }
    }

    fffeFrame_close(frame);
}

static int16_t ff_init(fffeFrame *frame);
static int16_t ff_close(fffeFrame *frame);
static uint16_t ff_write(fffeFrame *frame, const uint8_t *buffer, uint16_t size);
static uint16_t ff_read(fffeFrame *frame, uint8_t *buffer, uint16_t size);
static int16_t ff_rx_indicate(fffeFrame *frame, fffeFrameHead *head, uint8_t *buffer, fffeFrameEnd *end);

static fffeFrame frame = {
    .peer = FF_MASTER,
    .user_data = &coupler_controller_userdata,
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

    result = rt_mq_send(coupler_controller_userdata.rx_module_mq, &msg, sizeof(msg));
#ifdef ULOG_USING_ISR_LOG
    if (result != RT_EOK)
    {
        LOG_E("uart_input mq send error %d", result);
    }
#endif
    return result;
}

static void serial_thread_entry(void *parameter)
{
    fffeFrame *frame = (fffeFrame *)parameter;
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;

    struct rx_msg msg;
    rt_err_t result;
    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->rx_module_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            /* 从串口读取数据 */
            fffeFrame_accept(frame, msg.size);
        }
    }
}

static int16_t ff_init(fffeFrame *frame)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    /* step1：查找串口设备 */
    puserdata->module_dev = rt_device_find(puserdata->module_devname);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_9600;          // 修改波特率为 9600
    config.data_bits = DATA_BITS_8;             // 数据位 8
    config.stop_bits = STOP_BITS_1;             // 停止位 1
    config.parity    = PARITY_NONE;             // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
    config.rx_bufsz  = FFFE_RX_MAX_LEN;
    config.tx_bufsz  = FFFE_RX_MAX_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
    config.bufsz     = FFFE_RX_MAX_LEN;
#endif

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(puserdata->module_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(puserdata->module_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
    if (rt_device_open(puserdata->module_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
    {
        return -RT_ERROR;
    }

    /* 初始化消息队列 */
    puserdata->rx_module_mq = rt_mq_create("ffferx", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(puserdata->module_dev, uart_input);

    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, frame, 4096, 22, 10);
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
    puserdata->process_module_mq = rt_mq_create("process", sizeof(struct process_msg), 8, RT_IPC_FLAG_FIFO);

    /* 创建 协议数据处理 线程 */
    thread = rt_thread_create("process", process_thread_entry, frame, 4096, 23, 10);
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
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;

    rt_mq_delete(puserdata->rx_module_mq);
    rt_mq_delete(puserdata->process_module_mq);
    rt_device_close(puserdata->module_dev);
    puserdata->isThreadRun = 0;

    return RT_EOK;
}
static uint16_t ff_write(fffeFrame *frame, const uint8_t *buffer, uint16_t size)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    return rt_device_write(puserdata->module_dev, 0, buffer, size);
}
static uint16_t ff_read(fffeFrame *frame, uint8_t *buffer, uint16_t size)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    return rt_device_read(puserdata->module_dev, 0, buffer, size);
}
static int16_t ff_rx_indicate(fffeFrame *frame, fffeFrameHead *head, uint8_t *buffer, fffeFrameEnd *end)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    LOG_D("rxdata: %x %d %x %d %x %x", head->start, head->len, head->cmd, head->sn, end->check, end->end);
    /* TODO：用户的协议处理就在这里
            建议协议处理放置在单独的线程进行
            此处的例子使用消息队列通知线程执行 */
    rt_err_t result;
    struct process_msg msg;
    rt_memcpy(&msg.head, head, sizeof(fffeFrameHead));
    rt_memcpy(msg.buffer, buffer, fffeFrame_get_buffer_len(head));
    rt_memcpy(&msg.end, end, sizeof(fffeFrameEnd));

    result = rt_mq_send(puserdata->process_module_mq, &msg, sizeof(msg));
    if (result != RT_EOK)
    {
        LOG_E("process mq send error %d", result);
    }

    return result;
}

void coupler_controller_moduleinit(void)
{
    fffeFrame_init(&frame);
}

void module_ctrl_open(uint8_t isopen)
{
    CMD_OPENDef cmd = {
        .function = isopen,
    };
    fffeFrame_cmd(&frame, ID_CMD_OPEN, (uint8_t *)&cmd, sizeof(cmd));
}
