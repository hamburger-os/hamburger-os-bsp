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
#include "support_rs485.h"
#include "support_mvb.h"
#include "support_hdlc.h"

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
#if 0
static void test_task01(void)
{
    static uint32 cnt = 0U;
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_CAN_FRAME can_frame;


    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
    {
        if (supprot_can_getData(E_CAN_ID_1, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_1, &can_frame))
            {
                MY_Printf("send error\r\n");
            }
        }

        if (supprot_can_getData(E_CAN_ID_2, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_2, &can_frame))
            {
                MY_Printf("send error\r\n");
            }
        }

        if (supprot_can_getData(E_CAN_ID_3, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_3, &can_frame))
            {
                MY_Printf("send error\r\n");
            }
            else
            {
                MY_Printf("ch3 data\r\n");
                for(int i = 0; i < can_frame.length_u8; i++)
                {
                    MY_Printf("%d ", can_frame.data_u8[i]);
                }
                MY_Printf("\r\n");
            }
        }

        if (supprot_can_getData(E_CAN_ID_4, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_4, &can_frame))
            {
                MY_Printf("send error\r\n");
            }
            else
            {
                MY_Printf("ch4 data\r\n");
                for(int i = 0; i < can_frame.length_u8; i++)
                {
                    MY_Printf("%d ", can_frame.data_u8[i]);
                }
                MY_Printf("\r\n");
                MY_Printf("%x, %x %d\r\n", can_frame.no_u8, can_frame.priority_u8, can_frame.length_u8);
            }
        }

//        if (supprot_can_getData(E_CAN_ID_5, &can_frame) == E_CAN_OK)
//        {
//            if (E_CAN_OK != support_can_sendData(E_CAN_ID_5, &can_frame))
//            {
//                MY_Printf("send error\r\n");
//            }
//        }

//        support_gpio_set(LED1_ID, IO_HIGH);
//        support_gpio_toggle(LED1_ID);
    }
}

#else
//static void test_task01(void)
//{
//    static uint32 cnt = 0U;
//    static S_TIMER_INFO timer = { FALSE, 0U };
//    S_CAN_FRAME tx_can_frame;
//    S_CAN_FRAME rx_can_frame;
//
//
//    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
//    {
////        tx_can_frame.priority_u8 = 0xf8;
////        tx_can_frame.no_u8 = 0x01;
//////        tx_can_frame.can_mode = E_CAN_NORMAL_MODE;
////        tx_can_frame.can_mode = E_CAN_FD_MODE;
////        for(int i = 0; i < 8; i++)
////        {
////            tx_can_frame.data_u8[i] = i;
////        }
////        tx_can_frame.length_u8 = 8;
////
////        if (E_CAN_OK != support_can_sendData(E_CAN_ID_3, &tx_can_frame))
////        {
////            MY_Printf("ch 3 send error\r\n");
////        }
////
////        if (supprot_can_getData(E_CAN_ID_4, &rx_can_frame) == E_CAN_OK)
////        {
////            rx_can_frame.can_mode = E_CAN_NORMAL_MODE;
////            rx_can_frame.data_u8[7] = 0xff;
//////            rx_can_frame.can_mode = E_CAN_FD_MODE;
////            if (E_CAN_OK != support_can_sendData(E_CAN_ID_4, &rx_can_frame))
////            {
////                MY_Printf("ch 4 send error\r\n");
////            }
////            else
////            {
////                MY_Printf("ch4 data\r\n");
////                for(int i = 0; i < rx_can_frame.length_u8; i++)
////                {
////                    MY_Printf("%d ", rx_can_frame.data_u8[i]);
////                }
////                MY_Printf("\r\n");
////            }
////        }
////
////        if (supprot_can_getData(E_CAN_ID_3, &rx_can_frame) == E_CAN_OK)
////        {
////            MY_Printf("ch3 data, len %d\r\n", rx_can_frame.length_u8);
////            for(int i = 0; i < rx_can_frame.length_u8; i++)
////            {
////                MY_Printf("%d ", rx_can_frame.data_u8[i]);
////            }
////            MY_Printf("\r\n");
////        }
//
//    }
//}

