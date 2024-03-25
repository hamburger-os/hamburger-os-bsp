/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_BOARD_APP_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_BOARD_APP_H_

#include <rtthread.h>
#include "data_handle.h"

#define ENABLE_RECORD_BOARD_APP 0

void Processing_HMB_HLRT_Message( uint8_t low3bit, uint8_t data[] );
void Processing_CEU_Message( uint8_t low3bit, uint8_t data[] );
void Processing_IAP_Message( uint8_t low3bit, uint8_t high8bit, uint8_t data[] );
void Processing_EBV_Controlling( uint8_t low3bit, uint8_t data[] );
void Processing_ABV_WorkingCondition( uint8_t low3bit, uint8_t data[]);
void Processing_IBV_WorkingCondition( uint8_t low3bit, uint8_t data[] );
void Processing_LKJ_LLRT_Message( uint8_t low3bit, uint8_t data[] );
/* 06-July-2020, by DuYanPo. */
void Processing_Gradepacket_Message(CAN_FRAME *ps_can_frame);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_RECORD_BOARD_APP_H_ */
