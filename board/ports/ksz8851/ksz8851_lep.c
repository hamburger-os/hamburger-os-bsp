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
  * @param  none
  * @retval none;
  */
rt_err_t lep_eth_if_init(S_ETH_IF *ps_eth_if)
{
  uint32_t i;

  if(NULL == ps_eth_if)
  {
      return -RT_EEMPTY;
  }

  memset((void *)ps_eth_if, 0, sizeof(S_ETH_IF));

  ps_eth_if->rx_buf = rt_malloc(sizeof(S_LEP_BUF) * BSP_LINK_LAYER_RX_BUF_NUM);
  if(NULL == ps_eth_if->rx_buf)
  {
      LOG_E("malloc size %d error", sizeof(S_LEP_BUF) * BSP_LINK_LAYER_RX_BUF_NUM);
      return -RT_EEMPTY;
  }

  memset((void *)ps_eth_if->rx_buf, 0, sizeof(S_LEP_BUF) * BSP_LINK_LAYER_RX_BUF_NUM);

  for (i= 0U; i<(BSP_LINK_LAYER_RX_BUF_NUM-1U); i++)
  {
    ps_eth_if->rx_buf[i].pnext = &ps_eth_if->rx_buf[i+1];
  }
  ps_eth_if->rx_buf[i].pnext = &ps_eth_if->rx_buf[0];
  ps_eth_if->prx_rptr = &ps_eth_if->rx_buf[0];
  ps_eth_if->prx_wptr = &ps_eth_if->rx_buf[0];

  return RT_EOK;
}

/*
  * @brief  lep_if_is_received
  * @param  none
  * @retval none
  */
S_LEP_BUF *lep_if_is_received(const S_ETH_IF *ps_eth_if)
{
  if ((ps_eth_if->prx_rptr->flag & LEP_RBF_RV) != 0U)
  {
    return ps_eth_if->prx_rptr;
  }
  else
  {
    return NULL;
  }
}
/*
  * @brief  释放一个接收缓冲区
  * @param  none
  * @retval none
  */
void lep_if_release_rptr( S_ETH_IF *ps_eth_if)
{
  ps_eth_if->prx_rptr->flag &= ~LEP_RBF_RV;
  /* if (ps_eth_if->prx_rptr != ps_eth_if->prx_wptr)*/
  ps_eth_if->prx_rptr = ps_eth_if->prx_rptr->pnext;
}

/*
  * @brief  取空闲缓冲区指针.
  * @param  *ps_eth_if: 网口数据缓冲区控制结构指针
  * @retval =NULL_ 无空闲缓冲区 !=NULL_ 空闲的缓冲区
  */
S_LEP_BUF *lep_get_free_buf( S_ETH_IF *ps_eth_if)
{
  if (ps_eth_if != NULL && (ps_eth_if->prx_wptr != NULL))
  {
    if ((ps_eth_if->prx_wptr->flag & LEP_RBF_RV) == 0U)
    {
      return ps_eth_if->prx_wptr;
    }
    else if((LEP_RBF_RV==ps_eth_if->prx_wptr->flag & LEP_RBF_RV)
      &&(ps_eth_if->prx_rptr==ps_eth_if->prx_wptr))
    {
      /* 接收缓冲区满，抛弃最先接收的*/
      lep_if_release_rptr(ps_eth_if);
      return ps_eth_if->prx_wptr;
    }
    else
    {
        /* nothing*/
    }
  }
  else
  {
    /* nothing*/
  }
  return NULL;
}

#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
