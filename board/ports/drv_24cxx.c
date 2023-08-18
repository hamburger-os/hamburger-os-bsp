/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-29     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_EEPROM_24Cxx
#include "fal_cfg.h"

#define DBG_TAG "24cxx"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#if defined(RT_USING_FAL) || defined(PKG_USING_FAL)
#include "fal.h"

#define AT24C01     128
#define AT24C02     256
#define AT24C04     512
#define AT24C08     1024
#define AT24C16     2048
#define AT24C32     4096
#define AT24C64     8192
#define AT24C128    16384
#define AT24C256    32768

static struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */

static int fal_eeprom_init(void);
static int fal_eeprom_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_eeprom_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_eeprom_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev hg24cxx_eeprom =
{
    .name = EEPROM_24Cxx_DEV_NAME,
    .addr = EEPROM_24Cxx_START_ADRESS,
    .len = EEPROM_24Cxx_SIZE_GRANULARITY_TOTAL,
    .blk_size = EEPROM_24Cxx_BLK_SIZE,
    .ops = {fal_eeprom_init, fal_eeprom_read, fal_eeprom_write, fal_eeprom_erase},
    .write_gran = 0,
};

//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址
//返回值  :读到的数据
static int8_t AT24CXX_ReadBytes(uint16_t ReadAddr, uint8_t *Buffer, uint16_t Len)
{
    uint8_t buf[2] = {0};
    if (hg24cxx_eeprom.len > AT24C16)
    {
        buf[0] = ReadAddr>>8; // 高地址
        buf[1] = ReadAddr;    // 低地址

        /* 调用I2C设备接口传输数据 */
        if (rt_i2c_master_send(i2c_bus, EEPROM_24Cxx_I2C_ADD, 0, buf, 2) != 2)
        {
            LOG_E("i2c_master_send read error!");
            return -1;
        }
    }
    else
    {
        buf[0] = ReadAddr;    // 低地址
        //发送器件地址,写数据
        uint8_t addr = EEPROM_24Cxx_I2C_ADD + ReadAddr/256;
        /* 调用I2C设备接口传输数据 */
        if (rt_i2c_master_send(i2c_bus, addr, 0, buf, 1) != 1)
        {
            LOG_E("i2c_master_send read error!");
            return -1;
        }
    }

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_recv(i2c_bus, EEPROM_24Cxx_I2C_ADD, 0, Buffer, Len) != Len)
    {
        LOG_E("i2c_master_recv read error!");
        return -2;
    }

    return 0;
}

//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址
//DataToWrite:要写入的数据
static uint8_t WriteBuffer[EEPROM_24Cxx_BLK_SIZE + 2];
static int8_t AT24CXX_WriteBytes(uint16_t WriteAddr, uint8_t *Buffer, uint16_t Len)
{
    uint32_t index = Len;
    uint32_t LenWr = 0;
    uint32_t nbr = (Len % hg24cxx_eeprom.blk_size == 0)?(Len/hg24cxx_eeprom.blk_size):(Len/hg24cxx_eeprom.blk_size + 1);

    uint8_t buf[2] = {0};
    if (hg24cxx_eeprom.len > AT24C16)
    {
        for (uint32_t i = 0; i < nbr; i++)
        {
            if (index > hg24cxx_eeprom.blk_size)
            {
                LenWr = hg24cxx_eeprom.blk_size;
            }
            else
            {
                LenWr = index;
            }
            buf[0] = WriteAddr>>8; // 高地址
            buf[1] = WriteAddr;    // 低地址
            rt_memcpy(WriteBuffer, buf, 2);
            rt_memcpy(&WriteBuffer[2], Buffer, LenWr);

            /* 调用I2C设备接口传输数据 */
            if (rt_i2c_master_send(i2c_bus, EEPROM_24Cxx_I2C_ADD, 0, WriteBuffer, LenWr + 2) != LenWr + 2)
            {
                LOG_E("i2c_master_send write error!");
                return -2;
            }

            WriteAddr += hg24cxx_eeprom.blk_size;
            Buffer += hg24cxx_eeprom.blk_size;
            index -= hg24cxx_eeprom.blk_size;

            rt_thread_delay(10);
        }
    }
    else
    {
        for (uint32_t i = 0; i < nbr; i++)
        {
            if (index > hg24cxx_eeprom.blk_size)
            {
                LenWr = hg24cxx_eeprom.blk_size;
            }
            else
            {
                LenWr = index;
            }
            buf[0] = WriteAddr;    // 低地址
            rt_memcpy(WriteBuffer, buf, 1);
            rt_memcpy(&WriteBuffer[1], Buffer, LenWr);

            //发送器件地址,写数据
            uint8_t addr = EEPROM_24Cxx_I2C_ADD + WriteAddr/256;
            /* 调用I2C设备接口传输数据 */
            if (rt_i2c_master_send(i2c_bus, addr, 0, WriteBuffer, LenWr + 1) != LenWr + 1)
            {
                LOG_E("i2c_master_send write error!");
                return -2;
            }

            WriteAddr += hg24cxx_eeprom.blk_size;
            Buffer += hg24cxx_eeprom.blk_size;
            index -= hg24cxx_eeprom.blk_size;

            rt_thread_delay(10);
        }
    }

    return 0;
}

