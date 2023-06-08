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

#define SOUND_DEVICE "sound0"

/*******************************************************
 *
 * @brief  PCM初始化化
 *
 * @param  *config: PCM设备的配置
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int pcm_init(pcm_config_t *config)
{
    int ret = 0;
    struct rt_audio_caps caps;

    /* 判断指针有效性 */
    if (config == NULL)
        return -1;
    config->size = PCM_WRITE_BUFFER_SIZE;

    config->buffer = (char *)malloc(PCM_WRITE_BUFFER_SIZE);
    if (config->buffer == NULL)
        return -1;

    /* find device */
    config->dev = rt_device_find(SOUND_DEVICE);
    if (config->dev == RT_NULL)
    {
        log_print(LOG_ERROR, "device %s not find. \n", SOUND_DEVICE);
        return -1;
    }
    /* set the play time */
    config->start = rt_tick_get_millisecond();

    /* open sound device */
    if (config->mode == SOUND_MODE_CAPTURE)
        ret = rt_device_open(config->dev, RT_DEVICE_OFLAG_RDONLY);
    else
        ret = rt_device_open(config->dev, RT_DEVICE_OFLAG_WRONLY);

    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "open %s device failed. \n", SOUND_DEVICE);
        return -1;
    }
    LOG_I("open sound, device %s", SOUND_DEVICE);

    /* 设置音频设备参数 */
    if (config->mode == SOUND_MODE_CAPTURE)
        caps.main_type = AUDIO_TYPE_INPUT;
    else
        caps.main_type = AUDIO_TYPE_OUTPUT;
    caps.sub_type = AUDIO_DSP_PARAM;
    caps.udata.config.samplerate = config->sample_rate;
    caps.udata.config.channels = config->channels;
    caps.udata.config.samplebits = config->format;
    /*caps.udata.value = 0; */
    /* read amrfile header information from file */
    log_print(LOG_ERROR, "Information: samplerate %d, channels %d, samplebits %d.\n",
          caps.udata.config.samplerate, caps.udata.config.channels, caps.udata.config.samplebits);

    /* set sampletate,channels, samplebits */
    ret = rt_device_control(config->dev, AUDIO_CTL_CONFIGURE, &caps);

    /* 设置音量 */
    caps.main_type = AUDIO_TYPE_MIXER;
    caps.sub_type = AUDIO_MIXER_VOLUME;
    caps.udata.value = 60;
    ret = rt_device_control(config->dev, AUDIO_CTL_CONFIGURE, &caps);

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

rt_size_t pcm_write(rt_device_t dev, const void *buffer, rt_size_t size)
{
    /* write raw data from sound device */
    return rt_device_write(dev, 0, buffer, size);
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

rt_size_t pcm_read(rt_device_t dev, void *buffer, rt_size_t size)
{
    /* read raw data from sound device */
    return rt_device_read(dev, 0, buffer, size);
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
int pcm_exit(pcm_config_t *config)
{
    /* 判断数据指针有效性 */
    if (config->dev == NULL)
        return -1;

    /* 关闭音频设备 */
    if (config->dev)
    {
        rt_device_close(config->dev);
        config->dev = NULL;
    }
    config->start = rt_tick_get_millisecond() - config->start;

    /* 释放缓冲区 */
    if (config->buffer != NULL)
    {
        free(config->buffer);
        config->buffer = NULL;
    }

    return 0;
}
