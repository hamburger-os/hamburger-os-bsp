/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 日志模块的实现.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _LOG_H_
#define _LOG_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include "type.h"

/*******************************************************
 * 宏定义
 *******************************************************/

#define LOG_DEBUG 0       /*调试信息 */
#define LOG_INFO 1        /*信息 */
#define LOG_WARNING 2     /*警告信息 */
#define LOG_ERROR 3       /*错误 */
#define LOG_FATAL_ERROR 3 /*重大错误 */

/*当前的打印等级 */
#define LOG_LEVEL LOG_DEBUG

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  初始化提示音播放列表
 *
 * @param  level: 日志记录等级
 * @param  format: 记录格式
 * @retval none
 *
 *******************************************************/
void log_print(int level, char *format, ...);

/*******************************************************
 *
 * @brief  日志模块初始化
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
int log_init(void);

#endif
