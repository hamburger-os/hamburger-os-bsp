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
static uint8 eth_txbuf[1500];
static uint8 eth_rxbuf[1500];
static void test_task01(void)
{
    static uint32 cnt = 0U;
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_CAN_FRAME can_frame;


    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
    {
        if (supprot_can_getData(E_CAN_ID_2, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_2, &can_frame))
            {
                MY_Printf("send error\r\n");
            }
        }

//        support_gpio_set(LED1_ID, IO_HIGH);
//        support_gpio_toggle(LED1_ID);
    }
}

/*******************************************************************************************
 ** @brief: test_task02
 ** @param: null
 *******************************************************************************************/
static void test_task02(void)
{
    static uint32 cnt = 0U;
    static S_TIMER_INFO timer = { FALSE, 0U };
    int i = 0;
    static uint8 data_init = 0;
    S_ETH_FRAME rx_msg;

    if ( TRUE == support_timer_timeoutM(&timer, 10U))
    {


        if(0 == data_init)
        {
            for(i=0; i<1500; i++)
                eth_txbuf[i] = i + 8;
            data_init = 1;
        }
#if 0
        eth_txbuf[0] = 0xF8;
        eth_txbuf[1] = 0x09;
        eth_txbuf[2] = 0xA4;
        eth_txbuf[3] = 0x27;
        eth_txbuf[4] = 0x00;
        eth_txbuf[5] = 0x44;

        eth_txbuf[6] = 0xF8;
        eth_txbuf[7] = 0x09;
        eth_txbuf[8] = 0xA4;
        eth_txbuf[9] = 0x51;
        eth_txbuf[10] = 0x0e;
        eth_txbuf[11] = 0x00;
        if(support_eth_sendData(E_ETH_ID_1, eth_txbuf, 64) != E_ETH_OK)
        {
            MY_Printf("E_ETH_ID_1 send error\r\n");
        }

        eth_txbuf[0] = 0xF8;
        eth_txbuf[1] = 0x09;
        eth_txbuf[2] = 0xA4;
        eth_txbuf[3] = 0x51;
        eth_txbuf[4] = 0x0e;
        eth_txbuf[5] = 0x00;

        eth_txbuf[6] = 0xF8;
        eth_txbuf[7] = 0x09;
        eth_txbuf[8] = 0xA4;
        eth_txbuf[9] = 0x27;
        eth_txbuf[10] = 0x00;
        eth_txbuf[11] = 0x44;

        if(support_eth_sendData(E_ETH_ID_2, eth_txbuf, 64) != E_ETH_OK)
        {
            MY_Printf("E_ETH_ID_2 send error\r\n");
        }
#endif

//        if(supprot_eth_getData(E_ETH_ID_1, &rx_msg) != E_ETH_OK)
//        {
//            MY_Printf("E_ETH_ID_1 recv error\r\n");
//        }
//        else
//        {
//            MY_Printf("eth 1 recv len %d\r\n", rx_msg.len);
////            for(int i = 0; i <rx_msg.len; i++)
////            {
////                MY_Printf("%x ", rx_msg.data_u8[i]);
////            }
////            MY_Printf("\r\n");
//        }
//        if(supprot_eth_getData(E_ETH_ID_2, &rx_msg) != E_ETH_OK)
//        {
//            MY_Printf("E_ETH_ID_2 recv error\r\n");
//        }
//        else
//        {
//            MY_Printf("eth 2 recv len %d\r\n", rx_msg.len);
////            for(int i = 0; i <rx_msg.len; i++)
////            {
////                MY_Printf("%x ", rx_msg.data_u8[i]);
////            }
////            MY_Printf("\r\n");
//        }
    }
}

/*******************************************************************************************
 ** @brief: test_task03
 ** @param: null
 *******************************************************************************************/
static void test_task03(void)
{
    static uint32 cnt = 0U;
    static S_TIMER_INFO timer = { FALSE, 0U };

    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
    {
//		MY_Printf("task03-->running:%d\r\n", cnt++ );
    }
}

/*******************************************************************************************
 ** @brief: test_task04
 ** @param: null
 *******************************************************************************************/
