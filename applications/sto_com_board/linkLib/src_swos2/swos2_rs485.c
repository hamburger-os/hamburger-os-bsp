/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_rs485.h"

#define DBG_TAG "if_rs485"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "if_gpio.h"

#define SWOS2_RS_THREAD_STACK      (2048U)
#define SWOS2_RS_THREAD_PRIORITY   (20U)
#define SWOS2_RS_THREAD_TICK       (5U)

#define SWOS2_RS_MQ_NUM  (256U)
#define SWOS2_RS_RX_BUFFER_MA_NUM  (256U)
#define SWOS2_RS_UART_BUFFER_LEN   (512U)

typedef struct
{
    const char *pin_name;
    rt_base_t pin_index;
} S_RS_GPIO_INFO;

typedef struct
{
    rt_base_t re_pin_index;
    rt_base_t te_pin_index;
    rt_base_t de_pin_index;
    const char* usart_name;
    E_RS_TYPE rs_type;

    void (*SendBefor)(void);
    void (*SendOver)(void);
} S_RS_CFG;

typedef struct
{
    uint8_t buffer[SWOS2_RS_UART_BUFFER_LEN];
    uint32_t len;
} S_RS_MSG;

typedef struct
{
    rt_device_t dev;
    rt_mq_t rs_mq;
    rt_mq_t msg_mq;
    E_SLOT_ID id;            /** 板子id */
    S_RS_CFG cfg;

    S_RS_MSG rx_msg;
} S_RS_DEV;

static S_RS_DEV s_rs_dev;

static BOOL swos2_rs_cfg(S_RS_DEV *p_rs_dev, E_RS_TYPE type)
{
    if (NULL == p_rs_dev)
    {
        return FALSE;
    }

    rt_memset(&p_rs_dev->cfg, 0, sizeof(S_RS_CFG));

    p_rs_dev->cfg.rs_type = type;

    switch (p_rs_dev->id)
    {
    case E_SLOT_ID_1:  /** 通信1底板 */
    case E_SLOT_ID_5:
        p_rs_dev->cfg.usart_name = "uart3";
        p_rs_dev->cfg.re_pin_index = rt_pin_get("PD.12");
        p_rs_dev->cfg.te_pin_index = rt_pin_get("PA.4");
        p_rs_dev->cfg.de_pin_index = rt_pin_get("PA.5");
        break;
    case E_SLOT_ID_2:  /** 子板 */
    case E_SLOT_ID_4:
    case E_SLOT_ID_6:
    case E_SLOT_ID_8:
        p_rs_dev->cfg.usart_name = "uart3";
        p_rs_dev->cfg.re_pin_index = rt_pin_get("PD.12");
        p_rs_dev->cfg.te_pin_index = rt_pin_get("PH.10");
        p_rs_dev->cfg.de_pin_index = rt_pin_get("PH.12");
        break;
    default:
        LOG_E("rs cfg slot id error");
        return FALSE;
    }
    return TRUE;
}

//static void swos2_rs485_tx_set(S_RS_DEV *p_rs_dev)
//{
//    if (NULL == p_rs_dev)
//    {
//        return;
//    }
//
//    rt_pin_write(p_rs_dev->cfg.re_pin_index, PIN_HIGH);
//    rt_thread_mdelay(5);
//    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_HIGH);
//    rt_thread_mdelay(5);
//}
//
static void swos2_rs485_rx_set(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.re_pin_index, PIN_LOW);
    rt_thread_mdelay(5);
    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_LOW);
    rt_thread_mdelay(5);
}

static void swos2_rs422_mode_set(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.re_pin_index, PIN_LOW);
    rt_thread_mdelay(5);
    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_HIGH);
    rt_thread_mdelay(5);
}

static void swos2_rs485_send_before(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.re_pin_index, PIN_HIGH);
    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_HIGH);
}

static void swos2_rs485_send_over(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.re_pin_index, PIN_LOW);
    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_LOW);
}
static void swos2_rs422_send_before(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_HIGH);
}

static void swos2_rs422_send_over(void)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_pin_write(p_rs_dev->cfg.de_pin_index, PIN_LOW);
}

