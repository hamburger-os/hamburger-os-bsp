/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-19     SummerGift   first version
 * 2018-12-25     zylx         fix some bugs
 * 2019-06-10     SummerGift   optimize PHY state detection process
 * 2019-09-03     xiaofan      optimize link change detection process
 */

#include "board.h"
#include "drv_eth.h"
#include "lan8720.h"
#include "jl5104.h"
#include <netif/ethernetif.h>
#include <lwipopts.h>

#ifdef ETH_USING_KVDB_NET_IF
#include "flashdb_port.h"
#endif

/*
* Emac driver uses CubeMX tool to generate emac and phy's configuration,
* the configuration files can be found in CubeMX_Config folder.
*/

/* debug option */
#define DRV_DEBUG
#define LOG_TAG             "drv.emac"
#include <drv_log.h>

#ifdef SOC_SERIES_STM32H7
#define ETH_NOCACHE_RAM_SECTION     __attribute__((section(".bss.bufferable")))
#else
#define ETH_NOCACHE_RAM_SECTION
#endif

#define ETH_RX_BUFFER_SIZE 1524

/* Data Type Definitions */
typedef enum
{
    RX_ALLOC_OK = 0x00,
    RX_ALLOC_ERROR = 0x01
} RxAllocStatusTypeDef;

typedef struct
{
    struct pbuf_custom pbuf_custom;
    uint8_t buff[(ETH_RX_BUFFER_SIZE + 31) & ~31] __ALIGNED(32);
} RxBuff_t;

/* Memory Pool Declaration */
#define ETH_RX_BUFFER_CNT             12U
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");

/* Variable Definitions */
static uint8_t RxAllocStatus;

#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#pragma location=0x30044000
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30044200
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30044000))) static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30044200))) static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] ETH_NOCACHE_RAM_SECTION; /* Ethernet Rx DMA Descriptors */
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] ETH_NOCACHE_RAM_SECTION; /* Ethernet Tx DMA Descriptors */

#endif

struct rt_stm32_eth stm32_eth_device = {
    .dev_name = "e",
    .phy_addr = LAN8720_ADDR,
};

#define ETH_RST_PIN "PA.3"
//#ifdef BSP_USING_ETH_RST_PIN
static void phy_reset(void)
{
    rt_base_t rst_pin = rt_pin_get(ETH_RST_PIN);
    rt_pin_mode(rst_pin, PIN_MODE_OUTPUT);
    rt_pin_write(rst_pin, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(rst_pin, PIN_HIGH);
}
//#endif

static void phy_linkchange(void *parameter);

/* EMAC initialization function */
static rt_err_t rt_stm32_eth_init(rt_device_t dev)
{
    struct rt_stm32_eth *eth = dev->user_data;
//#ifdef BSP_USING_ETH_RST_PIN
    phy_reset();
//#endif

    eth->heth.Instance = ETH;
    eth->heth.Init.MACAddr = &eth->mac[0];
#ifdef ETH_USING_MII_MODE
    eth->heth.Init.MediaInterface = HAL_ETH_MII_MODE;
#endif
#ifdef ETH_USING_RMII_MODE
    eth->heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
#endif
    eth->heth.Init.TxDesc = DMATxDscrTab;
    eth->heth.Init.RxDesc = DMARxDscrTab;
    eth->heth.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

    if (HAL_ETH_Init(&eth->heth) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_ETH_SetMDIOClockRange(&eth->heth);

    rt_memset(&eth->TxConfig, 0 , sizeof(ETH_TxPacketConfig));
    eth->TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    eth->TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    eth->TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    /* Initialize the RX POOL */
    LWIP_MEMPOOL_INIT(RX_POOL);
#ifdef PHY_USING_LAN8720A
    if (LAN8720_Init() != LAN8720_STATUS_OK)
    {
        LOG_E("LAN8720 init failed!");
    }
#endif

#ifdef SWITCH_USING_JL5104
    if(JL5104_Init() != JL5104_STATUS_OK)
    {
        LOG_E("JL5104 init failed");
    }
#endif
    phy_linkchange(eth);
    return RT_EOK;
}

static rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_stm32_eth *eth = dev->user_data;

    return lep_eth_if_clear(&eth->link_layer_buf, E_ETH_IF_CLER_MODE_ALL);
#else
    return RT_EOK;
#endif
}

static rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_stm32_eth *eth = dev->user_data;

    if(NULL == eth)
    {
        return -RT_EEMPTY;
    }

    return lep_eth_if_clear(&eth->link_layer_buf, E_ETH_IF_CLER_MODE_ALL);
#else
    return RT_EOK;
#endif
}

static rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_stm32_eth *eth = dev->user_data;

    rt_uint16_t read_size = 0;
    S_LEP_BUF *p_s_LepBuf = RT_NULL;
    rt_list_t *list_pos = NULL;
    rt_list_t *list_next = NULL;

    rt_mutex_take(&eth->eth_mux, RT_WAITING_FOREVER);

    /* step1：遍历链表 */
    if(eth->link_layer_buf.rx_head != RT_NULL)
    {
        rt_list_for_each_safe(list_pos, list_next, &eth->link_layer_buf.rx_head->list)
        {
            p_s_LepBuf = rt_list_entry(list_pos, struct tagLEP_BUF, list);
            if (p_s_LepBuf != RT_NULL)
            {
                if ((p_s_LepBuf->flag & LEP_RBF_RV) != 0U)
                {
                    /* step2：获取接收包数据长度 */
                    if(p_s_LepBuf->len > LEP_MAC_PKT_MAX_LEN)
                    {
                        read_size = LEP_MAC_PKT_MAX_LEN;
                    }
                    else
                    {
                        read_size = p_s_LepBuf->len;
                        if (read_size > size)
                        {
                            read_size = size;
                        }
                    }

                    /* step3：提取包数据 */
                    rt_memcpy(buffer, p_s_LepBuf->buf, read_size);
                    rt_list_remove(list_pos);
                    /* step4：释放接收接收缓冲区 */
                    rt_free(p_s_LepBuf);
                    eth->link_layer_buf.rx_lep_buf_num--;

                    rt_mutex_release(&eth->eth_mux);
                    return read_size;
                }
            }
        }
    }

    rt_mutex_release(&eth->eth_mux);
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    return 0;
}

static rt_size_t rt_stm32_eth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_stm32_eth *eth = dev->user_data;
    ETH_BufferTypeDef tx_buffer;

    if(size <= 0)
    {
        LOG_E("size error");
        return 0;
    }

    rt_mutex_take(&eth->eth_mux, RT_WAITING_FOREVER);

    rt_memset((void *)&tx_buffer, 0, sizeof(ETH_BufferTypeDef));
    tx_buffer.buffer = (uint8_t *)buffer;
    tx_buffer.len = size;
    tx_buffer.next = RT_NULL;

    eth->TxConfig.Length = size;
    eth->TxConfig.TxBuffer = &tx_buffer;
    eth->TxConfig.pData = RT_NULL;

#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache();
#endif

    HAL_ETH_Transmit_IT(&eth->heth, &eth->TxConfig);
    if (rt_completion_wait(&eth->TxPkt_completion, eth->TxConfig.Length * 8) != RT_EOK)
    {
        LOG_E("eth link tx timeout %d!", eth->TxConfig.Length);
    }

    HAL_ETH_ReleaseTxPacket(&eth->heth);

    rt_mutex_release(&eth->eth_mux);
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    return size;
}

static rt_err_t rt_stm32_eth_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_stm32_eth *eth = dev->user_data;

    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
        {
            SMEMCPY(args, eth->mac, MAC_ADDR_LEN);
        }
        else
        {
            return -RT_ERROR;
        }
        break;

    default :
        break;
    }

    return RT_EOK;
}

