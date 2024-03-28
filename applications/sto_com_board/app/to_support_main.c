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

#include <string.h>

#define STO_COM_BOARD_TEST_ENABLE 1

#if STO_COM_BOARD_TEST_ENABLE

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


///* hdlc */
//static void test_task01(void)
//{
//    static S_TIMER_INFO timer = { FALSE, 0U };
//    S_HDLC_FRAME hdlc_frame;
//
//    if ( TRUE == support_timer_timeoutM(&timer, 1000U))
//    {
////        if(supprot_hdlc_getData(E_HDLC_ID_1, &hdlc_frame) == E_HDLC_OK)
////        {
////            if(hdlc_frame.len != 0)
////            {
////                MY_Printf("hdlc read len %d\r\n", hdlc_frame.len);
////                for(int i = 0; i < hdlc_frame.len; i++)
////                {
////                    MY_Printf("%x ", hdlc_frame.data_u8[i]);
////                }
////                MY_Printf("\r\n");
////            }
////        }
////        hdlc_frame.data_u8[0] = 0xFE;
////        hdlc_frame.data_u8[1] = 0xFE;
////        hdlc_frame.data_u8[2] = 0x68;
////        hdlc_frame.data_u8[3] = 0x3c;
////        memset(&hdlc_frame.data_u8[4], 0xAA, 60);
////        hdlc_frame.data_u8[63] = 0x16;
////        hdlc_frame.len = 64;
////
////        if(support_hdlc_sendData(E_HDLC_ID_1, &hdlc_frame) != E_HDLC_OK)
////        {
////            MY_Printf("hdlc send error\r\n");
////        }
//    }
//}


static void can_test(void)
{
    static E_BOARD_ID board_id = ID_NONE;
    S_CAN_FRAME can_frame;

    if(ID_NONE == board_id)
    {
        board_id = support_gpio_getBoardId();
    }

    if (supprot_can_getData(E_CAN_ID_1, &can_frame) == E_CAN_OK)
    {
        if (E_CAN_OK != support_can_sendData(E_CAN_ID_1, &can_frame))
        {
            MY_Printf("send error %d\r\n", E_CAN_ID_1);
        }
    }

    if (supprot_can_getData(E_CAN_ID_2, &can_frame) == E_CAN_OK)
    {
        if (E_CAN_OK != support_can_sendData(E_CAN_ID_2, &can_frame))
        {
            MY_Printf("send error %d\r\n", E_CAN_ID_2);
        }
    }

    if (supprot_can_getData(E_CAN_ID_3, &can_frame) == E_CAN_OK)
    {
        if (E_CAN_OK != support_can_sendData(E_CAN_ID_3, &can_frame))
        {
            MY_Printf("send error %d\r\n", E_CAN_ID_3);
        }
    }

    if (supprot_can_getData(E_CAN_ID_4, &can_frame) == E_CAN_OK)
    {
        if (E_CAN_OK != support_can_sendData(E_CAN_ID_4, &can_frame))
        {
            MY_Printf("send error %d\r\n", E_CAN_ID_4);
        }
    }

    if(ID_TX2_Child == board_id || ID_TX1_Child == board_id) /* 子板 */
    {
        if (supprot_can_getData(E_CAN_ID_5, &can_frame) == E_CAN_OK)
        {
            if (E_CAN_OK != support_can_sendData(E_CAN_ID_5, &can_frame))
            {
                MY_Printf("send error %d\r\n", E_CAN_ID_5);
            }
        }
    }
}

