/*
 * Copyright (c) 2006-2030, Hnhinker Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2023-03-29     ccy    first implementation
 */

#ifndef __AMRRECORDER_H__
#define __AMRRECORDER_H__

#include <rtthread.h>

struct amrrecord_info
{
    char *uri;
    rt_uint32_t samplerate;
    rt_uint16_t channels;
    rt_uint16_t samplebits;
};

/**
 * @brief             Start to record
 *
 * @param info        amrfile informations
 *
 * @return
 *      - RT_EOK      Success
 *      - < 0         Failed
 */
rt_err_t amrrecorder_start(struct amrrecord_info *info);

/**
 * @brief             Stop record
 *
 * @return
 *      - RT_EOK      Success
 *      - < 0         Failed
 */
rt_err_t amrrecorder_stop(void);

#endif