/* ethernet device interface */
/* transmit data*/
rt_err_t rt_stm32_eth_tx(rt_device_t dev, struct pbuf *p)
{
    struct rt_stm32_eth *eth = dev->user_data;

    uint32_t i = 0U;
    struct pbuf *q = NULL;
    err_t errval = ERR_OK;
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT] = { 0 };

    rt_mutex_take(&eth->eth_mux, RT_WAITING_FOREVER);

    rt_memset(Txbuffer, 0, ETH_TX_DESC_CNT * sizeof(ETH_BufferTypeDef));

    for (q = p; q != NULL; q = q->next)
    {
        if (i >= ETH_TX_DESC_CNT)
            return ERR_IF;

        Txbuffer[i].buffer = q->payload;
        Txbuffer[i].len = q->len;

        if (i > 0)
        {
            Txbuffer[i - 1].next = &Txbuffer[i];
        }

        if (q->next == NULL)
        {
            Txbuffer[i].next = NULL;
        }

        i++;
    }

    eth->TxConfig.Length = p->tot_len;
    eth->TxConfig.TxBuffer = Txbuffer;
    eth->TxConfig.pData = p;

    pbuf_ref(p);

#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache();
#endif

    HAL_ETH_Transmit_IT(&eth->heth, &eth->TxConfig);
    if (rt_completion_wait(&eth->TxPkt_completion, eth->TxConfig.Length * 8) != RT_EOK)
    {
        LOG_E("eth tx timeout %d!", eth->TxConfig.Length);
    }

    HAL_ETH_ReleaseTxPacket(&eth->heth);

#ifdef ETH_TX_DUMP
    LOG_D("tx start >>>>>>>>>>>>>>>>");
    if (p != NULL)
    {
        struct pbuf *q = NULL;
        for (q = p; q != NULL; q = q->next)
        {
            LOG_HEX("tx", 16, (uint8_t *)((uint8_t *)q->payload), q->len);
            LOG_D("tx pbuf_len %d >>>>>>>>>>>>>>>>", q->len);
        }
        LOG_D("tx tot_len %d >>>>>>>>>>>>>>>>", p->tot_len);
    }
    LOG_D("tx end <<<<<<<<<<<<<<<<");
#endif

    rt_mutex_release(&eth->eth_mux);
    return errval;
}

/* receive data*/
struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{
    struct rt_stm32_eth *eth = dev->user_data;

    struct pbuf *p = NULL;

    rt_mutex_take(&eth->eth_mux, RT_WAITING_FOREVER);

    if (RxAllocStatus == RX_ALLOC_OK)
    {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanInvalidateDCache();    //无效化并且清除Dcache
#endif

        HAL_ETH_ReadData(&eth->heth, (void **) &p);
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
        struct pbuf *q= NULL;

        if(eth->link_layer_buf.rx_head != RT_NULL)
        {
            if(p != RT_NULL)
            {
                if(RT_NULL == p->next && p->tot_len < LEP_MAC_PKT_MAX_LEN)
                {
                    for(q = p; q != NULL; q = q->next)
                    {
                        if(is_ip_package((uint8_t *)q->payload, p->tot_len) == 0)
                        {
                            if (eth->rx_num >= BSP_LINK_LAYER_RX_BUF_NUM)
                            {
                                lep_eth_if_clear(&eth->link_layer_buf, E_ETH_IF_CLER_MODE_ONE);
                            }

                            S_LEP_BUF *ps_lep_buf = rt_malloc(sizeof(S_LEP_BUF));
                            if(RT_NULL != ps_lep_buf)
                            {
                                eth->rx_num++;
                                ps_lep_buf->flag = 0;
                                ps_lep_buf->flag |= LEP_RBF_RV;
                                ps_lep_buf->len = q->len;
                                rt_memcpy(ps_lep_buf->buf, q->payload, q->len);
                                rt_list_insert_before(&eth->link_layer_buf.rx_head->list, &ps_lep_buf->list);
                                if(dev->rx_indicate != NULL)
                                {
                                    dev->rx_indicate(dev, q->len);
                                }
                            }
                            else
                            {
                                LOG_E("ps_lep_buf rx null");
                            }
                        }
                    }
                }
            }
        }

#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    }

#ifdef ETH_RX_DUMP
    LOG_D("rx start >>>>>>>>>>>>>>>>");
    if (p != NULL)
    {
        struct pbuf *q = NULL;
        for (q = p; q != NULL; q = q->next)
        {
            LOG_HEX("rx", 16, (uint8_t * )((uint8_t * )q->payload), q->len);
            LOG_D("rx pbuf_len %d >>>>>>>>>>>>>>>>", q->len);
        }
        LOG_D("rx tot_len %d >>>>>>>>>>>>>>>>", p->tot_len);
    }
    LOG_D("rx end <<<<<<<<<<<<<<<<");
#endif
    rt_mutex_release(&eth->eth_mux);
    return p;
}

/**
 * @brief  Custom Rx pbuf free callback
 * @param  pbuf: pbuf to be freed
 * @retval None
 */
static void pbuf_free_custom(struct pbuf *p)
{
    struct pbuf_custom* custom_pbuf = (struct pbuf_custom*) p;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);

    /* If the Rx Buffer Pool was exhausted, signal the ethernetif_input task to
     * call HAL_ETH_GetRxDataBuffer to rebuild the Rx descriptors. */

    if (RxAllocStatus == RX_ALLOC_ERROR)
    {
        RxAllocStatus = RX_ALLOC_OK;
        eth_device_ready(&stm32_eth_device.parent);
    }
}

