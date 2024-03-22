/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-20     zm       the first version
 */

#include "board.h"

#ifdef BSP_USING_Z8523L16

#define DBG_TAG "z8523l16"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>

/** z85230 registers */
#define     R0      (0)
#define     R1      (1)
#define     R2      (2)
#define     R3      (3)
#define     R4      (4)
#define     R5      (5)
#define     R6      (6)
#define     R7      (7)
#define     R8      (8)
#define     R9      (9)
#define     R10     (10)
#define     R11     (11)
#define     R12     (12)
#define     R13     (13)
#define     R14     (14)
#define     R15     (15)

/* Write Register 0 */

#define RES_EXT_INT     0x10    /* Reset Ext. Status Interrupts */
#define SEND_ABORT      0x18    /* HDLC Abort */
#define RES_RxINT_FC    0x20    /* Reset RxINT on First Character */
#define RES_Tx_P        0x28    /* Reset TxINT Pending */
#define ERR_RES         0x30    /* Error Reset */
#define RES_H_IUS       0x38    /* Reset highest IUS */

#define RES_Rx_CRC  0x40    /* Reset Rx CRC Checker */
#define RES_Tx_CRC  0x80    /* Reset Tx CRC Checker */
#define RES_EOM_L   0xC0    /* Reset EOM latch */

/* Write Register 1 */

#define EXT_INT_ENAB    0x1 /* Ext Int Enable */
#define TxINT_ENAB      0x2 /* Tx Int Enable */
#define PAR_SPEC        0x4 /* Parity is special condition */

#define RxINT_DISAB   0     /* Rx Int Disable */
#define RxINT_FCERR 0x8     /* Rx Int on First Character Only or Error */
#define INT_ALL_Rx  0x10    /* Int on all Rx Characters or error */
#define INT_ERR_Rx  0x18    /* Int on error only */

#define WT_RDY_RT   0x20    /* Wait/Ready on R/T */
#define WT_FN_RDYFN 0x40    /* Wait/FN/Ready FN */
#define WT_RDY_ENAB 0x80    /* Wait/Ready Enable */

/* Write Register #2 (Interrupt Vector) */

/* Write Register 3 */

#define RxENABLE    0x1     /* Rx Enable */
#define SYNC_L_INH  0x2     /* Sync Character Load Inhibit */
#define ADD_SM      0x4     /* Address Search Mode (SDLC) */
#define RxCRC_ENAB  0x8     /* Rx CRC Enable */
#define ENT_HM      0x10    /* Enter Hunt Mode */
#define AUTO_ENAB   0x20    /* Auto Enables */
#define Rx5         0x0     /* Rx 5 Bits/Character */
#define Rx7         0x40    /* Rx 7 Bits/Character */
#define Rx6         0x80    /* Rx 6 Bits/Character */
#define Rx8         0xc0    /* Rx 8 Bits/Character */

/* Write Register 4 */

#define PAR_ENA     0x1 /* Parity Enable */
#define PAR_EVEN    0x2 /* Parity Even/Odd* */

#define SYNC_ENAB   0   /* Sync Modes Enable */
#define SB1         0x04    /* 1 stop bit/char */
#define SB15        0x08    /* 1.5 stop bits/char */
#define SB2         0x0c    /* 2 stop bits/char */

#define MONSYNC     0       /* 8 Bit Sync character */
#define BISYNC      0x10    /* 16 bit sync character */
#define SDLC        0x20    /* SDLC Mode (01111110 Sync Flag) */
#define EXTSYNC     0x30    /* External Sync Mode */

#define X1CLK       0x0     /* x1 clock mode */
#define X16CLK      0x40    /* x16 clock mode */
#define X32CLK      0x80    /* x32 clock mode */
#define X64CLK      0xC0    /* x64 clock mode */

/* Write Register 5 */

#define TxCRC_ENAB  0x01    /* Tx CRC Enable */
#define RTS         0x02    /* RTS */
#define SDLC_CRC    0x04    /* SDLC/CRC-16 */
#define TxENAB      0x08    /* Tx Enable */
#define SND_BRK     0x10    /* Send Break */
#define Tx5         0x0     /* Tx 5 bits (or less)/character */
#define Tx7         0x20    /* Tx 7 bits/character */
#define Tx6         0x40    /* Tx 6 bits/character */
#define Tx8         0x60    /* Tx 8 bits/character */
#define DTR         0x80    /* DTR */

