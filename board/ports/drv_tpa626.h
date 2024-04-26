/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-25     Administrator       the first version
 */
#ifndef BOARD_PORTS_DRV_TPA626_H_
#define BOARD_PORTS_TPA626_H_

enum
{
    TPA626_CURRENT_CHANL = 0,
    TPA626_BUS_VOL_CHANL,
};


#define CFG_REG_ADDR      (0x00)          /* 配置寄存器地址 */
#define CFG_REG_DFLT_VAL  (0x4127U)
#define SHUNT_REG_ADDR    (0x01)          /* shunt寄存器地址 */
#define SHUNT_REG_V_UNIT  (25)            /* SHUNT reg val unit, 0.1uV */
#define SHUNT_REG_V_TO_MV(h, l) ((((uint32_t)(h) << 8U) + (l)) * SHUNT_REG_V_UNIT / 10000U)  /* SHUNT寄存器值转为mV */
#define BUS_VOLT_REG_ADDR (0x02)          /* 电压寄存器地址 */
#define BUSV_REG_V_UNIT  (125)            /* bus voltage reg val unit, 0.01mV */
#define BUSV_REG_V_TO_MV(h, l) ((((uint32_t)(h) << 8U) + (l)) * BUSV_REG_V_UNIT / 100U)  /* 电压寄存器值转为mV */
#define CALI_REG_ADDR     (0x05)          /* 校准寄存器地址 */
#define CALI_REG_DFLT_VAL (0x104U)        /* 校验缺省值 */
#define CURRENT_REG_ADDR  (0x04)          /* 电流寄存器地址 */
#define CURRENT_REG_V_UNIT  (1)           /* current reg val unit, 1mA */
#define CURRENT_REG_V_TO_MA(h, l) ((((uint32_t)(h) << 8U) + (l)) * CURRENT_REG_V_UNIT)  /* 电流寄存器值转为mA */
#define POWER_REG_ADDR    (0x03)          /* 功率寄存器地址 */
#define POWER_REG_V_UNIT  (25)           /* power reg val unit, 1mW */
#define POWER_REG_V_TO_MW(h, l) ((((uint32_t)(h) << 8U) + (l)) * POWER_REG_V_UNIT)  /* 功率寄存器值转为mW */


#endif /* BOARD_PORTS_DRV_TPA626_H_ */
