/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-24     MingZhao       the first version
 */
#include "led_ctrl.h"

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "led"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static LedCtrl led_ctrl;

void LedCtrlON(LedIndex index)
{
  rt_pin_write(led_ctrl.pin_index[index], PIN_LOW);
}

void LedCtrlOFF(LedIndex index)
{
  rt_pin_write(led_ctrl.pin_index[index], PIN_HIGH);
}

void LedInit(void)
{
  led_ctrl.pin_index[ERR_LED] = rt_pin_get(LED_CTRL_ERR_NAME);
  rt_pin_mode(led_ctrl.pin_index[ERR_LED], PIN_MODE_OUTPUT);
  rt_pin_write(led_ctrl.pin_index[ERR_LED], PIN_LOW);

  led_ctrl.pin_index[SELF_LED] = rt_pin_get(LED_CTRL_SELF_NAME);
  rt_pin_mode(led_ctrl.pin_index[SELF_LED], PIN_MODE_OUTPUT);
  rt_pin_write(led_ctrl.pin_index[SELF_LED], PIN_LOW);

  led_ctrl.pin_index[DUMP_LED] = rt_pin_get(LED_CTRL_DUMP_NAME);
  rt_pin_mode(led_ctrl.pin_index[DUMP_LED], PIN_MODE_OUTPUT);
  rt_pin_write(led_ctrl.pin_index[DUMP_LED], PIN_LOW);

  led_ctrl.pin_index[ETH_LED] = rt_pin_get(LED_CTRL_ETH_NAME);
  rt_pin_mode(led_ctrl.pin_index[ETH_LED], PIN_MODE_OUTPUT);
  rt_pin_write(led_ctrl.pin_index[ETH_LED], PIN_LOW);

  led_ctrl.pin_index[CAN_LED] = rt_pin_get(LED_CTRL_CAN_NAME);
  rt_pin_mode(led_ctrl.pin_index[CAN_LED], PIN_MODE_OUTPUT);
  rt_pin_write(led_ctrl.pin_index[CAN_LED], PIN_LOW);
}

INIT_DEVICE_EXPORT(LedInit);

