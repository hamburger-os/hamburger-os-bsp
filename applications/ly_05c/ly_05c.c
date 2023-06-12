
/*******************************************************
 *
 * @FileName: ly_05c.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: ly-05c产品的主程序实现.
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
#include <event.h>

#include "file_manager.h"
#include "tax.h"
#include "voice.h"
#include "delay.h"
#include "usb.h"
#include "key.h"
#include "log.h"

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  ly-05c的主线程
 *
 * @param  *args: 线程输入参数
 * @retval none
 *
 *******************************************************/
static void *ly_05c_entry(void *args)
{
    sint32_t ret = 0;

    /* 初始化日志模块 */
    ret = log_init();
    if (ret < 0)
    {
        log_print(LOG_ERROR, "init log module error, error code: %d.\n", ret);
    }

    log_print(LOG_INFO, "==========================\n");
    log_print(LOG_INFO, " hello, LY-05C! \n");
    log_print(LOG_INFO, "==========================\n");

    /* 初始化数据管理模块 */
    ret = data_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "init data module error, error code: %d.\n", ret);
    }
#if 1
    /* 初始化LED模块 */
    ret = led_init();
    if (ret < 0)
    {
        log_print(LOG_ERROR, "init led module error, error code: %d.\n", ret);
    }
#endif

#if 1
    /* 初始化文件管理模块 */
    ret = fm_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "init file manager module error, error code: %d.\n", ret);
    }
#endif

#if 1
    /* 语音模块初始化 */
    ret = voice_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "init voice module error, error code: %d.\n", ret);
    }
#endif

#if 1
    /* 初始化TAX通信模块 */
    ret = tax_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "init tax module error, error code: %d.\n", ret);
    }
#endif

    /* 初始化事件管理模块 */
    ret = event_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "init event module error, error code: %d.\n", ret);
    }

#if 1
    /* 播放语音测试 */
    play_event(EVENT_DUMP_START_LAST);
#endif

#if 1
    /* 初始化Key模块 */
    ret = key_init();
    if (ret < 0)
    {
        log_print(LOG_FATAL_ERROR, "key_init error, error code: %d.\n", ret);
    }
#endif

#if 1
    /* 初始化USB模块 */
    ret = usb_init();
    if (ret < 0)
    {
        log_print(LOG_ERROR, "usb_init error, error code: %d.\n", ret);
    }
#endif
    /* 启动事件管理模块 */
    event_run();

    return NULL;
}

/*******************************************************
 *
 * @brief  LY-05C录音板主线程函数
 *
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t ly_05c_init(void)
{
    sint32_t ret = 0;
    pthread_t s_MainTid;
    pthread_attr_t pthread_attr_data;

    /* 创建主线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 5;
    pthread_attr_data.schedparam.sched_priority = 17;
    ret = pthread_create(&s_MainTid, &pthread_attr_data, (void *)ly_05c_entry, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        return (sint32_t)-1;
    }
    return RT_EOK;
}

INIT_APP_EXPORT(ly_05c_init);
