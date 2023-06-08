/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __DRV_I2C__
#define __DRV_I2C__

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>

/* stm32 config class */
struct stm32_soft_i2c_config
{
    rt_uint8_t scl;
    rt_uint8_t sda;
    const char *bus_name;
};
/* stm32 i2c dirver class */
struct stm32_soft_i2c
{
    struct rt_i2c_bit_ops ops;
    struct rt_i2c_bus_device i2c2_bus;
};

#if defined(BSP_USING_SOFT_I2C1)
#define SOFT_I2C1_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c1",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C2)
#define SOFT_I2C2_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c2",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C3)
#define SOFT_I2C3_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c3",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C4)
#define SOFT_I2C4_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c4",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C5)
#define SOFT_I2C5_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c5",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C6)
#define SOFT_I2C6_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c6",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C7)
#define SOFT_I2C7_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c7",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C8)
#define SOFT_I2C8_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c8",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C9)
#define SOFT_I2C9_BUS_CONFIG        \
    {                               \
        .bus_name = "si2c9",        \
    }
#endif

#if defined(BSP_USING_SOFT_I2C10)
#define SOFT_I2C10_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c10",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C11)
#define SOFT_I2C11_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c11",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C12)
#define SOFT_I2C12_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c12",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C13)
#define SOFT_I2C13_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c13",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C14)
#define SOFT_I2C14_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c14",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C15)
#define SOFT_I2C15_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c15",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C16)
#define SOFT_I2C16_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c16",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C17)
#define SOFT_I2C17_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c17",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C18)
#define SOFT_I2C18_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c18",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C19)
#define SOFT_I2C19_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c19",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C20)
#define SOFT_I2C20_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c20",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C21)
#define SOFT_I2C21_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c21",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C22)
#define SOFT_I2C22_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c22",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C23)
#define SOFT_I2C23_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c23",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C24)
#define SOFT_I2C24_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c24",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C25)
#define SOFT_I2C25_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c25",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C26)
#define SOFT_I2C26_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c26",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C27)
#define SOFT_I2C27_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c27",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C28)
#define SOFT_I2C28_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c28",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C29)
#define SOFT_I2C29_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c29",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C30)
#define SOFT_I2C30_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c30",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C31)
#define SOFT_I2C31_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c31",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C32)
#define SOFT_I2C32_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c32",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C33)
#define SOFT_I2C33_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c33",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C34)
#define SOFT_I2C34_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c34",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C35)
#define SOFT_I2C35_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c35",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C36)
#define SOFT_I2C36_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c36",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C37)
#define SOFT_I2C37_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c37",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C38)
#define SOFT_I2C38_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c38",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C39)
#define SOFT_I2C39_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c39",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C40)
#define SOFT_I2C40_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c40",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C41)
#define SOFT_I2C41_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c41",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C42)
#define SOFT_I2C42_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c42",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C43)
#define SOFT_I2C43_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c43",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C44)
#define SOFT_I2C44_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c44",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C45)
#define SOFT_I2C45_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c45",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C46)
#define SOFT_I2C46_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c46",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C47)
#define SOFT_I2C47_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c47",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C48)
#define SOFT_I2C48_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c48",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C49)
#define SOFT_I2C49_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c49",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C50)
#define SOFT_I2C50_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c50",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C51)
#define SOFT_I2C51_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c51",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C52)
#define SOFT_I2C52_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c52",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C53)
#define SOFT_I2C53_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c53",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C54)
#define SOFT_I2C54_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c54",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C55)
#define SOFT_I2C55_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c55",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C56)
#define SOFT_I2C56_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c56",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C57)
#define SOFT_I2C57_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c57",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C58)
#define SOFT_I2C58_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c58",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C59)
#define SOFT_I2C59_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c59",       \
    }
#endif

#if defined(BSP_USING_SOFT_I2C60)
#define SOFT_I2C60_BUS_CONFIG       \
    {                               \
        .bus_name = "si2c60",       \
    }
#endif

#endif