static void mvb_test(void)
{
    static S_MVB_FRAME rx_mvb = {0};
    static S_MVB_FRAME old_rx_mvb = {0};
    static S_TIMER_INFO timer = { FALSE, 0U };
    static uint32 recv_cnt = 0;

    if ( TRUE == support_timer_timeoutM(&timer, 100U))
    {
        if(supprot_mvb_getData(E_MVB_ID_1, &rx_mvb) == E_MVB_OK)
        {
            if(rx_mvb.len != 0)
            {
//                MY_Printf("mvb read len %d\r\n", rx_mvb.len);
//                for(int i = 0; i < rx_mvb.len; i++)
//                {
//                    MY_Printf("%x ", rx_mvb.data_u8[i]);
//                }
//                MY_Printf("\r\n");
                if(memcmp(old_rx_mvb.data_u8, rx_mvb.data_u8, rx_mvb.len)!=0)
                {
                    memcpy(&old_rx_mvb, &rx_mvb, sizeof(S_MVB_FRAME));
//                    MY_Printf("mvb read len %d\r\n", old_rx_mvb.len);
//                    for(int i = 0; i < old_rx_mvb.len; i++)
//                    {
//                        MY_Printf("%x ", old_rx_mvb.data_u8[i]);
//                    }
//                    MY_Printf("\r\n");
                    if(support_mvb_sendData(E_MVB_ID_1, &old_rx_mvb) != E_MVB_OK)
                    {
                        MY_Printf("mvb send error\r\n");
                    }
                    else
                    {
                        recv_cnt++;
                        MY_Printf("d %d\r\n", recv_cnt);
                    }
                }
            }
        }
    }
}

static void hldc_send_test(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME hdlc_frame;
    static uint8 send_cnt = 0;

    if ( TRUE == support_timer_timeoutM(&timer, 40U))
    {
        hdlc_frame.data_u8[0] = send_cnt;
        memset(&hdlc_frame.data_u8[1], 0xAA, 141);
        hdlc_frame.len = 142;

        if(support_hdlc_sendData(E_HDLC_ID_1, &hdlc_frame) != E_HDLC_OK)
        {
            MY_Printf("hdlc send error\r\n");
        }
        else
        {
            send_cnt++;
            if(send_cnt > 255)
            {
                send_cnt = 0;
            }
        }
    }
}


static void hldc_recv_test(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME rx_hdlc_frame;
    S_HDLC_FRAME expect_hdlc_frame;
    static uint8 expect_index = 0;
    static uint8 first_recv = 0;

//    if ( TRUE == support_timer_timeoutM(&timer, 40U))
    if ( TRUE == support_timer_timeoutM(&timer, 20U))
    {
        if(supprot_hdlc_getData(E_HDLC_ID_1, &rx_hdlc_frame) == E_HDLC_OK)
        {
            if(rx_hdlc_frame.len != 0)
            {
                memset(expect_hdlc_frame.data_u8, 0xAA, 143);
                if(memcmp(&expect_hdlc_frame.data_u8[1], &rx_hdlc_frame.data_u8[1], (int)141) == 0)
                {
                  if(expect_index == rx_hdlc_frame.data_u8[0])
                  {
                      MY_Printf("hdlc %d\r\n", rx_hdlc_frame.data_u8[0]);
                  }
                  else
                  {
                      MY_Printf("----hdlc recv index error %d %d\r\n", rx_hdlc_frame.data_u8[0], expect_index);
                      if(first_recv == 0)  /* 第一次收到数据时，期望值根据接收到的数据设置 */
                      {
                          first_recv = 1;
                          expect_index = rx_hdlc_frame.data_u8[0];
                      }
                  }
                  expect_index++;
                  if(expect_index > 255)
                  {
                    expect_index = 0;
                  }
                }
                else
                {
                    MY_Printf("----hdlc recv error %d\r\n", rx_hdlc_frame.data_u8[0]);
                }
            }
        }
    }
}

static void hldc_echo_test(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME rx_hdlc_frame;

//    if ( TRUE == support_timer_timeoutM(&timer, 40U))
//    {
        if(supprot_hdlc_getData(E_HDLC_ID_1, &rx_hdlc_frame) == E_HDLC_OK)
        {
            if(rx_hdlc_frame.len != 0)
            {
//                for(int i = 0; i < rx_hdlc_frame.len; i++)
//                {
//                    MY_Printf("%x", rx_hdlc_frame.data_u8[i]);
//                }
//                MY_Printf("\r\n");
                if(support_hdlc_sendData(E_HDLC_ID_1, &rx_hdlc_frame) != E_HDLC_OK)
                {
                    MY_Printf("hdlc send error\r\n");
                }
            }
        }
//    }
}

