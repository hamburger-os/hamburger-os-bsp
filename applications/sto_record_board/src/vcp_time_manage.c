/***************************************************************************
文件名：vcp_time_manage.c
模  块：安全层所需时间管理模块
详  述：
***************************************************************************/
#include "vcp_time_manage.h"

#define DBG_TAG "vcp time"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

static int32_t diff_ETH0 = 0, diff_ETH1 = 0;
static rt_bool_t diff_ETH0_sig = RT_TRUE, diff_ETH1_sig = RT_TRUE;

/****************************************************************************
* 函数名: tick_5MS_get
* 说    明: 用于系统时钟同步，用户实现每5ms进行一次计数
* 参    数: 无
* 返回值: 时钟信号计数值
 ****************************************************************************/
static uint32_t tick_5MS_get(void)
{
    static uint32_t counter_5ms_u32 = 0U;

    counter_5ms_u32 = ( rt_tick_get() / 5 ) ;

    return counter_5ms_u32;
}

/****************************************************************************
* 函数名: SetTime_ETH0_diff
* 说    明: 设置安全层ETH通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 无
 ****************************************************************************/
extern void SetTime_ETH0_diff( uint32_t Rx_ctl_time)
{
    /* 1.开辟空间 */
    int32_t Get_MS_count;
    Get_MS_count = tick_5MS_get();

    /* 2. 计算时钟差值 */
    if(Rx_ctl_time >= Get_MS_count)
    {
      diff_ETH0 = Rx_ctl_time - Get_MS_count;
      diff_ETH0_sig = RT_TRUE;
    }
    else
    {
      diff_ETH0 = Get_MS_count- Rx_ctl_time;
      diff_ETH0_sig = RT_FALSE;
    }

    //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[LT:%d,VT:%d]\r\n",diff_ETH_sig, diff_ETH,Get_MS_count,Rx_ctl_time);

}

/****************************************************************************
* 函数名: SetTime_ETH1_diff
* 说    明: 设置安全层ETH通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 无
 ****************************************************************************/
extern void SetTime_ETH1_diff( uint32_t Rx_ctl_time)
{
    /* 1.开辟空间 */
    int32_t Get_MS_count;
    Get_MS_count = tick_5MS_get();

    /* 2. 计算时钟差值 */
    if(Rx_ctl_time >= Get_MS_count)
    {
         diff_ETH1 = Rx_ctl_time - Get_MS_count;
         diff_ETH1_sig = RT_TRUE;
    }
    else
    {
        diff_ETH1 = Get_MS_count- Rx_ctl_time;
        diff_ETH1_sig = RT_FALSE;
    }

    //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[LT:%d,VT:%d]\r\n",diff_ETH_sig, diff_ETH,Get_MS_count,Rx_ctl_time);

}

/****************************************************************************
* 函数名: GetTime_ETH0_diff
* 说    明: 获取安全层ETH通道时钟信号
* 参    数: 无
* 返回值: 时钟信号计数值
 ****************************************************************************/
extern int32_t GetTime_ETH0_diff( void)
{
    if(diff_ETH0_sig == RT_TRUE )
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() + diff_ETH));
      return (tick_5MS_get() + diff_ETH0);
    }
    else
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() - diff_ETH));
      return (tick_5MS_get()- diff_ETH0);
    }
}

/****************************************************************************
* 函数名: GetTime_ETH1_diff
* 说    明: 获取安全层ETH通道时钟信号
* 参    数: 无
* 返回值: 时钟信号计数值
 ****************************************************************************/
extern int32_t GetTime_ETH1_diff( void)
{
    if(diff_ETH1_sig == RT_TRUE )
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() + diff_ETH));
      return (tick_5MS_get() + diff_ETH1);
    }
    else
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() - diff_ETH));
      return (tick_5MS_get()- diff_ETH1);
    }
}


 /*******************************************************
 * 功能:消息发送序列号计算
 * 参数:now_msg_no_u32 -当前消息序列号
 * 返回:按规则计算后的序列号
  ******************************************************/
extern uint32_t count_msg_no(uint32_t now_msg_no_u32)
 {
   /*递增*/
   now_msg_no_u32++;
   if((now_msg_no_u32 == 0xFFFFFFFFU) || (now_msg_no_u32 == 0x0U))
   {
       now_msg_no_u32 = 1U;
   }
   else
   {
     /** ntd */
   }
   return now_msg_no_u32;
 }


 /*******************************************************
  * 功能：消息发送序列号计算，1－0XFFFE之间
  * 参数:now_msg_no_u16 - 当前消息序列号
  * 返回:按规则计算后的序列号
  ******************************************************/
 extern uint16_t count_msg_no16(uint16_t now_msg_no_u16)
 {
   /** 递增 */
   now_msg_no_u16++;
   if((now_msg_no_u16 == (uint16_t)0xFFFF) || (now_msg_no_u16 == (uint16_t)0x0))
   {
     now_msg_no_u16 = (uint16_t)1;
   }
   else
   {
     /** ntd */
   }
   return now_msg_no_u16;
 }
