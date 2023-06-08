/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2019-07-16     Zero-Free    first implementation
 */

#ifndef __WAVRECORDER_H__
#define __WAVRECORDER_H__

#include <rtthread.h>

struct wavrecord_info
{
    char *uri;
    rt_uint32_t samplerate;
    rt_uint16_t channels;
    rt_uint16_t samplebits;
};

/**
 * @brief             Start to record
 *
 * @param info        wavfile informations
 *
 * @return
 *      - RT_EOK      Success
 *      - < 0         Failed
 */
rt_err_t wavrecorder_start(struct wavrecord_info *info);

/**
 * @brief             Stop record
 *
 * @return
 *      - RT_EOK      Success
 *      - < 0         Failed
 */
rt_err_t wavrecorder_stop(void);

#endif
