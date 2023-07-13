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

#include "coupler_controller.h"
#include "Hdlc7c7eFrame.h"

#define DBG_TAG "coupler"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define DEBUG_LED   LED_CAN1

#define HDLC_RX_MAX_LEN 512

CouplerCtrlUserData coupler_controller_userdata = {
    .station_devname = BSP_DEV_TABLE_UART1,
    .module_devname = BSP_DEV_TABLE_UART4,
    .adc_devname = "ltc186x",
    .led_devname = {BSP_GPIO_TABLE_GPIO5, BSP_GPIO_TABLE_SPI1_CS2, BSP_GPIO_TABLE_SPI1_CS1, BSP_GPIO_TABLE_SPI1_CS0, BSP_GPIO_TABLE_GPIO4},
    .ctrl_devname = {"PC.1", "PA.9", BSP_GPIO_TABLE_PWM3, BSP_GPIO_TABLE_PWM4},
    .bat_devname = {BSP_GPIO_TABLE_GPIO3, BSP_GPIO_TABLE_GPIO8},

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
    uint8_t put_hook;           //挂钩状态
    uint8_t out_hook;           //摘钩状态
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
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    LOG_I("station com startup...");

#if 0
    //测试一下cmd能否发出
    TYPE_STATION_POLLING cmd1 = {
        .hook = 0x1,
        .reserve = 0x7c7e,
    };
    Hdlc7c7eFrame_type(frame, 0x2, ID_STATION_POLLING, (uint8_t *)&cmd1, sizeof(cmd1));
    rt_thread_delay(200);

    TYPE_STATION_POLLING cmd2 = {
        .hook = 0x2,
        .reserve = 0x7c7e,
    };
    Hdlc7c7eFrame_type(frame, 0x2, ID_STATION_POLLING, (uint8_t *)&cmd2, sizeof(cmd2));
    rt_thread_delay(200);

    TYPE_CONTROLLER_ACK ack1 = {
        .distance_h = 123456,
        .distance_l = 123,
        .logo = 0,
        .pressure_1 = 456,
        .pressure_2 = 789,
        .put_hook = 0,
        .out_hook = 0,
        .reserve = 0x7c7e,
    };
    Hdlc7c7eFrame_ack(frame, 0x1, HDLC_ACK_OK, 1, ID_CONTROLLER_ACK, (uint8_t *)&ack1, sizeof(ack1));
    rt_thread_delay(200);
#endif

    rt_err_t result;
    struct process_msg msg = {0};
    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->process_station_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            coupler_controller_led_toggle(DEBUG_LED);
            LOG_D("proces: %x %x %d %x", msg.head.src_addr, msg.head.dst_addr, msg.head.len, msg.head.type);
            LOG_HEX("    proces", 16, msg.buffer, Hdlc7c7eFrame_get_buffer_len(&msg.head));

            switch(msg.head.type)
            {
            case ID_STATION_POLLING:
            {
                /* 用户在这里进行数据处理,并给出ack */
                LOG_D("   cmd: ID_STATION_POLLING");
                TYPE_STATION_POLLING *type = (TYPE_STATION_POLLING *)msg.buffer;
                if(type->hook == 0x1)
                {
                    //执行挂钩
                    LOG_I("   cmd: ID_STATION_POLLING execution hooks");
                    module_ctrl_open(1);
                }
                else if (type->hook == 0x2)
                {
                    //执行摘钩
                    LOG_I("   cmd: ID_STATION_POLLING Perform unhooking");
                    module_ctrl_open(1);
                    ctrl_air_pressure(1);
                }
                TYPE_CONTROLLER_ACK ack = {
                    .distance_h = puserdata->distance_h,
                    .distance_l = puserdata->distance_l,
                    .logo = puserdata->logo,
                    .pressure_1 = puserdata->adc[0],
                    .pressure_2 = puserdata->adc[1],
                    .put_hook = 0,
                    .out_hook = puserdata->out_hook,
                    .reserve = 0x7c7e,
                };
                Hdlc7c7eFrame_ack(frame, msg.head.src_addr, HDLC_ACK_OK, msg.head.id, msg.head.type, (uint8_t *)&ack, sizeof(ack));
                LOG_I("   cmd: ID_STATION_POLLING %d %d %d %d %d %d %d"
                        , ack.distance_h, ack.distance_l, ack.logo, ack.pressure_1, ack.pressure_2, ack.put_hook, ack.out_hook);
            }
                break;
            default:
                break;
            }
        }
    }

    Hdlc7c7eFrame_close(frame);
}
/* ------------- */

static int8_t hdlc_init(Hdlc7c7eFrame *frame);
static int8_t hdlc_close(Hdlc7c7eFrame *frame);
static uint16_t hdlc_write(Hdlc7c7eFrame *frame, const uint8_t *buffer, uint16_t size);
static uint16_t hdlc_read(Hdlc7c7eFrame *frame, uint8_t *buffer, uint16_t size);
static int8_t hdlc_rx_indicate(Hdlc7c7eFrame *frame, Hdlc7c7eFrameHead *head, uint8_t *buffer, Hdlc7c7eFrameEnd *end);

