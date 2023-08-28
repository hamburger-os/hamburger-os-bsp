
/*******************************************************
 *
 * @FileName: ly_05c.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: TAX的RS485接口测试.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>

/*******************************************************
 *
 * @brief  LY-05C录音板主线程函数
 *
 * @retval 0:成功 <0:失败
 *
 *******************************************************/

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
#include <stdbool.h>
#include <stdio.h>

#define DBG_TAG "tax"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*******************************************************
 * 宏定义
 *******************************************************/

#define UART_DEVICE_NAME BSP_DEV_TABLE_UART3 /* 串口设备 */

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

/*******************************************************
 * 全局变量
 *******************************************************/

/* TAX接收缓冲区 */
static unsigned char tax_info_all[TAX_BOARD1_PACKAGE_LEN + TAX_BOARD2_PACKAGE_LEN] = {0};
/* uart设备 */
static rt_device_t uart_dev;
/* uart消息队列,用于uart底层向tax线程发送消息. */
static rt_mq_t uart_mq;
/* 标志位, 表示测试的运行状态*/
static volatile bool rs485_test_running = true;

static rt_thread_t tax_test_thread = RT_NULL;

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  rs485接收回调函数.
 *
 * @param  *dev: rs485设备
 * @param  size: rs485字符串的大小
 * @retval 错误码
 *
 *******************************************************/
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;
    struct rx_msg msg;

    msg.dev = dev;
    msg.size = size;
    if (size > BUFFER_LEN)
    {
        size = BUFFER_LEN;
    }
    result = rt_mq_send(uart_mq, (void *)&msg, sizeof(msg));

    return result;
}
/*******************************************************
 *
 * @brief  打开rs485设备.
 *
 * @param  *name: 设备名
 * @retval >=0: 设备号 <0:打开设备失败
 *
 *******************************************************/
static int tax_open(char *name)
{
    rt_err_t ret;

    /* 查找串口设备 */
    uart_dev = rt_device_find(name);
    if (uart_dev == RT_NULL)
    {
        rt_kprintf("can not find %s  device.\n", name);
        return -1;
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
        rt_kprintf("rt_device_control error!\n");
        rt_device_close(uart_dev);
        return -1;
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
        rt_kprintf("init uart mq error. ");
        rt_device_close(uart_dev);
        return -1;
    }

    /* 设置接收回调函数 */
    ret = rt_device_set_rx_indicate(uart_dev, uart_input);
    if (ret != RT_EOK)
    {
        rt_kprintf("set uart recv callback error. error_code:%d.\n", ret);
        rt_mq_delete(uart_mq);
        rt_device_close(uart_dev);
        return -1;
    }

    return 0;
}
/*******************************************************
 *
 * @brief  关闭rs485设备,并释放相关资源.
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
static void tax_close(void)
{
    if (uart_dev != RT_NULL)
    {
        rt_device_close(uart_dev);
    }

    if (uart_mq != RT_NULL)
    {
        rt_thread_mdelay(1000);
        rt_mq_delete(uart_mq);
    }
}
/*******************************************************
 *
 * @brief  从rs485接收串口数据
 *
 * @retval 接收的字节数
 *
 *******************************************************/
static int tax_recv_data(void)
{
    struct rx_msg msg;              /* 消息队列中的一个消息数据 */
    static bool is_recving = false; /* 接收通信数据中 */
    static unsigned char *ptr = NULL;
    static unsigned char packet_len /* 通信包的长度*/, recv_cnt, check_sum /* 校验和结果*/;
    static int len = 0 /*未处理的数据长度*/, len_r /*从缓冲区中读取的数据长度*/;
    static unsigned char buf[BUFFER_LEN] = {0};
    int ret = 0;
    unsigned char byte = 0;

    /* 清空消息队列 */
    rt_memset((void *)&msg, 0, sizeof(msg));
    /* 环形缓冲区中的数据已经处理完成, 开始接受新的数据. */
    if (len <= 0)
    {
        /* 从消息队列中读取消息 */
        ret = rt_mq_recv(uart_mq, (void *)&msg, sizeof(msg),
                         (rt_int32_t)READ_MESSAGE_QUEUE_TIMEOUT);
        if (ret != RT_EOK)
        {
            return -1;
        }
        /* 读取串口数据. */
        rt_memset(buf, 0, sizeof(buf));
        len_r = rt_device_read(uart_dev, 0, buf, msg.size);
        if (len_r < 0)
        {
            return -2;
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

                return (check_sum == 0) ? packet_len : -3;
            }
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
static void tax_thread(void *args)
{
    int ret = 0;

    /* 打开和TAX通讯的串口 */
    ret = tax_open(UART_DEVICE_NAME); /* 打开读取TAX信息的串口 */
    if (ret < 0)
    {
        return ;
    }

    /* 尝试10次接收数据 */
    rs485_test_running = true;
    while (rs485_test_running)
    {
        /* 接收数据 */
        ret = tax_recv_data();
        if (ret > 0)
        {
            rt_kprintf("Get Tax RS485 Data.\n");
        }
    }
    tax_close(); /* 关闭设备 */
    rs485_test_running = false;

    tax_test_thread = RT_NULL;

}
/*******************************************************
 *
 * @brief  rs485测试程序
 *
 * @param  *args: 传输参数
 * @retval 0:成功 <0:失败.
 *
 *******************************************************/
int ly_05c1_tax_test(int argc, char *argv[])
{
    if (argc != 2)
    {
        rt_kprintf("Usage: \n");
        rt_kprintf("    TaxInfo -t      start tax test.\n");
    }
    else
    {
        if (strcmp(argv[1], "-t") == 0) /* 开始测试 */
        {
            if (tax_test_thread != RT_NULL)
            {
                rt_kprintf("tax test thread already exists!\n");
            }
            /* 创建TAX线程 */
            tax_test_thread =
                rt_thread_create("tax_test",
                                 tax_thread,
                                 NULL,
                                 5 * 1024,
                                 23,
                                 10);
            if (tax_test_thread != RT_NULL)
            {
                rt_thread_startup(tax_test_thread);
            }
            
            /* 结束 */
            while (getchar() != 'c')
            {
                rt_thread_mdelay(10);
            }
            rs485_test_running = false;
        }
        else
        {
            rt_kprintf("Usage: \n");
            rt_kprintf("    TaxInfo -t      start tax test.\n");
        }
    }

    return 0;
}

MSH_CMD_EXPORT_ALIAS(ly_05c1_tax_test, TaxInfo, ly - 05c tax test);
