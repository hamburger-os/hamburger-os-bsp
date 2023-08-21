/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-21     zm       the first version
 */

#include "ksz8851_lep.h"

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION

#include <string.h>

#define DBG_TAG "ksz8851_lep"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*
  * @brief  初始化网口数据缓冲区.
  * @param  ps_eth_if 缓冲区指针
  * @retval rt_err_t;
  */
rt_err_t lep_eth_if_init(S_ETH_IF *ps_eth_if)
{

  if(NULL == ps_eth_if)
  {
      return -RT_EEMPTY;
  }

  memset((void *)ps_eth_if, 0, sizeof(S_ETH_IF));

  /* 1.申请接收缓冲区空间 */
  ps_eth_if->rx_head = rt_malloc(sizeof(S_LEP_BUF));
  if(NULL == ps_eth_if->rx_head)
  {
      LOG_E("malloc size %d error", sizeof(S_LEP_BUF));
      return -RT_EEMPTY;
  }

  /* 2.接收缓冲区清零 */
  memset((void *)ps_eth_if->rx_head, 0, sizeof(S_LEP_BUF));

  ps_eth_if->rx_head->flag = LEP_RBF_HEAD;
  /* 3.初始化链表 */
  rt_list_init(&ps_eth_if->rx_head->list);
  if(rt_list_isempty(&ps_eth_if->rx_head->list))
  {
      LOG_I("rx_list init ok");
  }
  else
  {
      LOG_E("rx_list init error");
      return -RT_EEMPTY;
  }

  return RT_EOK;
}
/*
  * @brief  清除网口数据缓冲区，释放缓冲区.
  * @param  ps_eth_if 缓冲区指针
  * @retval rt_err_t;
  */
rt_err_t lep_eth_if_clear(S_ETH_IF *ps_eth_if)
{
    S_LEP_BUF *p_s_LepBuf = RT_NULL;
    rt_list_t *list_pos = NULL;
    rt_list_t *list_next = NULL;

    if(NULL == ps_eth_if)
    {
        return -RT_EEMPTY;
    }
    if(NULL == ps_eth_if->rx_head)
    {
        LOG_I("rx_head error");
        return -RT_EEMPTY;
    }

    rt_list_for_each_safe(list_pos, list_next, &ps_eth_if->rx_head->list)
    {
        p_s_LepBuf = rt_list_entry(list_pos, struct tagLEP_BUF, list);
        if (p_s_LepBuf != RT_NULL)
        {
            if ((p_s_LepBuf->flag & LEP_RBF_RV) != 0U)
            {
                rt_list_remove(list_pos);
                /* 释放接收接收缓冲区 */
                rt_free(p_s_LepBuf);
            }
        }
    }
    LOG_I("lep_eth_if_clear ok");
    return RT_EOK;
}

#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
