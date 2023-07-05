/*******************************************************
 *
 * @FileName: tax.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: tax通信模块.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <rtthread.h>

#include "board.h"
#include "common.h"
#include "file_manager.h"
#include "log.h"
#include "rtc.h"

/*******************************************************
 * 宏定义
 *******************************************************/

#define UART_DEVICE_NAME "uart4" /* 串口设备 */

#define TAX_EVENT_NUMS 16    /* 可以缓存的最大事件的长度 */
#define TAX_ACK_MSG_LEN 19   /* 有记录内容应答通信包长度 */
#define TAX_ACK_NO_MSG_LEN 3 /* 无记录内容应答通信包长度 */

#define TAX_BOARD1_PACKAGE_LEN 32 /* 本板1数据包长度 */
#define TAX_BOARD2_PACKAGE_LEN 40 /* 本板2数据包长度 */

#define TAX_BOARD1_ADDR 0x38 /* 本板地址1 */
#define TAX_BOARD2_ADDR 0x39 /* 本板地址2 */

#define TAX_RECV_SUCCESS 0x3    /* 接收成功 */
#define TAX_RECV_FAIL 0xC       /* 接收失败 */
#define TAX_VALID_RECOED 0xC    /* 随后16个字节为有效内容,且要求记录 */
#define TAX_VALID_NO_RECOED 0x1 /* 随后16个字节有内容,但不要求记录 */
#define TAX_INVALID 0x0         /* 表无记录内容,仅返回3个字节 */
#define TAX_DISPLAY_INFO 0x4    /* 表随后16个字节为显示信息,不要求记录. */
#define TAX_VOICE_INFO 0x5      /* 表随后16个字节为发声信息,不要求记录. */

/* 各单元代号 */
#define TAX_UNIT_TRACK_DETECT 0x01 /* 轨道检测单元代号 */
#define TAX_UNIT_VOICE 0x06        /* 语音记录单元代号 */

/* 接收缓冲区长度 */
#define BUFFER_LEN 256

/* 串口配置 */
#define BAUD_CONFIG 28800            /* 波特率 */
#define PARITY_CONFIG PARITY_NONE    /* 校验 */
#define DATA_BITS_CONFIG DATA_BITS_8 /* 数据位 */
#define STOP_BITS_CONFIG STOP_BITS_1 /* 停止位 */

/* 从串口读取数据的超时时间,单位ms */
#define READ_MESSAGE_QUEUE_TIMEOUT 1000

/*******************************************************
 * 数据结构
 *******************************************************/

/* 串口回调消息结构体 */
typedef struct rx_msg
{
    rt_device_t dev; /* 串口设备 */
    rt_size_t size;  /* 数据长度 */
} rx_msg_t;

/* TAX2应答事件通信包 */
typedef struct
{
    char data[TAX_ACK_MSG_LEN]; /*应答包通信数据*/
} tax2_echo_event_t;

/* TAX2应答事件通信包列表*/
typedef struct
{
    tax2_echo_event_t echo_event[TAX_EVENT_NUMS]; /*应答事件通信包*/
    sint32_t head;                                /*头部序号*/
    sint32_t tail;                                /*尾部序号*/
    pthread_mutex_t mutex;                        /*互斥量*/
} tax2_echo_event_list_t;

/*******************************************************
 * 全局变量
 *******************************************************/

/* TAX接收缓冲区 */
static uint8_t tax_info_all[TAX_BOARD1_PACKAGE_LEN + TAX_BOARD2_PACKAGE_LEN] = {0};
/* TAX应答发送缓冲区 */
static tax2_echo_event_list_t *echo_list = NULL;
/* uart设备 */
static rt_device_t uart_dev;
/* uart消息队列,用于uart底层向tax线程发送消息. */
static rt_mq_t uart_mq;

/*******************************************************
 * 函数声明
 *******************************************************/