/* interrupt service routine */
void ETH_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_ETH_IRQHandler(&stm32_eth_device.heth);

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief  Ethernet Rx Transfer completed callback
 * @param  handlerEth: ETH handler
 * @retval None
 */
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *handlerEth)
{
    eth_device_ready(&stm32_eth_device.parent);
}

/**
 * @brief  Ethernet Tx Transfer completed callback
 * @param  handlerEth: ETH handler
 * @retval None
 */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *handlerEth)
{
    rt_completion_done(&stm32_eth_device.TxPkt_completion);
}

/**
 * @brief  Tx Free callback.
 * @param  buff: pointer to buffer to free
 * @retval None
 */
void HAL_ETH_TxFreeCallback(uint32_t * buff)
{
    if(buff != RT_NULL)
    {
        if(((struct pbuf *) buff)->ref > 0)
        {
            pbuf_free((struct pbuf *) buff);
        }
    }
}

/**
 * @brief  Ethernet DMA transfer error callback
 * @param  handlerEth: ETH handler
 * @retval None
 */
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *handlerEth)
{
    uint32_t error = HAL_ETH_GetError(handlerEth);
    uint32_t dmaerror = HAL_ETH_GetDMAError(handlerEth);

    switch(error)
    {
    case HAL_ETH_ERROR_DMA:
#ifdef SOC_SERIES_STM32F4
        if((dmaerror & ETH_DMASR_RBUS) == ETH_DMASR_RBUS)
#endif
#ifdef SOC_SERIES_STM32H7
        if((dmaerror & ETH_DMACSR_RBU) == ETH_DMACSR_RBU)
#endif
        {
            eth_device_ready(&stm32_eth_device.parent);
            LOG_E("ErrorCallback DMA RX Error!");
        }

#ifdef SOC_SERIES_STM32F4
        if((dmaerror & ETH_DMASR_TBUS) == ETH_DMASR_TBUS)
#endif
#ifdef SOC_SERIES_STM32H7
        if((dmaerror & ETH_DMACSR_TBU) == ETH_DMACSR_TBU)
#endif
        {
            rt_completion_done(&stm32_eth_device.TxPkt_completion);
            LOG_E("ErrorCallback DMA TX Error!");
        }
        break;
    default:
        LOG_E("ErrorCallback 0x%x 0x%x", error, dmaerror);
        break;
    }
}

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
    struct pbuf_custom *p = LWIP_MEMPOOL_ALLOC(RX_POOL);
    if (p)
    {
        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *) p + offsetof(RxBuff_t, buff);
        p->custom_free_function = pbuf_free_custom;
        /* Initialize the struct pbuf.
         * This must be performed whenever a buffer's allocated because it may be
         * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
        pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUFFER_SIZE);
    }
    else
    {
        RxAllocStatus = RX_ALLOC_ERROR;
        *buff = NULL;
    }
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length)
{
    struct pbuf **ppStart = (struct pbuf **) pStart;
    struct pbuf **ppEnd = (struct pbuf **) pEnd;
    struct pbuf *p = NULL;

    /* Get the struct pbuf from the buff address. */
    p = (struct pbuf *) (buff - offsetof(RxBuff_t, buff));
    p->next = NULL;
    p->tot_len = 0;
    p->len = Length;

    /* Chain the buffer. */
    if (!*ppStart)
    {
        /* The first buffer of the packet. */
        *ppStart = p;
    }
    else
    {
        /* Chain the buffer to the end of the packet. */
        (*ppEnd)->next = p;
    }
    *ppEnd = p;

    /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
     * set to its own length, plus the length of all the following pbufs in the chain. */
    for (p = *ppStart; p != NULL; p = p->next)
    {
        p->tot_len += Length;
    }
}

