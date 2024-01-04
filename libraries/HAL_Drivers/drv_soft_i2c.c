/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#include <board.h>

#ifdef BSP_USING_SOFT_I2C

#if defined(RT_USING_I2C) && defined(RT_USING_I2C_BITOPS) && defined(RT_USING_PIN)
#include "drv_soft_i2c.h"
#include "drv_config.h"

//#define DRV_DEBUG
#define LOG_TAG              "drv.si2c"
#ifdef DRV_DEBUG
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

enum
{
#ifdef BSP_USING_SOFT_I2C1
    SOFT_I2C1_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C2
    SOFT_I2C2_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C3
    SOFT_I2C3_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C4
    SOFT_I2C4_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C5
    SOFT_I2C5_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C6
    SOFT_I2C6_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C7
    SOFT_I2C7_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C8
    SOFT_I2C8_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C9
    SOFT_I2C9_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C10
    SOFT_I2C10_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C11
    SOFT_I2C11_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C12
    SOFT_I2C12_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C13
    SOFT_I2C13_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C14
    SOFT_I2C14_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C15
    SOFT_I2C15_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C16
    SOFT_I2C16_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C17
    SOFT_I2C17_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C18
    SOFT_I2C18_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C19
    SOFT_I2C19_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C20
    SOFT_I2C20_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C21
    SOFT_I2C21_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C22
    SOFT_I2C22_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C23
    SOFT_I2C23_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C24
    SOFT_I2C24_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C25
    SOFT_I2C25_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C26
    SOFT_I2C26_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C27
    SOFT_I2C27_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C28
    SOFT_I2C28_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C29
    SOFT_I2C29_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C30
    SOFT_I2C30_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C31
    SOFT_I2C31_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C32
    SOFT_I2C32_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C33
    SOFT_I2C33_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C34
    SOFT_I2C34_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C35
    SOFT_I2C35_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C36
    SOFT_I2C36_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C37
    SOFT_I2C37_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C38
    SOFT_I2C38_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C39
    SOFT_I2C39_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C40
    SOFT_I2C40_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C41
    SOFT_I2C41_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C42
    SOFT_I2C42_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C43
    SOFT_I2C43_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C44
    SOFT_I2C44_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C45
    SOFT_I2C45_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C46
    SOFT_I2C46_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C47
    SOFT_I2C47_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C48
    SOFT_I2C48_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C49
    SOFT_I2C49_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C50
    SOFT_I2C50_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C51
    SOFT_I2C51_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C52
    SOFT_I2C52_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C53
    SOFT_I2C53_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C54
    SOFT_I2C54_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C55
    SOFT_I2C55_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C56
    SOFT_I2C56_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C57
    SOFT_I2C57_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C58
    SOFT_I2C58_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C59
    SOFT_I2C59_INDEX,
#endif

#ifdef BSP_USING_SOFT_I2C60
    SOFT_I2C60_INDEX,
#endif
};

static struct stm32_soft_i2c_config soft_i2c_config[] =
{
#if defined(BSP_USING_SOFT_I2C1)
    SOFT_I2C1_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C2)
    SOFT_I2C2_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C3)
    SOFT_I2C3_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C4)
    SOFT_I2C4_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C5)
    SOFT_I2C5_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C6)
    SOFT_I2C6_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C7)
    SOFT_I2C7_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C8)
    SOFT_I2C8_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C9)
    SOFT_I2C9_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C10)
    SOFT_I2C10_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C11)
    SOFT_I2C11_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C12)
    SOFT_I2C12_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C13)
    SOFT_I2C13_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C14)
    SOFT_I2C14_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C15)
    SOFT_I2C15_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C16)
    SOFT_I2C16_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C17)
    SOFT_I2C17_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C18)
    SOFT_I2C18_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C19)
    SOFT_I2C19_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C20)
    SOFT_I2C20_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C21)
    SOFT_I2C21_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C22)
    SOFT_I2C22_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C23)
    SOFT_I2C23_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C24)
    SOFT_I2C24_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C25)
    SOFT_I2C25_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C26)
    SOFT_I2C26_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C27)
    SOFT_I2C27_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C28)
    SOFT_I2C28_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C29)
    SOFT_I2C29_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C30)
    SOFT_I2C30_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C31)
    SOFT_I2C31_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C32)
    SOFT_I2C32_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C33)
    SOFT_I2C33_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C34)
    SOFT_I2C34_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C35)
    SOFT_I2C35_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C36)
    SOFT_I2C36_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C37)
    SOFT_I2C37_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C38)
    SOFT_I2C38_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C39)
    SOFT_I2C39_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C40)
    SOFT_I2C40_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C41)
    SOFT_I2C41_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C42)
    SOFT_I2C42_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C43)
    SOFT_I2C43_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C44)
    SOFT_I2C44_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C45)
    SOFT_I2C45_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C46)
    SOFT_I2C46_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C47)
    SOFT_I2C47_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C48)
    SOFT_I2C48_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C49)
    SOFT_I2C49_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C50)
    SOFT_I2C50_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C51)
    SOFT_I2C51_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C52)
    SOFT_I2C52_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C53)
    SOFT_I2C53_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C54)
    SOFT_I2C54_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C55)
    SOFT_I2C55_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C56)
    SOFT_I2C56_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C57)
    SOFT_I2C57_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C58)
    SOFT_I2C58_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C59)
    SOFT_I2C59_BUS_CONFIG,