/* 初始化TAX2应答事件列表 */
static void tax_init_echo_event_list(tax2_echo_event_list_t *list);
/* 向TAX2应答事件列表中增加一个元素 */
static sint32_t tax_push_echo_event_list(tax2_echo_event_list_t *list, uint8_t *data);
/* 从TAX2应答事件列表中取出一个元素 */
static sint32_t tax_read_echo_event_list(tax2_echo_event_list_t *list, uint8_t *data);
/* 计算检查和结果,并将检查和结果填充到最后一个字节中. */
static void tax_set_check_sum(uint8_t *buf, sint32_t len);
/* 更新本地的TAX数据 */
static void update_tax_data(void);
/* 初始化TAX串口设备 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size);
/* 打开tax接口 */
static sint32_t tax_open(char *name);
/* 从TAX接收串口数据 */
static sint32_t tax_recv_data(void);
/* 发送TAX2应答信息 */
static sint32_t tax_send_tax2_back(void);
/* TAX线程处理函数 */
static void *tax_thread(void *args);

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  初始化TAX2应答事件列表
 *
 * @param  *list: TAX2应答事件列表
 * @retval 无
 *
 *******************************************************/
static void tax_init_echo_event_list(tax2_echo_event_list_t *list)
{
    memset(list, 0, sizeof(tax2_echo_event_list_t));
    pthread_mutex_init(&list->mutex, NULL);
    list->head = 0;
    list->tail = 0;
}
/*******************************************************
 *
 * @brief  向TAX2应答事件列表中增加一个元素
 *
 * @param  *list: TAX2应答事件列表
 * @param  *data: 通信包数据
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t tax_push_echo_event_list(tax2_echo_event_list_t *list, uint8_t *data)
{
    pthread_mutex_lock(&list->mutex);
    memcpy(list->echo_event[list->head].data, data, TAX_ACK_MSG_LEN);
    list->head++;
    list->head %= TAX_EVENT_NUMS;
    pthread_mutex_unlock(&list->mutex);
    return 0;
}

/*******************************************************
 *
 * @brief  从TAX2应答事件列表中取出一个元素
 *
 * @param  *list: TAX2应答事件列表
 * @param  *data: 通信包数据
 * @retval 0: 成功 -1: 失败
 *
 *******************************************************/
static sint32_t tax_read_echo_event_list(tax2_echo_event_list_t *list, uint8_t *data)
{
    pthread_mutex_lock(&list->mutex);
    if (list->head == list->tail)
    {
        pthread_mutex_unlock(&list->mutex);
        return (sint32_t)-1;
    }
    memcpy(data, list->echo_event[list->tail].data, TAX_ACK_MSG_LEN);
    list->tail++;
    list->tail %= TAX_EVENT_NUMS;
    pthread_mutex_unlock(&list->mutex);
    return 0;
}
/*******************************************************
 *
 * @brief  计算检查和结果,并将检查和结果填充到最后一个字节中.
 *
 * @param  *buf: 输入数据
 * @param  len: 输入数据的总长度
 * @retval 无
 *
 *******************************************************/
static void tax_set_check_sum(uint8_t *buf, sint32_t len)
{
    uint8_t mask_val = 0;
    sint32_t i = 0;

    while (i < (len - 1))
    {
        mask_val += buf[i];
        i++;
    }
    buf[len - 1] = (uint8_t)(~mask_val) + 1;
}
/*******************************************************
 *
 * @brief  发送TAX2应答事件
 *
 * @param  Event: 事件代号
 * @param  *tax40: 当前TAX数据指针
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t tax_send_echo_event(uint8_t Event, const tax40_t *tax40)
{
    uint8_t buf[TAX_ACK_MSG_LEN] = {0};

    /* 判断指针的有效性 */
    if (tax40 == NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    buf[0] = TAX_UNIT_VOICE; /* 语音记录单元代号 */
    buf[1] = (TAX_RECV_SUCCESS << 4) | (TAX_VALID_RECOED << 0);
    buf[2] = Event;

    buf[3] = tax40->date[0];
    buf[4] = tax40->date[1];
    buf[5] = tax40->date[2];

    buf[6] = tax40->kilometer_post[0];
    buf[7] = tax40->kilometer_post[1];
    buf[8] = tax40->kilometer_post[2];

    buf[9] = tax40->speed[0];
    buf[10] = tax40->speed[1];

    buf[11] = 0;
    buf[12] = 0;

    buf[13] = tax40->locomotive_signal_type;
    buf[14] = tax40->signal_machine_type;

    buf[15] = 0;
    buf[16] = 0;
    buf[17] = 0;

    /* 设置校验和 */
    tax_set_check_sum(buf, TAX_ACK_MSG_LEN);
    return tax_push_echo_event_list(echo_list, buf);
}