/* Write Register 6 (Sync bits 0-7/SDLC Address Field) */

/* Write Register 7 (Sync bits 8-15/SDLC 01111110) */

/* Write Register 7' (WR7 Extended) */
#define     WR7E_AutoTxFlag     (1)
#define     WR7E_AutoEomRst     (1 << 1)
#define     WR7E_AutoRtsDeAct   (1 << 2)
#define     WR7E_FrcTxHigh      (1 << 3)
#define     WR7E_DtrFastMode    (1 << 4)
#define     WR7E_CrcRecToDat    (1 << 5)
#define     WR7E_ExtReadEn      (1 << 6)

/* Write Register 8 (transmit buffer) */

/* Write Register 9 (Master interrupt control) */
#define VIS        1    /* Vector Includes Status */
#define NV         2    /* No Vector */
#define DLC        4    /* Disable Lower Chain */
#define MIE        8    /* Master Interrupt Enable */
#define STATHI  0x10    /* Status high */
#define NORESET    0    /* No reset on write to R9 */
#define CHRB    0x40    /* Reset channel B */
#define CHRA    0x80    /* Reset channel A */
#define FHWRES  0xc0    /* Force hardware reset */

/* Write Register 10 (misc control bits) */
#define BIT6       1    /* 6 bit/8bit sync */
#define LOOPMODE   2    /* SDLC Loop mode */
#define ABUNDER    4    /* Abort/flag on SDLC xmit underrun */
#define MARKIDLE   8    /* Mark/flag on idle */
#define GAOP    0x10    /* Go active on poll */
#define NRZ        0    /* NRZ mode */
#define NRZI    0x20    /* NRZI mode */
#define FM1     0x40    /* FM1 (transition = 1) */
#define FM0     0x60    /* FM0 (transition = 0) */
#define CRCPS   0x80    /* CRC Preset I/O */

/* Write Register 11 (Clock Mode control) */
#define TRxCXT     0    /* TRxC = Xtal output */
#define TRxCTC     1    /* TRxC = Transmit clock */
#define TRxCBR  0x02    /* TRxC = BR Generator Output */
#define TRxCDP     3    /* TRxC = DPLL output */
#define TRxCOI     4    /* TRxC O/I */
#define TCRTxCP    0    /* Transmit clock = RTxC pin */
#define TCTRxCP    8    /* Transmit clock = TRxC pin */
#define TCBR    0x10    /* Transmit clock = BR Generator output */
#define TCDPLL  0x18    /* Transmit clock = DPLL output */
#define RCRTxCP    0    /* Receive clock = RTxC pin */
#define RCTRxCP 0x20    /* Receive clock = TRxC pin */
#define RCBR    0x40    /* Receive clock = BR Generator output */
#define RCDPLL  0x60    /* Receive clock = DPLL output */
#define RTxCX   0x80    /* RTxC Xtal/No Xtal */

/* Write Register 12 (lower byte of baud rate generator time constant) */

/* Write Register 13 (upper byte of baud rate generator time constant) */

/* Write Register 14 (Misc control bits) */
#define BRENABL    1    /* Baud rate generator enable */
#define BRSRC      2    /* Baud rate generator source */
#define DTRREQ     4    /* DTR/Request function */
#define AUTOECHO   8    /* Auto Echo */
#define LOOPBAK 0x10    /* Local loopback */
#define SEARCH  0x20    /* Enter search mode */
#define RMC     0x40    /* Reset missing clock */
#define DISDPLL 0x60    /* Disable DPLL */
#define SSBR    0x80    /* Set DPLL source = BR generator */
#define SSRTxC  0xa0    /* Set DPLL source = RTxC */
#define SFMM    0xc0    /* Set FM mode */
#define SNRZI   0xe0    /* Set NRZI mode */

/* Write Register 15 (external/status interrupt control) */
#define PRIME   1       /* R5' etc register access (Z85C30/230 only) */
#define ZCIE    2       /* Zero count IE */
#define FIFOE   4       /* Z85230 only */
#define DCDIE   8       /* DCD IE */
#define SYNCIE  0x10    /* Sync/hunt IE */
#define CTSIE   0x20    /* CTS IE */
#define TxUIE   0x40    /* Tx Underrun/EOM IE */
#define BRKIE   0x80    /* Break/Abort IE */

