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
#include <netif/ethernetif.h>
#include <lwipopts.h>

/*
* Emac driver uses CubeMX tool to generate emac and phy's configuration,
* the configuration files can be found in CubeMX_Config folder.
*/

/* debug option */
#define DRV_DEBUG
#define LOG_TAG             "drv.emac"
#include <drv_log.h>

#ifdef SOC_SERIES_STM32H7
#define ETH_NOCACHE_RAM_SECTION     __attribute__((section(".bss.noncacheable")))
#else
#define ETH_NOCACHE_RAM_SECTION
#endif

#define ETH_RX_BUFFER_SIZE 1536

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

#pragma location=0x30000000
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000200
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000200))) ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] ETH_NOCACHE_RAM_SECTION; /* Ethernet Rx DMA Descriptors */
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] ETH_NOCACHE_RAM_SECTION; /* Ethernet Tx DMA Descriptors */

#endif

struct rt_stm32_eth stm32_eth_device = {
    .phy_addr = LAN8720_ADDR,
};

static void phy_reset(void)
{
    rt_base_t rst_pin = rt_pin_get(ETH_RST_PIN);
    rt_pin_mode(rst_pin, PIN_MODE_OUTPUT);
    rt_pin_write(rst_pin, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(rst_pin, PIN_HIGH);
}

static void phy_linkchange(void *parameter);

/* EMAC initialization function */
static rt_err_t rt_stm32_eth_init(rt_device_t dev)
{
    struct rt_stm32_eth *eth = dev->user_data;
    phy_reset();

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

    if (LAN8720_Init() != LAN8720_STATUS_OK)
    {
        LOG_E("LAN8720 init failed!");
    }

    phy_linkchange(eth);

    LOG_D("init success 0x%x 0x%x", eth->heth.Init.TxDesc, eth->heth.Init.RxDesc);
    return RT_EOK;
}

static rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct rt_stm32_eth *eth = dev->user_data;
    LOG_D("open");

    return RT_EOK;
}

static rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
    struct rt_stm32_eth *eth = dev->user_data;
    LOG_D("close");

    return RT_EOK;
}

static rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_stm32_eth *eth = dev->user_data;
    LOG_D("read");

    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t rt_stm32_eth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_stm32_eth *eth = dev->user_data;
    LOG_D("write");

    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t rt_stm32_eth_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_stm32_eth *eth = dev->user_data;
    LOG_D("control");

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

    return errval;
}

/* receive data*/
struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{
    struct rt_stm32_eth *eth = dev->user_data;

    struct pbuf *p = NULL;

    if (RxAllocStatus == RX_ALLOC_OK)
    {
        HAL_ETH_ReadData(&eth->heth, (void **) &p);
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
    pbuf_free((struct pbuf *) buff);
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

#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    /* Invalidate data cache because Rx DMA's writing to physical memory makes it stale. */
    SCB_InvalidateDCache_by_Addr((uint32_t *) buff, Length);
#endif
}

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
        HAL_ETH_SetMACConfig(&eth->heth, &MACConf);  //设置MAC

        HAL_ETH_Start_IT(&eth->heth);
        eth_device_linkchange(&eth->parent, RT_TRUE);
    }
}

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

/* Register the EMAC device */
static int rt_hw_stm32_eth_init(void)
{
    rt_err_t state = RT_EOK;

    /* 初始化完成量 */
    rt_completion_init(&stm32_eth_device.TxPkt_completion);

    /* OUI 00-80-E1 STMICROELECTRONICS.前三个字节为厂商ID */
    stm32_eth_device.mac[0] = 0xF8;
    stm32_eth_device.mac[1] = 0x09;
    stm32_eth_device.mac[2] = 0xA4;
    /* generate MAC addr from 96bit unique ID (only for test). */
    stm32_eth_device.mac[3] = *(uint8_t *)(UID_BASE + 2 + 3);
    stm32_eth_device.mac[4] = *(uint8_t *)(UID_BASE + 1 + 3);
    stm32_eth_device.mac[5] = *(uint8_t *)(UID_BASE + 0 + 3);

    stm32_eth_device.parent.parent.init       = rt_stm32_eth_init;
    stm32_eth_device.parent.parent.open       = rt_stm32_eth_open;
    stm32_eth_device.parent.parent.close      = rt_stm32_eth_close;
    stm32_eth_device.parent.parent.read       = rt_stm32_eth_read;
    stm32_eth_device.parent.parent.write      = rt_stm32_eth_write;
    stm32_eth_device.parent.parent.control    = rt_stm32_eth_control;
    stm32_eth_device.parent.parent.user_data  = &stm32_eth_device;

    stm32_eth_device.parent.eth_rx     = rt_stm32_eth_rx;
    stm32_eth_device.parent.eth_tx     = rt_stm32_eth_tx;

    /* register eth device */
    state = eth_device_init(&(stm32_eth_device.parent), "e");
    if (RT_EOK == state)
    {
        LOG_D("device init success");
    }
    else
    {
        LOG_E("device init faild: %d", state);
        state = -RT_ERROR;
        goto __exit;
    }

    /* start phy monitor */
    rt_thread_t tid;
    tid = rt_thread_create("phy",
                           phy_monitor_thread_entry,
                           &stm32_eth_device,
                           1024,
                           RT_THREAD_PRIORITY_MAX - 2,
                           2);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        state = -RT_ERROR;
    }

__exit:

    return state;
}
INIT_COMPONENT_EXPORT(rt_hw_stm32_eth_init);
