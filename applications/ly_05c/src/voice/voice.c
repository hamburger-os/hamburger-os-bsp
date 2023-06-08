/*******************************************************
 *
 * @FileName: voice.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 语音模块的实现, 用于启动录音线程和放音线程.
 * 
 * Copyright (c) 2023 by thinker, All Rights Reserved. 
 *
*******************************************************/

/*******************************************************
 * 头文件
*******************************************************/

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "voice.h"
#include "data.h"

/*******************************************************
 *
 * @brief  初始化音频模块
 *
 * @retval 0:成功 -1:失败
 * 
*******************************************************/

int voice_init(void)
{
    int ret = 0;

    /* 初始化录音模块 */
    ret = record_init();
    if (ret < 0)
    {
        log_print(LOG_ERROR, "record_init Err:%d", ret);
        return -1;
    }

    /* 初始化放音模块 */
    ret = play_init();
    if (ret < 0)
    {
       log_print(LOG_ERROR, "record_init Err:%d", ret);
        return -1;
    }

    return 0;
}