/* 使用调试器测试 */
#define HDLC_SEND_TEST_LEN (142)
static void hldc_send_test_by_tiaoshiqi(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME hdlc_frame;
    static uint32 send_cnt = 0, buf_index = 0,send_pack = 0;

    if ( TRUE == support_timer_timeoutM(&timer, 40U))
    {
        memset(&hdlc_frame.data_u8[0], 0xAA, HDLC_SEND_TEST_LEN);
        hdlc_frame.len = HDLC_SEND_TEST_LEN;

        hdlc_frame.data_u8[buf_index] = send_cnt;

        if(support_hdlc_sendData(E_HDLC_ID_1, &hdlc_frame) != E_HDLC_OK)
        {
            MY_Printf("hdlc send error\r\n");
        }
        else
        {
            send_pack++;
            send_cnt++;
            if(send_cnt > 0xff)
            {
                send_cnt = 0;
                buf_index++;
                if(buf_index > HDLC_SEND_TEST_LEN)
                {
                    buf_index = 0;
                }
            }
            MY_Printf("-%d-\r\n", send_pack);
        }
    }
}

#define HDLC_SEND_RX_TEST_LEN (140)//
static void hldc_recv_test_by_tiaoshiqi(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME rx_hdlc_frame;
    S_HDLC_FRAME expect_hdlc_frame;
    static uint32 expect_index = 0;

    if ( TRUE == support_timer_timeoutM(&timer, 1U))
    {
        if(supprot_hdlc_getData(E_HDLC_ID_1, &rx_hdlc_frame) == E_HDLC_OK)
        {
            if(rx_hdlc_frame.len != 0)
            {
                memset(expect_hdlc_frame.data_u8, 0xAA, HDLC_SEND_RX_TEST_LEN);
                if(rx_hdlc_frame.data_u8[0] == 0x55)
                {
                    if(memcmp(&expect_hdlc_frame.data_u8[1], &rx_hdlc_frame.data_u8[1], (int)(HDLC_SEND_RX_TEST_LEN - 1)) == 0)
                    {
                        expect_index++;
                        MY_Printf("-o-%d\r\n", expect_index);
                    }
                    else
                    {
                        MY_Printf("--err--%d\r\n", expect_index);
                    }

//                    MY_Printf("hdlc recv %d\r\n", rx_hdlc_frame.len);
//                    for(int i =0; i < rx_hdlc_frame.len; i++)
//                    {
//                        MY_Printf("%x ", rx_hdlc_frame.data_u8[i]);
//                    }
//                    MY_Printf("\r\n");
                }
                else
                {
                    MY_Printf("--ierr--%d\r\n", expect_index);
                }
            }
        }
    }
}

static void rs422_test(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_RS485_FRAME rx_485_frame;

    if ( TRUE == support_timer_timeoutM(&timer, 100U))
    {
        if(supprot_rs485_getData(E_RS485_ID_1, &rx_485_frame)!= E_RS485_OK)
        {
            MY_Printf("recv 485 error\r\n");
        }
        else
        {
            if(rx_485_frame.len != 0)
            {
                 if(support_rs485_sendData(E_RS485_ID_1, &rx_485_frame) != E_RS485_OK)
                 {
                     MY_Printf("send 485 error\r\n");
                 }
            }
        }
    }
}

static void rs485_test_echo(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_RS485_FRAME rx_485_frame;

//    if ( TRUE == support_timer_timeoutM(&timer, 100U))
    if ( TRUE == support_timer_timeoutM(&timer, 10U))
    {
        if(supprot_rs485_getData(E_RS485_ID_1, &rx_485_frame)!= E_RS485_OK)
        {
            MY_Printf("recv 485 error\r\n");
        }
        else
        {
           if(rx_485_frame.len != 0)
           {
                if(support_rs485_sendData(E_RS485_ID_1, &rx_485_frame) != E_RS485_OK)
                {
                    MY_Printf("send 485 error\r\n");
                }
           }
        }
    }
}

