/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-17     Administrator       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_BOARD_INFO_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_BOARD_INFO_H_

#include "app_layer.h"

#pragma pack(1)

typedef struct comm_chanl_info
{
    uint8_t chanl_index;
    uint8_t chanl_sta;
    uint32_t Tx_datalen;
    uint32_t Rx_datalen;
} s_chanl_info;
/*插件通道信系电压、电流*/

typedef struct comm_plug_info
{
    uint32_t ctl_cpu_soft_ver;
    uint32_t comm_cpu_soft_ver;
    uint32_t ctl_self_A;
    uint32_t ctl_self_B;
    uint8_t chl_self_all; /*0x55 正常 0xAA异常*/
    uint8_t plug_id[8];
    uint32_t open_Times;
    uint32_t open_long_Time;
    uint8_t tempr_num;
    uint16_t plug_tempr;
    uint8_t power_channls;
    uint16_t power_vot;
    uint16_t power_curret;
    uint8_t chl_num;
} s_plug_info;

#pragma pack()

rt_err_t BoardInfoInit(void);
void get_comm_plug_info(uint8_t *pSafe , r_app_layer *pApp_layer);
int BoardInfoThreadInit(void);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_BOARD_INFO_H_ */