static void test_task04(void)
{
    static uint32 cnt = 0U;
    static S_TIMER_INFO timer = { FALSE, 0U };

    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
    {
//		MY_Printf("task04-->running:%d\r\n", cnt++ );
    }
}

/*******************************************************************************************
 ** @brief: test_main_init
 ** @param: null
 *******************************************************************************************/
static void task_init(void)
{
    support_osRegisterFunc(TASK01_ID, test_task01);
    support_osRegisterFunc(TASK02_ID, test_task02);
    support_osRegisterFunc(TASK03_ID, test_task03);
    support_osRegisterFunc(TASK04_ID, test_task04);
}

/*******************************************************************************************
 ** @brief: test_main_init
 ** @param: null
 *******************************************************************************************/
extern void test_main_init(void)
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


static uint8 send_data = 0;

static void support_eth_test(int argc, char **argv)
{
    S_ETH_FRAME rx_msg;

#if 1  //底层处理mac

#if 0
    if(support_eth_sendData(E_ETH_ID_1, eth_txbuf, 1024) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_1 send error\r\n");
    }

    if(supprot_eth_getData(E_ETH_ID_2, &rx_msg) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_2 recv error\r\n");
    }
    else
    {
        MY_Printf("recv len %d\r\n", rx_msg.len);
        for(int i = 0; i <rx_msg.len; i++)
        {
            MY_Printf("%x ", rx_msg.data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#else
    if(support_eth_sendData(E_ETH_ID_2, eth_txbuf, 1024) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_2 send error\r\n");
    }

    if(supprot_eth_getData(E_ETH_ID_1, &rx_msg) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_1 recv error\r\n");
    }
    else
    {
        MY_Printf("recv len %d\r\n", rx_msg.len);
        for(int i = 0; i <rx_msg.len; i++)
        {
            MY_Printf("%x ", rx_msg.data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#endif

#else

#if 1
    //目标地址
    eth_txbuf[0] = 0xF8;
    eth_txbuf[1] = 0x09;
    eth_txbuf[2] = 0xA4;
    eth_txbuf[3] = 0x51;
    eth_txbuf[4] = 0x0e;
//    eth_txbuf[5] = 0x00;
    eth_txbuf[5] = 0x01;

    //源地址
    eth_txbuf[6] = 0xF8;
    eth_txbuf[7] = 0x09;
    eth_txbuf[8] = 0xA4;
    eth_txbuf[9] = 0x27;
    eth_txbuf[10] = 0x00;
//    eth_txbuf[11] = 0x44;
    eth_txbuf[11] = 0x45;

    if(support_eth_sendData(E_ETH_ID_1, eth_txbuf, 1024) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_1 send error\r\n");
    }

    if(supprot_eth_getData(E_ETH_ID_2, &rx_msg) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_2 recv error\r\n");
    }
    else
    {
        MY_Printf("recv len %d\r\n", rx_msg.len);
        for(int i = 0; i <rx_msg.len; i++)
        {
            MY_Printf("%x ", rx_msg.data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#else
    //目标地址
    eth_txbuf[0] = 0xF8;
    eth_txbuf[1] = 0x09;
    eth_txbuf[2] = 0xA4;
    eth_txbuf[3] = 0x27;
    eth_txbuf[4] = 0x00;
    eth_txbuf[5] = 0x44;
    eth_txbuf[5] = 0x45;

    //源地址
    eth_txbuf[6] = 0xF8;
    eth_txbuf[7] = 0x09;
    eth_txbuf[8] = 0xA4;
    eth_txbuf[9] = 0x51;
    eth_txbuf[10] = 0x0e;
    eth_txbuf[11] = 0x00;
    eth_txbuf[11] = 0x01;

    if(support_eth_sendData(E_ETH_ID_2, eth_txbuf, 1024) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_2 send error\r\n");
    }

    if(supprot_eth_getData(E_ETH_ID_1, &rx_msg) != E_ETH_OK)
    {
        MY_Printf("E_ETH_ID_1 recv error\r\n");
    }
    else
    {
        MY_Printf("recv len %d\r\n", rx_msg.len);
        for(int i = 0; i <rx_msg.len; i++)
        {
            MY_Printf("%x ", rx_msg.data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#endif
#endif
}
#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(support_eth_test, sethtest, support eth test);
#endif /* RT_USING_FINSH */
