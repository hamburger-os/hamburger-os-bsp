/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-25     zm       the first version
 */
#ifndef BOARD_PORTS_DRV_DS2484_H_
#define BOARD_PORTS_DRV_DS2484_H_

#include "board.h"

#ifdef MAX31826_USING_I2C_DS2484

typedef enum {
  DS2484_Control_Reset      = (rt_uint8_t)(0x01),
  DS2484_Control_Write_Byte = (rt_uint8_t)(0x02),
  DS2484_Control_Read_Byte  = (rt_uint8_t)(0x03)
} DS2484_Control_CMD;

int rt_hw_ds2484_init(void);

#endif

#endif /* BOARD_PORTS_DRV_DS2484_H_ */
