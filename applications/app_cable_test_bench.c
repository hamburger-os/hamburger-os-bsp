/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-15     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#ifdef APP_CABLE_TEST_BENCH

#include "fffeFrame.h"
#include "drv_tca9555.h"

#define DBG_TAG "cable "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FFFE_RX_MAX_LEN 2048

enum
{
    STATE_PEN = 0,
    STATE_SELF,
    STATE_CONFIG,
    STATE_RESULT,
};

typedef struct
{
    char *devname;
    char *keyname;
    rt_device_t serial;
    rt_base_t keypin;
    struct rt_messagequeue *rx_mq;
    struct rt_messagequeue *process_mq;
    struct rt_messagequeue *tca9555_mq;
    int isThreadRun;
} fffeFrameUserData;

static fffeFrameUserData userdata = {
    .devname = "uart2",
    .keyname = "PD.7",
    .isThreadRun = 1,
};

/* 用户自定义协议 */
enum
{
    ID_CMD_SELF = 0x10,
    ID_CMD_HEART = 0x11,
    ID_CMD_CONFIG = 0x12,
    ID_CMD_RESULT = 0x13,
    ID_CMD_TEST = 0x14,
    ID_CMD_PEN = 0x15,
};

struct CableDef
{
    uint8_t num;    //插头编号
    uint8_t i2c;    //对应i2c总线号
    uint8_t count;  //插头针个数
};
struct PanelDef
{
    uint8_t lines;  //电缆个数
    struct CableDef cable[60];
};
struct DevDef
{
    struct PanelDef write;      //写插头
    struct PanelDef read;       //读插头
};
static struct DevDef dev_cable;

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
    struct process_msg msg = {0};
    fffeFrame *frame = (fffeFrame *)parameter;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

#if 1
    //测试一下cmd能否发出
    uint8_t write_id[] = {6,1,2,3,4,5};
    uint8_t write_num[] = {1,26,3,6,5,5};
    uint8_t read_id[] = {};
    uint8_t read_num[] = {};
    uint8_t line_write = sizeof(write_id);
    uint8_t line_read = sizeof(read_id);
    msg.buffer[0] = line_write;             //写插头个数
    for(uint8_t i = 0; i < 2*line_write; i += 2)
    {
        msg.buffer[1 + i] = write_id[i/2];      //插头编号
        msg.buffer[2 + i] = write_num[i/2];;    //插头针个数
    }
    msg.buffer[1+2*line_write] = line_read;    //读插头个数
    for(uint8_t i = 0; i < 2*line_read; i += 2)
    {
        msg.buffer[2 + 2*line_write + i] = read_id[i/2];      //插头编号
        msg.buffer[3 + 2*line_write + i] = read_num[i/2];     //插头针个数
    }
//    fffeFrame_cmd(frame, ID_CMD_SELF, msg.buffer, 2 + 2*line_write + 2*line_read);
    fffeFrame_cmd(frame, ID_CMD_CONFIG, msg.buffer, 2 + 2*line_write + 2*line_read);
    fffeFrame_cmd(frame, ID_CMD_TEST, NULL, 0);

    uint32_t tick = 0xffffff;
    rt_memcpy(msg.buffer, &tick, 4);
    fffeFrame_cmd(frame, ID_CMD_HEART, msg.buffer, 4);