/* Read Register 0 */
#define Rx_CH_AV    0x1     /* Rx Character Available */
#define ZCOUNT      0x2     /* Zero count */
#define Tx_BUF_EMP  0x4     /* Tx Buffer empty */
#define DCD         0x8     /* DCD */
#define SYNC_HUNT   0x10    /* Sync/hunt */
#define CTS         0x20    /* CTS */
#define TxEOM       0x40    /* Tx underrun */
#define BRK_ABRT    0x80    /* Break/Abort */

/* Read Register 1 */
#define END_OF_FRAME      0x80    /* end of frame */
#define ALL_SNT           0x01  /* All sent */
/* Residue Data for 8 Rx bits/char programmed */
#define RES3        0x8 /* 0/3 */
#define RES4        0x4 /* 0/4 */
#define RES5        0xc /* 0/5 */
#define RES6        0x2 /* 0/6 */
#define RES7        0xa /* 0/7 */
#define RES8        0x6 /* 0/8 */
#define RES18       0xe /* 1/8 */
#define RES28       0x0 /* 2/8 */
/* Special Rx Condition Interrupts */
#define PAR_ERR     0x10    /* Parity error */
#define Rx_OVR      0x20    /* Rx Overrun Error */
#define CRC_ERR     0x40    /* CRC/Framing Error */
#define END_FR      0x80    /* End of Frame (SDLC) */

/* Read Register 2 (channel b only) - Interrupt vector */

/* Read Register 3 (interrupt pending register) ch a only */
#define CHBEXT  0x1     /* Channel B Ext/Stat IP */
#define CHBTxIP 0x2     /* Channel B Tx IP */
#define CHBRxIP 0x4     /* Channel B Rx IP */
#define CHAEXT  0x8     /* Channel A Ext/Stat IP */
#define CHATxIP 0x10    /* Channel A Tx IP */
#define CHARxIP 0x20    /* Channel A Rx IP */

/* Read Register 8 (receive data register) */

/* Read Register 10  (misc status bits) */
#define ONLOOP      2    /* On loop */
#define LOOPSEND 0x10    /* Loop sending */
#define CLK2MIS  0x40    /* Two clocks missing */
#define CLK1MIS  0x80    /* One clock missing */

/****************************** z85230 baudrate time constant ***************************************/
#ifdef BSP_USING_Z85230_HDLC_MODE
#define Z85230_BAUD ( 32*100000UL )
#else
#define Z85230_BAUD ( 9600UL )
#endif
/*20220407---HDLC*/
#define Z85230_CLK_FREQ    ( 16000*1000UL )

#define Z85230_CLK_MODE     ( 1UL )
#define Z85230_BAUD_TC      ( Z85230_CLK_FREQ/(2 * Z85230_BAUD * Z85230_CLK_MODE) - 2)

#define BYTE_H(_x)           ( (uint8_t)(((_x) >> 8) & 0x00FF) )
#define BYTE_L(_x)           ( (uint8_t)((_x) & 0x00FF) )

#define RX_BUFFER_SIZE      2048

typedef struct
{
    const char *pin_name;
    rt_base_t pin_index;
} S_Z8523L16_GPIO;

typedef struct
{
    uint32_t ne; /** NE 片选号 */
    volatile void *hw_addr; /** 芯片数据端口地址 */
    volatile void *hw_addr_cmd; /** 芯片命令端口地址 */
} S_Z8523L16_FMC_CFG;

typedef struct
{
    struct rt_device device; /** 设备 */
    const char *name;
    S_Z8523L16_GPIO iei_pin; /** 引脚 */
    S_Z8523L16_GPIO isr_pin; /** 中断引脚 */
    S_Z8523L16_GPIO te_pin; /** 引脚 */
    S_Z8523L16_FMC_CFG fmc;

    struct rt_mutex mux; /** 接收发送互斥 */
    struct rt_semaphore sem;/** 接收信号量 */

    struct rt_ringbuffer *rx_ringbuffer;
} S_Z8523L16_DEV;

static S_Z8523L16_DEV z8523l16_dev =
{
    .name = "hdlc",
    .iei_pin.pin_name = Z8523L16_HDLC_IEI_PIN,
    .isr_pin.pin_name = Z8523L16_HDLC_INT_PIN,
    .te_pin.pin_name = Z8523L16_HDLC_TE_PIN,
    .fmc.ne = Z8523L16_NE,
    .fmc.hw_addr = (volatile void *)(Z8523L16_CMD - 2),
    .fmc.hw_addr_cmd = (volatile void *)(Z8523L16_CMD),
};

