/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-14     whj4674672   first version
 */
#include <rtthread.h>
#include "stm32h7xx.h"

#ifdef BSP_ENABLE_MPU

#define DBG_TAG "drv.mpu"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static MPU_Region_InitTypeDef mpu_cfg[] =
{
#ifdef BSP_USING_ITCMSRAM
    {
        .BaseAddress = BSP_ITCMSRAM_ADDR,
        .Size = MPU_REGION_SIZE_64KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_DTCMSRAM
    {
        .BaseAddress = BSP_DTCMSRAM_ADDR,
        .Size = MPU_REGION_SIZE_128KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_AXISRAM
    {
        .BaseAddress = BSP_AXISRAM_ADDR,
        .Size = MPU_REGION_SIZE_512KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL1,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_AHBSRAM_D2
    {
        .BaseAddress = BSP_AHBSRAM_D2_ADDR,
        .Size = MPU_REGION_SIZE_256KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL1,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
    {
        .BaseAddress = BSP_AHBSRAM_D2_ADDR + 0x40000,
        .Size = MPU_REGION_SIZE_16KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL1,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
    {
        .BaseAddress = BSP_AHBSRAM_D2_ADDR + 0x44000,
        .Size = MPU_REGION_SIZE_16KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_AHBSRAM_D3
    {
        .BaseAddress = BSP_AHBSRAM_D3_ADDR,
        .Size = MPU_REGION_SIZE_64KB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL1,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_NORFLASH
    {
        .BaseAddress = BSP_NORFLASH_ADDR,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_FMCSRAM
    {
        .BaseAddress = BSP_FMCSRAM_ADDR,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif

#ifdef BSP_USE_ETH1
    {
        .BaseAddress = 0x60000000 + ETH1_NE/2*0x4000000,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif

#ifdef BSP_USE_ETH2
    {
        .BaseAddress = 0x60000000 + ETH2_NE/2*0x4000000,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif

#ifdef BSP_USE_ETH3
    {
        .BaseAddress = 0x60000000 + ETH3_NE/2*0x4000000,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE,
        .IsCacheable = MPU_ACCESS_NOT_CACHEABLE,
        .IsBufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif

#ifdef BSP_USING_SDRAM
    {
        .BaseAddress = SDRAM_BANK_ADDR,
        .Size = MPU_REGION_SIZE_64MB,
        .SubRegionDisable = 0x0,
        .TypeExtField = MPU_TEX_LEVEL0,
        .AccessPermission = MPU_REGION_FULL_ACCESS,
        .DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE,
        .IsCacheable = MPU_ACCESS_CACHEABLE,
        .IsBufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif
};

int rt_hw_mpu_init(void)
{
    /* Disables the MPU */
    HAL_MPU_Disable();

    for (uint8_t i=0; i < sizeof(mpu_cfg)/sizeof(MPU_Region_InitTypeDef); i++)
    {
        MPU_Region_InitTypeDef MPU_InitStruct = {0};
        /** Initializes and configures the Region and the memory to be protected
        */
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER0 + i;
        MPU_InitStruct.BaseAddress = mpu_cfg[i].BaseAddress;
        MPU_InitStruct.Size = mpu_cfg[i].Size;
        MPU_InitStruct.SubRegionDisable = mpu_cfg[i].SubRegionDisable;
        MPU_InitStruct.TypeExtField = mpu_cfg[i].TypeExtField;
        MPU_InitStruct.AccessPermission = mpu_cfg[i].AccessPermission;
        MPU_InitStruct.DisableExec = mpu_cfg[i].DisableExec;
        MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
        MPU_InitStruct.IsCacheable = mpu_cfg[i].IsCacheable;
        MPU_InitStruct.IsBufferable = mpu_cfg[i].IsBufferable;

        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }

    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    return RT_EOK;
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void _MemManage_Handler(char *s, int num)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    LOG_E("MemManage_Handler at file:%s num:%d", s, num);
    while (1)
    {
    }
}

#endif
