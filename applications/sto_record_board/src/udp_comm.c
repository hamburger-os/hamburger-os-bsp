/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-18     zm       the first version
 */
#include "udp_comm.h"

#define DBG_TAG "udp_comm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>

#if !defined(SAL_USING_POSIX)
#error "Please enable SAL_USING_POSIX!"
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <sys/socket.h> /* socket.h header file is needed when using BSD socket */ /* 使用BSD socket，需要包含socket.h头文件 */
#include "netdb.h"

#define UDP_RCV_THREAD_STACK_SIZE   (1024 * 2)
#define UDP_RCV_THREAD_PRIORITY     (15)
#define UDP_RCV_THREAD_TIMESLICE    (20)

#define UDP_SERVER_PORT    (8090U)
#define UDP_RCV_BUFSZ      (1500U)

#define UDP_RCV_MQ_NUM     (10)



uint8_t udp_recv_buffer[UDP_RCV_BUFSZ];
extern uint32_t GetNewDatagram( uint8_t dgm[], uint32_t len );
extern uint32_t GetDownloadDatagram( uint8_t dgm[], uint32_t size );

typedef struct {
    int sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    uint8_t *recv_data;
    uint8_t *recv_data_by_mq;
    uint32_t recv_len;
    rt_mq_t rcv_mq;
    rt_mutex_t mutex;
} S_UDP_SERVER;

static S_UDP_SERVER udp_server_dev;

void UDPServerSendData(const void *data, size_t size)
{
    S_UDP_SERVER *dev = &udp_server_dev;

    sendto(dev->sock, data, size, 0,
           (struct sockaddr *)&dev->client_addr, sizeof(struct sockaddr));

    rt_thread_mdelay(1000);
}

static rt_err_t UDPServerInit(S_UDP_SERVER *dev)
{

    memset(dev, 0, sizeof(S_UDP_SERVER));

    dev->rcv_mq = rt_mq_create("udp mq", sizeof(uint8_t) * UDP_RCV_BUFSZ, UDP_RCV_MQ_NUM, RT_IPC_FLAG_FIFO);
    if(RT_NULL == dev->rcv_mq)
    {
        LOG_E("rt_mq_create failed");
        return -RT_ERROR;
    }

    /* 创建一个动态互斥量 */
    dev->mutex = rt_mutex_create("udp mutex", RT_IPC_FLAG_PRIO);
    if (RT_NULL == dev->mutex)
    {
        LOG_E("create udp mutex failed");
        return -1;
    }

    /* Allocate space for recv_data */
    /* 分配接收用的数据缓冲 */
    dev->recv_data = rt_malloc(UDP_RCV_BUFSZ);
    if (dev->recv_data == RT_NULL)
    {
        LOG_E("rcv data no memory");
        return -RT_ERROR;
    }

    dev->recv_data_by_mq = rt_malloc(UDP_RCV_BUFSZ);
    if (dev->recv_data_by_mq == RT_NULL)
    {
        rt_free(dev->recv_data);
        dev->recv_data = RT_NULL;
        LOG_E("rcv data by mq no memory");
        return -RT_ERROR;
    }

    /* Initialize server side address */
    /* 初始化服务端地址 */
    dev->server_addr.sin_family = AF_INET;
    dev->server_addr.sin_port = htons(UDP_SERVER_PORT);
    dev->server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(dev->server_addr.sin_zero), 0, sizeof(dev->server_addr.sin_zero));
    return RT_EOK;
}