/*******************************************************
 *
 * @brief  更新本地的TAX数据
 *
 * @retval none
 *
 *******************************************************/
static void update_tax_data(void)
{
    struct tm tmdata;
    static int times = 0;
    uint32_t date;

    memcpy((void *)&g_tax32, (void *)&tax_info_all[0], TAX_BOARD1_PACKAGE_LEN);
    memcpy((void *)&g_tax40, (void *)&tax_info_all[TAX_BOARD1_PACKAGE_LEN], TAX_BOARD2_PACKAGE_LEN);

    /* 更新系统时间*/
    if (times % 100 == 0)
    {

        date = (g_tax40.date[0] << 0) |
               (g_tax40.date[1] << 8) |
               (g_tax40.date[2] << 16) |
               (g_tax40.date[3] << 24);
        tmdata.tm_year = (0x3f & (date >> 26)) + 2000 - 1900;
        tmdata.tm_mon = (0x0f & (date >> 22)) - 1;
        tmdata.tm_mday = 0x1f & (date >> 17);
        tmdata.tm_hour = 0x1f & (date >> 12);
        tmdata.tm_min = 0x3f & (date >> 6);
        tmdata.tm_sec = 0x3f & (date >> 0);

        rtc_setdata(&tmdata);
    }
}
/*******************************************************
 *
 * @brief  初始化TAX串口设备
 *
 * @param  *pu8_TtyName: 设备名
 * @retval 打开的设备号
 *
 *******************************************************/
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(uart_mq, (void *)&msg, sizeof(msg));
    return result;
}
/*******************************************************
 *
 * @brief  TAX模块
 *
 * @param  *name: 设备名
 * @retval =0: 设备号 <0:打开设备失败
 *
 *******************************************************/

static sint32_t tax_open(char *name)
{
    rt_err_t ret;

    /* 查找串口设备 */
    uart_dev = rt_device_find(name);
    if (uart_dev == RT_NULL)
    {
        log_print(LOG_ERROR, "can not find %s device.\n", name);
        return (sint32_t)-1;
    }

    /* 修改串口配置参数 */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_CONFIG;      /* 波特率 */
    config.data_bits = DATA_BITS_CONFIG; /* 数据位 8 */
    config.stop_bits = STOP_BITS_CONFIG; /* 停止位 1 */
    config.parity = PARITY_CONFIG;       /* 无奇偶校验位 */
#ifdef RT_USING_SERIAL_V2
    config.rx_bufsz = BUFFER_LEN;
    config.tx_bufsz = BUFFER_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
    config.bufsz = BUFFER_LEN;
#endif

    /* step3:控制串口设备.通过控制接口传入命令控制字,与控制参数 */
    ret = rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "rt_device_control error!\n");
        rt_device_close(uart_dev);
        return (sint32_t)-1;
    }
    else
    {
    }

    /* step4:打开串口设备.以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
    if (rt_device_open(uart_dev, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
        if (rt_device_open(uart_dev, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
        {
            rt_device_close(uart_dev);
        }

    /* 初始化消息队列 */
    uart_mq = rt_mq_create("uart", sizeof(struct rx_msg), 8, RT_IPC_FLAG_FIFO);
    if (uart_mq == RT_NULL)
    {
        log_print(LOG_ERROR, "init uart mq error. ");
        rt_device_close(uart_dev);
        return (sint32_t)-1;
    }

    /* 设置接收回调函数 */
    ret = rt_device_set_rx_indicate(uart_dev, uart_input);
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "set uart recv callback error. error_code:%d.\n", ret);
        rt_mq_delete(uart_mq);
        rt_device_close(uart_dev);
        return (sint32_t)-1;
    }

    return 0;
}
/*******************************************************
 *
 * @brief  从TAX接收串口数据
 *
 * @retval 接收的字节数
 *
 *******************************************************/