/* 485 */
//static void test_task01(void)
//{
//    static S_TIMER_INFO timer = { FALSE, 0U };
//    S_RS485_FRAME tx_485_frame;
//    S_RS485_FRAME rx_485_frame;
//
//    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
//    {
//        tx_485_frame.len = 256;
////        memset(tx_485_frame.data_u8, 0x55, 64);
//        for(int i = 0; i < 256; i++)
//        {
//            tx_485_frame.data_u8[i] = i;
//        }
//        if(support_rs485_sendData(E_RS485_ID_1, &tx_485_frame) != E_RS485_OK)
//        {
//            MY_Printf("send 485 error\r\n");
//        }
//
////    if ( TRUE == support_timer_timeoutM(&timer, 500U))
////    {
//        if(supprot_rs485_getData(E_RS485_ID_1, &rx_485_frame)!= E_RS485_OK)
//        {
//            MY_Printf("recv 485 error\r\n");
//        }
//        else
//        {
//           if(rx_485_frame.len != 0)
//           {
//               MY_Printf("recv len %d\r\n", rx_485_frame.len);
//              for(int i = 0; i < rx_485_frame.len; i++)
//              {
//                  MY_Printf("%x ", rx_485_frame.data_u8[i]);
//              }
//              MY_Printf("\r\n");
//           }
//        }
//
//
//    }
//}

//
///* mvb */
//static S_MVB_FRAME rx_mvb = {0};
//static S_MVB_FRAME old_rx_mvb = {0};
//static void test_task01(void)
//{
//    static S_TIMER_INFO timer = { FALSE, 0U };
//
//    if ( TRUE == support_timer_timeoutM(&timer, 100U))
//    {
//        if(supprot_mvb_getData(E_MVB_ID_1, &rx_mvb) == E_MVB_OK)
//        {
//            if(rx_mvb.len != 0)
//            {
////                MY_Printf("mvb read len %d\r\n", rx_mvb.len);
////                for(int i = 0; i < rx_mvb.len; i++)
////                {
////                    MY_Printf("%x ", rx_mvb.data_u8[i]);
////                }
////                MY_Printf("\r\n");
//                if(memcmp(old_rx_mvb.data_u8, rx_mvb.data_u8, rx_mvb.len)!=0)
//                {
//                    memcpy(&old_rx_mvb, &rx_mvb, sizeof(S_MVB_FRAME));
////                    MY_Printf("mvb read len %d\r\n", old_rx_mvb.len);
////                    for(int i = 0; i < old_rx_mvb.len; i++)
////                    {
////                        MY_Printf("%x ", old_rx_mvb.data_u8[i]);
////                    }
////                    MY_Printf("\r\n");
//                    if(support_mvb_sendData(E_MVB_ID_1, &old_rx_mvb) != E_MVB_OK)
//                    {
//                        MY_Printf("mvb send error\r\n");
//                    }
//                }
//            }
//        }
//    }
//}


/* hdlc */
static void test_task01(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    static S_TIMER_INFO rx_timer = { FALSE, 0U };
    S_HDLC_FRAME hdlc_frame;
    S_HDLC_FRAME rx_hdlc_frame;
    static uint8 send_data_str[256] = {"TCMS-qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789+qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789"};


    if ( TRUE == support_timer_timeoutM(&timer, 2000U))
    {
        hdlc_frame.data_u8[0] = 0xFE;
        hdlc_frame.data_u8[1] = 0xFE;
        hdlc_frame.data_u8[2] = 0x68;
        hdlc_frame.data_u8[3] = 0x3c;
        memset(&hdlc_frame.data_u8[4], 0xAA, 60);
        hdlc_frame.data_u8[63] = 0x16;
        hdlc_frame.len = 64;

//        MY_Printf("send len %d\r\n", sizeof(send_data_str));
//        memcpy(hdlc_frame.data_u8, send_data_str, sizeof(send_data_str));
//        hdlc_frame.len = sizeof(send_data_str);

        if(support_hdlc_sendData(E_HDLC_ID_1, &hdlc_frame) != E_HDLC_OK)
        {
            MY_Printf("hdlc send error\r\n");
        }
    }

    if ( TRUE == support_timer_timeoutM(&rx_timer, 10U))
    {
        if(supprot_hdlc_getData(E_HDLC_ID_1, &rx_hdlc_frame) == E_HDLC_OK)
        {
            if(rx_hdlc_frame.len != 0)
            {
                MY_Printf("hdlc read len %d\r\n", rx_hdlc_frame.len);
                for(int i = 0; i < rx_hdlc_frame.len; i++)
                {
                    MY_Printf("%x ", rx_hdlc_frame.data_u8[i]);
                }
                MY_Printf("\r\n");
            }


//            if(support_hdlc_sendData(E_HDLC_ID_1, &rx_hdlc_frame) != E_HDLC_OK)
//            {
//                MY_Printf("hdlc send error\r\n");
//            }

        }
    }
}

#endif

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

#if 1
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
