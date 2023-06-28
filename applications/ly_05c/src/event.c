/*******************************************************
 *
 * @FileName: event.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 事件管理模块的实现
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "event.h"
#include "voice.h"
#include "data.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 消息队列缓冲区大小 */
#define EVENT_MSG_QUEUE_MAX_LEN 64

/*******************************************************
 * 全局变量
 *******************************************************/

/* 按键延迟处理线程 */
static rt_mq_t event_mq = NULL;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 从事件队列中获取事件 */
static E_EVENT pop_event_queue(void);

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  将事件加入事件队列中
 *
 * @param  event: 事件
 * @retval none
 *
 *******************************************************/
void event_push_queue(E_EVENT event)
{
    if (event_mq != NULL)
    {
        rt_mq_send(event_mq, &event, sizeof(E_EVENT));
    }
    else
    {
    }
}
/*******************************************************
 *
 * @brief  从事件队列中获取事件
 *
 * @param  event: 事件
 * @retval none
 *
 *******************************************************/
static E_EVENT pop_event_queue(void)
{
    rt_err_t ret = 0;
    E_EVENT event;

    /* 从消息队列中接收消息, 阻塞方式 */
    ret = rt_mq_recv(event_mq, (void *)&event, sizeof(E_EVENT), (rt_int32_t)RT_WAITING_FOREVER);
    if (ret < 0)
    {
        event = EVENT_INVALID;
    }
    else
    {
    }
    return event;
}

/*******************************************************
 *
 * @brief  事件管理模块初始化
 *
 * @param  none
 * @retval 0:成功 -1:失败
 *
 *******************************************************/

sint32_t event_init(void)
{
    /* 创建消息队列 */
    event_mq = rt_mq_create("event_mq", sizeof(E_EVENT),
                            (rt_size_t)EVENT_MSG_QUEUE_MAX_LEN,
                            (rt_uint8_t)RT_IPC_FLAG_FIFO);
    if (event_mq == NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
        return (sint32_t)0;
    }
}

/*******************************************************
 *
 * @brief  从事件队列中获取事件
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
void event_run(void)
{
    E_EVENT event;

    while (true)
    {
        /* 取出事件 */
        event = pop_event_queue();
        /* 处理事件 */
        switch (event)
        {
        case EVENT_PLUG_IN_USB: /* 插上U盘 */
            led_set(LED_STATE_USB_PLUGED_IN);
            break;
        case EVENT_UNPLUG_USB: /* 拔下U盘 */
            led_set(LED_STATE_USB_NO);
            break;
        case EVENT_DUMP_START_LAST: /* 开始转储最新文件 */
            play_event(EVENT_DUMP_START_LAST);
            led_set(LED_STATE_DUMPING);
            break;
        case EVENT_DUMP_START_ALL: /* 开始转储全部文件 */
            play_event(EVENT_DUMP_START_ALL);
            led_set(LED_STATE_DUMPING);
            break;
        case EVENT_DUMP_END_LAST: /* 最新文件转储结束 */
            play_event(EVENT_DUMP_END_LAST);
            led_set(LED_STATE_DUMP_SUCCESS);
            break;
        case EVENT_DUMP_END_ALL: /* 全部文件转储结束 */
            play_event(EVENT_DUMP_END_ALL);
            led_set(LED_STATE_DUMP_SUCCESS);
            break;
        case EVENT_DUMP_FAIL: /* 转储失败 */
            play_event(EVENT_DUMP_FAIL);
            led_set(LED_STATE_DUMP_FAIL);
            break;
        case EVENT_DUMP_USB_FULL: /* U盘已满 */
            play_event(EVENT_DUMP_USB_FULL);
            break;
        case EVENT_DUMP_USB_ILLEGAL: /* U盘没有鉴权 */
            play_event(EVENT_DUMP_USB_ILLEGAL);
            break;
        case EVENT_PLAY_LAST: /* 回放最新 */
            if (data_get_pcm_state() != PCM_STATE_RECORDING)
            {
                play_voice();
                led_set(LED_STATE_PLAYING);
            }
            else
            {
            }
            break;
        case EVENT_PLAY_END: /* 回放结束 */
            if (data_get_pcm_state() != PCM_STATE_RECORDING)
            {
                led_set(LED_STATE_RS485_NORMAL);
            }
            else
            {
            }
            break;
        case EVENT_TAX_COMM_NORMAL: /* TAX通信正常 */
            led_set(LED_STATE_RS485_NORMAL);
            break;
        case EVENT_TAX_COMM_ERROR: /* TAX通信失败 */
            led_set(LED_STATE_RS485_ERROR);
            break;
        case EVENT_RECORD_START: /* 开始录音 */
            led_set(LED_STATE_RECORDING);
            led_set(LED_STATE_VOICE_DATA_RECORDING);
            break;
        case EVENT_RECORD_END: /* 停止录音 */
            led_set(LED_STATE_RS485_NORMAL);
            led_set(LED_STATE_VOICE_DATA_NO);
            break;
        case EVENT_UPDATE_BEGIN: /* 开始升级程序 */
            play_event(EVENT_UPDATE_BEGIN);
            break;
        case EVENT_UPDATE_SUCCESS: /* 升级程序成功 */
            play_event(EVENT_UPDATE_SUCCESS);
            break;
        case EVENT_UPDATE_FAIL: /* 升级程序失败 */
            play_event(EVENT_UPDATE_FAIL);
            break;
        case EVENT_BEGIN_FORMAT_STORAGE: /* 开始擦除全部语音数据 */
            play_event(EVENT_BEGIN_FORMAT_STORAGE);
            break;
        case EVENT_FINISH_FORMAT_STORAGE: /* 全部语音数据擦除完成 */
            play_event(EVENT_FINISH_FORMAT_STORAGE);
            break;
        case EVENT_WORK_NORMAL: /* 工作正常 */
            led_set(LED_STATE_WORK_NORMAL);
            break;
        case EVENT_WORK_ERROR: /* 工作故障 */
            led_set(LED_STATE_WORK_ERROR);
            break;
        case EVENT_SYS_WORK_OK: /* 系统工作正常 */
            led_set(LED_STATE_SYS_OK);
            break;
        case EVENT_SYS_WORK_ERR: /* 系统工作错误 */
            led_set(LED_STATE_SYS_ERROR);
            break;
        default: /* 缺省 */
            break;
        }
    }
}
