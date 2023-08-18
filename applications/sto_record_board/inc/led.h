/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-24     MingZhao       the first version
 */
#ifndef APPLICATIONS_RECORD_BOARD_LED_H_
#define APPLICATIONS_RECORD_BOARD_LED_H_

#include <rtthread.h>

#define LED_CTRL_ERR_NAME "PI.6"
#define LED_CTRL_SELF_NAME "PE.3"
#define LED_CTRL_DUMP_NAME "PI.9"
#define LED_CTRL_ETH_NAME "PB.6"
#define LED_CTRL_CAN_NAME "PB.7"

#define LED_CTRL_NUM (5U)

typedef enum {
  ERR_LED = 0,
  USB_LED,
  DUMP_LED,
  ETH_LED,
  CAN_LED
} LedIndex;

typedef struct {
  rt_base_t pin_index[LED_CTRL_NUM];
} LedCtrl;

void LedCtrlON(LedIndex index);
void LedCtrlOFF(LedIndex index);
void LedCtrlInit(void);


#endif /* APPLICATIONS_RECORD_BOARD_LED_H_ */
