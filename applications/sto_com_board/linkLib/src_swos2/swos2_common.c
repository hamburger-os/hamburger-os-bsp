/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/common.h"

#define DBG_TAG "common"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

static char fmt_buf[HAL_OS_FMT_BUF_SIZE] = { 0 };
sint32 MY_Printf( const char *fmt,... )
{

//    sint32 vsn_len_s32 = 0;
//    va_list args;
//
//    va_start(args, fmt);
//
//    vsn_len_s32 = rt_vsnprintf((char *)fmt_buf, HAL_OS_FMT_BUF_SIZE - 1, fmt, args );
//
//    if( vsn_len_s32 > (sint32)( HAL_OS_FMT_BUF_SIZE - 1U ))
//    {
//        vsn_len_s32 = (sint32)( HAL_OS_FMT_BUF_SIZE - 1U );
//    }
//
//    va_end(args);
//
//    return rt_kprintf("%s", fmt_buf);
}

sint32 MY_PrintfLog( const char *fmt,... )
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, fmt);

    ulog_voutput(DBG_LOG, "app", RT_TRUE, fmt, args);

    va_end(args);

    return 0;
}

/*******************************************************************************************
 ** @brief: int8_abs
 ** @param: a
 *******************************************************************************************/
sint8 int8_abs(sint8 a)
{
    if (a < (sint8) 0)
    {
        return (sint8) -a;
    }
    else
    {
        return a;
    }
}
/*******************************************************************************************
 ** @brief: int16_abs
 ** @param: a
 *******************************************************************************************/
sint16 int16_abs(sint16 a)
{
    if (a < (sint16) 0)
    {
        return (sint16) -a;
    }
    else
    {
        return a;
    }
}

/*******************************************************************************************
 ** @brief: int32_abs
 ** @param: a
 *******************************************************************************************/
sint32 int32_abs(sint32 a)
{
    if (a < (sint32) 0)
    {
        return -a;
    }
    else
    {
        return a;
    }
}

/*******************************************************************************************
 ** @brief: int64_abs
 ** @param: a
 *******************************************************************************************/
sint64 int64_abs(sint64 a)
{
    if (a < (sint64) 0)
    {
        return -a;
    }
    else
    {
        return a;
    }
}

/*******************************************************************************************
 ** @brief: f32_abs
 ** @param: a
 *******************************************************************************************/
real32 f32_abs(real32 a)
{
    if (a < (real32) 0)
    {
        return -a;
    }
    else
    {
        return a;
    }
}

/*******************************************************************************************
 ** @brief: d64_abs
 ** @param: a
 *******************************************************************************************/
real64 d64_abs(real64 a)
{
    if (a < (real64) 0)
    {
        return -a;
    }
    else
    {
        return a;
    }
}

/*******************************************************************************************
 ** @brief: real32_sqrt
 ** @param: a
 *******************************************************************************************/
real32 f32_sqrt(real32 a)
{
    real32 xhalf = 0.5f * a;
    sint32 i = *(sint32 *) &a;

    i = 0x5f375a86 - (i >> 1);
    a = *(real32 *) &i;
    a = a * (1.5f - xhalf * a * a);
    a = a * (1.5f - xhalf * a * a);
    a = a * (1.5f - xhalf * a * a);
    if (0 == (sint32) a)
    {
        return 0;
    }
    return 1 / a;
}