static uint32 ch1_tx_cnt = 0, ch2_rx_cnt = 0;
static uint32 ch3_tx_cnt = 0, ch4_rx_cnt = 0, ch5_rx_cnt = 0;
static void can_test_all(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    static E_BOARD_ID board_id = ID_NONE;
    S_CAN_FRAME can_frame;

    if ( TRUE == support_timer_timeoutM(&timer, 100U))
    {
        static E_BOARD_ID board_id = ID_NONE;
        S_CAN_FRAME can_frame;
        S_CAN_FRAME tx_can_frame;
        S_CAN_FRAME rx_can_frame;

        if(ID_NONE == board_id)
        {
            board_id = support_gpio_getBoardId();
        }

        tx_can_frame.priority_u8 = 0xf8;
        tx_can_frame.no_u8 = 0x01;
        tx_can_frame.can_mode = E_CAN_NORMAL_MODE;
        tx_can_frame.data_u8[0] = 0x55;
        tx_can_frame.data_u8[1] = 0x55;
        tx_can_frame.data_u8[2] = 0x55;
        tx_can_frame.data_u8[3] = 0x55;
        tx_can_frame.data_u8[4] = 0x55;
        tx_can_frame.data_u8[5] = 0x55;
        tx_can_frame.data_u8[6] = 0x55;
        tx_can_frame.data_u8[7] = 0x55;
        tx_can_frame.length_u8 = 8;

        if (E_CAN_OK != support_can_sendData(E_CAN_ID_1, &tx_can_frame))
        {
            MY_Printf("ch 1 send error\r\n");
        }
        else
        {
            ch1_tx_cnt++;
        }

        if (supprot_can_getData(E_CAN_ID_2, &rx_can_frame) == E_CAN_OK)
        {
            if(rx_can_frame.length_u8 != 0)
            {
                if(memcmp(&rx_can_frame.data_u8[0], &tx_can_frame.data_u8[0], (int)8) != 0)
                {
                    MY_Printf("ch2 rcv error\r\n");
                    for(int i = 0; i < rx_can_frame.length_u8; i++)
                    {
                        MY_Printf("%d ", rx_can_frame.data_u8[i]);
                    }
                    MY_Printf("\r\n");
                }
                else
                {
                    ch2_rx_cnt++;
                }
            }
        }

        if (E_CAN_OK != support_can_sendData(E_CAN_ID_3, &tx_can_frame))
        {
            MY_Printf("ch 3 send error\r\n");
        }
        else
        {
            ch3_tx_cnt++;
        }

        if (supprot_can_getData(E_CAN_ID_4, &rx_can_frame) == E_CAN_OK)
        {
            if(rx_can_frame.length_u8 != 0)
            {
                if(memcmp(&rx_can_frame.data_u8[0], &tx_can_frame.data_u8[0], (int)8) != 0)
                {
                    MY_Printf("ch4 rcv error\r\n");
                    for(int i = 0; i < rx_can_frame.length_u8; i++)
                    {
                        MY_Printf("%d ", rx_can_frame.data_u8[i]);
                    }
                    MY_Printf("\r\n");
                }
                else
                {
                    ch4_rx_cnt++;
//                    MY_Printf("ch3 tx %d ch4 rx %d\r\n", ch3_tx_cnt, ch4_rx_cnt);
                }
            }
        }

        if(ID_TX2_Child == board_id || ID_TX1_Child == board_id) /* 子板 */
        {
            if (supprot_can_getData(E_CAN_ID_5, &rx_can_frame) == E_CAN_OK)
            {

                if(rx_can_frame.length_u8 != 0)
                {
                    if(memcmp(&rx_can_frame.data_u8[0], &tx_can_frame.data_u8[0], (int)8) != 0)
                    {
                        MY_Printf("ch5 rcv error\r\n");
                        for(int i = 0; i < rx_can_frame.length_u8; i++)
                        {
                            MY_Printf("%d ", rx_can_frame.data_u8[i]);
                        }
                        MY_Printf("\r\n");
                    }
                    else
                    {
                        ch5_rx_cnt++;
    //                    MY_Printf("ch3 tx %d ch4 rx %d\r\n", ch3_tx_cnt, ch4_rx_cnt);
                    }
                }
            }
        }
    }
}

