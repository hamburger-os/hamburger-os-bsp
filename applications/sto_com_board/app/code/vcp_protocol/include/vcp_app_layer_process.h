/***************************************************************************
文件名：vcp_app_layer_process.h
模    块：主控插件与通信插件通信协议——应用层数据处理——头文件
详    述：无
作    者：jiaqx,20231129
***************************************************************************/

#include "common.h"

#ifndef VCP_APP_LAYER_PROCESS_H
#define VCP_APP_LAYER_PROCESS_H


typedef enum
{
    E_CLOCKSYNC_NONE = 0U,
    E_CLOCKSYNC_OK
}E_CLOCKSYNC_STATE;

typedef enum
{
    E_ROUNDPLSE_NONE = 0U,
    E_ROUNDPLSE_OK
}E_ROUNDPLSE_STATE;

extern void time_set_local_process( uint8 *pAppData ,uint16 applayer_datalen );
extern void comm_plug_info_process( void );

extern void tast_ClockSync( void );

extern E_CLOCKSYNC_STATE get_ClockSyncState( void );
extern void set_ClockSyncState( E_CLOCKSYNC_STATE e_clocksync_state);

extern void set_RoundPluseState( E_ROUNDPLSE_STATE e_roundplse_state);
extern E_ROUNDPLSE_STATE get_RoundPluseState( void );

#endif