#define U16_WRITE(__ADDRESS__, __DATA__)   do{                                                             \
                                               (*(__IO uint16_t *)((uint32_t)(__ADDRESS__)) = (__DATA__)); \
                                               __DSB();                                                   \
                                             } while(0)

inline static uint8_t hdlc_readb(volatile uint8_t *addr)
{
    uint8_t val;           //防止被优化
    val = *addr;
    return val;
}

inline static void hdlc_writeb(uint8_t value_u8, volatile void *addr)
{
    U16_WRITE(addr, value_u8);
}

static uint8_t hdlc_rdreg(S_Z8523L16_FMC_CFG *p_fmc, uint8_t reg_u8)
{
    hdlc_writeb(reg_u8, p_fmc->hw_addr_cmd);
    return hdlc_readb(p_fmc->hw_addr_cmd);
}

static void hdlc_wrreg(S_Z8523L16_FMC_CFG *p_fmc, uint8_t reg_u8, uint8_t value_u8)
{
    hdlc_writeb(reg_u8, p_fmc->hw_addr_cmd);
    hdlc_writeb(value_u8, p_fmc->hw_addr_cmd);
}

static rt_err_t z8523l16_pin_init(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    dev->iei_pin.pin_index = rt_pin_get(dev->iei_pin.pin_name);
    dev->isr_pin.pin_index = rt_pin_get(dev->isr_pin.pin_name);
    dev->te_pin.pin_index = rt_pin_get(dev->te_pin.pin_name);

    rt_pin_mode(dev->iei_pin.pin_index, PIN_MODE_OUTPUT);
    rt_pin_write(dev->iei_pin.pin_index, PIN_HIGH);

    rt_pin_mode(dev->te_pin.pin_index, PIN_MODE_OUTPUT);
    rt_pin_write(dev->te_pin.pin_index, PIN_HIGH);
    return RT_EOK;
}

static void hdlc_int_pin_hdr(void *args)
{
    S_Z8523L16_DEV *dev = (S_Z8523L16_DEV *)args;

    if(RT_NULL == dev)
    {
        return;
    }

    rt_sem_release(&dev->sem);
}

static rt_err_t z8523l16_isr_pin_init(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    /* set irq pin */
    rt_pin_mode(dev->isr_pin.pin_index, PIN_MODE_INPUT_PULLUP);
    if(rt_pin_attach_irq(dev->isr_pin.pin_index, PIN_IRQ_MODE_FALLING, hdlc_int_pin_hdr, (void *)dev) != RT_EOK)
    {
        LOG_E("attach irq error!");
        return -RT_ERROR;
    }
    if(rt_pin_irq_enable(dev->isr_pin.pin_index, PIN_IRQ_ENABLE) != RT_EOK)
    {
        LOG_E("irq enable error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t z8523l16_isr_pin_deinit(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    if(rt_pin_irq_enable(dev->isr_pin.pin_index, PIN_IRQ_DISABLE) != RT_EOK)
    {
        LOG_E("irq disenable error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t z8523l16_fmc_init(S_Z8523L16_FMC_CFG *fmc)
{
    if(RT_NULL == fmc)
    {
        return -RT_EEMPTY;
    }

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx) || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx)  || defined (STM32H747xx) || defined (STM32H747xG) || defined (STM32H757xx) || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx)  || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ)  || defined (STM32H725xx) || defined (STM32H723xx)

    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /**
     * Perform the SRAM memory initialization sequence
    */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    hsram.State = HAL_SRAM_STATE_RESET;
    /* hsram.Init */
    hsram.Init.NSBank = fmc->ne;
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
    hsram.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;

    /* 地址建立时间6个hclk  hclk = 1/240M=4.166666ns */
    /* Timing */
    Timing.AddressSetupTime = BSP_USING_Z8523L16_ADDRESSSETUPTIME;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_USING_Z8523L16_DATASETUPTIME;
    Timing.BusTurnAroundDuration = BSP_USING_Z8523L16_BUSTURNAROUNDDURATION;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler();
    }
#endif
    return RT_EOK;
}

