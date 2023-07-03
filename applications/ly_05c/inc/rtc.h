/*******************************************************
 *
 * @FileName: rtc.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 用于访问系统时间.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _RTC_H_
#define _RTC_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include <time.h>

/*******************************************************
 *
 * @brief  设置系统时间
 *
 * @param  无
 * @retval tptr * 系统时间数据.
 *
 *******************************************************/
void rtc_setdata(struct tm *ptm);

#endif
