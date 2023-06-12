/*******************************************************
 *
 * @FileName: pcm.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 语音模块PCM设备的相关接口实现.
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
#include <stdio.h>
#include <sp_enc.h>

#include "pcm.h"
#include "log.h"

/*******************************************************
 * 宏定义
 *******************************************************/
/* 声卡设备 */
#define SOUND_DEVICE "sound0"

/*******************************************************
 *
 * @brief  PCM初始化化
 *
 * @param  *config: PCM设备的配置
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t pcm_init(pcm_config_t *config)
{
    sint32_t ret = 0;
    struct rt_audio_caps caps;

    /* 判断指针有效性 */
    if (config == NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
    }
    config->size = (uint32_t)PCM_WRITE_BUFFER_SIZE;

    if (config->mode == SOUND_MODE_CAPTURE)
    {
        config->size = (uint32_t)PCM_READ_BUFFER_SIZE;
    }
    else
    {
        config->size = (uint32_t)PCM_WRITE_BUFFER_SIZE;
    }

    config->p_buffer = (char *)malloc(config->size);
    if (config->p_buffer == NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    /* find device */
    config->dev = rt_device_find(SOUND_DEVICE);
    if (config->dev == RT_NULL)
    {
        log_print(LOG_ERROR, "device %s not find. \n", SOUND_DEVICE);
        return (sint32_t)-1;
    }
    else
    {
    }
    /* set the play time */
    config->start = rt_tick_get_millisecond();

    /* open sound device */
    if (config->mode == SOUND_MODE_CAPTURE)
    {
        ret = rt_device_open(config->dev, (rt_uint16_t)RT_DEVICE_OFLAG_RDONLY);
    }
    else
    {
        ret = rt_device_open(config->dev, (rt_uint16_t)RT_DEVICE_OFLAG_WRONLY);
    }

    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "open %s device failed. \n", SOUND_DEVICE);
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 设置音频设备参数 */
    if (config->mode == SOUND_MODE_CAPTURE)
    {
        caps.main_type = AUDIO_TYPE_INPUT;
    }
    else
    {
        caps.main_type = AUDIO_TYPE_OUTPUT;
    }
    caps.sub_type = AUDIO_DSP_PARAM;
    caps.udata.config.samplerate = config->sample_rate;
    caps.udata.config.channels = config->channels;
    caps.udata.config.samplebits = config->format;
    /* caps.udata.value = 0; */
    /* read amrfile header information from file */
#if 0
    log_print(LOG_ERROR, "Information: samplerate %d, channels %d, samplebits %d.\n",
              caps.udata.config.samplerate,
              caps.udata.config.channels, caps.udata.config.samplebits);
#endif

    /* set sampletate,channels, samplebits */
    ret = rt_device_control(config->dev, AUDIO_CTL_CONFIGURE, (void *)&caps);

    /* 设置音量 */
    caps.main_type = AUDIO_TYPE_MIXER;
    caps.sub_type = AUDIO_MIXER_VOLUME;
    caps.udata.value = 60;
    ret = rt_device_control(config->dev, AUDIO_CTL_CONFIGURE, (void *)&caps);

    return ret;
}

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

rt_size_t pcm_write(rt_device_t pcm_dev, const void *buffer_w, rt_size_t size_w)
{
    /* write raw data from sound device */
    return rt_device_write(pcm_dev, (rt_off_t)0, buffer_w, size_w);
}

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

rt_size_t pcm_read(rt_device_t pcm_dev, void *buffer_r, rt_size_t size_r)
{
    /* read raw data from sound device */
    return rt_device_read(pcm_dev, (rt_off_t)0, buffer_r, size_r);
}

/*******************************************************
 *
 * @brief  结束PCM相关配置
 *
 * @param  *config: PCM设备配置数据指针
 * @param  *fp: 读取文件
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t pcm_exit(pcm_config_t *config)
{
    /* 关闭音频设备 */
    if (config->dev != NULL)
    {
        rt_device_close(config->dev);
        config->dev = NULL;
    }
    else
    {
        return (sint32_t)-1;
    }
    config->start = rt_tick_get_millisecond() - config->start;

    /* 释放缓冲区 */
    if (config->p_buffer != NULL)
    {
        free((void *)config->p_buffer);
        config->p_buffer = NULL;
    }
    else
    {
    }

    return 0;
}
