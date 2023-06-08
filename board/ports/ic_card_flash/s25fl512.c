/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-31     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_ICCARD_FLASH

#include "fal_cfg.h"

#define DBG_TAG "s25fl512"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#if defined(RT_USING_FAL)
#include "fal.h"

#define S25FL512_SIZE (0x4000000U) /*  S25FL512存储容量 */
#define S25FL512_PSIZE (512U)      /*  S25FL512页大小 */
/*  S25FL512页地址掩码 */
#define S25FL512_PAMASK (S25FL512_PSIZE - 1U)
#define S25FL512_OTP_SIZE (1024U)
/*  OTP区地址掩码 */
#define S25FL512_OTP_AMASK (S25FL512_OTP_SIZE - 1U)

#define S25FL512_BUSY_TO (10U)     /*  检查写忙状态的最大等待时间，单位us，此超时只是为重复读设置 */
#define S25FL512_WEN_TO (60U)      /*  使能读的最大等待时间，单位us */
#define S25FL512_WRR_TO (2000000U) /*  写寄存器操作的最大等待时间，单位us */
#define S25FL512_PSL_TO (50U)      /*  挂起编程操作的最大等待时间，单位us */
#define S25FL512_ESL_TO (60U)      /*  挂起擦除操作的最大等待时间，单位us */
#define S25FL512_PP_TO (1300U)     /*  挂起擦除操作的最大等待时间，单位us */
#define S25FL512_ES_TO (2600000U)  /*  挂起擦除操作的最大等待时间，单位us */

#define S25FL512_ID_HADR ((uint8_t)0x00) /*  读ID的命令地址高 */
#define S25FL512_ID_MADR ((uint8_t)0x00) /*  读ID的命令地址中 */
#define S25FL512_ID_LADR ((uint8_t)0x00) /*  读ID的命令地址低 */

#define S25FL512_RD_ID ((uint8_t)0x90)   /*  读芯片ID命令 */
#define S25FL512_RD_SR1 ((uint8_t)0x05)  /*  读状态寄存器1 */
#define S25FL512_RD_SR2 ((uint8_t)0x07)  /*  读状态寄存器2 */
#define S25FL512_RD_CR ((uint8_t)0x35)   /*  读配置寄存器 */
#define S25FL512_RD_BRRD ((uint8_t)0x16) /*  读BANK寄存器 */
#define S25FL512_WRR ((uint8_t)0x01)     /*  写寄存器命令 */
#define S25FL512_WREN ((uint8_t)0x06)    /*  写允许命令 */
#define S25FL512_WRDI ((uint8_t)0x04)    /*  禁止写命令 */
#define S25FL512_CLSR ((uint8_t)0x30)    /*  清除状态寄存器命令 */

#define S25FL512_4READ ((uint8_t)0x13)      /*  4字节地址的读数据命令 */
#define S25FL512_4FAST_READ ((uint8_t)0x0C) /*  4字节地址的快速读数据命令 */
#define S25FL512_OTPR ((uint8_t)0x4B)       /*  读OTP数据区 */
#define S25FL512_4PP ((uint8_t)0x12)        /*  4字节地址的编程命令 */
#define S25FL512_PGSP ((uint8_t)0x85)       /*  编程挂起命令 */
#define S25FL512_PGRS ((uint8_t)0x8A)       /*  编程恢复命令 */
#define S25FL512_4SE ((uint8_t)0xDC)        /*  4字节地址的擦除命令 */
#define S25FL512_BE ((uint8_t)0x60)         /*  擦除整个芯片 */
#define S25FL512_ERSP ((uint8_t)0x75)       /*  擦除挂起命令 */
#define S25FL512_ERRS ((uint8_t)0x7A)       /*  擦除恢复命令 */

/*  状态寄存器1 */
#define S25FL512_SR1_SRWD (0x80)         /*  状态寄存器写禁止 */
#define S25FL512_SR1_PER ((uint8_t)0x40) /*  编程错误 */
#define S25FL512_SR1_EER ((uint8_t)0x20) /*  擦除错误 */
#define S25FL512_SR1_WEL ((uint8_t)0x02) /*  写允许 */
#define S25FL512_SR1_WIP ((uint8_t)0x01) /*  写操作忙状态 */

/*  状态寄存器2 */
#define S25FL512_SR2_ES ((uint8_t)0x02) /*  擦除挂起标志 */
#define S25FL512_SR2_PS ((uint8_t)0x01) /*  编程挂起标志 */

/*  配置寄存器 */
#define S25FL512_CR1_FREEZE ((uint8_t)0x01) /*  数据保护标志锁定 */

#define S25FL512_ID ((uint16_t)0x1901)         /*  S25FL512芯片ID */
#define S25FL512_INVALID_ID ((uint16_t)0xFFFF) /*  无效的ID */
#define S25FL512_DMA_TO (20)                   /*  单位:ms */
#define S25FL512_DMA_SIZE (2048U)              /*  单次DMA的传输字节数 */

