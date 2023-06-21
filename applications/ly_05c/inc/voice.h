/*******************************************************
 *
 * @FileName: voice.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 语音模块头文件
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef VOICE_H_
#define VOICE_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "event.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/*******************************************************
 *
 * @brief  播放语音事件
 *
 * @param  event: 事件
 * @retval 无
 *
 *******************************************************/
void play_event(E_EVENT event);

/*******************************************************
 *
 * @brief  播放最后一条录音
 *
 * @retval None
 *
 *******************************************************/
void play_voice(void);

/*******************************************************
 *
 * @brief  立即停止播放录音
 *
 * @retval None
 *
 *******************************************************/
void play_stop(void);

/*******************************************************
 *
 * @brief  放音模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t play_init(void);

/*******************************************************
 *
 * @brief  开始录制语音
 *
 * @retval none
 *
 *******************************************************/
sint32_t record_start_record(void);

/*******************************************************
 *
 * @brief  停止录制语音
 *
 * @retval none
 *
 *******************************************************/
sint32_t record_stop_record(void);

/*******************************************************
 *
 * @brief  录音模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t record_init(void);

/*******************************************************
 *
 * @brief  初始化音频模块
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t voice_init(void);


void set_play_detail_state(int state);
int get_play_detail_state();
#endif
