/*******************************************************
 *
 * @FileName: event.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 事件管理模块头文件.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef EVENT_H_
#define EVENT_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include "pthread.h"
#include "type.h"
#include "led.h"

/*******************************************************
 * 数据结构
 *******************************************************/
typedef enum
{
    /* 转储事件 */
    EVENT_PLUG_IN_USB,      /* 插上U盘 */
    EVENT_UNPLUG_USB,       /* 拔下U盘 */
    EVENT_DUMP_START_LAST,  /* 开始转储最新文件 */
    EVENT_DUMP_START_ALL,   /* 开始转储全部文件 */
    EVENT_DUMP_END_LAST,    /* 最新文件转储结束 */
    EVENT_DUMP_END_ALL,     /* 全部文件转储结束 */
    EVENT_DUMP_FAIL,        /* 转储失败 */
    EVENT_DUMP_USB_FULL,    /* U盘已满 */
    EVENT_DUMP_USB_ILLEGAL, /* U盘没有鉴权 */

    /* 播放事件 */
    EVENT_PLAY_LAST, /* 回放最新 */
    EVENT_PLAY_END,  /* 回放结束 */

    /* 通信事件 */
    EVENT_TAX_COMM_NORMAL, /* TAX通信正常 */
    EVENT_TAX_COMM_ERROR,  /* TAX通信失败 */

    /* TAX通信 */
    EVENT_RECORD_START, /* 开始录音 */
    EVENT_RECORD_END,   /* 录音结束 */

    /* 维护事件 */
    EVENT_UPDATE_BEGIN,   /* 开始升级程序 */
    EVENT_UPDATE_SUCCESS, /* 升级程序成功 */
    EVENT_UPDATE_FAIL,    /* 升级程序失败 */

    /* 文件管理事件 */
    EVENT_BEGIN_FORMAT_STORAGE,  /* 开始擦除全部语音数据,请稍后. */
    EVENT_FINISH_FORMAT_STORAGE, /* 全部语音数据擦除完成 */

    /* 文件管理事件 */
    EVENT_WORK_NORMAL, /* 工作正常 */
    EVENT_WORK_ERROR,  /* 工作故障 */

    /* 无效事件 */
    EVENT_INVALID,

} E_EVENT;

/*******************************************************
 *
 * @brief  将事件加入事件队列中
 *
 * @param  event: 事件
 * @retval none
 *
 *******************************************************/
void event_push_queue(E_EVENT event);

/*******************************************************
 *
 * @brief  事件管理模块初始化
 *
 * @param  none
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t event_init(void);

/*******************************************************
 *
 * @brief  从事件队列中获取事件
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
void event_run(void);

#endif
