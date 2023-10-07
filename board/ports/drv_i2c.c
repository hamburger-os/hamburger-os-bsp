/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-27     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_I2C

#if defined(BSP_USING_I2C1) || defined(BSP_USING_I2C2) || defined(BSP_USING_I2C3)

#define DBG_TAG "drv.i2c"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* stm32 config class */
struct stm32_i2c_config
{
    I2C_HandleTypeDef hi2c;
    I2C_TypeDef *Instance;
    const char *bus_name;
};
/* stm32 i2c dirver class */
struct stm32_i2c
{
    struct rt_i2c_bus_device i2c2_bus;
};

#if defined(BSP_USING_I2C1)
#define I2C1_BUS_CONFIG                                  \
    {                                                    \
        .bus_name = "i2c1",                              \
        .Instance = I2C1,                                \
    }
#endif

#if defined(BSP_USING_I2C2)
#define I2C2_BUS_CONFIG                                  \
    {                                                    \
        .bus_name = "i2c2",                              \
        .Instance = I2C2,                                \
    }
#endif

#if defined(BSP_USING_I2C3)
#define I2C3_BUS_CONFIG                                  \
    {                                                    \
        .bus_name = "i2c3",                              \
        .Instance = I2C3,                                \
    }
#endif

static struct stm32_i2c_config i2c_config[] =
{
#if defined(BSP_USING_I2C1)
    I2C1_BUS_CONFIG,
#endif
#if defined(BSP_USING_I2C2)
    I2C2_BUS_CONFIG,
#endif
#if defined(BSP_USING_I2C3)
    I2C3_BUS_CONFIG,
#endif
};

static struct stm32_i2c i2c_obj[sizeof(i2c_config) / sizeof(i2c_config[0])];

static rt_size_t stm32_master_xfer(struct rt_i2c_bus_device *bus,
                                 struct rt_i2c_msg msgs[],
                                 rt_uint32_t num)
{
    if (num == 0) return 0;
    rt_int32_t i, ret = 0;

    struct stm32_i2c_config *config = bus->priv;
    struct rt_i2c_msg *msg;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        msg->addr = msg->addr << 1;

        if (msg->flags & RT_I2C_RD)
        {
            ret = HAL_I2C_Master_Receive(&config->hi2c, msg->addr, msg->buf, msg->len, msg->len);
            if (ret != HAL_OK)
            {
                if (msg->len > 64)
                {
                    LOG_HEX("rd", 32, msg->buf, 64);
                }
                else
                {
                    LOG_HEX("rd", 32, msg->buf, msg->len);
                }
                LOG_E("master_xfer %s read : %d 0x%x %d %d/%d %d", config->bus_name, msg->flags, msg->addr, msg->len, i, num, ret);
                ret = -RT_EIO;
                goto out;
            }
            else
            {
                LOG_D("master_xfer %s read : %d 0x%x %d %d/%d", config->bus_name, msg->flags, msg->addr, msg->len, i, num);
                ret = msg->len;
            }
        }
        else
        {
            ret = HAL_I2C_Master_Transmit(&config->hi2c, msg->addr, msg->buf, msg->len, msg->len);
            if (ret != HAL_OK)
            {
                if (msg->len > 64)
                {
                    LOG_HEX("wr", 32, msg->buf, 64);
                }
                else
                {
                    LOG_HEX("wr", 32, msg->buf, msg->len);
                }
                LOG_E("master_xfer %s write : %d 0x%x %d %d/%d %d", config->bus_name, msg->flags, msg->addr, msg->len, i, num, ret);
                ret = -RT_EIO;
                goto out;
            }
            else
            {
                LOG_D("master_xfer %s write : %d 0x%x %d %d/%d", config->bus_name, msg->flags, msg->addr, msg->len, i, num);
                ret = msg->len;
            }
        }
    }
    ret = i;

out:
    return ret;
}

static struct rt_i2c_bus_device_ops stm32_ops_default =
{
    stm32_master_xfer,
    NULL,
    NULL
};

static void stm32_i2c_hal_init(struct stm32_i2c_config *config)
{
#if defined (STM32F100xB) || defined (STM32F100xE) || defined (STM32F101x6) || \
    defined (STM32F101xB) || defined (STM32F101xE) || defined (STM32F101xG) || defined (STM32F102x6) || defined (STM32F102xB) || defined (STM32F103x6) || \
    defined (STM32F103xB) || defined (STM32F103xE) || defined (STM32F103xG) || defined (STM32F105xC) || defined (STM32F107xC)

    config->hi2c.Instance = config->Instance;
    config->hi2c.Init.ClockSpeed = 100000;
    config->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    config->hi2c.Init.OwnAddress1 = 0;
    config->hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    config->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    config->hi2c.Init.OwnAddress2 = 0;
    config->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    config->hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&config->hi2c) != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    config->hi2c.Instance = config->Instance;
    config->hi2c.Init.ClockSpeed = 400000;
    config->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    config->hi2c.Init.OwnAddress1 = 0;
    config->hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    config->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    config->hi2c.Init.OwnAddress2 = 0;
    config->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    config->hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&config->hi2c) != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx) || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx)  || defined (STM32H747xx) || defined (STM32H747xG) || defined (STM32H757xx) || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx)  || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ)  || defined (STM32H725xx) || defined (STM32H723xx)

    config->hi2c.Instance = config->Instance;
    config->hi2c.Init.Timing = 0x00B045E1;
    config->hi2c.Init.OwnAddress1 = 0;
    config->hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    config->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    config->hi2c.Init.OwnAddress2 = 0;
    config->hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    config->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    config->hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&config->hi2c) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Analogue filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&config->hi2c, I2C_ANALOGFILTER_DISABLE) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Digital filter
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&config->hi2c, 0) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

/* I2C initialization function */
static int rt_hw_i2c_init(void)
{
    rt_err_t result = RT_EOK;

    for (rt_size_t i = 0; i < sizeof(i2c_obj) / sizeof(struct stm32_i2c); i++)
    {
        i2c_obj[i].i2c2_bus.priv = (void*)&i2c_config[i];
        i2c_obj[i].i2c2_bus.ops = &stm32_ops_default;
        stm32_i2c_hal_init(&i2c_config[i]);

        result = rt_i2c_bus_device_register(&i2c_obj[i].i2c2_bus, i2c_config[i].bus_name);
        if (result == RT_EOK)
        {
            LOG_D("hard hal %s init done", i2c_config[i].bus_name);
        }
        else
        {
            LOG_E("hard hal %s init error %d", i2c_config[i].bus_name, result);
        }
    }

    return result;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);

#endif
#endif
