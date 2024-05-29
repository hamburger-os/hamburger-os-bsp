/***************************************************************************
文件名：vcp_app_layer.h
模    块：
详    述：
作    者：jiaqx 20231128
***************************************************************************/

#include "comm_proc_ctl.h"
#include "common.h"

#ifndef VCP_TIME_MANAGE_H
#define VCP_TIME_MANAGE_H

extern void SetTime_CAN1_diff( uint32 Rx_ctl_time);
extern sint32 GetTime_CAN1_diff( void );
extern void SetTime_CAN2_diff( uint32 Rx_ctl_time);
extern sint32 GetTime_CAN2_diff( void );
extern void SetTime_ETH_diff( uint32 Rx_ctl_time);
extern sint32 GetTime_ETH_diff( void );
extern uint32 count_msg_no(uint32 now_msg_no_u32);
extern uint16 count_msg_no16(uint16 now_msg_no_u16);

#endif