static sint32_t tax_recv_data(void)
{
    struct rx_msg msg;              /* 消息队列中的一个消息数据 */
    static bool is_recving = false; /* 接收通信数据中 */
    static uint8_t *ptr = NULL;
    static uint8_t packet_len /* 通信包的长度*/, recv_cnt, check_sum /* 校验和结果*/;
    static sint32_t len /*未处理的数据长度*/, len_r /*从缓冲区中读取的数据长度*/;
    static uint8_t buf[BUFFER_LEN];
    sint32_t ret;
    uint8_t byte;

    /* 清空消息队列 */
    rt_memset((void *)&msg, 0, sizeof(msg));

    /* 从消息队列中读取消息 */
    ret = rt_mq_recv(uart_mq, (void *)&msg, sizeof(msg), (rt_int32_t)READ_MESSAGE_QUEUE_TIMEOUT);
    if (ret != RT_EOK)
    {
        /* log_print(LOG_ERROR, "send uart message queue error. \n");*/
        return (sint32_t)-1;
    }

    /* 环形缓冲区中的数据已经处理完成, 开始接受新的数据. */
    if (len == 0)
    {
        /* 读取串口数据. */
        rt_memset(buf, 0, sizeof(buf));
        len_r = rt_device_read(uart_dev, 0, buf, msg.size);
        if (len_r < 0)
        {
            log_print(LOG_ERROR, "read uart device error.\n");
            return (sint32_t)-2;
        }
        /* 读取的数据压入环形缓冲区中 */
        len = len_r;
    }

    while (len > 0)
    {
        byte = buf[len_r - len];
        len--;

        if (is_recving == false)
        {
            if (byte == TAX_BOARD1_ADDR) /* 读取0x38 */
            {
                recv_cnt = 0;
                ptr = &tax_info_all[0];
                ptr[recv_cnt] = byte;
                recv_cnt++;
                packet_len = TAX_BOARD1_PACKAGE_LEN;
                check_sum = byte;
                is_recving = true;
            }
            else if (byte == TAX_BOARD2_ADDR) /* 读取0x39 */
            {
                recv_cnt = 0;
                ptr = &tax_info_all[0] + TAX_BOARD1_PACKAGE_LEN;
                ptr[recv_cnt] = byte;
                recv_cnt++;
                packet_len = TAX_BOARD2_PACKAGE_LEN;
                check_sum = byte;
                is_recving = true;
            }
            else
            {
                /* 未收到通信帧的头部, 是无效的数据. */
                continue;
            }
        }
        else /* 接收数据中 */
        {
            ptr[recv_cnt] = byte;
            recv_cnt++;
            check_sum += byte;
            if (recv_cnt >= packet_len)
            {
                /* 数据收够了, 判断校验和 */
                is_recving = false;
#if 0
                for (i = 0; i < packet_len; i++)
                {
                    log_print(LOG_DEBUG, "%02x ", ptr[i]);
                }
                log_print(LOG_DEBUG, "\n");
#endif
                return (check_sum == 0) ? (sint32_t)packet_len : (sint32_t)-3;
            }
        }
    }
    return (sint32_t)0;
}