#endif

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
            case ID_CMD_SELF:
            case ID_CMD_CONFIG:
            {
                dev_cable.write.lines = msg.buffer[0];                      //写插头个数
                for(uint8_t i = 0; i < 2*dev_cable.write.lines; i += 2)
                {
                    dev_cable.write.cable[i/2].num = msg.buffer[1 + i];      //插头编号
                    dev_cable.write.cable[i/2].i2c = dev_cable.write.cable[i/2].num;
                    dev_cable.write.cable[i/2].count = msg.buffer[2 + i];     //插头针个数
                }
                dev_cable.read.lines = msg.buffer[1 + 2*dev_cable.write.lines];                     //读插头个数
                for(uint8_t i = 0; i < 2*dev_cable.read.lines; i += 2)
                {
                    dev_cable.read.cable[i/2].num = msg.buffer[2 + 2*dev_cable.write.lines + i];      //插头编号
                    dev_cable.read.cable[i/2].i2c = dev_cable.read.cable[i/2].num;
                    dev_cable.read.cable[i/2].count = msg.buffer[3 + 2*dev_cable.write.lines + i];     //插头针个数
                }
                fffeFrame_ack(frame, FF_ACK_OK, msg.head.sn, msg.head.cmd, NULL, 0);
                if (msg.head.cmd == ID_CMD_SELF)
                {
                    LOG_D("   cmd: ID_CMD_SELF");
                    int runState = STATE_SELF;
                    rt_err_t result = rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
                    if (result != RT_EOK)
                    {
                        LOG_E("ID_CMD_SELF to tca9555_mq error %d", result);
                    }
                }
                else if (msg.head.cmd == ID_CMD_CONFIG)
                {
                    LOG_D("   cmd: ID_CMD_CONFIG");
                    //自动进入散线读取
                    int runState = STATE_CONFIG;
                    rt_err_t result = rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
                    if (result != RT_EOK)
                    {
                        LOG_E("ID_CMD_CONFIG to tca9555_mq error %d", result);
                    }
                }
            }
                break;
            case ID_CMD_TEST:
            {
                int runState = STATE_RESULT;
                rt_err_t result = rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
                if (result != RT_EOK)
                {
                    LOG_E("ID_CMD_TEST to tca9555_mq error %d", result);
                    fffeFrame_ack(frame, FF_ACK_ERROR, msg.head.sn, msg.head.cmd, NULL, 0);
                }
                else
                {
                    fffeFrame_ack(frame, FF_ACK_OK, msg.head.sn, msg.head.cmd, NULL, 0);
                }
                LOG_D("   cmd: ID_CMD_RESULT");
            }
                break;
            default:
                break;
            }
        }
    }
}