static int fal_s25fl512_init(void);
static int fal_s25fl512_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_s25fl512_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_s25fl512_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
static struct rt_spi_device *s25fl512_spidev = NULL;
struct fal_flash_dev s25fl512_faldev =
{
    .name = "s25fl512",
    .addr = 0,
    .len = S25FL512_SIZE,
    .blk_size = S25FL512_PSIZE,
    .ops = {fal_s25fl512_init, fal_s25fl512_read, fal_s25fl512_write, fal_s25fl512_erase},
    .write_gran = 0,
};

uint32_t s25fl512_read_id(struct rt_spi_device *device)
{
    rt_err_t ret = RT_EOK;
    s25fl512_spidev = device;

    uint32_t id = 0;
    uint8_t command[] = {S25FL512_RD_ID, S25FL512_ID_HADR, S25FL512_ID_HADR, S25FL512_ID_LADR};
    ret = rt_spi_send_then_recv(s25fl512_spidev, command, 4, &id, 2);
    if (ret == RT_EOK)
    {
        if (id == S25FL512_ID)
        {
            return id;
        }
        else
        {
            LOG_D("id : 0x%x", id);
        }
    }
    else
    {
        LOG_E("read id error 0x%x!", id);
    }

    return 0;
}

static int fal_s25fl512_init(void)
{
    LOG_I("init succeed.");
    return RT_EOK;
}

static int fal_s25fl512_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = s25fl512_faldev.addr + offset;
    if (addr + size > s25fl512_faldev.addr + s25fl512_faldev.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

    rt_err_t ret = RT_EOK;
    /* 发送读命令 */
    uint8_t cmd[] = {0, (uint8_t)((addr) >> 16U), (uint8_t)((addr) >> 8U), (uint8_t)((addr))};

    /* 读数据 */
    ret = rt_spi_send_then_recv(s25fl512_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("read data error %d!", ret);
        return -RT_EIO;
    }

    LOG_HEX("read", 16, buf, (size > 64)?(64):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}

/*
 * @brief 等待写操作完成
 * @param timeout_u32 -超时时间
 */
static rt_err_t wait_s25fl512_write_end(uint32_t timeout)
{
    rt_err_t ret = RT_EBUSY;
    uint32_t i = 0;

    /* 发送读状态命令 */
    uint8_t cmd[] = {S25FL512_RD_SR1};
    uint8_t reg;

    for(i = 0; i < timeout; i++)
    {
        /* 读状态 */
        ret = rt_spi_send_then_recv(s25fl512_spidev, cmd, sizeof(cmd), &reg, 1);
        if (ret != RT_EOK)
        {
            LOG_E("read reg error %d!", ret);
            return -RT_EIO;
        }
        /* 分析状态设置返回值 */
        if((reg & S25FL512_SR1_PER) != (uint8_t)0)
        {
            ret = RT_ERROR;
        }
        else if((reg & S25FL512_SR1_EER) != (uint8_t)0)
        {
            ret = RT_ERROR;
        }
        else if((reg & S25FL512_SR1_WIP) != (uint8_t)0)
        {
            ret = RT_EBUSY;
        }
        else
        {
            ret = RT_EOK;
        }

        if(ret == RT_EBUSY)
        {
            rt_thread_delay(1);
        }
        else
        {
            break;
        }
    }
    if (i >= timeout)
    {
        ret = RT_ETIMEOUT;
    }
    LOG_D("wait used %d ms", i);

    return ret;
}

static int fal_s25fl512_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = s25fl512_faldev.addr + offset;
    if (addr + size > s25fl512_faldev.addr + s25fl512_faldev.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

    rt_err_t ret = RT_EOK;
    /* 发送写命令 */
    uint8_t cmd[] = {S25FL512_4PP, (uint8_t)((addr) >> 24U), (uint8_t)((addr) >> 16U), (uint8_t)((addr) >> 8U), (uint8_t)((addr))};

    /* 写数据 */
    ret = rt_spi_send_then_send(s25fl512_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("write data error %d!", ret);
        return -RT_EIO;
    }

    /* 等待写入结束 */
    ret = wait_s25fl512_write_end(2000);
    if (ret != RT_EOK)
    {
        LOG_E("write wait error %d!", ret);
        return -RT_ERROR;
    }

    LOG_HEX("write", 16, (uint8_t *)buf, (size > 64)?(64):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_s25fl512_erase(long offset, size_t size)
{
    rt_uint32_t addr = s25fl512_faldev.addr + offset;

    if ((addr + size) > s25fl512_faldev.addr + s25fl512_faldev.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    if (size < 1)
    {
        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }



    LOG_D("erase (0x%p) %d", (void*)(addr), size);

    return size;
}

#endif
#endif
