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


#define JUDGE_COUNTER 3
#define OUT_TIME	2000
#define SELFCHECK_TIME 500


extern uint8_t Version[4];
extern uint16_t Verdate;
extern uint8_t ID;

extern uint32_t CAN0_time,CAN1_time,MCPCAN0_time,MCPCAN1_time;
extern CAN_FRAME SelfCheck_txMailbox;
extern uint16_t ERROR_FLAG;

extern uint32_t I_CAN0_time, I_CAN1_time, E_CAN0_time, E_CAN1_time;
extern uint32_t ZK1ZJ_time,  ZK2ZJ_time,  JCTXZJ_time, JCJKZJ_time, JLZJ_time;
extern uint32_t XSQ1ZJ_time, XSQ2ZJ_time, SK1ZJ_time,  SK2ZJ_time,  WJJKZJ_time;
extern uint32_t SKZJ_time,   XSQZJ_time,  SYSHEART_time;

//TODO(mingzhao)
//extern sArrayList *i_can0_recv_list;
//extern sArrayList  *i_can1_recv_list;
//
//extern sArrayList *p_can0_recv_list;
//extern sArrayList  *p_can1_recv_list;

/* UART BUFFER */
typedef struct
{
	uint8_t buf[100];
	uint8_t read_pos;
	uint8_t write_pos;
	uint8_t size;
}UART_BUF;

//TODO(mingzhao)
///* Operation_interface */
//typedef struct
//{
//	BoardType Board_Name;
//
//	/* Relevant Board Interface */
//	void (*Board_DataInit)(void);
//	void (*Board_InterruptInit)(void);
//	void (*Board_Device_Init)(void);
//	void (*Board_MainApp)(void);
//
//	/* Relevant Interrupt Handle Interface */
//	/* TIMER */
//	void (*TC0_IrqHandle)(void);
//	void (*TC1_IrqHandle)(void);
//	/* CAN */
//	void (*CAN0_IrqHandle)(void);
//	void (*CAN1_IrqHandle)(void);
//	void  (*MCP_CAN0_IrqHandle)(void);
//	void  (*MCP_CAN1_IrqHandle)(void);
//	void (*SPI0_IrqHandle)(void);
//	void (*SPI1_IrqHandle)(void);
//	void (*USART1_IrqHandle)(void);
//	void (*USART2_IrqHandle)(void);
//	void	(*UART0_IrqHandle)(void);
//	void	(*UART1_IrqHandle)(void);
//	/* RS485 */
//	void (*RS485a_IrqHandle)(void);
//	void (*RS485b_IrqHandle)(void);
//	void (*RS485c_IrqHandle)(void);
//	/* ETH */
//	void (*ETH_IrqHandle)(void);
//	/* USB */
//	void (*USB_IrqHandle)(void);
//	/* DMA */
//	void (*DMA_IrqHandle)(void);
//
//	/* task process flag */
//	void (*TASK_Handler_Periodic)(void);
//}Board_Interface;


/* The following macro definition was created by Liang Zhen, 22-November-2017. ================= */
/* public macro definition --------------------------------------------------------------------- */
/* The priority of train interface board. */
#define PRIORITY_TRAIN_SELFCHECK         ( 0x90u )
#define PRIORITY_TRAIN_STATE_INFO        ( 0x91u )

/* Firmware date. */
/* ------------------------------------------------------------------------------------------
   |      Author      |           Date          |                   Note                    |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 30-November-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 22-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 23-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 28-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 29-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 30-December-2017        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 2-January-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 4-January-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 5-January-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 7-January-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-January-2018         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 22-January-2018         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 31-January-2018         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-March-2018           | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 23-March-2018           | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-April-2018           | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 25-April-2018           | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-April-2018           | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 2-May-2018              | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 3-May-2018              | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 10-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 11-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 30-May-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 11-June-2018            | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-June-2018            | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 13-June-2018            | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 14-June-2018            | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 2-July-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 6-July-2018             | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 16-July-2018            | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 16-August-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-August-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-August-2018          | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 5-September-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 10-September-2018       | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 18-September-2018       | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 19-September-2018       | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 26-September-2018       | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 28-September-2018       | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 9-November-2018         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-November-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 28-November-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 29-November-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-December-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-December-2018        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 14-January-2019         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-January-2019         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 22-January-2019         | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 18-February-2019        | Used for application.                     |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 19-April-2019           | Used for application.                     |
   --------------------------------------------------------------------------------------- */
#define FIRMWARE_YEAR                    ( 19U )

#define FIRMWARE_MONTH                   ( 4U )

#define FIRMWARE_DAY                     ( 19U )


/* The following type definition was created by Liang Zhen, 15-November-2017. ================== */
/* public type definition ---------------------------------------------------------------------- */
/* CPU type. */
typedef enum { CPU_UNKNOWN = 0u, CPU_A, CPU_B } CPU_TYPE;

/* Board type. */
typedef enum 
{
  BOARD_UNKNOWN = 0u, 
  BOARD_TRAIN_INTERFACE, 
  BOARD_RECORD, 
  BOARD_COMMUNICATION
} BOARD_TYPE;


/* export variable */
//extern Board_Interface Board_Operation;  //TODO(mingzhao)

/* export functions relevant Common module */
extern void Common_CPU_Init(void);
extern void Common_BoardType_Detect(void);
extern void Common_Interface_Init(void);
extern void Common_Board_Init(void);
extern void Common_Board_Error(void);

/* export functions relevant Board module */
extern void TrainBoard_Operation_Init(void);
extern void RecordBoard_Operation_Init(void);
extern void CommunicationBoard_Operation_Init(void);

/* export function relevant CRC16 */
extern uint16_t Common_CRC16(uint8_t* pData, uint32_t nLength);
extern uint8_t Common_BeTimeOutMN(uint32_t *time,uint32_t ms);
extern uint8_t CAN_OutTime(uint32_t time,uint32_t ms );

//TODO(mingzhao)
/* export function for operating BufferList */
//extern sArrayList *Common_arraylist_init(uint8_t *pname, uint32_t item_count, uint32_t item_size);
//extern bool Common_arraylist_add(sArrayList *pal, uint8_t *pdata, uint32_t len);
//extern uint32_t Common_arraylist_get(sArrayList *pal, uint8_t *pdata);

extern void UartBuf_add(UART_BUF *rec_buf,uint8_t data);
extern uint8_t UartBuf_get(UART_BUF *rec_buf,uint8_t *data);

extern uint8_t u16_abscmp(uint16_t a,uint16_t b,uint16_t val);


/* The following function(s) was created by Liang Zhen, 15-November-2017. ====================== */
CPU_TYPE Get_CPU_Type( void );
BOARD_TYPE GetBoardType( void );
/* 06-July-2020, by DuYanPo. */
uint32_t CRC32CCITT(uint8_t *pData, uint32_t len, uint32_t u32_nReg);
#endif

/**************************************end file*********************************************/

