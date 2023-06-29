/*******************************************************
 *
 * @FileName: rtc.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 用于访问系统时间.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/time.h>
#include "rtc.h"

// #include <time.h>

/*******************************************************
 *
 * @brief  设置系统时间
 *
 * @param  无
 * @retval tptr * 系统时间数据.
 *
 *******************************************************/
void rtc_setdata(struct tm *ptm)
{
    struct timeval tv;
    if (ptm == NULL)
    {
        return;
    }
    tv.tv_sec = mktime(ptm);
    tv.tv_usec = 0;
    int ret = settimeofday(&tv, NULL);
    if (ret < 0)
    {
        // printf("settimeofday error. ret = %d\n", ret);
    }
}

/*******************************************************
 *
 * @brief  测试rtc函数
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
void rtc_test(void)
{
    struct tm tm;
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 1 - 1;
    tm.tm_mday = 2;
    tm.tm_hour = 3;
    tm.tm_min = 15;
    tm.tm_sec = 16;

    rtc_setdata(&tm);
}
