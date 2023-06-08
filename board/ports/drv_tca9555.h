/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-09     lvhan       the first version
 */
#ifndef BOARD_PORTS_DRV_TCA9555_H_
#define BOARD_PORTS_DRV_TCA9555_H_

enum tca9555_address
{
    tca9555_address_1 = 0x20,
    tca9555_address_2,
    tca9555_address_3,
    tca9555_address_4
};

void rt_tca9555_pin_mode(rt_base_t pin, rt_base_t mode);
void rt_tca9555_fast_mode(uint8_t i2c, uint8_t address, uint16_t mode);

void rt_tca9555_pin_write(rt_base_t pin, rt_base_t value);
void rt_tca9555_fast_write(uint8_t i2c, uint8_t address, uint16_t data);

int rt_tca9555_pin_read(rt_base_t pin);
void rt_tca9555_fast_read(uint8_t i2c, uint8_t address, uint16_t *data);

rt_base_t rt_tca9555_pin_get(const char *name);

#endif /* BOARD_PORTS_DRV_TCA9555_H_ */
