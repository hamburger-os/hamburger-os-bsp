/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-11     lvhan       the first version
 */
#ifndef BOARD_PORTS_DRV_LED_H_
#define BOARD_PORTS_DRV_LED_H_

rt_base_t led_creat(rt_base_t pin, rt_base_t level);
void led_execution_phase(rt_base_t index);
void led_error_handler(rt_base_t index);

#endif /* BOARD_PORTS_DRV_LED_H_ */
