/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-06     lvhan       the first version
 */

#include "board.h"
#include <rtthread.h>

#include <math.h>

#include "arm_math.h"

#define DBG_TAG "dsp_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define BLKSIZE 64

#define LOOP(tick, x)       tick = rt_tick_get_millisecond(); \
                            for (rt_base_t i = 0; i < count; i++) \
                            { x; } \
                            tick = rt_tick_get_millisecond() - tick;

#define LOOP_BLK(x)         for (rt_base_t j = 0; j < BLKSIZE; j++) { x; } \

#define LOG_RESULT(label)   if (rt_memcmp(result_f32_cpu, result_f32_dsp, BLKSIZE * 4) == 0) \
                            { \
                                LOG_D("%16s dsp/cpu: %4d/%4d %3d%% (%10d == %10d)" \
                                    , label, tick_dsp, tick_cpu, tick_dsp * 100/tick_cpu, (int)(result_f32_dsp[0]*1000000), (int)(result_f32_cpu[0]*1000000)); \
                            } \
                            else \
                            { \
                                LOG_W("%16s dsp/cpu: %4d/%4d %3d%% (%10d != %10d)(%10d != %10d)" \
                                    , label, tick_dsp, tick_cpu, tick_dsp * 100/tick_cpu \
                                    , (int)(result_f32_dsp[0]*1000000), (int)(result_f32_cpu[0]*1000000) \
                                    , (int)(result_f32_dsp[1]*1000000), (int)(result_f32_cpu[1]*1000000)); \
                            }

void mult_f32(const float32_t * pSrcA, const float32_t * pSrcB, float32_t * pDst, uint32_t blockSize)
{
    uint32_t blkCnt;                               /* Loop counter */

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = A * B */

        /* Multiply input and store result in destination buffer. */
        *pDst++ = (*pSrcA++) * (*pSrcB++);

        /* Decrement loop counter */
        blkCnt--;
    }
}

void scale_f32(const float32_t * pSrc, float32_t scale, float32_t * pDst, uint32_t blockSize)
{
    uint32_t blkCnt;                               /* Loop counter */

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = A * scale */

        /* Scale input and store result in destination buffer. */
        *pDst++ = (*pSrc++) * scale;

        /* Decrement loop counter */
        blkCnt--;
    }
}

void dot_prod_f32(const float32_t * pSrcA, const float32_t * pSrcB, uint32_t blockSize, float32_t * result)
{
    uint32_t blkCnt;                               /* Loop counter */
    float32_t sum = 0.0f;                          /* Temporary return variable */

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */

        /* Calculate dot product and store result in a temporary buffer. */
        sum += (*pSrcA++) * (*pSrcB++);

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store result in destination buffer */
    *result = sum;
}

void vfun_f32(const float32_t * pSrc, float32_t * pDst, uint32_t blockSize, float32_t (*fun)(float32_t))
{
    uint32_t blkCnt;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = log(A) */

        /* Calculate log and store result in destination buffer. */
        *pDst++ = fun(*pSrc++);

        /* Decrement loop counter */
        blkCnt--;
    }
}

void vlog_f32(const float32_t * pSrc, float32_t * pDst, uint32_t blockSize)
{
    uint32_t blkCnt;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = log(A) */

        /* Calculate log and store result in destination buffer. */
        *pDst++ = logf(*pSrc++);

        /* Decrement loop counter */
        blkCnt--;
    }
}

void vexp_f32(const float32_t * pSrc, float32_t * pDst, uint32_t blockSize)
{
    uint32_t blkCnt;

    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = log(A) */

        /* Calculate log and store result in destination buffer. */
        *pDst++ = expf(*pSrc++);

        /* Decrement loop counter */
        blkCnt--;
    }
}

static rt_thread_t dsp_thread = RT_NULL;

static void dsp_thread_entry(void* parameter)
{
    LOG_D("dsp test start...");

    rt_base_t tick_cpu = 0;
    rt_base_t tick_dsp = 0;
    rt_base_t count = 0;

    float32_t para_f32 = 1.234567;

    float32_t para_f32_1[BLKSIZE];
    float32_t para_f32_2[BLKSIZE];
    LOOP_BLK(para_f32_1[j] = para_f32+j;para_f32_2[j] = para_f32+j;);

    float32_t result_f32_cpu[BLKSIZE];
    float32_t result_f32_dsp[BLKSIZE];

#ifdef PKG_CMSIS_DSP_BASIC_MATH
    count = 10000;
    LOOP(tick_cpu, mult_f32(para_f32_1, para_f32_2, result_f32_cpu, BLKSIZE));
    LOOP(tick_dsp, arm_mult_f32(para_f32_1, para_f32_2, result_f32_dsp, BLKSIZE));
    LOG_RESULT("multiplication");

    count = 20000;
    LOOP(tick_cpu, scale_f32(para_f32_1, para_f32, result_f32_cpu, BLKSIZE));
    LOOP(tick_dsp, arm_scale_f32(para_f32_1, para_f32, result_f32_dsp, BLKSIZE));
    LOG_RESULT("scalar");

    count = 20000;
    LOOP(tick_cpu, dot_prod_f32(para_f32_1, para_f32_2, BLKSIZE, result_f32_cpu));
    LOOP(tick_dsp, arm_dot_prod_f32(para_f32_1, para_f32_2, BLKSIZE, result_f32_dsp));
    LOG_RESULT("dot product");
#endif

#ifdef PKG_CMSIS_DSP_FAST_MATH
    count = 2000;
    LOOP(tick_cpu, vfun_f32(para_f32_1, result_f32_cpu, BLKSIZE, sinf));
    LOOP(tick_dsp, vfun_f32(para_f32_1, result_f32_dsp, BLKSIZE, arm_sin_f32));
    LOG_RESULT("sin");

    count = 2000;
    LOOP(tick_cpu, vfun_f32(para_f32_1, result_f32_cpu, BLKSIZE, cosf));
    LOOP(tick_dsp, vfun_f32(para_f32_1, result_f32_dsp, BLKSIZE, arm_cos_f32));
    LOG_RESULT("cos");

    count = 2000;
    LOOP(tick_cpu, vlog_f32(para_f32_1, result_f32_cpu, BLKSIZE));
    LOOP(tick_dsp, arm_vlog_f32(para_f32_1, result_f32_dsp, BLKSIZE));
    LOG_RESULT("logf");

    count = 2000;
    LOOP(tick_cpu, vexp_f32(para_f32_1, result_f32_cpu, BLKSIZE));
    LOOP(tick_dsp, arm_vexp_f32(para_f32_1, result_f32_dsp, BLKSIZE));
    LOG_RESULT("expf");

    count = 2000;
    LOOP(tick_cpu, vfun_f32(para_f32_1, result_f32_cpu, BLKSIZE, sqrtf));
    LOOP(tick_dsp, LOOP_BLK(arm_sqrt_f32(para_f32_1[j], &result_f32_dsp[j])));
    LOG_RESULT("sqrtf");
#endif

    LOG_D("dsp test end.");
    dsp_thread = RT_NULL;
}

static void dsp_test(int argc, char *argv[])
{
    if( dsp_thread != RT_NULL )
    {
        rt_kprintf("dsp thread already exists!\n");
    }
    else
    {
        dsp_thread = rt_thread_create( "dsp",
                                        dsp_thread_entry,
                                        NULL,
                                        4096,
                                        RT_THREAD_PRIORITY_MAX-2,
                                        10);
        if ( dsp_thread != RT_NULL)
        {
            rt_thread_startup(dsp_thread);
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(dsp_test, dsp_test, cmsis dsp test.);
#endif
