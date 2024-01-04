#include "board.h"

#ifdef BSP_USING_MVB
#include "drv_mvb.h"

#define DBG_TAG "drv.mvb"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>
#include <drv_config.h>

#include "mue_pd_full.h"

typedef struct
{
    uint32_t ne;                /** NE 片选号 */
} MVB_FMC_CFG;

typedef struct
{
    struct rt_device dev;       /** 设备 */
    const char *dev_name;       /** 设备名 */
    MVB_FMC_CFG fmc_cfg;
    rt_base_t rst_pin;          /** 复位引脚 */
    rt_base_t isr_pin;          /** 中断引脚 */
    rt_base_t rdy_pin;          /** 就绪引脚 */
    MVB_PORT_INFO port_info;
} MVB_DEV;

static MVB_DEV mvb_dev =
{
    .dev_name = "mvb",
    .fmc_cfg.ne = MVB_NE,
};

static void mvb_gpio_init(MVB_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return;
    }

    dev->rst_pin = rt_pin_get(MVB_RST_PIN);
    dev->rdy_pin = rt_pin_get(MVB_RDY_PIN);
    dev->isr_pin = rt_pin_get(MVB_ISR_PIN);  /** 海天MVB 中断管脚不起作用，所以不能用中断接收 */

    rt_pin_mode(dev->rst_pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->rst_pin, PIN_HIGH);

    rt_pin_mode(dev->rdy_pin, PIN_MODE_INPUT);
}

/* FMC initialization function */
static void mvb_fmc_init(uint32_t NSBank)
{
    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef Timing = {0};

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx) || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx)  || defined (STM32H747xx) || defined (STM32H747xG) || defined (STM32H757xx) || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx)  || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ)  || defined (STM32H725xx) || defined (STM32H723xx)

    /** Perform the SRAM memory initialization sequence
    */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    hsram.State = HAL_SRAM_STATE_RESET;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    /* 地址建立时间6个hclk 1/240M=4.166666ns */
    Timing.AddressSetupTime = BSP_USING_MVB_ADDRESSSETUPTIME;
    Timing.AddressHoldTime = 0;
    Timing.DataSetupTime = BSP_USING_MVB_DATASETUPTIME;
    Timing.BusTurnAroundDuration = BSP_USING_MVB_BUSTURNAROUNDDURATION;
    Timing.CLKDivision = 0;
    Timing.DataLatency = 0;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */
    if (HAL_SRAM_Init(&hsram, &Timing, &Timing) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

static rt_size_t mvb_read (rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    if(MVB_Rx_data(pos, (unsigned char *)buffer) != MUE_RESULT_OK)
    {
        return 0;
    }
    return size;
}

static rt_size_t mvb_write (rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    if(MVB_tx_data(pos, (unsigned char *)buffer, size) != MUE_RESULT_OK)
    {
        return 0;
    }
    return size;
}

static rt_err_t  mvb_control(rt_device_t dev, int cmd, void *args)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }
    if(RT_DEVICE_CTRL_BLK_GETGEOME == cmd)
    {
        if(RT_NULL == args)
        {
            return -RT_EEMPTY;
        }

        MVB_DEV *mvb_dev = (MVB_DEV *)dev;
        MVB_PORT_INFO *port_info = (MVB_PORT_INFO *)args;

        rt_memcpy(port_info, &mvb_dev->port_info, sizeof(MVB_PORT_INFO));
    }
    return RT_EOK;
}

static int rt_hw_mvb_init(void)
{
    MVB_DEV *p_dev = &mvb_dev;

    mvb_gpio_init(p_dev);

    mvb_fmc_init(p_dev->fmc_cfg.ne);

    if(mue_app_main(&p_dev->port_info) != MUE_RESULT_OK)
    {
        LOG_E("mvb cfg error");
        return -RT_ERROR;
    }

    p_dev->dev.user_data = (void *)p_dev;
    p_dev->dev.type = RT_Device_Class_Char;

    p_dev->dev.init = RT_NULL;
    p_dev->dev.open = RT_NULL;
    p_dev->dev.close = RT_NULL;
    p_dev->dev.read = mvb_read;
    p_dev->dev.write = mvb_write;
    p_dev->dev.control = mvb_control;

    /* register mvb device */
    if (rt_device_register(&p_dev->dev, p_dev->dev_name, RT_DEVICE_FLAG_RDWR) == RT_EOK)
    {
        LOG_I("register '%s' success", p_dev->dev_name);
    }
    else
    {
        LOG_E("register '%s' failed", p_dev->dev_name);
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_mvb_init);

#endif /* BSP_USING_MVB */
