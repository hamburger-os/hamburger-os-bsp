/*******************************************************
 *
 * @FileName: pcm.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 语音模块中PCM设备相关的定义和函数声明.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _PCM_H_
#define _PCM_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>

/*******************************************************
 * 宏定义
 *******************************************************/

/*声卡的缓冲区大小,单位:byte. */
#define PCM_WRITE_BUFFER_SIZE (320 * 2)

/*******************************************************
 * 数据结构
 *******************************************************/

/*声卡模式 */
typedef enum
{
    SOUND_MODE_PLAY = 0,
    SOUND_MODE_CAPTURE = 1
} E_SOUND_MODE;

/* PCM设备配置 */
typedef struct _pcm_config_t
{
    rt_device_t dev;      /* PCM设备句柄 */
    uint32_t sample_rate; /* 采样率 */
    rt_uint16_t format;   /* 样本大小 */
    rt_uint16_t channels; /* 通道数 */
    E_SOUND_MODE mode;    /* 声卡模式,播放模式还是录音模式 */
    char *buffer;         /* 缓冲区 */
    uint32_t size;        /* 缓冲区大小 */
    uint32_t start;       /* 播放时间 */
} pcm_config_t;

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  PCM初始化化
 *
 * @param  *config: PCM设备的配置
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int pcm_init(pcm_config_t *config);

/*******************************************************
 *
 * @brief  向PCM设备中写入语音数据
 *
 * @param  *pcm: PCM设备文件句柄
 * @param  *buffer: 语音数据
 * @param  size: 语音数据大小
 * @retval 实际写入的数据长度
 *
 *******************************************************/
rt_size_t pcm_write(rt_device_t dev, const void *buffer, rt_size_t size);

/*******************************************************
 *
 * @brief  从PCM设备中读出语音数据
 *
 * @param  *pcm: PCM设备文件句柄
 * @param  *buffer: 语音数据
 * @param  size: 语音数据大小
 * @retval 实际读出的数据长度
 *
 *******************************************************/
rt_size_t pcm_read(rt_device_t dev, void *buffer, rt_size_t size);

/*******************************************************
 *
 * @brief  结束PCM相关配置
 *
 * @param  *config: PCM设备配置数据指针
 * @param  *fp: 读取文件
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int pcm_exit(pcm_config_t *config);

#endif /*_PCM_H_ */
