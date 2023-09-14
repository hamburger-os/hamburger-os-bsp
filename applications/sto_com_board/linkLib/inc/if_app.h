/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : if_app.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Implement the function interfaces of if_app
********************************************************************************************/
#ifndef _IF_APP_H
#define _IF_APP_H

#include "common.h"

/* app初始化 */
typedef void ( *p_init )( void );

/* app_arch必要接口 */
typedef struct
{
	/* 应用初始化接口 */
	void ( *app_init )( void );

	/* 应用实时轮询处理 */
	void ( *app_polling_proc)( void );

	/* 应用空闲 */
	void ( *app_idle_proc)( void );

}S_APP_ARCH_IF;

/* 应用架构接口 */
extern S_APP_ARCH_IF g_appArchIf;

/* 架构运行接口 */
extern BOOL app_archRunning( p_init appArchIf_init );
extern void app_archInit( void );

#endif

/*******************************************end file*******************************************************/