static void rs485_test_echo_all(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_RS485_FRAME rx_485_frame;

    if ( TRUE == support_timer_timeoutM(&timer, 50U))
    {
        if(supprot_rs485_getData(E_RS485_ID_1, &rx_485_frame)!= E_RS485_OK)
        {
            MY_Printf("recv 485 error\r\n");
        }
        else
        {
           if(rx_485_frame.len != 0)
           {
                if(support_rs485_sendData(E_RS485_ID_1, &rx_485_frame) != E_RS485_OK)
                {
                    MY_Printf("send 485 error\r\n");
                }
           }
        }
    }
}

static void hldc_echo_test_all(void)
{
    static S_TIMER_INFO timer = { FALSE, 0U };
    S_HDLC_FRAME rx_hdlc_frame;

    if(supprot_hdlc_getData(E_HDLC_ID_1, &rx_hdlc_frame) == E_HDLC_OK)
    {
        if(rx_hdlc_frame.len != 0)
        {
            if(support_hdlc_sendData(E_HDLC_ID_1, &rx_hdlc_frame) != E_HDLC_OK)
            {
                MY_Printf("hdlc send error\r\n");
            }
        }
    }
}

static uint32 recv_cnt = 0;
static void mvb_test_all(void)
{
    static S_MVB_FRAME rx_mvb = {0};
    static S_MVB_FRAME old_rx_mvb = {0};
    static S_TIMER_INFO timer = { FALSE, 0U };

    if ( TRUE == support_timer_timeoutM(&timer, 100U))
    {
        if(supprot_mvb_getData(E_MVB_ID_1, &rx_mvb) == E_MVB_OK)
        {
            if(rx_mvb.len != 0)
            {
//                MY_Printf("mvb read len %d\r\n", rx_mvb.len);
//                for(int i = 0; i < rx_mvb.len; i++)
//                {
//                    MY_Printf("%x ", rx_mvb.data_u8[i]);
//                }
//                MY_Printf("\r\n");
                if(memcmp(old_rx_mvb.data_u8, rx_mvb.data_u8, rx_mvb.len)!=0)
                {
                    memcpy(&old_rx_mvb, &rx_mvb, sizeof(S_MVB_FRAME));
//                    MY_Printf("mvb read len %d\r\n", old_rx_mvb.len);
//                    for(int i = 0; i < old_rx_mvb.len; i++)
//                    {
//                        MY_Printf("%x ", old_rx_mvb.data_u8[i]);
//                    }
//                    MY_Printf("\r\n");
                    if(support_mvb_sendData(E_MVB_ID_1, &old_rx_mvb) != E_MVB_OK)
                    {
                        MY_Printf("mvb send error\r\n");
                    }
                    else
                    {
                        recv_cnt++;
//                        MY_Printf("d %d\r\n", recv_cnt);
                    }
                }
            }
        }
    }
}

static void tx1_load_all_test(void)
{
    can_test_all();
    rs485_test_echo_all();
}

static void tx2_load_all_test(void)
{
//    can_test_all();
    hldc_echo_test_all();
//    mvb_test_all();
}


static void test_task01(void)
{
//    can_test();
    mvb_test();
//    hldc_send_test();
//    hldc_recv_test();
//    rs422_test();
//    rs485_test_echo();
//    hldc_echo_test();
//    hldc_send_test_by_tiaoshiqi();
//    hldc_recv_test_by_tiaoshiqi();
//    tx1_load_all_test();
//    tx2_load_all_test();
}

#endif

/*******************************************************************************************
 ** @brief: test_task02
 ** @param: null
 *******************************************************************************************/
static void test_task02(void)
{
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

static void show_info(int argc, char **argv)
{
    MY_Printf("ch1 tx %d ch2 rx %d\r\n", ch1_tx_cnt, ch2_rx_cnt);
    MY_Printf("ch3 tx %d ch4 rx %d ch5 rx %d\r\n", ch3_tx_cnt, ch4_rx_cnt, ch5_rx_cnt);
    MY_Printf("mvb %d\r\n", recv_cnt);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(show_info, show_info, show info);
#endif /* RT_USING_FINSH */

#endif /* STO_COM_BOARD_TEST_ENABLE */