static BOOL swos2_rs_gpio_init(S_RS_DEV *p_rs_dev)
{
    if (NULL == p_rs_dev)
    {
        return FALSE;
    }

    /** 1.设置为输出模式 */
    rt_pin_mode(p_rs_dev->cfg.re_pin_index, PIN_MODE_OUTPUT);
    rt_pin_mode(p_rs_dev->cfg.te_pin_index, PIN_MODE_OUTPUT);
    rt_pin_mode(p_rs_dev->cfg.de_pin_index, PIN_MODE_OUTPUT);


    /** 2.RE DE 引脚设置为高电平 */
    swos2_rs485_send_before();

    /** 3.根据系别设置TE */
    switch (p_rs_dev->id)
    {
    case E_SLOT_ID_1:  /** I系 */
    case E_SLOT_ID_2:
    case E_SLOT_ID_4:
        rt_pin_write(p_rs_dev->cfg.te_pin_index, PIN_LOW);  /** TE=0 禁止终端电阻 */
        break;
    case E_SLOT_ID_5:  /** II系 */
    case E_SLOT_ID_6:
    case E_SLOT_ID_8:
        rt_pin_write(p_rs_dev->cfg.te_pin_index, PIN_HIGH);  /** TE=0 使能终端电阻 */
        break;
    default:
        LOG_E("set te slot id error");
        return FALSE;
    }

    /** 4.根据485/422模式设置引脚 */
    if(E_RS_485_TYPE == p_rs_dev->cfg.rs_type)
    {
        swos2_rs485_rx_set();  /** 设置为接收模式 */
        p_rs_dev->cfg.SendBefor = swos2_rs485_send_before;
        p_rs_dev->cfg.SendOver = swos2_rs485_send_over;
        swos2_rs485_send_over();
    }
    else if(E_RS_422_TYPE == p_rs_dev->cfg.rs_type)
    {
        p_rs_dev->cfg.SendBefor = swos2_rs422_send_before;
        p_rs_dev->cfg.SendOver = swos2_rs422_send_over;
        swos2_rs422_mode_set();
    }
    else
    {
        LOG_E("rs type %d error", p_rs_dev->cfg.rs_type);
        return FALSE;
    }
    return TRUE;
}

static rt_err_t swos2_rs_inituart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;
    S_RS_DEV *p_rs_dev = &s_rs_dev;
    uint32_t read_size;

    read_size = size;

    result = rt_mq_send(p_rs_dev->rs_mq, &read_size, sizeof(uint32_t));
    if (result != RT_EOK)
    {
        LOG_E("swos2 uart input mq send error %d", result);
    }
    return result;
}

static BOOL swos2_rs_init(S_RS_DEV *p_rs_dev)
{
    if (NULL == p_rs_dev)
    {
        return FALSE;
    }

    /** 1.查找串口设备 */
    p_rs_dev->dev = rt_device_find(p_rs_dev->cfg.usart_name);
    if(RT_NULL == p_rs_dev->dev)
    {
        LOG_E("find %s error", p_rs_dev->cfg.usart_name);
        return FALSE;
    }

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /** 2.配置串口 */
    config.baud_rate = BAUD_RATE_115200;          /** 修改波特率为 115200 */
    config.data_bits = DATA_BITS_8;               /** 数据位 8 */
    config.stop_bits = STOP_BITS_1;               /** 停止位 1 */
    config.parity    = PARITY_NONE;               /** 无奇偶校验位 */
    config.rx_bufsz  = SWOS2_RS_UART_BUFFER_LEN;
    config.tx_bufsz  = SWOS2_RS_UART_BUFFER_LEN;

    rt_device_control(p_rs_dev->dev, RT_DEVICE_CTRL_CONFIG, &config);

    /** 3.打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
    if (rt_device_open(p_rs_dev->dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
    {
        rt_device_close(p_rs_dev->dev);
        LOG_E("open %s error", p_rs_dev->cfg.usart_name);
        return FALSE;
    }

    /** 4.初始化消息队列 */
    /** 存储接收数据size的消息队列 */
    if(E_RS_485_TYPE == p_rs_dev->cfg.rs_type)
    {
        p_rs_dev->rs_mq = rt_mq_create("rs485 mq", sizeof(uint32_t), SWOS2_RS_MQ_NUM, RT_IPC_FLAG_FIFO);
    }
    else if(E_RS_422_TYPE == p_rs_dev->cfg.rs_type)
    {
        p_rs_dev->rs_mq = rt_mq_create("rs422 mq", sizeof(uint32_t), SWOS2_RS_MQ_NUM, RT_IPC_FLAG_FIFO);
    }
    else
    {
        LOG_E("rs type %d error", p_rs_dev->cfg.rs_type);
        return FALSE;
    }

    if (RT_NULL == p_rs_dev->rs_mq)
    {
        LOG_E("mq creat error");
        return FALSE;
    }

    /** 存储接收到数据的消息队列 */
    p_rs_dev->msg_mq = rt_mq_create("rs rxmq", sizeof(S_RS_MSG), SWOS2_RS_RX_BUFFER_MA_NUM, RT_IPC_FLAG_FIFO);

    if (RT_NULL == p_rs_dev->msg_mq)
    {
        LOG_E("rxmq creat error");
        return FALSE;
    }

    /** 5.设置串口接收回调函数 */
    rt_device_set_rx_indicate(p_rs_dev->dev, swos2_rs_inituart_input);
    return TRUE;
}

