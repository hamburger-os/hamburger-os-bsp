/***************************************************************************
文件名：vcp_app_layer.h
模    块：
详    述：
作    者：jiaqx 20231128
***************************************************************************/

#include "common.h"

#ifndef VCP_TIME_MANAGE_H
#define VCP_TIME_MANAGE_H

extern void SetTime_ETH0_diff( uint32_t Rx_ctl_time);
extern void SetTime_ETH1_diff( uint32_t Rx_ctl_time);

extern int32_t GetTime_ETH0_diff( void );
extern int32_t GetTime_ETH1_diff( void );

extern uint32_t count_msg_no(uint32_t now_msg_no_u32);
extern uint16_t count_msg_no16(uint16_t now_msg_no_u16);

#endif