static void UDPServerRcvThreadEntry(void *paramemter)
{
    int bytes_read;
    socklen_t addr_len;
    rt_err_t ret = -RT_ERROR;

    struct timeval timeout;
    fd_set readset;

    S_UDP_SERVER *dev = &udp_server_dev;

    if(UDPServerInit(dev) !=RT_EOK)
    {
        LOG_E("UDPServerInit(&udp_server_dev) error");
        goto __exit;
    }

    /* Create a socket and set it to SOCK_DGRAM(UDP) */
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((dev->sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG_E("Create socket error");
        goto __exit;
    }

    /* Bind socket to server side address */
    /* 绑定socket到服务端地址 */
    if (bind(dev->sock, (struct sockaddr *)&dev->server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        LOG_E("Unable to bind");
        goto __exit;
    }

    addr_len = sizeof(struct sockaddr);
    LOG_I("UDPServer Waiting for client on port %d...", UDP_SERVER_PORT);


    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (1)
    {

        FD_ZERO(&readset);
        FD_SET(dev->sock, &readset);

        /* Wait for read or write */
        if (select(dev->sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
        {
            continue;
        }
        /* The maximum size received from sock is BUFSZ-1 bytes*/
        /* 从sock中收取最大BUFSZ - 1字节数据 */
        bytes_read = recvfrom(dev->sock, dev->recv_data, UDP_RCV_BUFSZ - 1, 0,
                              (struct sockaddr *)&dev->client_addr, &addr_len);
        if (bytes_read < 0)
        {
            LOG_E("Received error, close the connect.");
            goto __exit;
        }
        else if (bytes_read == 0)
        {
            LOG_W("Received warning, recv function return 0.");
            continue;
        }
        else
        {
            dev->recv_data[bytes_read] = '\0'; /* Append '\0' at the end of message *//* 把末端清零 */
            /* Output received message */
            /* 输出接收的数据 */
            LOG_D("Received data = %s size = %d", dev->recv_data, bytes_read);
//            UDPServerSendData(dev->recv_data, rt_strlen(dev->recv_data));


            rt_mutex_take(dev->mutex, RT_WAITING_FOREVER);
            dev->recv_len = bytes_read;
            rt_mutex_release(dev->mutex);

            ret = rt_mq_send(dev->rcv_mq, (const void *)dev->recv_data, sizeof(uint8_t) * UDP_RCV_BUFSZ);
            if(ret != RT_EOK)
            {
                LOG_E("udp mq send error %d", ret);
            }
        }
    }
__exit:
    if (dev->recv_data)
    {
        rt_free(dev->recv_data);
        dev->recv_data = RT_NULL;
    }

    if(dev->recv_data_by_mq)
    {
        rt_free(dev->recv_data_by_mq);
        dev->recv_data_by_mq = RT_NULL;
    }
    if (dev->sock >= 0)
    {
        closesocket(dev->sock);
        dev->sock = -1;
    }
}

rt_err_t UDPServerRcvMQData(void)
{
    rt_err_t ret = -RT_ERROR;
    S_UDP_SERVER *dev = &udp_server_dev;
    uint32_t udp_recv_len;

    ret = rt_mq_recv(dev->rcv_mq, (void *)dev->recv_data_by_mq, sizeof(uint8_t) * UDP_RCV_BUFSZ, 0);
    if(ret != RT_EOK)
    {
        return -RT_ERROR;
    }

    rt_mutex_take(dev->mutex, RT_WAITING_FOREVER);
    udp_recv_len = dev->recv_len;
    rt_mutex_release(dev->mutex);
    LOG_D("Rcv mq data = %s len %d", dev->recv_data_by_mq, udp_recv_len);

    memset(udp_recv_buffer, 0, UDP_RCV_BUFSZ);
    memcpy(udp_recv_buffer, dev->recv_data_by_mq, udp_recv_len);

//    for(int i = 0; i < udp_recv_len; i++)
//    {
//        LOG_D("udp_recv_buffer i = %d, data %d", i, udp_recv_buffer[i]);
//    }

    /* 2. Copy network data to error code buffer. */
    GetNewDatagram( udp_recv_buffer, udp_recv_len );

    /* 3. Copy network data to recording file buffer. */
    GetDownloadDatagram( udp_recv_buffer, udp_recv_len );

    return RT_EOK;
}

int UDPServerRcvThreadInit(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("udp rcv",
                            UDPServerRcvThreadEntry, RT_NULL,
                            UDP_RCV_THREAD_STACK_SIZE,
                            UDP_RCV_THREAD_PRIORITY, UDP_RCV_THREAD_TIMESLICE);

    if(tid != NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

