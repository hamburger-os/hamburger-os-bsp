/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: LED模块的初始化
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef LED_H_
#define LED_H_

#include "type.h"

/*******************************************************
 * 数据结构
 *******************************************************/

/* LED的状态 */
typedef enum
{
    LED_STATE_RS485_NORMAL,         /* 485通信正常 */
    LED_STATE_RS485_ERROR,          /* 未收到正确的485数据 */
    LED_STATE_RECORDING,            /* 正在录音 */
    LED_STATE_PLAYING,              /* 正在放音 */
    LED_STATE_DUMPING,              /* 正在转储 */
    LED_STATE_DUMP_SUCCESS,         /* 转储成功 */
    LED_STATE_DUMP_FAIL,            /* 转储失败 */
    LED_STATE_VOICE_DATA_RECORDING, /* 正在记录语音数据 */
    LED_STATE_VOICE_DATA_NO,        /* 没有记录语音数据 */
    LED_STATE_WORK_NORMAL,          /* 工作指示 */
    LED_STATE_WORK_ERROR,           /* 故障指示 */
    LED_STATE_USB_PLUGED_IN,        /* 插上U盘 */
    LED_STATE_USB_NO,               /* 没插U盘 */
    LED_STATE_SYS_OK,               /* 系统工作正常 */
    LED_STATE_SYS_ERROR,            /* 系统工作错误 */
} E_LED_State;

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  设置LED灯状态
 *
 * @param  led_state: led灯的状态
 * @retval none
 *
 *******************************************************/
void led_set(E_LED_State led_state);

/*******************************************************
 *
 * @brief  初始化LED模块
 *
 * @param  args: led显示线程参数
 * @retval sint32_t 0:成功 <0:失败
 *
 *******************************************************/
sint32_t led_init(void);

#endif