#ifdef PHY_USING_LAN8720A
static void phy_linkchange(void *parameter)
{
    struct rt_stm32_eth *eth = parameter;
    ETH_MACConfigTypeDef MACConf;
    uint32_t PHYLinkState;
    uint32_t speed = 0, duplex = 0;

    PHYLinkState = LAN8720_GetLinkState();    //获取连接状态
    //如果检测到连接断开或者不正常就关闭网口
    if ((eth->parent.link_status == RT_TRUE) && (PHYLinkState <= LAN8720_STATUS_LINK_DOWN))
    {
        HAL_ETH_Stop_IT(&eth->heth);
        LOG_W("link down");
        eth_device_linkchange(&eth->parent, RT_FALSE);
    }
    //LWIP网卡还未打开，但是LAN8720已经协商成功
    else if ((eth->parent.link_status == RT_FALSE) && (PHYLinkState > LAN8720_STATUS_LINK_DOWN))
    {
        switch (PHYLinkState)
        {
        case LAN8720_STATUS_100MBITS_FULLDUPLEX:    //100M全双工
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            LOG_I("link up 100Mb/s FullDuplex");
            break;
        case LAN8720_STATUS_100MBITS_HALFDUPLEX:    //100M半双工
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            LOG_I("link up 100Mb/s HalfDuplex");
            break;
        case LAN8720_STATUS_10MBITS_FULLDUPLEX:     //10M全双工
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            LOG_I("link up 100Mb/s HalfDuplex");
            break;
        case LAN8720_STATUS_10MBITS_HALFDUPLEX:     //10M半双工
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            LOG_I("link up 100Mb/s HalfDuplex");
            break;
        default:
            LOG_W("link up unknown");
            break;
        }

        HAL_ETH_GetMACConfig(&eth->heth, &MACConf);
        MACConf.DuplexMode = duplex;
        MACConf.Speed = speed;
#ifdef SOC_SERIES_STM32H7
        MACConf.SourceAddrControl = ETH_SRC_ADDR_REPLACE;
#endif
        HAL_ETH_SetMACConfig(&eth->heth, &MACConf);  //设置MAC

        HAL_ETH_Start_IT(&eth->heth);
        eth_device_linkchange(&eth->parent, RT_TRUE);
    }
}
#endif

#ifdef SWITCH_USING_JL5104
static void phy_linkchange(void *parameter)
{
    struct rt_stm32_eth *eth = parameter;
    ETH_MACConfigTypeDef MACConf;
    uint32_t PHYLinkState;
    uint32_t speed = 0, duplex = 0;

    duplex = ETH_FULLDUPLEX_MODE;
    speed = ETH_SPEED_100M;

    HAL_ETH_GetMACConfig(&eth->heth, &MACConf);
    MACConf.DuplexMode = duplex;
    MACConf.Speed = speed;
#ifdef SOC_SERIES_STM32H7
    MACConf.SourceAddrControl = ETH_SRC_ADDR_REPLACE;
#endif
    HAL_ETH_SetMACConfig(&eth->heth, &MACConf);  //设置MAC

    HAL_ETH_Start_IT(&eth->heth);
    eth_device_linkchange(&eth->parent, RT_TRUE);
}