#endif

#if defined(BSP_USING_SOFT_I2C60)
    SOFT_I2C60_BUS_CONFIG,
#endif
};

static struct stm32_soft_i2c i2c_obj[sizeof(soft_i2c_config) / sizeof(soft_i2c_config[0])];

/**
 * This function initializes the i2c pin.
 *
 * @param Stm32 i2c dirver class.
 */
static void stm32_i2c_gpio_init(struct stm32_soft_i2c *i2c)
{
    struct stm32_soft_i2c_config* cfg = (struct stm32_soft_i2c_config*)i2c->ops.data;

    rt_pin_mode(cfg->scl, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(cfg->sda, PIN_MODE_OUTPUT_OD);

    rt_pin_write(cfg->scl, PIN_HIGH);
    rt_pin_write(cfg->sda, PIN_HIGH);
}

/**
 * This function sets the sda pin.
 *
 * @param Stm32 config class.
 * @param The sda pin state.
 */
static void stm32_set_sda(void *data, rt_int32_t state)
{
    struct stm32_soft_i2c_config* cfg = (struct stm32_soft_i2c_config*)data;
    if (state)
    {
        rt_pin_write(cfg->sda, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->sda, PIN_LOW);
    }
}

/**
 * This function sets the scl pin.
 *
 * @param Stm32 config class.
 * @param The scl pin state.
 */
static void stm32_set_scl(void *data, rt_int32_t state)
{
    struct stm32_soft_i2c_config* cfg = (struct stm32_soft_i2c_config*)data;
    if (state)
    {
        rt_pin_write(cfg->scl, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->scl, PIN_LOW);
    }
}

/**
 * This function gets the sda pin state.
 *
 * @param The sda pin state.
 */
static rt_int32_t stm32_get_sda(void *data)
{
    struct stm32_soft_i2c_config* cfg = (struct stm32_soft_i2c_config*)data;
    return rt_pin_read(cfg->sda);
}

/**
 * This function gets the scl pin state.
 *
 * @param The scl pin state.
 */
static rt_int32_t stm32_get_scl(void *data)
{
    struct stm32_soft_i2c_config* cfg = (struct stm32_soft_i2c_config*)data;
    return rt_pin_read(cfg->scl);
}
/**
 * The time delay function.
 *
 * @param microseconds.
 */
static void stm32_udelay(rt_uint32_t us)
{
    rt_uint32_t ticks;
    rt_uint32_t told, tnow, tcnt = 0;
    rt_uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

static const struct rt_i2c_bit_ops stm32_bit_ops_default =
{
    .data     = RT_NULL,
    .set_sda  = stm32_set_sda,
    .set_scl  = stm32_set_scl,
    .get_sda  = stm32_get_sda,
    .get_scl  = stm32_get_scl,
    .udelay   = stm32_udelay,
    .delay_us = 1,
    .timeout  = 100
};

/**
 * if i2c is locked, this function will unlock it
 *
 * @param stm32 config class
 *
 * @return RT_EOK indicates successful unlock.
 */
static rt_err_t stm32_i2c_bus_unlock(const struct stm32_soft_i2c_config *cfg)
{
    rt_int32_t i = 0;

    if (PIN_LOW == rt_pin_read(cfg->sda))
    {
        while (i++ < 9)
        {
            rt_pin_write(cfg->scl, PIN_HIGH);
            stm32_udelay(100);
            rt_pin_write(cfg->scl, PIN_LOW);
            stm32_udelay(100);
        }
    }
    if (PIN_LOW == rt_pin_read(cfg->sda))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static void soft_i2c_get_config(void)
{
#ifdef BSP_USING_SOFT_I2C1
    soft_i2c_config[SOFT_I2C1_INDEX].scl = rt_pin_get(BSP_SOFT_I2C1_SCL);
    soft_i2c_config[SOFT_I2C1_INDEX].sda = rt_pin_get(BSP_SOFT_I2C1_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C2
    soft_i2c_config[SOFT_I2C2_INDEX].scl = rt_pin_get(BSP_SOFT_I2C2_SCL);
    soft_i2c_config[SOFT_I2C2_INDEX].sda = rt_pin_get(BSP_SOFT_I2C2_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C3
    soft_i2c_config[SOFT_I2C3_INDEX].scl = rt_pin_get(BSP_SOFT_I2C3_SCL);
    soft_i2c_config[SOFT_I2C3_INDEX].sda = rt_pin_get(BSP_SOFT_I2C3_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C4
    soft_i2c_config[SOFT_I2C4_INDEX].scl = rt_pin_get(BSP_SOFT_I2C4_SCL);
    soft_i2c_config[SOFT_I2C4_INDEX].sda = rt_pin_get(BSP_SOFT_I2C4_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C5
    soft_i2c_config[SOFT_I2C5_INDEX].scl = rt_pin_get(BSP_SOFT_I2C5_SCL);
    soft_i2c_config[SOFT_I2C5_INDEX].sda = rt_pin_get(BSP_SOFT_I2C5_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C6
    soft_i2c_config[SOFT_I2C6_INDEX].scl = rt_pin_get(BSP_SOFT_I2C6_SCL);
    soft_i2c_config[SOFT_I2C6_INDEX].sda = rt_pin_get(BSP_SOFT_I2C6_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C7
    soft_i2c_config[SOFT_I2C7_INDEX].scl = rt_pin_get(BSP_SOFT_I2C7_SCL);
    soft_i2c_config[SOFT_I2C7_INDEX].sda = rt_pin_get(BSP_SOFT_I2C7_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C8
    soft_i2c_config[SOFT_I2C8_INDEX].scl = rt_pin_get(BSP_SOFT_I2C8_SCL);
    soft_i2c_config[SOFT_I2C8_INDEX].sda = rt_pin_get(BSP_SOFT_I2C8_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C9
    soft_i2c_config[SOFT_I2C9_INDEX].scl = rt_pin_get(BSP_SOFT_I2C9_SCL);
    soft_i2c_config[SOFT_I2C9_INDEX].sda = rt_pin_get(BSP_SOFT_I2C9_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C10
    soft_i2c_config[SOFT_I2C10_INDEX].scl = rt_pin_get(BSP_SOFT_I2C10_SCL);
    soft_i2c_config[SOFT_I2C10_INDEX].sda = rt_pin_get(BSP_SOFT_I2C10_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C11
    soft_i2c_config[SOFT_I2C11_INDEX].scl = rt_pin_get(BSP_SOFT_I2C11_SCL);
    soft_i2c_config[SOFT_I2C11_INDEX].sda = rt_pin_get(BSP_SOFT_I2C11_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C12
    soft_i2c_config[SOFT_I2C12_INDEX].scl = rt_pin_get(BSP_SOFT_I2C12_SCL);
    soft_i2c_config[SOFT_I2C12_INDEX].sda = rt_pin_get(BSP_SOFT_I2C12_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C13
    soft_i2c_config[SOFT_I2C13_INDEX].scl = rt_pin_get(BSP_SOFT_I2C13_SCL);
    soft_i2c_config[SOFT_I2C13_INDEX].sda = rt_pin_get(BSP_SOFT_I2C13_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C14
    soft_i2c_config[SOFT_I2C14_INDEX].scl = rt_pin_get(BSP_SOFT_I2C14_SCL);
    soft_i2c_config[SOFT_I2C14_INDEX].sda = rt_pin_get(BSP_SOFT_I2C14_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C15
    soft_i2c_config[SOFT_I2C15_INDEX].scl = rt_pin_get(BSP_SOFT_I2C15_SCL);
    soft_i2c_config[SOFT_I2C15_INDEX].sda = rt_pin_get(BSP_SOFT_I2C15_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C16
    soft_i2c_config[SOFT_I2C16_INDEX].scl = rt_pin_get(BSP_SOFT_I2C16_SCL);
    soft_i2c_config[SOFT_I2C16_INDEX].sda = rt_pin_get(BSP_SOFT_I2C16_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C17
    soft_i2c_config[SOFT_I2C17_INDEX].scl = rt_pin_get(BSP_SOFT_I2C17_SCL);
    soft_i2c_config[SOFT_I2C17_INDEX].sda = rt_pin_get(BSP_SOFT_I2C17_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C18
    soft_i2c_config[SOFT_I2C18_INDEX].scl = rt_pin_get(BSP_SOFT_I2C18_SCL);
    soft_i2c_config[SOFT_I2C18_INDEX].sda = rt_pin_get(BSP_SOFT_I2C18_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C19
    soft_i2c_config[SOFT_I2C19_INDEX].scl = rt_pin_get(BSP_SOFT_I2C19_SCL);
    soft_i2c_config[SOFT_I2C19_INDEX].sda = rt_pin_get(BSP_SOFT_I2C19_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C20
    soft_i2c_config[SOFT_I2C20_INDEX].scl = rt_pin_get(BSP_SOFT_I2C20_SCL);
    soft_i2c_config[SOFT_I2C20_INDEX].sda = rt_pin_get(BSP_SOFT_I2C20_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C21
    soft_i2c_config[SOFT_I2C21_INDEX].scl = rt_pin_get(BSP_SOFT_I2C21_SCL);
    soft_i2c_config[SOFT_I2C21_INDEX].sda = rt_pin_get(BSP_SOFT_I2C21_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C22
    soft_i2c_config[SOFT_I2C22_INDEX].scl = rt_pin_get(BSP_SOFT_I2C22_SCL);
    soft_i2c_config[SOFT_I2C22_INDEX].sda = rt_pin_get(BSP_SOFT_I2C22_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C23
    soft_i2c_config[SOFT_I2C23_INDEX].scl = rt_pin_get(BSP_SOFT_I2C23_SCL);
    soft_i2c_config[SOFT_I2C23_INDEX].sda = rt_pin_get(BSP_SOFT_I2C23_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C24
    soft_i2c_config[SOFT_I2C24_INDEX].scl = rt_pin_get(BSP_SOFT_I2C24_SCL);
    soft_i2c_config[SOFT_I2C24_INDEX].sda = rt_pin_get(BSP_SOFT_I2C24_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C25
    soft_i2c_config[SOFT_I2C25_INDEX].scl = rt_pin_get(BSP_SOFT_I2C25_SCL);
    soft_i2c_config[SOFT_I2C25_INDEX].sda = rt_pin_get(BSP_SOFT_I2C25_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C26
    soft_i2c_config[SOFT_I2C26_INDEX].scl = rt_pin_get(BSP_SOFT_I2C26_SCL);
    soft_i2c_config[SOFT_I2C26_INDEX].sda = rt_pin_get(BSP_SOFT_I2C26_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C27
    soft_i2c_config[SOFT_I2C27_INDEX].scl = rt_pin_get(BSP_SOFT_I2C27_SCL);
    soft_i2c_config[SOFT_I2C27_INDEX].sda = rt_pin_get(BSP_SOFT_I2C27_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C28
    soft_i2c_config[SOFT_I2C28_INDEX].scl = rt_pin_get(BSP_SOFT_I2C28_SCL);
    soft_i2c_config[SOFT_I2C28_INDEX].sda = rt_pin_get(BSP_SOFT_I2C28_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C29
    soft_i2c_config[SOFT_I2C29_INDEX].scl = rt_pin_get(BSP_SOFT_I2C29_SCL);
    soft_i2c_config[SOFT_I2C29_INDEX].sda = rt_pin_get(BSP_SOFT_I2C29_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C30
    soft_i2c_config[SOFT_I2C30_INDEX].scl = rt_pin_get(BSP_SOFT_I2C30_SCL);
    soft_i2c_config[SOFT_I2C30_INDEX].sda = rt_pin_get(BSP_SOFT_I2C30_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C31
    soft_i2c_config[SOFT_I2C31_INDEX].scl = rt_pin_get(BSP_SOFT_I2C31_SCL);
    soft_i2c_config[SOFT_I2C31_INDEX].sda = rt_pin_get(BSP_SOFT_I2C31_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C32
    soft_i2c_config[SOFT_I2C32_INDEX].scl = rt_pin_get(BSP_SOFT_I2C32_SCL);
    soft_i2c_config[SOFT_I2C32_INDEX].sda = rt_pin_get(BSP_SOFT_I2C32_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C33
    soft_i2c_config[SOFT_I2C33_INDEX].scl = rt_pin_get(BSP_SOFT_I2C33_SCL);
    soft_i2c_config[SOFT_I2C33_INDEX].sda = rt_pin_get(BSP_SOFT_I2C33_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C34
    soft_i2c_config[SOFT_I2C34_INDEX].scl = rt_pin_get(BSP_SOFT_I2C34_SCL);
    soft_i2c_config[SOFT_I2C34_INDEX].sda = rt_pin_get(BSP_SOFT_I2C34_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C35
    soft_i2c_config[SOFT_I2C35_INDEX].scl = rt_pin_get(BSP_SOFT_I2C35_SCL);
    soft_i2c_config[SOFT_I2C35_INDEX].sda = rt_pin_get(BSP_SOFT_I2C35_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C36
    soft_i2c_config[SOFT_I2C36_INDEX].scl = rt_pin_get(BSP_SOFT_I2C36_SCL);
    soft_i2c_config[SOFT_I2C36_INDEX].sda = rt_pin_get(BSP_SOFT_I2C36_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C37
    soft_i2c_config[SOFT_I2C37_INDEX].scl = rt_pin_get(BSP_SOFT_I2C37_SCL);
    soft_i2c_config[SOFT_I2C37_INDEX].sda = rt_pin_get(BSP_SOFT_I2C37_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C38
    soft_i2c_config[SOFT_I2C38_INDEX].scl = rt_pin_get(BSP_SOFT_I2C38_SCL);
    soft_i2c_config[SOFT_I2C38_INDEX].sda = rt_pin_get(BSP_SOFT_I2C38_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C39
    soft_i2c_config[SOFT_I2C39_INDEX].scl = rt_pin_get(BSP_SOFT_I2C39_SCL);
    soft_i2c_config[SOFT_I2C39_INDEX].sda = rt_pin_get(BSP_SOFT_I2C39_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C40
    soft_i2c_config[SOFT_I2C40_INDEX].scl = rt_pin_get(BSP_SOFT_I2C40_SCL);
    soft_i2c_config[SOFT_I2C40_INDEX].sda = rt_pin_get(BSP_SOFT_I2C40_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C41
    soft_i2c_config[SOFT_I2C41_INDEX].scl = rt_pin_get(BSP_SOFT_I2C41_SCL);
    soft_i2c_config[SOFT_I2C41_INDEX].sda = rt_pin_get(BSP_SOFT_I2C41_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C42
    soft_i2c_config[SOFT_I2C42_INDEX].scl = rt_pin_get(BSP_SOFT_I2C42_SCL);
    soft_i2c_config[SOFT_I2C42_INDEX].sda = rt_pin_get(BSP_SOFT_I2C42_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C43
    soft_i2c_config[SOFT_I2C43_INDEX].scl = rt_pin_get(BSP_SOFT_I2C43_SCL);
    soft_i2c_config[SOFT_I2C43_INDEX].sda = rt_pin_get(BSP_SOFT_I2C43_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C44
    soft_i2c_config[SOFT_I2C44_INDEX].scl = rt_pin_get(BSP_SOFT_I2C44_SCL);
    soft_i2c_config[SOFT_I2C44_INDEX].sda = rt_pin_get(BSP_SOFT_I2C44_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C45
    soft_i2c_config[SOFT_I2C45_INDEX].scl = rt_pin_get(BSP_SOFT_I2C45_SCL);
    soft_i2c_config[SOFT_I2C45_INDEX].sda = rt_pin_get(BSP_SOFT_I2C45_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C46
    soft_i2c_config[SOFT_I2C46_INDEX].scl = rt_pin_get(BSP_SOFT_I2C46_SCL);
    soft_i2c_config[SOFT_I2C46_INDEX].sda = rt_pin_get(BSP_SOFT_I2C46_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C47
    soft_i2c_config[SOFT_I2C47_INDEX].scl = rt_pin_get(BSP_SOFT_I2C47_SCL);
    soft_i2c_config[SOFT_I2C47_INDEX].sda = rt_pin_get(BSP_SOFT_I2C47_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C48
    soft_i2c_config[SOFT_I2C48_INDEX].scl = rt_pin_get(BSP_SOFT_I2C48_SCL);
    soft_i2c_config[SOFT_I2C48_INDEX].sda = rt_pin_get(BSP_SOFT_I2C48_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C49
    soft_i2c_config[SOFT_I2C49_INDEX].scl = rt_pin_get(BSP_SOFT_I2C49_SCL);
    soft_i2c_config[SOFT_I2C49_INDEX].sda = rt_pin_get(BSP_SOFT_I2C49_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C50
    soft_i2c_config[SOFT_I2C50_INDEX].scl = rt_pin_get(BSP_SOFT_I2C50_SCL);
    soft_i2c_config[SOFT_I2C50_INDEX].sda = rt_pin_get(BSP_SOFT_I2C50_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C51
    soft_i2c_config[SOFT_I2C51_INDEX].scl = rt_pin_get(BSP_SOFT_I2C51_SCL);
    soft_i2c_config[SOFT_I2C51_INDEX].sda = rt_pin_get(BSP_SOFT_I2C51_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C52
    soft_i2c_config[SOFT_I2C52_INDEX].scl = rt_pin_get(BSP_SOFT_I2C52_SCL);
    soft_i2c_config[SOFT_I2C52_INDEX].sda = rt_pin_get(BSP_SOFT_I2C52_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C53
    soft_i2c_config[SOFT_I2C53_INDEX].scl = rt_pin_get(BSP_SOFT_I2C53_SCL);
    soft_i2c_config[SOFT_I2C53_INDEX].sda = rt_pin_get(BSP_SOFT_I2C53_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C54
    soft_i2c_config[SOFT_I2C54_INDEX].scl = rt_pin_get(BSP_SOFT_I2C54_SCL);
    soft_i2c_config[SOFT_I2C54_INDEX].sda = rt_pin_get(BSP_SOFT_I2C54_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C55
    soft_i2c_config[SOFT_I2C55_INDEX].scl = rt_pin_get(BSP_SOFT_I2C55_SCL);
    soft_i2c_config[SOFT_I2C55_INDEX].sda = rt_pin_get(BSP_SOFT_I2C55_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C56
    soft_i2c_config[SOFT_I2C56_INDEX].scl = rt_pin_get(BSP_SOFT_I2C56_SCL);
    soft_i2c_config[SOFT_I2C56_INDEX].sda = rt_pin_get(BSP_SOFT_I2C56_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C57
    soft_i2c_config[SOFT_I2C57_INDEX].scl = rt_pin_get(BSP_SOFT_I2C57_SCL);
    soft_i2c_config[SOFT_I2C57_INDEX].sda = rt_pin_get(BSP_SOFT_I2C57_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C58
    soft_i2c_config[SOFT_I2C58_INDEX].scl = rt_pin_get(BSP_SOFT_I2C58_SCL);
    soft_i2c_config[SOFT_I2C58_INDEX].sda = rt_pin_get(BSP_SOFT_I2C58_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C59
    soft_i2c_config[SOFT_I2C59_INDEX].scl = rt_pin_get(BSP_SOFT_I2C59_SCL);
    soft_i2c_config[SOFT_I2C59_INDEX].sda = rt_pin_get(BSP_SOFT_I2C59_SDA);
#endif

#ifdef BSP_USING_SOFT_I2C60
    soft_i2c_config[SOFT_I2C60_INDEX].scl = rt_pin_get(BSP_SOFT_I2C60_SCL);
    soft_i2c_config[SOFT_I2C60_INDEX].sda = rt_pin_get(BSP_SOFT_I2C60_SDA);
#endif
}

/* I2C initialization function */
int rt_hw_soft_i2c_init(void)
{
    rt_err_t result;
    soft_i2c_get_config();

    for (rt_size_t i = 0; i < sizeof(i2c_obj) / sizeof(struct stm32_soft_i2c); i++)
    {
        i2c_obj[i].ops = stm32_bit_ops_default;
        i2c_obj[i].ops.data = (void*)&soft_i2c_config[i];
        i2c_obj[i].i2c2_bus.priv = &i2c_obj[i].ops;
        stm32_i2c_gpio_init(&i2c_obj[i]);
        result = rt_i2c_bit_add_bus(&i2c_obj[i].i2c2_bus, soft_i2c_config[i].bus_name);
        RT_ASSERT(result == RT_EOK);
        stm32_i2c_bus_unlock(&soft_i2c_config[i]);

        LOG_D("software simulation %s init done, pin scl: %d, pin sda %d",
        soft_i2c_config[i].bus_name,
        soft_i2c_config[i].scl,
        soft_i2c_config[i].sda);
    }

    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_hw_soft_i2c_init);

#endif
#endif /* RT_USING_I2C */
