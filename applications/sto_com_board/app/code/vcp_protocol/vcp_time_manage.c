/***************************************************************************
文件名：vcp_time_manage.c
模  块：安全层所需时间管理模块
详  述：
***************************************************************************/
#include "support_timer.h"
#include "vcp_time_manage.h"

static sint32 diff_CAN1 = 0, diff_CAN2 = 0, diff_ETH = 0;
static BOOL diff_CAN1_sig = TRUE, diff_CAN2_sig = TRUE, diff_ETH_sig = TRUE;


/****************************************************************************
* 函数名: tick_5MS_get
* 说    明: 用于系统时钟同步，用户实现每5ms进行一次计数
* 参    数: 无
* 返回值: 时钟信号计数值
 ****************************************************************************/
static uint32 tick_5MS_get(void)
{
    static uint32 counter_5ms_u32 = 0U;

    counter_5ms_u32 = ( support_timer_getTick() / 5 ) ;

    return counter_5ms_u32;
}

/****************************************************************************
* 函数名: SetTime_CAN1_diff
* 说    明: 设置安全层CAN1通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 无
 ****************************************************************************/
extern void SetTime_CAN1_diff( uint32 Rx_ctl_time)
{
    /* 1.开辟空间 */
     sint32 Get_MS_count;
     Get_MS_count = tick_5MS_get();

     /* 2. 计算时钟差值 */
    if(Rx_ctl_time >= Get_MS_count)
    {
        diff_CAN1 = Rx_ctl_time - Get_MS_count;
        diff_CAN1_sig = TRUE;
    }
    else
    {
        diff_CAN1 =Get_MS_count - Rx_ctl_time;
        diff_CAN1_sig = FALSE;
    }
}

/****************************************************************************
* 函数名: GetTime_CAN1_diff
* 说    明: 获取安全层CAN1通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 时钟信号计数值
 ****************************************************************************/
extern sint32 GetTime_CAN1_diff( void )
{
    if(diff_CAN1_sig == TRUE )
    {
        return (tick_5MS_get() + diff_CAN1);
    }
    else
    {
        return (tick_5MS_get() - diff_CAN1);
    }
}

/****************************************************************************
* 函数名: SetTime_CAN2_diff
* 说    明: 设置安全层CAN2通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 无
 ****************************************************************************/
extern void SetTime_CAN2_diff( uint32 Rx_ctl_time)
{
    /* 1.开辟空间 */
    sint32 Get_MS_count;
    Get_MS_count = tick_5MS_get();

    /* 2. 计算时钟差值 */
    if(Rx_ctl_time >= Get_MS_count)
    {

        diff_CAN2 = Rx_ctl_time - Get_MS_count;
        diff_CAN2_sig = TRUE;
    }
    else
    {
        diff_CAN2 =Get_MS_count - Rx_ctl_time;
        diff_CAN2_sig = FALSE;
    }
}

/****************************************************************************
* 函数名: GetTime_CAN2_diff
* 说    明: 获取安全层CAN2通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 时钟信号计数值
 ****************************************************************************/
extern sint32 GetTime_CAN2_diff( void)
{
    if(diff_CAN2_sig == TRUE )
    {
        return (tick_5MS_get() + diff_CAN2);
    }
    else
    {
        return (tick_5MS_get() - diff_CAN2);
    }
}

/****************************************************************************
* 函数名: SetTime_ETH_diff
* 说    明: 设置安全层ETH通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 无
 ****************************************************************************/
extern void SetTime_ETH_diff( uint32 Rx_ctl_time)
{
    /* 1.开辟空间 */
    sint32 Get_MS_count;
    Get_MS_count = tick_5MS_get();

    /* 2. 计算时钟差值 */
    if(Rx_ctl_time >= Get_MS_count)
    {
         diff_ETH = Rx_ctl_time - Get_MS_count;
         diff_ETH_sig = TRUE;
    }
    else
    {
        diff_ETH = Get_MS_count- Rx_ctl_time;
        diff_ETH_sig = FALSE;
    }

    //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[LT:%d,VT:%d]\r\n",diff_ETH_sig, diff_ETH,Get_MS_count,Rx_ctl_time);

}

/****************************************************************************
* 函数名: GetTime_ETH_diff
* 说    明: 获取安全层ETH通道时钟信号
* 参    数: 接受主控的时钟信号
* 返回值: 时钟信号计数值
 ****************************************************************************/
extern sint32 GetTime_ETH_diff( void)
{
    if(diff_ETH_sig == TRUE )
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() + diff_ETH));
      return (tick_5MS_get() + diff_ETH);
    }
    else
    {
      //MY_Printf("TIME_SYNC-->flg:%d diff_ETH:%d,[VT_NOW:%d]\r\n",diff_ETH_sig, diff_ETH,(tick_5MS_get() - diff_ETH));
      return (tick_5MS_get()- diff_ETH);
    }
}


 /*******************************************************
 * 功能:消息发送序列号计算
 * 参数:now_msg_no_u32 -当前消息序列号
 * 返回:按规则计算后的序列号
  ******************************************************/
extern uint32 count_msg_no(uint32 now_msg_no_u32)
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
 extern uint16 count_msg_no16(uint16 now_msg_no_u16)
 {
   /** 递增 */
   now_msg_no_u16++;
   if((now_msg_no_u16 == (uint16)0xFFFF) || (now_msg_no_u16 == (uint16)0x0))
   {
     now_msg_no_u16 = (uint16)1;
   }
   else
   {
     /** ntd */
   }
   return now_msg_no_u16;
 }
