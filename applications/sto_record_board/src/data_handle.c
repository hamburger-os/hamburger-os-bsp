/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-20     zm       the first version
 */
#include "data_handle.h"

#define DBG_TAG "data handle"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>

#include "can_common_def.h"
#include "Record_FileCreate.h"
#include "Record_Board_App.h"
#include "RecordErrorCode.h"
#include "sto_record_board.h"

S_CAN_PACKE_Grade s_packet_gradeInfo;           /* 曲线的打包数据缓存区 */
static uint8_t read_time_flag = 0;

rt_err_t DataHandleInit(S_DATA_HANDLE *p_data_handle)
{
    static bool is_init = 0;

    if(RT_NULL == p_data_handle)
    {
        return -RT_EINVAL;
    }
    memset(p_data_handle, 0, sizeof(S_DATA_HANDLE));

    p_data_handle->can_data_mq = rt_mq_create("can data queue", sizeof(CAN_FRAME), CAN_DATA_MQ_MAX_NUM, RT_IPC_FLAG_FIFO);
    if(RT_NULL == p_data_handle->can_data_mq)
    {
        LOG_E("rt_mq_create failed");
        return -RT_ERROR;
    }

    if(0 == is_init)
    {
        is_init = 1;
        if(can_buf_init(&s_packet_gradeInfo) != RT_EOK)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}

/*
以太网协议转CAN协议
 */
//static uint32_t send_ok_num;
void ETHToCanDataHandle(S_DATA_HANDLE *p_data_handle, uint8_t *pbuf, uint16_t data_len)
{
    rt_err_t ret = -RT_ERROR;

    if(RT_NULL == p_data_handle)
    {
        return;
    }

    S_ETH_CAN_FRAME *s_can_frame = NULL;
    S_APP_INETH_PACK *p_can_PACK = (S_APP_INETH_PACK*) pbuf;
    uint16_t send_len = 0;
//    static uint8_t eth_start_flag = 0;

//    LKJ2K_CAN1_RxFcnt = (uint32_t) (p_can_PACK->frameNum);      //本包的数据帧个数
//    LKJ2K_CAN1_Rxcnt = p_can_PACK->frameAllNum;                 //从上电到目前所发的数据帧个数
//    if (eth_start_flag)
//    {
//        LKJ2K_CAN1_localcnt += LKJ2K_CAN1_RxFcnt;
//    }
//    else
//    {
//        eth_start_flag = 1;
//        LKJ2K_CAN1_localcnt = LKJ2K_CAN1_Rxcnt;
//    }
#if 0
    LOG_I("rx size %d", data_len);
    LOG_I("des mac %x %x %x %x %x %x ",
            ((uint8_t *)pbuf)[0], ((uint8_t *)pbuf)[1], ((uint8_t *)pbuf)[2], ((uint8_t *)pbuf)[3], ((uint8_t *)pbuf)[4], ((uint8_t *)pbuf)[5]);
    LOG_I("src mac %x %x %x %x %x %x ",
            ((uint8_t *)pbuf)[6], ((uint8_t *)pbuf)[7], ((uint8_t *)pbuf)[8], ((uint8_t *)pbuf)[9], ((uint8_t *)pbuf)[10], ((uint8_t *)pbuf)[11]);
#endif

    /* 2.转换CAN数据 */
    while (send_len < p_can_PACK->datalen)
    {
        /* 获取CNA帧地址 */
        s_can_frame = (S_ETH_CAN_FRAME*) (p_can_PACK->data + send_len);

        /* CAN帧有效长度判断 */
        send_len += s_can_frame->len + 5;
        if (send_len > p_can_PACK->datalen)
        {
            LOG_E("can len error");
            break;
        }

        /* 放入队列 */
        CAN_FRAME can_tmp;
        memset(&can_tmp, 0, sizeof(CAN_FRAME));

        can_tmp.priority_u8 = (uint8_t)(s_can_frame->ID >> 3u);
        can_tmp.no_u8 = (uint8_t)(s_can_frame->ID & 0x07);

        can_tmp.length_u8 = s_can_frame->len;
        rt_memcpy (can_tmp.data_u8, s_can_frame->Data, s_can_frame->len);

        ret = rt_mq_send(p_data_handle->can_data_mq, (const void *)&can_tmp, sizeof(CAN_FRAME));
        if(ret != RT_EOK)
        {
            LOG_E("can data mq send error %d", ret);
        }
        else
        {
//            LOG_I("ttt--- pr = %x, no = %x data = %x", can_tmp.priority_u8, can_tmp.no_u8, can_tmp.data_u8[0]);  //打开这个会导致上面队列发送出错 返回值为-3   过一会导致lwip里宕机
        }
    }
}

/* 解析CAN数据 */
rt_err_t CanDataHandle(S_DATA_HANDLE *p_data_handle)
{
    rt_err_t ret = -RT_ERROR;
    struct tm tm_new;
    static uint32_t set_rtc_time = 0u;

    if(RT_NULL == p_data_handle)
    {
        return -RT_EINVAL;
    }

    CAN_FRAME  can_tmp;

    ret = rt_mq_recv(p_data_handle->can_data_mq, (void *)&can_tmp, sizeof(CAN_FRAME), 0);
    if(ret != RT_EOK)
    {
//        LOG_W("recv msg error");
        return -RT_EEMPTY;
    }
#if 0
    can_tmp.priority_u8 = (uint8_t)((uint8_t)s_can_frame.ID>>3U);
    can_tmp.no_u8 = (uint8_t)(s_can_frame.ID & 0x07);

    can_tmp.length_u8 = s_can_frame.len;
    rt_memcpy (can_tmp.data_u8, s_can_frame.Data, s_can_frame.len);
#endif
//    LOG_I("pr = %x, no = %x data = %x", can_tmp.priority_u8, can_tmp.no_u8, can_tmp.data_u8[0]);  //打开这个会导致上面队列发送出错 返回值为-3   过一会导致lwip里宕机

    switch (can_tmp.priority_u8)
    {
        case CANPRI_ZKQSSA:
        case CANPRI_ZKQSSB:
            QSS_(can_tmp.no_u8) = can_tmp;
            SoftWare_Cycle_Flag = 1U;
            u8_Gonggongxinxi_Flag = 1U;

            if (can_tmp.priority_u8 == CANPRI_ZKQSSA)
                g_ZK_DevCode = 0x11;
            else if (can_tmp.priority_u8 == CANPRI_ZKQSSB)
                g_ZK_DevCode = 0x12;
            else
                g_ZK_DevCode = 0x11;

//            LOG_I("no %d data %d %d %d %d %d %d %d %d", can_tmp.no_u8,
//                    can_tmp.data_u8[0], can_tmp.data_u8[1], can_tmp.data_u8[2], can_tmp.data_u8[3],
//                    can_tmp.data_u8[4], can_tmp.data_u8[5], can_tmp.data_u8[6], can_tmp.data_u8[7]);
            /* 14-June-2018, by Liang Zhen. */
            Processing_HMB_HLRT_Message(can_tmp.no_u8, can_tmp.data_u8);
            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorCodeTime(&can_tmp.data_u8[4]);
            } /* end if */


            if(read_time_flag == 0)
            {
                tm_new.tm_year = 2000 + TIME_N;
                tm_new.tm_mon = TIME_Y;
                tm_new.tm_mday = TIME_R;

                tm_new.tm_hour = TIME_S;
                tm_new.tm_min = TIME_F;
                tm_new.tm_sec = TIME_M;
                if(tm_new.tm_year <= 2020)
                {
                    read_time_flag = 0;
                }
                else
                {
//                    LOG_I("time %d-%d-%d %d.%d.%d", tm_new.tm_year, tm_new.tm_mon, tm_new.tm_mday, tm_new.tm_hour, tm_new.tm_min, tm_new.tm_sec);
                    set_boart_time(&tm_new);
                    read_time_flag = 1;
                }
            }
            else if(1 == read_time_flag)       /* 10分钟更新一次rtc时间 */
            {
                tm_new.tm_year = 2000 + TIME_N;
                tm_new.tm_mon = TIME_Y;
                tm_new.tm_mday = TIME_R;

                tm_new.tm_hour = TIME_S;
                tm_new.tm_min = TIME_F;
                tm_new.tm_sec = TIME_M;

                if(tm_new.tm_year <= 2020)
                {
                    read_time_flag = 0;
                }
                else
                {
//                    LOG_I("time %d-%d-%d %d.%d.%d", tm_new.tm_year, tm_new.tm_mon, tm_new.tm_mday, tm_new.tm_hour, tm_new.tm_min, tm_new.tm_sec);
                }
                if(Common_BeTimeOutMN(&set_rtc_time, SET_RTC_CYCLE_MS))
                {
                    set_boart_time(&tm_new);
                }
            }

            break;

        case CANPRI_ZKRSSA:
        case CANPRI_ZKRSSB:
            RSS_(can_tmp.no_u8) = can_tmp;

            if (can_tmp.priority_u8 == CANPRI_ZKRSSA)
                g_ZK_DevCode = 0x11;
            else if (can_tmp.priority_u8 == CANPRI_ZKRSSB)
                g_ZK_DevCode = 0x12;
            else
                g_ZK_DevCode = 0x11;

            /* 28-September-2018, by Liang Zhen. */
            if (0x02U == can_tmp.no_u8)
            {
                UpdateErrorCodeDate(&can_tmp.data_u8[4]);
            } /* end if */


            break;

        case CANPRI_KONGCHEA:
        case CANPRI_KONGCHEB:
            CAN_KONGCHE(can_tmp.no_u8) = can_tmp;

            if (can_tmp.priority_u8 == CANPRI_KONGCHEA)
                g_ZK_DevCode = 0x11;
            else if (can_tmp.priority_u8 == CANPRI_KONGCHEB)
                g_ZK_DevCode = 0x12;
            else
                g_ZK_DevCode = 0x11;

            break;

        case CANPRI_XSQCMD1:
            XSQ1ZJ_time = rt_tick_get();
            CAN_0X45_(can_tmp.no_u8) = can_tmp;

            g_XSQ_DevCode = 0x21;

            if ((0x01U == can_tmp.no_u8) && (can_tmp.data_u8[2] == 0x55U))
            {
//                reset_ksz8895_enable = 1u;  //TODO(mingzhao)
                LOG_I("xsq1 reset ksz8895.....\r\n");
            } /* end if */
            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_DMI(DEV_SIDE_I, &can_tmp.data_u8[2]);
            } /* end if */
            break;

        case CANPRI_XSQCMD2:
            XSQ2ZJ_time = rt_tick_get();
            CAN_0X46_(can_tmp.no_u8) = can_tmp;

            g_XSQ_DevCode = 0x22;

            if ((0x01U == can_tmp.no_u8) && (can_tmp.data_u8[2] == 0x55U))
            {
//                reset_ksz8895_enable = 1u;  //TODO(mingzhao)
                LOG_I("xsq2 reset ksz8895.....");
            } /* end if */
            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_DMI(DEV_SIDE_II, &can_tmp.data_u8[2]);
            } /* end if */
            break;

        case CANPRI_TXZJ:
            JCTXZJ_time = rt_tick_get();
            CAN_0X50_(can_tmp.no_u8) = can_tmp;
            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_HCB(can_tmp.data_u8);
            } /* end if */
            break;

        case CANPRI_ZKZJA:
            ZK1ZJ_time = rt_tick_get();
            CAN_0X63_(can_tmp.no_u8) = can_tmp;

            g_ZK_DevCode = 0x11;

            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_HMB(CPU_A, can_tmp.data_u8);
            } /* end if */
            break;

        case CANPRI_ZKZJB:
            ZK2ZJ_time = rt_tick_get();
            CAN_0X73_(can_tmp.no_u8) = can_tmp;

            g_ZK_DevCode = 0x12;

            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_HMB(CPU_B, can_tmp.data_u8);
            } /* end if */
            break;

        case CANPRI_JCJKZJ:
        case CAN_PRI_HIB_B:
            JCJKZJ_time = rt_tick_get();
            CAN_0X90_(can_tmp.no_u8) = can_tmp;
            /* 28-September-2018, by Liang Zhen. */
            if (0x00U == can_tmp.no_u8)
            {
                UpdateErrorFlag_HIB(can_tmp.data_u8);
            } /* end if...else */
            break;

            /* 23-March-2018, by Liang Zhen. */
        case CAN_PRI_CEU:
            Processing_CEU_Message(can_tmp.no_u8, can_tmp.data_u8);
            break;

            /* 10-May-2018, by Liang Zhen. */
        case 0xE0U:
            /* MCAN0 is not fault. */
            if (!(ERROR_FLAG & 0x02U))
            {
                Processing_IAP_Message(can_tmp.no_u8, 0xE0U, can_tmp.data_u8);
            } /* end if */
            break;

            /* 12-June-2018, by Liang Zhen. */
        case EBV_CTRL_A:
        case EBV_CTRL_B:
            if (can_tmp.priority_u8 == EBV_CTRL_A)
                g_ZK_DevCode = 0x11;
            else if (can_tmp.priority_u8 == EBV_CTRL_B)
                g_ZK_DevCode = 0x12;
            else
                g_ZK_DevCode = 0x11;

            Processing_EBV_Controlling(can_tmp.no_u8, can_tmp.data_u8 + 1U);
            break;

            /* 13-June-2018, by Liang Zhen. */
        case EBV_WC_ABV:
            Processing_ABV_WorkingCondition(can_tmp.no_u8, can_tmp.data_u8 + 1U);
            break;

            /* 13-June-2018, by Liang Zhen. */
        case EBV_WC_IBV:
            CAN_0x76(can_tmp.no_u8) = can_tmp;
            Processing_IBV_WorkingCondition(can_tmp.no_u8, can_tmp.data_u8 + 1U);
            break;

            /* 14-June-2018, by Liang Zhen. */
        case LKJ_LLRT_MSG:
            CAN_0x68(can_tmp.no_u8) = can_tmp;
            Processing_LKJ_LLRT_Message(can_tmp.no_u8, can_tmp.data_u8);