static Hdlc7c7eFrame frame = {
    .peer = HDLC_SLAVE,
    .addr = 0x02,
    .maxlen = HDLC_RX_MAX_LEN,
    .user_data = &coupler_controller_userdata,

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

    result = rt_mq_send(coupler_controller_userdata.rx_station_mq, &msg, sizeof(msg));
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
    Hdlc7c7eFrame *frame = (Hdlc7c7eFrame *)parameter;
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;

    struct rx_msg msg;
    rt_err_t result;

    while (puserdata->isThreadRun)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(puserdata->rx_station_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            /* 从串口读取数据 */
            Hdlc7c7eFrame_accept(frame, msg.size);
        }
    }
}

static int8_t hdlc_init(Hdlc7c7eFrame *frame)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    /* step1：查找串口设备 */
    puserdata->station_dev = rt_device_find(puserdata->station_devname);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_9600;          // 修改波特率为 9600
    config.data_bits = DATA_BITS_8;             // 数据位 8
    config.stop_bits = STOP_BITS_1;             // 停止位 1
    config.parity    = PARITY_NONE;             // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
    config.rx_bufsz  = HDLC_RX_MAX_LEN;
    config.tx_bufsz  = HDLC_RX_MAX_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
    config.bufsz     = HDLC_RX_MAX_LEN;
#endif

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(puserdata->station_dev, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(puserdata->station_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
    if (rt_device_open(puserdata->station_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
    {
        return -RT_ERROR;
    }

    /* 初始化消息队列 */
    puserdata->rx_station_mq = rt_mq_create("hdlcrx", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(puserdata->station_dev, uart_input);

    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, frame, 4096, 25, 10);
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
    puserdata->process_station_mq = rt_mq_create("process", sizeof(struct process_msg), 8, RT_IPC_FLAG_FIFO);

    /* 创建 协议数据处理 线程 */
    thread = rt_thread_create("process", process_thread_entry, frame, 4096, 26, 10);
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
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;

    rt_mq_delete(puserdata->rx_station_mq);
    rt_mq_delete(puserdata->process_station_mq);
    rt_device_close(puserdata->station_dev);
    puserdata->isThreadRun = 0;

    return RT_EOK;
}
static uint16_t hdlc_write(Hdlc7c7eFrame *frame, const uint8_t *buffer, uint16_t size)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    return rt_device_write(puserdata->station_dev, 0, buffer, size);
}
static uint16_t hdlc_read(Hdlc7c7eFrame *frame, uint8_t *buffer, uint16_t size)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    return rt_device_read(puserdata->station_dev, 0, buffer, size);
}
static int8_t hdlc_rx_indicate(Hdlc7c7eFrame *frame, Hdlc7c7eFrameHead *head, uint8_t *buffer, Hdlc7c7eFrameEnd *end)
{
    CouplerCtrlUserData *puserdata = (CouplerCtrlUserData *)frame->user_data;
    LOG_D("rxdata: %x %d %x %d %x %x", head->start, head->len, head->type, head->id, end->sum, end->end);
    /* TODO：用户的协议处理就在这里
            建议协议处理放置在单独的线程进行
            此处的例子使用消息队列通知线程执行 */
    rt_err_t result;
    struct process_msg msg;
    rt_memcpy(&msg.head, head, sizeof(Hdlc7c7eFrameHead));
    rt_memcpy(msg.buffer, buffer, Hdlc7c7eFrame_get_buffer_len(head));
    rt_memcpy(&msg.end, end, sizeof(Hdlc7c7eFrameEnd));

    result = rt_mq_send(puserdata->process_station_mq, &msg, sizeof(msg));
    if (result != RT_EOK)
    {
        LOG_E("process mq send error %d", result);
    }

    return result;
}

static int coupler_controller_init(void)
{
    //初始化数据库
    coupler_controller_dbinit();
    //启动指示灯
    coupler_controller_ledinit();
    //启动电池管理
    coupler_controller_batinit();
    //启动电车勾控制
    coupler_controller_ctrlinit();
    //启动风压adc读取
    coupler_controller_pressureinit();
    //启动和图像测距模组的通信
    coupler_controller_moduleinit();
    //启动和站防的485总线通信
    frame.addr = get_device_addr();     //从数据库获取总线地址
    Hdlc7c7eFrame_init(&frame);

    return RT_EOK;
}

#if 0 //如果使用dl动态模块方式
int main(int argc, char *argv[])
{
    coupler_controller_init();

    while(coupler_controller_userdata.isThreadRun)
    {
        rt_thread_delay(1000);
    }
    return 0;
}
#else
INIT_APP_EXPORT(coupler_controller_init);
#endif