#endif

#ifdef PHY_USING_INTERRUPT_MODE
static void eth_phy_isr(void *args)
{
    rt_uint32_t status = 0;

    HAL_ETH_ReadPHYRegister(&stm32_eth_device.heth, stm32_eth_device.phy_addr, PHY_INTERRUPT_FLAG_REG, (uint32_t *)&status);
    LOG_D("phy interrupt status reg is 0x%X", status);

    phy_linkchange();
}
#endif /* PHY_USING_INTERRUPT_MODE */

static void phy_monitor_thread_entry(void *parameter)
{
    struct rt_stm32_eth *eth = parameter;

#ifdef PHY_USING_INTERRUPT_MODE
    /* configuration intterrupt pin */
    rt_pin_mode(PHY_INT_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(PHY_INT_PIN, PIN_IRQ_MODE_FALLING, eth_phy_isr, (void *)"callbackargs");
    rt_pin_irq_enable(PHY_INT_PIN, PIN_IRQ_ENABLE);

    /* enable phy interrupt */
    HAL_ETH_WritePHYRegister(&eth->heth, eth->phy_addr, PHY_INTERRUPT_MASK_REG, PHY_INT_MASK);
#if defined(PHY_INTERRUPT_CTRL_REG)
    HAL_ETH_WritePHYRegister(&eth->heth, eth->phy_addr, PHY_INTERRUPT_CTRL_REG, PHY_INTERRUPT_EN);
#endif
#else /* PHY_USING_INTERRUPT_MODE */
    eth->poll_link_timer = rt_timer_create("phylnk", phy_linkchange,
                                            eth, RT_TICK_PER_SECOND, RT_TIMER_FLAG_PERIODIC);
    if (!eth->poll_link_timer || rt_timer_start(eth->poll_link_timer) != RT_EOK)
    {
        LOG_E("Start link change detection timer failed");
    }
#endif /* PHY_USING_INTERRUPT_MODE */
}

#if defined(ETH_USING_KVDB_NET_IF)
static int rt_hw_stm32_eth_set_if(void)
{
    rt_err_t state = RT_EOK;

    /* Config the lwip device */
    char ip_key[] = "e_ip";
    char gw_key[] = "e_gw";
    char mask_key[] = "e_mask";

    char ip_addr[16] = {0};
    char gw_addr[16] = {0};
    char nm_addr[16] = {0};

    extern void netdev_set_if(char* netdev_name, char* ip_addr, char* gw_addr, char* nm_addr);
#if !LWIP_DHCP
    if (kvdb_get(ip_key, ip_addr) == 0)
    {
        rt_strcpy(ip_addr, "192.168.1.29");
    }
    if (kvdb_get(gw_key, gw_addr) == 0)
    {
        rt_strcpy(gw_addr, "192.168.1.1");
    }
    if (kvdb_get(mask_key, nm_addr) == 0)
    {
        rt_strcpy(nm_addr, "255.255.255.0");
    }

    netdev_set_if(stm32_eth_device.dev_name, ip_addr, gw_addr, nm_addr);
    LOG_I("netdev %s set if %s %s %s", stm32_eth_device.dev_name, ip_addr, gw_addr, nm_addr);
#endif
    return state;
}
#endif

/* Register the EMAC device */
static int rt_hw_stm32_eth_init(void)
{
    rt_err_t state = RT_EOK;

    /* 初始化完成量 */
    rt_completion_init(&stm32_eth_device.TxPkt_completion);

    /* 初始化 eth 互斥 */
    state = rt_mutex_init(&stm32_eth_device.eth_mux, "eth", RT_IPC_FLAG_PRIO);
    if (state != RT_EOK)
    {
        LOG_E("mutex init error %d!", state);
        return -RT_ERROR;
    }

#if defined(ETH_USING_KVDB_MAC)
    char mac_key[] = "e_mac";
    char mac_addr[18] = {0};
    if (kvdb_get(mac_key, mac_addr) != 0)
    {
        /* OUI 厂商ID */
        stm32_eth_device.mac[0] = strtoul(&mac_addr[0], NULL, 16);
        stm32_eth_device.mac[1] = strtoul(&mac_addr[3], NULL, 16);
        stm32_eth_device.mac[2] = strtoul(&mac_addr[6], NULL, 16);
        /* 设备MAC地址 */
        stm32_eth_device.mac[3] = strtoul(&mac_addr[9], NULL, 16);
        stm32_eth_device.mac[4] = strtoul(&mac_addr[12], NULL, 16);
        stm32_eth_device.mac[5] = strtoul(&mac_addr[15], NULL, 16);
    }
#endif
    if (stm32_eth_device.mac[0] == 0 && stm32_eth_device.mac[1] == 0 && stm32_eth_device.mac[2] == 0 )
    {
        /* OUI 00-80-E1 STMICROELECTRONICS.前三个字节为厂商ID */
        stm32_eth_device.mac[0] = 0xF8;
        stm32_eth_device.mac[1] = 0x09;
        stm32_eth_device.mac[2] = 0xA4;
        /* generate MAC addr from 96bit unique ID (only for test). */
        stm32_eth_device.mac[3] = *(uint8_t *)(UID_BASE + 2 + 3);
        stm32_eth_device.mac[4] = *(uint8_t *)(UID_BASE + 1 + 3);
        stm32_eth_device.mac[5] = *(uint8_t *)(UID_BASE + 0 + 3);
    }

    stm32_eth_device.parent.parent.init       = rt_stm32_eth_init;
    stm32_eth_device.parent.parent.open       = rt_stm32_eth_open;
    stm32_eth_device.parent.parent.close      = rt_stm32_eth_close;
    stm32_eth_device.parent.parent.read       = rt_stm32_eth_read;
    stm32_eth_device.parent.parent.write      = rt_stm32_eth_write;
    stm32_eth_device.parent.parent.control    = rt_stm32_eth_control;
    stm32_eth_device.parent.parent.user_data  = &stm32_eth_device;

    stm32_eth_device.parent.eth_rx     = rt_stm32_eth_rx;
    stm32_eth_device.parent.eth_tx     = rt_stm32_eth_tx;

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    state = lep_eth_if_init(&stm32_eth_device.link_layer_buf);
    if(state != RT_EOK)
    {
        LOG_E("device %s init linklayer faild: %d", stm32_eth_device.dev_name, state);
        state = -RT_ERROR;
    }
#endif

    /* register eth device */
    state = eth_device_init(&(stm32_eth_device.parent), stm32_eth_device.dev_name);
    if (RT_EOK == state)
    {
        LOG_I("device %s init success MAC %02X %02X %02X %02X %02X %02X", stm32_eth_device.dev_name
                , stm32_eth_device.mac[0], stm32_eth_device.mac[1], stm32_eth_device.mac[2]
                , stm32_eth_device.mac[3], stm32_eth_device.mac[4], stm32_eth_device.mac[5]);
    }
    else
    {
        LOG_E("device %s init faild: %d", stm32_eth_device.dev_name, state);
        state = -RT_ERROR;
    }

    /* start phy monitor */
    rt_thread_t tid;
    tid = rt_thread_create("phy",
                           phy_monitor_thread_entry,
                           &stm32_eth_device,
                           1024,
                           RT_THREAD_PRIORITY_MAX - 2,
                           10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("device %s phy thread create error!", stm32_eth_device.dev_name);
        state = -RT_ERROR;
    }

#if defined(ETH_USING_KVDB_NET_IF)
    rt_hw_stm32_eth_set_if();
#endif
    return state;
}
INIT_ENV_EXPORT(rt_hw_stm32_eth_init);