//            LOG_I("68_no %d data %d %d %d %d %d %d %d %d", can_tmp.no_u8,
//                               can_tmp.data_u8[0], can_tmp.data_u8[1], can_tmp.data_u8[2], can_tmp.data_u8[3],
//                               can_tmp.data_u8[4], can_tmp.data_u8[5], can_tmp.data_u8[6], can_tmp.data_u8[7]);
            break;

            /* 06-July-2020, by DuYanPo. */
        case Host_SlopeInfo:
            Processing_Gradepacket_Message(&can_tmp);
            break;
            /* 06-July-2020, by DuYanPo. */
        case CANPRI_ZKQSSTC:
//            LOG_I("no %d data %d %d %d %d %d %d %d %d", can_tmp.no_u8,
//                               can_tmp.data_u8[0], can_tmp.data_u8[1], can_tmp.data_u8[2], can_tmp.data_u8[3],
//                               can_tmp.data_u8[4], can_tmp.data_u8[5], can_tmp.data_u8[6], can_tmp.data_u8[7]);
            CAN_0x66(can_tmp.no_u8) = can_tmp;
            break;
            /* 06-July-2020, by DuYanPo. */
        case CANPRI_ZKRSSTC1:
            CAN_0x67(can_tmp.no_u8) = can_tmp;
            break;
            /* 06-July-2020, by DuYanPo. */
        case CANPRI_DMIXSCX1:
        case CANPRI_DMIXSCX2:
            if (can_tmp.priority_u8 == CANPRI_DMIXSCX1)
                g_ZK_DevCode = 0x11;
            else if (can_tmp.priority_u8 == CANPRI_DMIXSCX2)
                g_ZK_DevCode = 0x12;
            else
                g_ZK_DevCode = 0x11;

            CAN_0x6A(can_tmp.no_u8) = can_tmp;

            break;
            /* 06-July-2020, by DuYanPo. */
        case CANPRI_WJSJ:
            CAN_0x81(can_tmp.no_u8) = can_tmp;
            break;
            /* 06-July-2020, by DuYanPo. */
        case CANPRI_WJJKZJ:
            CAN_0x82(can_tmp.no_u8) = can_tmp;
            break;
            /* 07-10-2020, by DuYanPo. */
        case CANPRI_ZKWJSJ1:
            CAN_0x30(can_tmp.no_u8) = can_tmp;
            break;
            /* 07-10-2020, by DuYanPo. */
        case CANPRI_ZKWJSJ2:
            CAN_0x31(can_tmp.no_u8) = can_tmp;
            break;
            /* 07-10-2020, by DuYanPo. */
        case CANPRI_WJZKSJ1:
            CAN_0x32(can_tmp.no_u8) = can_tmp;
            break;
            /* 07-10-2020, by DuYanPo. */
        case CANPRI_WJZKSJ2:
            CAN_0x33(can_tmp.no_u8) = can_tmp;
            break;
            /* 19-10-2020, by DuYanPo. */
        case CANPRI_JCJKSTAT:
            CAN_0x91(can_tmp.no_u8) = can_tmp;

        default:
            break;
    } /* end switch */
    return RT_EOK;
}

uint8_t DataHandleLKJIsOnline(void)
{
    if(read_time_flag > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}