/* 扩展io操作线程 */
static void tca9555_thread_entry(void *parameter)
{
    int runState = 0;
    rt_tick_t tick = 0;
    struct process_msg msg = {0};
    struct process_msg last_msg = {0};

    fffeFrame *frame = (fffeFrame *)parameter;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

    while(puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        rt_memset(&runState, 0, sizeof(runState));
        /* 从消息队列中读取消息 */
        rt_err_t result = rt_mq_recv(puserdata->tca9555_mq, &runState, sizeof(runState), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            switch(runState)
            {
            case STATE_PEN:
            {
                rt_thread_delay(1);
                //自动重启下次读取
                runState = STATE_PEN;
                rt_err_t result = rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
                if (result != RT_EOK)
                {
                    LOG_E("STATE_PEN to tca9555_mq error %d", result);
                }
                //读取一次io
                if (rt_tick_get_millisecond() - tick > 100)
                {
                    tick = rt_tick_get_millisecond();
                    msg.head.cmd = ID_CMD_PEN;
                    uint16_t *connectlines = (uint16_t *)&msg.buffer[0];
                    *connectlines = 0;
                    //读所有的插头
                    struct PanelDef *pPanel[] = {&dev_cable.write, &dev_cable.read};
                    for (uint8_t var = 0; var < 2; var ++)
                    {
                        for (uint8_t k = 0; k < pPanel[var]->lines; k ++)
                        {
                            uint16_t data[4] = {0};
                            rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_1, &data[0]);
                            rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_2, &data[1]);
                            rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_3, &data[2]);
                            rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_4, &data[3]);
                            for (uint8_t m = 0; m < pPanel[var]->cable[k].count; m ++)
                            {
                                uint8_t bit = m;
                                if (m > 25)
                                    bit += 6;
                                uint8_t less = bit%16;
                                if (!((data[bit/16] & (1 << less)) == 0))
                                {
                                    msg.buffer[2 + 2*(*connectlines)] = pPanel[var]->cable[k].num;      //插头编号
                                    msg.buffer[3 + 2*(*connectlines)] = m + 1;                          //插头针号
                                    (*connectlines) = (*connectlines) + 1;
                                }
                            }
                        }
                    }
                    if (rt_memcmp(last_msg.buffer, msg.buffer, 2 + 2*(*connectlines)) != 0)
                    {
                        rt_memcpy(last_msg.buffer, msg.buffer, 2 + 2*(*connectlines));
                        if ((*connectlines) > 0)
                        {
                            for (uint8_t i=0; i < (*connectlines); i++)
                            {
                                LOG_D("STATE_PEN read%d (%d %d)"
                                    , i, msg.buffer[2 + 2*i], msg.buffer[3 + 2*i]);
                            }
                            fffeFrame_cmd(frame, msg.head.cmd, msg.buffer, 2 + 2*(*connectlines));
                            LOG_D("STATE_PEN cmd %d %d", (*connectlines), 2 + 2*(*connectlines));
                            LOG_D("STATE_PEN used %d ms", rt_tick_get_millisecond() - tick);
                        }
                    }
                }
            }
                break;
            case STATE_SELF:
            case STATE_CONFIG:
            {
                tick = rt_tick_get_millisecond();
                struct PanelDef *pPanel[] = {&dev_cable.write, &dev_cable.read};
                for (uint8_t var = 0; var < 2; var ++)
                {
                    for (uint8_t k = 0; k < pPanel[var]->lines; k ++)
                    {
                        //设置为输入
                        rt_tca9555_fast_mode(pPanel[var]->cable[k].i2c, tca9555_address_1, 0xffff);
                        rt_tca9555_fast_mode(pPanel[var]->cable[k].i2c, tca9555_address_2, 0xffff);
                        rt_tca9555_fast_mode(pPanel[var]->cable[k].i2c, tca9555_address_3, 0xffff);
                        rt_tca9555_fast_mode(pPanel[var]->cable[k].i2c, tca9555_address_4, 0xffff);
                        LOG_D("config%d : i2c%d, %d", k, pPanel[var]->cable[k].i2c, pPanel[var]->cable[k].count);
                    }
                }

                if (runState == STATE_SELF)
                {
                    LOG_D("STATE_SELF used %d ms", rt_tick_get_millisecond() - tick);
                }
                else if (runState == STATE_CONFIG)
                {
                    LOG_D("STATE_CONFIG used %d ms", rt_tick_get_millisecond() - tick);
                    //自动重启下次读取
                    runState = STATE_PEN;
                    rt_err_t result = rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
                    if (result != RT_EOK)
                    {
                        LOG_E("STATE_CONFIG to tca9555_mq error %d", result);
                    }
                }
            }
                break;
            case STATE_RESULT:
            {
                //进行一次正常测试
                tick = rt_tick_get_millisecond();
                msg.head.cmd = ID_CMD_RESULT;
                LOG_D("STATE_RESULT %d %d", dev_cable.write.lines, dev_cable.read.lines);
                uint16_t *connectlines = (uint16_t *)&msg.buffer[0];
                *connectlines = 0;
                for(uint8_t i = 0; i < dev_cable.write.lines; i ++)
                {
                    for (uint8_t j = 0; j < dev_cable.write.cable[i].count; j ++)
                    {
                        uint8_t bit = j;
                        if (j > 25)
                            bit += 6;
                        //设置一个插针为输出并输出1
                        if (bit/16 == 0) {
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_1, ~(1 << (bit%16)));
                            rt_tca9555_fast_write(dev_cable.write.cable[i].i2c, tca9555_address_1, (1 << (bit%16)));
                        }
                        else
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_1, 0xffff);
                        if (bit/16 == 1) {
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_2, ~(1 << (bit%16)));
                            rt_tca9555_fast_write(dev_cable.write.cable[i].i2c, tca9555_address_2, (1 << (bit%16)));
                        }
                        else
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_2, 0xffff);
                        if (bit/16 == 2) {
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_3, ~(1 << (bit%16)));
                            rt_tca9555_fast_write(dev_cable.write.cable[i].i2c, tca9555_address_3, (1 << (bit%16)));
                        }
                        else
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_3, 0xffff);
                        if (bit/16 == 3) {
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_4, ~(1 << (bit%16)));
                            rt_tca9555_fast_write(dev_cable.write.cable[i].i2c, tca9555_address_4, (1 << (bit%16)));
                        }
                        else
                            rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_4, 0xffff);

                        //读所有的插头
                        struct PanelDef *pPanel[] = {&dev_cable.write, &dev_cable.read};
                        for (uint8_t var = 0; var < 2; ++var)
                        {
                            for (uint8_t k = 0; k < pPanel[var]->lines; k ++)
                            {
                                uint16_t data[4] = {0};
                                rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_1, &data[0]);
                                rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_2, &data[1]);
                                rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_3, &data[2]);
                                rt_tca9555_fast_read(pPanel[var]->cable[k].i2c, tca9555_address_4, &data[3]);
                                for (uint8_t m = 0; m < pPanel[var]->cable[k].count; m ++)
                                {
                                    uint8_t bit = m;
                                    if (m > 25)
                                        bit += 6;
                                    uint8_t less = bit%16;
                                    if (!((data[bit/16] & (1 << less)) == 0))
                                    {
                                        if (!((dev_cable.write.cable[i].num == pPanel[var]->cable[k].num) && (j == m)))
                                        {
                                            msg.buffer[2 + 4*(*connectlines)] = dev_cable.write.cable[i].num;   //插头编号
                                            msg.buffer[3 + 4*(*connectlines)] = j + 1;                          //插头针号
                                            msg.buffer[4 + 4*(*connectlines)] = pPanel[var]->cable[k].num;   //插头编号
                                            msg.buffer[5 + 4*(*connectlines)] = m + 1;                           //插头针号
                                            LOG_D("STATE_RESULT read%d (%d %d) : (%d %d)"
                                                , (*connectlines)
                                                , msg.buffer[2 + 4*(*connectlines)], msg.buffer[3 + 4*(*connectlines)]
                                                , msg.buffer[4 + 4*(*connectlines)], msg.buffer[5 + 4*(*connectlines)]);
                                            (*connectlines) = (*connectlines) + 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //设置为输入
                    rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_1, 0xffff);
                    rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_2, 0xffff);
                    rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_3, 0xffff);
                    rt_tca9555_fast_mode(dev_cable.write.cable[i].i2c, tca9555_address_4, 0xffff);
                }
                fffeFrame_cmd(frame, msg.head.cmd, msg.buffer, 2 + 4*(*connectlines));
                LOG_D("STATE_RESULT cmd %d %d", (*connectlines), 2 + 4*(*connectlines));
                LOG_D("STATE_RESULT used %d ms", rt_tick_get_millisecond() - tick);
            }
                break;
            default:
                break;
            }
        }
    }
}

