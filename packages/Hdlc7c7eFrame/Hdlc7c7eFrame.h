/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-24     myshow       the first version
 */
#ifndef PACKAGES_HDLC7c7eFRAME_HDLC7c7eFRAME_H_
#define PACKAGES_HDLC7c7eFRAME_HDLC7c7eFRAME_H_

#define HDLC7c7eFRAME_START             0x7c
#define HDLC7c7eFRAME_END               0x7e
#define HDLC7c7eFRAME_REPLACE           0x7d
#define HDLC7c7eFRAME_COVER             0x5f

#define HDLC7c7eFRAME_MAX_LEN           256

typedef enum {
    HDLC_SLAVE = 0,
    HDLC_MASTER,
} Hdlc7c7eFrame_Peer;

typedef enum {
    HDLC_ACK_OK = 0x80,
    HDLC_ACK_ERROR = 0x40,
} Hdlc7c7eFrame_Ack;

typedef struct __attribute__ ((packed))
{
    uint8_t start;
    uint8_t src_addr;
    uint8_t dst_addr;
    uint8_t len;
    uint8_t id;
    uint8_t type;
} Hdlc7c7eFrameHead;

typedef struct __attribute__ ((packed))
{
    uint8_t sum;
    uint8_t end;
} Hdlc7c7eFrameEnd;

typedef struct _Hdlc7c7eFrame
{
    //用户初始化需要定义的参数
    Hdlc7c7eFrame_Peer peer;    //设置主从
    uint16_t maxlen;            //设置缓冲区内存大小
    uint8_t addr;               //设置本机地址
    void *user_data;            //用户的自定义数据区

    //用户初始化无需关心的参数
    uint8_t id_count;
    rt_mutex_t tx_mutex;
    rt_mutex_t rx_mutex;
    uint8_t *rx_buffer;
    uint16_t rx_len;

    int8_t (*init)(struct _Hdlc7c7eFrame *hdlc);//用户定义dev初始化函数
    int8_t (*close)(struct _Hdlc7c7eFrame *hdlc);//用户定义dev关闭函数
    uint16_t (*write)(struct _Hdlc7c7eFrame *hdlc, const uint8_t *buffer, uint16_t size);//用户定义dev写函数
    uint16_t (*read)(struct _Hdlc7c7eFrame *hdlc, uint8_t *buffer, uint16_t size);//用户定义dev读函数
    int8_t (*rx_indicate)(struct _Hdlc7c7eFrame *hdlc, Hdlc7c7eFrameHead *head, uint8_t *buffer, Hdlc7c7eFrameEnd *end);//用户定义接收回调函数
} Hdlc7c7eFrame;

/* 初始化一个frame */
int8_t Hdlc7c7eFrame_init(Hdlc7c7eFrame *hdlc);
/* 关闭一个frame */
int8_t Hdlc7c7eFrame_close(Hdlc7c7eFrame *hdlc);

/* 使用frame发送一个cmd */
uint16_t Hdlc7c7eFrame_type(Hdlc7c7eFrame *hdlc, uint8_t dst_addr, uint8_t type, const uint8_t *buffer, uint16_t size);
/* 使用frame发送一个ack */
uint16_t Hdlc7c7eFrame_ack(Hdlc7c7eFrame *hdlc, uint8_t dst_addr, Hdlc7c7eFrame_Ack ack, uint8_t id, uint8_t type, const uint8_t *buffer, uint16_t size);
/* 获取buffer的长度 */
uint16_t Hdlc7c7eFrame_get_buffer_len(Hdlc7c7eFrameHead *head);

/* 用户需要在数据接收的位置调用此函数
      接收到正确数据会自动调用rx_indicate */
int8_t Hdlc7c7eFrame_accept(Hdlc7c7eFrame *hdlc, uint16_t size);

#endif /* PACKAGES_HDLC7c7eFRAME_HDLC7c7eFRAME_H_ */
