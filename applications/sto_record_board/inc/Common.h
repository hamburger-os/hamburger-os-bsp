/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : Common.h
**@author: Created By Chengt
**@date  : 2015.09.23
**@brief : Manage CAN Device
**@Change Logs:
**Date           Author       Notes
**2023-07-21      zm           add swos2
********************************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include "board.h"
#include "data_handle.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Firmware date. */

#define JUDGE_COUNTER 3
#define OUT_TIME    2000
#define SELFCHECK_TIME 500

#define FIRMWARE_VERSION_A               (0x01U)
#define FIRMWARE_VERSION_B               (0x00U)
#define FIRMWARE_VERSION_C               (0x00U)
#define FIRMWARE_VERSION_D               (0x0AU)

#define FIRMWARE_YEAR                    ( 24U )
#define FIRMWARE_MONTH                   ( 01U )
#define FIRMWARE_DAY                     ( 10U )

/*
 * JCK15 将该宏定义设置为15
 * JCK20 将该宏定义设置为20
 */
#define JL_JCK_VERSION                   (20)

extern uint8_t Version[4];
extern uint16_t Verdate;
extern uint16_t ERROR_FLAG;

extern uint32_t SK1ZJ_time;
extern uint32_t SK2ZJ_time;
extern uint32_t JCJKZJ_time;
extern uint32_t ZK1ZJ_time;
extern uint32_t ZK2ZJ_time;
extern uint32_t XSQ1ZJ_time;
extern uint32_t XSQ2ZJ_time;
extern uint32_t JCTXZJ_time;


/* The following type definition was created by Liang Zhen, 15-November-2017. ================== */
/* public type definition ---------------------------------------------------------------------- */
/* CPU type. */
typedef enum {
  CPU_UNKNOWN = 0u,
  CPU_A,
  CPU_B
} CPU_TYPE;

/* Board type. */
typedef enum 
{
  BOARD_UNKNOWN = 0u, 
  BOARD_TRAIN_INTERFACE, 
  BOARD_RECORD, 
  BOARD_COMMUNICATION
} BOARD_TYPE;

/* export function relevant CRC16 */
extern uint8_t Common_BeTimeOutMN(uint32_t *time,uint32_t ms);

extern uint8_t u16_abscmp(uint16_t a,uint16_t b,uint16_t val);

/* The following function(s) was created by Liang Zhen, 15-November-2017. ====================== */
CPU_TYPE Get_CPU_Type( void );
/* 06-July-2020, by DuYanPo. */
uint32_t CRC32CCITT(uint8_t *pData, uint32_t len, uint32_t u32_nReg);
uint16_t count_msg_no16(uint16_t now_msg_no_u16);
#endif

/**************************************end file*********************************************/