/*******************************************************
 *
 * @brief  发送TAX2应答信息
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t tax_send_tax2_back(void)
{
    uint8_t buf[TAX_ACK_MSG_LEN] = {0};
    sint32_t ret = 0;

    if (tax_read_echo_event_list(echo_list, buf) != 0) /* 没有缓冲的应答事件通信包 */
    {
        buf[0] = TAX_UNIT_TRACK_DETECT;
        buf[1] = (TAX_RECV_SUCCESS << 4) | (TAX_INVALID << 0);
        tax_set_check_sum(buf, TAX_ACK_NO_MSG_LEN); /* 设置校验和 */
#if 1
        int i = 0;
        printf("send:");
        for (i = 0; i < 3; i++)
            printf(" 0x%02x", buf[i]);
        printf("\n");
#endif
        ret = rt_device_write(uart_dev, 0, buf, 3); /* 发送数据 */
        if (ret < TAX_ACK_MSG_LEN)
        {
            return (sint32_t)-1;
        }
    }
    else /* 有缓冲的应答事件通信包 */
    {
        tax_set_check_sum(buf, TAX_ACK_MSG_LEN); /* 设置校验和 */
#if 1
        int i = 0;
        printf("send:");
        for (i = 0; i < TAX_ACK_MSG_LEN; i++)
            printf(" 0x%02x", buf[i]);
        printf("\n");
#endif
        ret = rt_device_write(uart_dev, 0, buf, TAX_ACK_MSG_LEN); /* 发送数据 */
        if (ret < TAX_ACK_MSG_LEN)
        {
            return (sint32_t)-1;
        }
    }
    return 0;
}

/*******************************************************
 *
 * @brief  TAX线程处理函数
 *
 * @param  *args: 传输参数
 * @retval None 线程结束返回数据
 *
 *******************************************************/
static void *tax_thread(void *args)
{
    sint32_t ret;
    uint8_t unit_code = 0;

    /* 打开和TAX通讯的串口 */
    ret = tax_open(UART_DEVICE_NAME); /* 打开读取TAX信息的串口 */
    if (ret < 0)
    {
        log_print(LOG_ERROR, "tax thread start error.\n");
        return NULL;
    }
    else
    {
        log_print(LOG_ERROR, "tax thread start ok.\n");
    }
    while (true)
    {
        /* 接收数据 */
        // todo, 增加超时控制.
        ret = tax_recv_data();
        if (ret < 0)
        {
            continue;
        }
        else if (ret == TAX_BOARD2_PACKAGE_LEN)
        {
            /**
             * 指将与通讯记录单元通讯的检测单元代号.定为
             * 01-轨道检测,
             * 02-弓网检测,
             * 03-TMIS,
             * 04-DMIS,
             * 05-列控通讯,
             * 06-语音录音,
             * 07-轴温报警,
             * 08-鸣笛检查,
             * 09-预留给备用单元.
             */
            unit_code = tax_info_all[TAX_BOARD1_PACKAGE_LEN + 2];
            switch (unit_code)
            {
            case TAX_UNIT_VOICE: /* 语音录音 */
#if 1
                printf("recv:");
                int i;
                for (i = 0; i < TAX_BOARD1_PACKAGE_LEN + TAX_BOARD2_PACKAGE_LEN; i++)
                    printf(" 0x%02x", tax_info_all[i]);
                printf("\n");
#endif
                tax_send_tax2_back();
                break;
            case TAX_UNIT_TRACK_DETECT: /* 轨道检测 */
                update_tax_data();
                break;
            default: /* 缺省 */
                break;
            }
        }
    }
    return NULL;
}

/*******************************************************
 *
 * @brief  初始化TAX模块,创建TAX通信线程.
 *
 * @retval 0:成功 负数:失败
 *
 *******************************************************/

sint32_t tax_init(void)
{
    sint32_t ret = 0;
    pthread_t tax_tid;
    struct pthread_attr pthread_attr_data;

    /* 申请事件列表缓冲区 */
    echo_list = (tax2_echo_event_list_t *)malloc(sizeof(tax2_echo_event_list_t));
    if (echo_list == NULL)
    {

        return (sint32_t)-1;
    }
    /* 初始化TAX2应答事件列表 */
    tax_init_echo_event_list(echo_list);

    /* 创建TAX线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 5;
    pthread_attr_data.schedparam.sched_priority = 23;
    ret = pthread_create(&tax_tid, &pthread_attr_data, tax_thread, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create tax thread error,error_code:%d.\n", ret);
        return (sint32_t)-1;
    }

    return 0;
}
