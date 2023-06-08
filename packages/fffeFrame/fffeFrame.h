/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-24     myshow       the first version
 */
#ifndef PACKAGES_FFFEFRAME_FFFEFRAME_H_
#define PACKAGES_FFFEFRAME_FFFEFRAME_H_

#define FFFEFRAME_START             0xfeff
#define FFFEFRAME_END               0xfdff

typedef enum {
    FF_SLAVE = 0,
    FF_MASTER,
} fffeFrame_Peer;

typedef enum {
    FF_ACK_OK = 0x8000,
    FF_ACK_ERROR = 0x4000,
} fffeFrame_Ack;

typedef struct __attribute__ ((packed))
{
    uint16_t start;
    uint16_t len;
    uint16_t cmd;
    uint16_t sn;
} fffeFrameHead;

typedef struct __attribute__ ((packed))
{
    uint32_t check;
    uint16_t end;
} fffeFrameEnd;

typedef struct _fffeFrame
{
    //用户初始化需要定义的参数
    fffeFrame_Peer peer;    //设置主从
    uint16_t maxlen;        //设置缓冲区内存大小
    void *user_data;        //用户的自定义数据区

    //用户初始化无需关心的参数
    uint16_t snCount;
    rt_mutex_t tx_mutex;
    rt_mutex_t rx_mutex;
    uint8_t *rx_buffer;
    uint16_t rx_len;

    int16_t (*init)(struct _fffeFrame *ff);//用户定义dev初始化函数
    int16_t (*close)(struct _fffeFrame *ff);//用户定义dev关闭函数
    uint16_t (*write)(struct _fffeFrame *ff, const uint8_t *buffer, uint16_t size);//用户定义dev写函数
    uint16_t (*read)(struct _fffeFrame *ff, uint8_t *buffer, uint16_t size);//用户定义dev读函数
    int16_t (*rx_indicate)(struct _fffeFrame *ff, fffeFrameHead *head, uint8_t *buffer, fffeFrameEnd *end);//用户定义接收回调函数
} fffeFrame;

/* 初始化一个frame */
int16_t fffeFrame_init(fffeFrame *ff);
/* 关闭一个frame */
int16_t fffeFrame_close(fffeFrame *ff);

/* 使用frame发送一个cmd */
uint16_t fffeFrame_cmd(fffeFrame *ff, uint16_t cmd, const uint8_t *buffer, uint16_t size);
/* 使用frame发送一个ack */
uint16_t fffeFrame_ack(fffeFrame *ff, fffeFrame_Ack ack, uint16_t sn, uint16_t cmd, const uint8_t *buffer, uint16_t size);
/* 获取buffer的长度 */
uint16_t fffeFrame_get_buffer_len(fffeFrameHead *head);

/* 用户需要在数据接收的位置调用此函数
      接收到正确数据会自动调用rx_indicate */
int16_t fffeFrame_accept(fffeFrame *ff, uint16_t size);

#endif /* PACKAGES_FFFEFRAME_FFFEFRAME_H_ */
