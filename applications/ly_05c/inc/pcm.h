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

#ifndef PCM_H_
#define PCM_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include "type.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 声卡的缓冲区大小,单位:byte. */
#define PCM_WRITE_BUFFER_SIZE (320 * 2)
#define PCM_READ_BUFFER_SIZE 2048

/*******************************************************
 * 数据结构
 *******************************************************/

/* 声卡模式 */
typedef enum
{
    SOUND_MODE_PLAY = 0,   /* 放音状态 */
    SOUND_MODE_CAPTURE = 1 /* 录音状态 */
} E_SOUND_MODE;

/* PCM设备配置 */
typedef struct
{
    rt_device_t dev;      /* PCM设备句柄 */
    uint32_t sample_rate; /* 采样率 */
    rt_uint16_t format;   /* 样本大小 */
    rt_uint16_t channels; /* 通道数 */
    E_SOUND_MODE mode;    /* 声卡模式,播放模式还是录音模式 */
    char *p_buffer;       /* 缓冲区 */
    uint32_t size;        /* 缓冲区大小 */
    uint32_t start;       /* 播放时间 */
} pcm_config_t;

/*******************************************************
 * 函数声明
 *******************************************************/
/*******************************************************
 *
 * @brief  pcm模块数据初始化
 *
 * @param  无
 * @retval 无
 *
 *******************************************************/
void pcm_init_data(void);
/*******************************************************
 *
 * @brief  单例获取pcm配置
 *
 * @param  无
 * @retval pcm_config_t * pcm配置的数据指针.
 *
 *******************************************************/
pcm_config_t *pcm_get_config_instance(void);
/*******************************************************
 *
 * @brief  PCM初始化化
 *
 * @param  *config: PCM设备的配置
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t pcm_init(pcm_config_t *config);

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
rt_size_t pcm_write(rt_device_t pcm_dev, const void *buffer_w, rt_size_t size_w);

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
rt_size_t pcm_read(rt_device_t pcm_dev, void *buffer_r, rt_size_t size_r);

/*******************************************************
 *
 * @brief  结束PCM相关配置
 *
 * @param  *config: PCM设备配置数据指针
 * @param  *fp: 读取文件
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t pcm_exit(pcm_config_t *config);

#endif /* _PCM_H_ */