static void swos2_rs_thread_entry(void *parameter)
{
    S_RS_DEV *p_rs_dev = (S_RS_DEV *)parameter;
    rt_err_t result;
    uint32_t read_size = 0;

    if(p_rs_dev != RT_NULL)
    {
        if(swos2_rs_init(p_rs_dev) != TRUE)
        {
            LOG_E("swos2_rs_init error");
            return;
        }

        while(1)
        {
            if(RT_EOK == rt_mq_recv(p_rs_dev->rs_mq, &read_size, sizeof(uint32_t), 0))
            {
                if(read_size > 0)
                {
                    rt_memset(p_rs_dev->rx_msg.buffer, 0, SWOS2_RS_UART_BUFFER_LEN);
                    p_rs_dev->rx_msg.len = rt_device_read(p_rs_dev->dev, 0, p_rs_dev->rx_msg.buffer, read_size);
                    if(p_rs_dev->rx_msg.len == read_size)
                    {
                        result = rt_mq_send(p_rs_dev->msg_mq, &p_rs_dev->rx_msg, sizeof(S_RS_MSG));
                        if (result != RT_EOK)
                        {
                            LOG_E("rs msg mq send error %d", result);
                        }
                    }
                }
            }
            rt_thread_mdelay(5);
        }
    }
}

BOOL if_rs485_init(E_RS485_CH ch, E_RS_TYPE type)
{
    rt_thread_t rs_thread = RT_NULL;
    S_RS_DEV *p_rs_dev = &s_rs_dev;

    rt_memset(p_rs_dev, 0, sizeof(S_RS_DEV));

    p_rs_dev->id = if_gpio_getSlotId();

    if(swos2_rs_cfg(p_rs_dev, type) != TRUE)
    {
        LOG_E("swos2_rs_cfg error");
        return FALSE;
    }

    if(swos2_rs_gpio_init(p_rs_dev) != TRUE)
    {
        LOG_E("swos2_rs_gpio_init error");
        return FALSE;
    }

    rs_thread = rt_thread_create("if rs485 rx", swos2_rs_thread_entry, (void *)p_rs_dev,
                        SWOS2_RS_THREAD_STACK, SWOS2_RS_THREAD_PRIORITY, SWOS2_RS_THREAD_TICK);
    if(rs_thread != RT_NULL)
    {
        rt_thread_startup(rs_thread);
    }
    else
    {
        LOG_E("rs thread creat error");
        return FALSE;
    }

    return TRUE;
}

BOOL if_rs485_send(E_RS485_CH ch, uint8 *pdata, uint16 len)
{
    S_RS_DEV *p_rs_dev = &s_rs_dev;
    rt_size_t size = 0;

    if(ch >= E_RS485_CH_MAX)
    {
        return FALSE;
    }

    if(p_rs_dev->cfg.SendBefor != RT_NULL)
    {
        p_rs_dev->cfg.SendBefor();
    }

    size = rt_device_write(p_rs_dev->dev, 0, (const void*)pdata, len);

    if(p_rs_dev->cfg.SendOver != RT_NULL)
    {
        p_rs_dev->cfg.SendOver();
    }

    if(size != len)
    {
        return FALSE;
    }

    return TRUE;
}

uint16 if_rs485_get(E_RS485_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_RS485_CH_MAX)
    {
        return FALSE;
    }

    S_RS_DEV *p_rs_dev = &s_rs_dev;
    S_RS_MSG rx_msg;
    rt_err_t ret;

    ret = rt_mq_recv(p_rs_dev->msg_mq, (void *)&rx_msg, sizeof(S_RS_MSG), RT_WAITING_NO);
    if (RT_EOK == ret)
    {
        rt_memcpy(pdata, &rx_msg.buffer, rx_msg.len);
        return rx_msg.len;
    }
    else
    {
        return 0;
    }

    return 0;
}