//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
static uint8_t AT24CXX_Check(void)
{
    uint8_t temp = 0;
    uint8_t addr = hg24cxx_eeprom.len - 1;
    uint8_t cmd = 0x55;//标志字

    AT24CXX_ReadBytes(addr, &temp, 1);//避免每次开机都写AT24CXX
    if(temp == cmd)
    {
        LOG_I("check frist succeed %d byte", hg24cxx_eeprom.len);
        return 0;
    }
    else//排除第一次初始化的情况
    {
        AT24CXX_WriteBytes(addr, &cmd, 1);
        AT24CXX_ReadBytes(addr, &temp, 1);
        if(temp == cmd)
        {
            LOG_I("check succeed %d byte", hg24cxx_eeprom.len);
            return 0;
        }
    }
    LOG_E("check failed 0x%x!", temp);
    return 1;
}

static int fal_eeprom_init(void)
{
    rt_err_t ret = RT_EOK;

    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(EEPROM_24Cxx_I2C_DEV);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }

    ret = AT24CXX_Check();
    return ret;
}
static int fal_eeprom_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = hg24cxx_eeprom.addr + offset;
    if (addr + size > hg24cxx_eeprom.addr + hg24cxx_eeprom.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    AT24CXX_ReadBytes(addr, buf, size);

    LOG_HEX("rd", 16, buf, size);
    LOG_D("read (0x%p) %d", (void*)(addr), size);
    return size;
}
static int fal_eeprom_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = hg24cxx_eeprom.addr + offset;
    if (addr + size > hg24cxx_eeprom.addr + hg24cxx_eeprom.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    AT24CXX_WriteBytes(addr, (uint8_t *)buf, size);

    LOG_HEX("wr", 16, (rt_uint8_t *)buf, size);
    LOG_D("write (0x%p) %d", (void*)(addr), size);
    return size;
}
static int fal_eeprom_erase(long offset, size_t size)
{
    rt_uint32_t addr = hg24cxx_eeprom.addr + offset;

    if ((addr + size) > hg24cxx_eeprom.addr + hg24cxx_eeprom.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    return size;
}

#ifdef EEPROM_24Cxx_ENABLE_TEST
static int eeprom_test(void)
{
    rt_err_t ret = RT_EOK;
    uint32_t testsize = hg24cxx_eeprom.blk_size;
    LOG_D("eeprom read write test start...");

    char *tbuf = rt_malloc(testsize);
    char *rbuf = rt_malloc(testsize);
    if (tbuf == NULL || rbuf == NULL)
    {
        rt_free(tbuf);
        rt_free(rbuf);
        LOG_W("Not enough memory to request a block.");

        testsize = 1024;
        tbuf = rt_malloc(testsize);
        rbuf = rt_malloc(testsize);
        if (tbuf == NULL || rbuf == NULL)
        {
            rt_free(tbuf);
            rt_free(rbuf);
            LOG_E("Not enough memory to complete the test.");
            return -RT_ERROR;
        }
    }

    for(uint8_t i = 0; i<testsize; i++)
    {
        tbuf[i] = i;
    }

    rt_tick_t tick1, tick2, tick3, tick4;
    uint32_t addr = 0;
    tick1 = rt_tick_get_millisecond();
    fal_eeprom_erase(addr, testsize);
    tick2 = rt_tick_get_millisecond();
    fal_eeprom_write(addr, (uint8_t *)tbuf, testsize);
    tick3 = rt_tick_get_millisecond();
    fal_eeprom_read (addr, (uint8_t *)rbuf, testsize);
    tick4 = rt_tick_get_millisecond();

    LOG_D("erase %08p use %u ms", addr, tick2 - tick1);
    LOG_D("write %08p use %u ms: %x %x", addr, tick3 - tick2, tbuf[0], tbuf[testsize-1]);
    LOG_D("read  %08p use %u ms: %x %x", addr, tick4 - tick3, tbuf[0], tbuf[testsize-1]);

    if (rt_memcmp(tbuf, rbuf, testsize) != 0)
    {
        ret = -RT_ERROR;
    }
    rt_memset(rbuf, 0, testsize);

    addr = hg24cxx_eeprom.len - hg24cxx_eeprom.blk_size;
    tick1 = rt_tick_get_millisecond();
    fal_eeprom_erase(addr, testsize);
    tick2 = rt_tick_get_millisecond();
    fal_eeprom_write(addr, (uint8_t *)tbuf, testsize);
    tick3 = rt_tick_get_millisecond();
    fal_eeprom_read (addr, (uint8_t *)rbuf, testsize);
    tick4 = rt_tick_get_millisecond();

    LOG_D("erase %08p use %u ms", addr, tick2 - tick1);
    LOG_D("write %08p use %u ms: %x %x", addr, tick3 - tick2, tbuf[0], tbuf[testsize-1]);
    LOG_D("read  %08p use %u ms: %x %x", addr, tick4 - tick3, tbuf[0], tbuf[testsize-1]);

    if (rt_memcmp(tbuf, rbuf, testsize) != 0)
    {
        ret = -RT_ERROR;
    }

    if (ret == RT_EOK)
    {
        LOG_I("eeprom read write test succsess.");
    }
    else
    {
        LOG_E("eeprom read write test failed!");
    }

    rt_free(tbuf);
    rt_free(rbuf);

    return ret;
}
MSH_CMD_EXPORT(eeprom_test, eeprom 24cxx test);
#endif

#endif
#endif
