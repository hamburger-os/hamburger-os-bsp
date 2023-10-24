/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : to_support_main.c
**@author: Created By Chengt
**@date  : 2023.09.07
**@brief : test support function
********************************************************************************************/
#include "support_init.h"
#include "support_libOS.h"
#include "support_libPub.h"

#include "support_can.h"
#include "support_eth.h"
#include "support_hdlc.h"
#include "support_mvb.h"
#include "support_rs485.h"
#include "support_timer.h"
#include "support_gpio.h"

/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/

/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/
/*******************************************************************************************
 ** @brief: test_task01
 ** @param: null
 *******************************************************************************************/
static void test_task01( void )
{
	static uint32 cnt = 0U;
	static S_TIMER_INFO timer = { FALSE, 0U };
	S_CAN_FRAME can_frame;

	if( TRUE == support_timer_timeoutM( &timer, 1000U ))
	{
//		MY_Printf("task01-->running:%d\r\n", cnt++ );
	    if(supprot_can_getData(E_CAN_ID_2, &can_frame) == E_CAN_OK)
	    {
	        if(E_CAN_OK != support_can_sendData(E_CAN_ID_2, &can_frame))
	        {
	            MY_Printf("send error\r\n");
	        }
	    }

	}
}

/*******************************************************************************************
 ** @brief: test_task02
 ** @param: null
 *******************************************************************************************/
static void test_task02( void )
{
	static uint32 cnt = 0U;
	static S_TIMER_INFO timer = { FALSE, 0U };

	if( TRUE == support_timer_timeoutM( &timer, 1000U ))
	{
		MY_Printf("task02-->running:%d\r\n", cnt++ );
	}
}

/*******************************************************************************************
 ** @brief: test_task03
 ** @param: null
 *******************************************************************************************/
static void test_task03( void )
{
	static uint32 cnt = 0U;
	static S_TIMER_INFO timer = { FALSE, 0U };

	if( TRUE == support_timer_timeoutM( &timer, 1000U ))
	{
		MY_Printf("task03-->running:%d\r\n", cnt++ );
	}
}

/*******************************************************************************************
 ** @brief: test_task04
 ** @param: null
 *******************************************************************************************/
static void test_task04( void )
{
	static uint32 cnt = 0U;
	static S_TIMER_INFO timer = { FALSE, 0U };

	if( TRUE == support_timer_timeoutM( &timer, 1000U ))
	{
		MY_Printf("task04-->running:%d\r\n", cnt++ );
	}
}

/*******************************************************************************************
 ** @brief: test_main_init
 ** @param: null
 *******************************************************************************************/
static void task_init( void )
{
	support_osRegisterFunc( TASK01_ID, test_task01 );
	support_osRegisterFunc( TASK02_ID, test_task02 );
	support_osRegisterFunc( TASK03_ID, test_task03 );
	support_osRegisterFunc( TASK04_ID, test_task04 );
}

/*******************************************************************************************
 ** @brief: test_main_init
 ** @param: null
 *******************************************************************************************/
extern void test_main_init( void )
{
	/* 1.֧�Ų��ʼ�� */
	support_init();

	/* 2.�����ʼ�� */
	task_init();
}

/*******************************************************************************************
 ** @brief: main
 ** @param: argc, argv
 *******************************************************************************************/
//extern void app_main()
//{
//	support_osRunning( test_main_init );
//
//	return 0;
//}
/**************************************end file*********************************************/