/* 固定周期线程 */
static void heart_thread_entry(void *parameter)
{
    rt_tick_t tick = 0;
    struct process_msg msg = {0};

    fffeFrame *frame = (fffeFrame *)parameter;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

    while(puserdata->isThreadRun)
    {
        rt_thread_delay(10000);
        msg.head.cmd = ID_CMD_HEART;
        tick = rt_tick_get_millisecond();
        LOG_D("HEART %d", tick);
        rt_memcpy(msg.buffer, &tick, 4);
        fffeFrame_cmd(frame, msg.head.cmd, msg.buffer, 4);
    }
}

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

static void key_irq(void *args)
{
    fffeFrame *frame = (fffeFrame *)args;
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;

    int runState = STATE_RESULT;
    rt_mq_send(puserdata->tca9555_mq, &runState, sizeof(runState));
}

static int16_t ff_init(fffeFrame *frame)
{
    fffeFrameUserData *puserdata = (fffeFrameUserData *)frame->user_data;
    /* step1:查找串口设备 */
    puserdata->serial = rt_device_find(puserdata->devname);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2:修改串口配置参数 */
    config.baud_rate = BAUD_RATE_115200;        // 修改波特率为 115200
    config.data_bits = DATA_BITS_8;             // 数据位 8
    config.stop_bits = STOP_BITS_1;             // 停止位 1
    config.parity    = PARITY_NONE;             // 无奇偶校验位
    config.rx_bufsz  = 512;
    config.tx_bufsz  = 512;

    /* step3:控制串口设备.通过控制接口传入命令控制字,与控制参数 */
    rt_device_control(puserdata->serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4:打开串口设备.以非阻塞接收和阻塞发送模式打开串口设备 */
    if (rt_device_open(puserdata->serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* 初始化消息队列 */
    puserdata->rx_mq = rt_mq_create("ffferx", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);

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
    puserdata->process_mq = rt_mq_create("process", sizeof(struct process_msg), 8, RT_IPC_FLAG_FIFO);

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

    /* 创建 固定周期 线程 */
    thread = rt_thread_create("heart", heart_thread_entry, frame, FFFE_RX_MAX_LEN*2, 28, 10);
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
    puserdata->tca9555_mq = rt_mq_create("tca9555", sizeof(int), 16, RT_IPC_FLAG_FIFO);

    /* 创建 扩展io操作 线程 */
    thread = rt_thread_create("tca9555", tca9555_thread_entry, frame, FFFE_RX_MAX_LEN*4, 29, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        return -RT_ERROR;
    }

    /* 初始化按键 */
    puserdata->keypin = rt_pin_get(puserdata->keyname);
    rt_pin_mode(puserdata->keypin, PIN_MODE_INPUT);
    /* 绑定中断,上升沿模式 */
    rt_pin_attach_irq(puserdata->keypin, PIN_IRQ_MODE_RISING, key_irq, frame);
    /* 使能中断 */
    rt_pin_irq_enable(puserdata->keypin, PIN_IRQ_ENABLE);

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
    /* TODO:用户的协议处理就在这里
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