static rt_err_t z85230_init(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    /** reset the chip */
    hdlc_wrreg(&dev->fmc, R9, FHWRES|NV|DLC);
    rt_thread_mdelay(100);
    /** chip valid check */
    hdlc_wrreg(&dev->fmc, R12, 0xAA);
    if(0xAA != hdlc_rdreg(&dev->fmc, R12))
    {
        LOG_E("check1 error!");
        return -RT_ERROR;
    }

    hdlc_wrreg(&dev->fmc, R12, 0x55);
    if(0x55 != hdlc_rdreg(&dev->fmc, R12))
    {
        LOG_E("check2 error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t z85230_hdlc_mode(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    /** x 1 clock, SDLC mode, parity disable */
    hdlc_wrreg(&dev->fmc, R4, X1CLK|SDLC);
    /** address for this device is 60*/
    hdlc_wrreg(&dev->fmc, R6, 0x02);
    /** SDLC flag pattern 01111110 */
    hdlc_wrreg(&dev->fmc, R7, 0x7e);//////////////////////////////////////////////
    /** Sync/hunt IE , WR7' SDLC Feature Enable */
    hdlc_wrreg(&dev->fmc, R15, PRIME|SYNCIE);
    /** Extended mode AUTO TX and EOM*/
    //hdlc_wrreg(chip, R7, WR7E_AutoTxFlag|WR7E_AutoEomRst|WR7E_ExtReadEn|WR7E_AutoRtsDeAct);   //自动发送
    //hdlc_wrreg(chip, R7, WR7E_AutoTxFlag|WR7E_AutoEomRst|WR7E_ExtReadEn);                       //手动发送

    /* 20230719 add by chenggt */
    hdlc_wrreg(&dev->fmc, R7, WR7E_AutoTxFlag|WR7E_AutoEomRst|WR7E_ExtReadEn|WR7E_AutoRtsDeAct);
    /** NRZ ,CRC Preset I/O*/
    hdlc_wrreg(&dev->fmc, R10, NRZI|CRCPS);///////////////   //////////////////////////   //////////////
    /*Tx & Rx = DPLL out, TRxC = DPLL out TRxC input */
    hdlc_wrreg(&dev->fmc, R11, RCDPLL|TCDPLL|TRxCOI|TRxCDP);
    /** baud lower */
    hdlc_wrreg(&dev->fmc, R12, BYTE_L(Z85230_BAUD_TC));
    /** baud upper */
    hdlc_wrreg(&dev->fmc, R13, BYTE_H(Z85230_BAUD_TC));
    /** Set source - BRG */
    hdlc_wrreg(&dev->fmc, R14, SSBR);
    /** Set NRZI Mode*/
    hdlc_wrreg(&dev->fmc, R14, SNRZI);///////////////////////////////////////////////////////////////
    /**  Enter Search Mode */
    hdlc_wrreg(&dev->fmc, R14, SEARCH);
    /**  BRG enable */
    hdlc_wrreg(&dev->fmc, R14, BRSRC|BRENABL );////////////////////////////////////////
    /** Tx enable  ,485 to recieve (default) */
    hdlc_wrreg(&dev->fmc, R5, Tx8|TxCRC_ENAB|TxENAB);
    /** Rx enable */
    hdlc_wrreg(&dev->fmc, R3, Rx8|RxCRC_ENAB|RxENABLE|ENT_HM);

    hdlc_wrreg(&dev->fmc, R1, INT_ALL_Rx);

    /** Master Interrupt Enable */
    hdlc_wrreg(&dev->fmc, R9, MIE);
    return RT_EOK;
}

static uint8_t z85230_outtime( uint32_t time, uint32_t ms )
{
    if( ( rt_tick_get() - time ) > ms )
    {
        return 1u;
    }
    else
    {
        return 0u;
    }
}

static void z85230_tx_byte(S_Z8523L16_DEV *dev, uint8_t chr)
{
    if(RT_NULL == dev)
    {
        return;
    }

    uint32_t z85230_time = 0u;

    z85230_time = rt_tick_get();
    while( (hdlc_rdreg(&dev->fmc, R0) & Tx_BUF_EMP) == 0 )
    {
        if (z85230_outtime(z85230_time, 1)== 1u )
        {
            hdlc_wrreg(&dev->fmc, R0, ERR_RES);
            hdlc_wrreg(&dev->fmc, R0, RES_EOM_L);
            break;
        }
    }
    hdlc_wrreg(&dev->fmc, R8, chr);
}

static rt_err_t z85230_serial_mode(S_Z8523L16_DEV *dev)
{
    if(RT_NULL == dev)
    {
        return -RT_EEMPTY;
    }

    /** x 16 clock, ays mode, parity disable */
    hdlc_wrreg(&dev->fmc, R4, X16CLK|SB1);
    /** NRZ */
    hdlc_wrreg(&dev->fmc, R10, NRZ|CRCPS|ABUNDER);
    /** Tx & Rx = BRG out, TRxC = BRG out */
    hdlc_wrreg(&dev->fmc, R11, TRxCBR|TCBR|RCBR);
    /** baud lower 32 9600  19 19200*/
    hdlc_wrreg(&dev->fmc, R12, 0x28);
    /** baud upper */
    hdlc_wrreg(&dev->fmc, R13, 0x00);
    /**  BRG enable */
    hdlc_wrreg(&dev->fmc, R14, BRENABL|BRSRC|DISDPLL);
    /** Tx disable  */
    hdlc_wrreg(&dev->fmc, R5, Tx8|DTR|TxENAB|TxCRC_ENAB);
    /** Rx enable */
    hdlc_wrreg(&dev->fmc, R3, RxENABLE|Rx8);
    /** Rx Int on First Character or Special Condition  */
    hdlc_wrreg(&dev->fmc, R1, INT_ALL_Rx);
    /** Master Interrupt Enable */
    hdlc_wrreg(&dev->fmc, R9, MIE);

    return RT_EOK;
}

static void z8523l16_rx_thread_entry(void *param)
{
    S_Z8523L16_DEV *p_dev = param;

    if(RT_NULL == p_dev)
    {
        return;
    }

    rt_size_t rx_len = 0;
    uint8_t rr1 = 0, rr3 = 0, rr0 = 0;

    while(1)
    {
        rt_sem_take(&p_dev->sem, RT_WAITING_FOREVER);

        rt_mutex_take(&p_dev->mux, RT_WAITING_FOREVER);

        rr3 = hdlc_rdreg(&p_dev->fmc, R3);
        rr0 = hdlc_rdreg(&p_dev->fmc, R0);
        if(rr3 & CHARxIP)
        {
            /* 当接收数据FIFO中至少有一个字符可用时，该位被设置为1。
                          *  当接收数据FIFO是完全空的。通道或硬件重置清空接收数据FIFO。
                          * 通道有数据，则一直读。
             */
            while(rr0 & Rx_CH_AV)
            {
                /* put buffer to ringbuffer */
                rt_ringbuffer_putchar_force(p_dev->rx_ringbuffer, hdlc_rdreg(&p_dev->fmc, R8));
                rr0 = hdlc_rdreg(&p_dev->fmc, R0);
            }
        }

        rr1 = hdlc_rdreg(&p_dev->fmc, R1);
        if(rr1 & END_FR)
        {
            if((rr1 & CRC_ERR) != 0)
            {
                LOG_E("rx crc err!");
            }
            else
            {
                if (rt_ringbuffer_space_len(p_dev->rx_ringbuffer) == 0)
                {
                    rt_ringbuffer_reset(p_dev->rx_ringbuffer);
                    LOG_W("rx buffer fail!");
                }
                else
                {
                    rx_len = rt_ringbuffer_data_len(p_dev->rx_ringbuffer);
                    if(p_dev->device.rx_indicate != RT_NULL)
                    {
                        p_dev->device.rx_indicate(&p_dev->device, rx_len);
                    }
                }
            }
        }

        if( rr3 & CHATxIP )
        {
            hdlc_wrreg(&p_dev->fmc, R0, RES_Tx_P);
        }

        rt_mutex_release(&p_dev->mux);
    }
}

static rt_err_t hdlc_init(rt_device_t dev)
{
    S_Z8523L16_DEV *p_dev;
    RT_ASSERT(dev != RT_NULL);
    p_dev = rt_container_of(dev, S_Z8523L16_DEV, device);

    if(rt_sem_init(&p_dev->sem, "hdlc", 0, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("init sem error!");
        return -RT_ERROR;
    }

    if(rt_mutex_init(&p_dev->mux, "hdlc", RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("init mutex error!");
        return -RT_ERROR;
    }

    /* init ringbuffer */
    p_dev->rx_ringbuffer = rt_ringbuffer_create(RX_BUFFER_SIZE);
    if (p_dev->rx_ringbuffer == NULL)
    {
        LOG_E("no memory");
        return -RT_ENOMEM;
    }

    z8523l16_pin_init(p_dev);
    z8523l16_fmc_init(&p_dev->fmc);

    rt_thread_t tid = rt_thread_create("hdlc", z8523l16_rx_thread_entry, (void *)p_dev, 1024, 14, 10);
    if (tid == RT_NULL)
    {
        LOG_E("rx thread create error!");
        return -RT_ERROR;
    }
    else
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

static rt_err_t hdlc_open(rt_device_t dev, rt_uint16_t oflag)
{
    S_Z8523L16_DEV *p_dev;
    RT_ASSERT(dev != RT_NULL);
    p_dev = rt_container_of(dev, S_Z8523L16_DEV, device);

    if(z85230_init(p_dev) != RT_EOK)
    {
        LOG_E("init error!");
        return -RT_ERROR;
    }
#ifdef BSP_USING_Z85230_HDLC_MODE
    z85230_hdlc_mode(p_dev);
#else
    z85230_serial_mode(p_dev);
#endif

    z8523l16_isr_pin_init(p_dev);

    rt_ringbuffer_reset(p_dev->rx_ringbuffer);

    return RT_EOK;
}

static rt_err_t hdlc_close(rt_device_t dev)
{
    S_Z8523L16_DEV *p_dev;
    RT_ASSERT(dev != RT_NULL);
    p_dev = rt_container_of(dev, S_Z8523L16_DEV, device);

    z8523l16_isr_pin_deinit(p_dev);
    return RT_EOK;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t hdlc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
#else
static rt_size_t hdlc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
#endif
{
    S_Z8523L16_DEV *p_dev;
    RT_ASSERT(dev != RT_NULL);
    p_dev = rt_container_of(dev, S_Z8523L16_DEV, device);

    rt_mutex_take(&p_dev->mux, RT_WAITING_FOREVER);

    rt_size_t result = rt_ringbuffer_get(p_dev->rx_ringbuffer, buffer, size);

    rt_mutex_release(&p_dev->mux);

    return result;
}

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t hdlc_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
#else
static rt_size_t hdlc_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
#endif
{
    S_Z8523L16_DEV *p_dev;
    RT_ASSERT(dev != RT_NULL);
    p_dev = rt_container_of(dev, S_Z8523L16_DEV, device);

    uint32_t i = 0;

    if(RT_NULL == p_dev || RT_NULL == buffer)
    {
        return 0;
    }

    rt_mutex_take(&p_dev->mux, RT_WAITING_FOREVER);

    hdlc_wrreg(&p_dev->fmc, R5, Tx8|RTS|TxCRC_ENAB|TxENAB);

#ifdef BSP_USING_Z85230_HDLC_MODE
    rt_thread_delay(1);
    hdlc_wrreg(&p_dev->fmc, R0, RES_Tx_CRC);
#endif

    for(i = 0; i < size; i++)
    {
        z85230_tx_byte(p_dev, ((uint8_t *)buffer)[i]);
    }
    hdlc_wrreg(&p_dev->fmc, R5, Tx8|TxCRC_ENAB|TxENAB);  //AUTO
    rt_mutex_release(&p_dev->mux);
    return size;
}

static rt_err_t hdlc_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
    static struct rt_device_ops _ops = {
        hdlc_init,
        hdlc_open,
        hdlc_close,
        hdlc_read,
        hdlc_write,
        hdlc_control
    };
#endif /* RT_USING_DEVICE_OPS */

int rt_hw_z8523l16_init(void)
{
    S_Z8523L16_DEV *p_dev = &z8523l16_dev;

    /* register char device */
    p_dev->device.type     = RT_Device_Class_Char;
#ifdef RT_USING_DEVICE_OPS
    p_dev->device.ops = &_ops;
#else
    p_dev->device.init     = hdlc_init;
    p_dev->device.open     = hdlc_open;
    p_dev->device.close    = hdlc_close;
    p_dev->device.read     = hdlc_read;
    p_dev->device.write    = hdlc_write;
    p_dev->device.control  = hdlc_control;
#endif /* RT_USING_DEVICE_OPS */

    /* no private */
    p_dev->device.user_data = RT_NULL;

    /* register telnet device */
    rt_device_register(&p_dev->device, p_dev->name, RT_DEVICE_FLAG_RDWR);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_z8523l16_init);

#endif /* BSP_USING_Z8523L16 */
