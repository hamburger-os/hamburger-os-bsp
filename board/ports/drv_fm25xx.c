/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-31     lvhan       the first version
 */
/** 简    介：用于实现FM25系列串行FRAM的操作，包括4K、16K、64K、128K、256K、           **/
/**           512K、1M等多种容量，不同容量其采用的设备地址位、寄存器地址位存         **/
/**           在差异，具体配置如下：                                                                                    **/
/**           +----------+---------------+-----------------+--------+        **/
/**           |   型号       |    容量结构      |    寄存器地址        |每页字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25L04B | 512x8(4K)     | 9位(1个寄存器)   |  8字节   |       **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25040B | 512x8(4K)     | 9位(1个寄存器)   |  8字节   |       **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25C160B| 2048x8(16K)   | 11位(1个寄存器)  | 32字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25L16B | 2048x8(16K)   | 11位(1个寄存器)  | 32字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25CL64B| 8192x8(64K)   | 13位(2个寄存器)  | 32字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25640B | 8192x8(64K)   | 13位(2个寄存器)  | 32字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25V01A | 16384x8(128K) | 14位(2个寄存器)  | 64字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25W256 | 32768x8(256K) | 15位(2个寄存器)  | 64字节 |        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25V02A | 32768x8(256K) | 15位(2个寄存器)  | 64字节 |         **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25V05  | 65536x8(512K) | 16位(2个寄存器)  | 128字节|        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           | FM25V10  | 131072x8(1M)  | 17位(3个寄存器)  | 256字节|        **/
/**           +----------+---------------+-----------------+--------+        **/
/**           FM25系列FRAM采用SPI接口，SPI接口模式支持Mode0和Mode3：                       **/
/**                   SPI Mode 0 (CPOL = 0, CPHA = 0)                        **/
/**                   SPI Mode 3 (CPOL = 1, CPHA = 1)                        **/
/**           状态寄存器不仅反映状态还用来设置写保护，转台寄存器各位：                    **/
/**           +-------+------+------+------+------+------+------+------+     **/
/**           |  Bit7 | Bit6 | Bit5 | Bit4 | Bit3 | Bit2 | Bit1 | Bit0 |     **/
/**           +-------+------+------+------+------+------+------+------+     **/
/**           |WPEN(0)| X(0) | X(0) | X(0) |BP1(0)|BP0(0)|WEL(0)| X(0) |     **/
/**           +-------+------+------+------+------+------+------+------+     **/
/**           其中WEL由写使能所存器决定，WPEN、BP1、BP0可以设定。                             **/
/**           BP1和BP0决定保护区域与非保护区域的大小：                                                   **/
/**             +-----+-----+------------------+                             **/
/**             | BP1 | BP0 |  被保护区域范围      |                             **/
/**             +-----+-----+------------------+                             **/
/**             |  0  |  0  | None             |                             **/
/**             +-----+-----+------------------+                             **/
/**             |  0  |  1  | 1800h-1FFFh(1/4) |                             **/
/**             +-----+-----+------------------+                             **/
/**             |  1  |  0  | 1000h-1FFFh(1/2) |                             **/
/**             +-----+-----+------------------+                             **/
/**             |  1  |  1  | 0000h-1FFFh(全部) |                            **/
/**             +-----+-----+------------------+                             **/
/**           FM25Cxx系列存储器采用多重写保护，关系如下：                                             **/
/**           +-----+------+----+----------+----------+----------+           **/
/**           | WEL | WPEN | WP | 被保护区域|不保护区域 |状态寄存器 |           **/
/**           +-----+------+----+----------+----------+----------+           **/
/**           |  0  |  X   | X  |  被保护     |  被保护     |  被保护    |           **/
/**           +-----+------+----+----------+----------+----------+           **/
/**           |  1  |  0   | X  |  被保护     |  不保护     |  不保护    |           **/
/**           +-----+------+----+----------+----------+----------+           **/
/**           |  1  |  1   | 0  |  被保护     |  不保护     |  被保护    |           **/
/**           +-----+------+----+----------+----------+----------+           **/
/**           |  1  |  1   | 1  |  被保护     |  不保护     |  不保护    |           **/
/**           +-----+------+----+----------+----------+----------+           **/
/**--------------------------------------------------------------------------**/
#include "board.h"

#ifdef BSP_USING_FM25xx
#include "fal_cfg.h"

#define DBG_TAG "fm25xx"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

#if defined(RT_USING_FAL)
#include "fal.h"

#define FM25xx_ManufacturerID           ((uint64_t)0xC27F7F7F7F7F7F)
#define FM25xx_SIZE_128K                ((uint8_t)0x01)         /*  128kbit  */
#define FM25xx_SIZE_256K                ((uint8_t)0x02)         /*  256kbit  */
#define FM25xx_SIZE_512K                ((uint8_t)0x03)         /*  512kbit  */
#define FM25xx_SIZE_1M                  ((uint8_t)0x04)         /*  1megabit */

#define FM25xx_CMD_WREN ((uint8_t)0x06U)  /* 写使能命令               */
#define FM25xx_CMD_WRDI ((uint8_t)0x04U)  /* 读使能命令               */
#define FM25xx_CMD_RDSR ((uint8_t)0x05U)  /* 读状态寄存器命令         */
#define FM25xx_CMD_WRSR ((uint8_t)0x01U)  /* 写状态寄存器命令         */
#define FM25xx_CMD_READ ((uint8_t)0x03U)  /* 读数据寄存器命令         */
#define FM25xx_CMD_WRITE ((uint8_t)0x02U) /* 写数据寄存器命令         */

#define FM25xx_CMD_SLEEP ((uint8_t)0xB9U) /* 进入sleep模式命令        */
#define FM25xx_CMD_RDID ((uint8_t)0x9FU)  /* 读设备ID命令             */
#define FM25xx_CMD_SNR ((uint8_t)0xC3U)   /* 读S/N命令(FM25VN10 Only) */

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif
struct  __attribute__ ((packed)) FM25xxDeviceIDDef
{
    uint64_t ManufacturerID: 56;
    struct  __attribute__ ((packed)) ProductID
    {
        uint8_t Family: 3;
        uint8_t Density: 5;
        uint8_t Sub: 2;
        uint8_t Rev: 3;
        uint8_t Rsvd: 3;
    }_ProductID;
};

union __attribute__ ((packed)) FM25xxStatusDef
{
    struct __attribute__ ((packed))
    {
        uint8_t X0 : 1;//This bit is non-writable and always returns ‘0’ upon read.
        uint8_t WEL : 1;//WEL indicates if the device is write enabled. This bit defaults to ‘0’ (disabled) on power-up. WEL = '1' --> Write enabled WEL = '0' --> Write disabled
        uint8_t BP0 : 1;//Used for block protection. For details, see Table 4 on page 7.
        uint8_t BP1 : 1;//Used for block protection. For details, see Table 4 on page 7.
        uint8_t X4 : 1;//These bits are non-writable and always return ‘0’ upon read
        uint8_t X5 : 1;//These bits are non-writable and always return ‘0’ upon read
        uint8_t X6 : 1;//This bit is non-writable and always return ‘1’ upon read.
        uint8_t WPEN : 1;//Used to enable the function of Write Protect Pin (WP). For details, see Table 5 on page 7.
    };
    uint8_t status;
};

static struct rt_spi_device *fm25xx_spidev;

static int fal_fram_init(void);
static int fal_fram_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_fram_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_fram_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev fm25xx_fram =
{
    .name = FM25xx_DEV_NAME,
    .addr = FM25xx_START_ADRESS,
    .len = FM25xx_SIZE_GRANULARITY_TOTAL,
    .blk_size = FM25xx_BLK_SIZE,
    .ops = {fal_fram_init, fal_fram_read, fal_fram_write, fal_fram_erase},
    .write_gran = 0,
};

static rt_err_t fm25xx_readDeviceID(struct FM25xxDeviceIDDef *id)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd = FM25xx_CMD_RDID;
    ret = rt_spi_send_then_recv(fm25xx_spidev, &cmd, 1, id, 9);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    if (id->ManufacturerID == FM25xx_ManufacturerID)
    {
        uint32_t fram_size = 0;
        switch(id->_ProductID.Density)
        {
        case FM25xx_SIZE_128K:
            fram_size = 128*1024;
            break;
        case FM25xx_SIZE_256K:
            fram_size = 2*128*1024;
            break;
        case FM25xx_SIZE_512K:
            fram_size = 4*128*1024;
            break;
        case FM25xx_SIZE_1M:
            fram_size = 8*128*1024;
            break;
        default:
            break;
        }
        fram_size /= 8;
        if (fram_size == FM25xx_SIZE_GRANULARITY_TOTAL)
        {
            LOG_I("init succeed %d KB.", fram_size/1024);
        }
        else
        {
            LOG_W("init succeed %d KB, Please modify the definition of size in fal.", fram_size/1024);
        }
    }
    else
    {
        LOG_HEX("id", 16, (uint8_t *)id, sizeof(struct FM25xxDeviceIDDef));
        LOG_E("init failed! read id 0x%lx%lx != 0x%lx%lx"
                , (uint32_t)(id->ManufacturerID >> 32), (uint32_t)id->ManufacturerID
                , (uint32_t)(FM25xx_ManufacturerID >> 32), (uint32_t)FM25xx_ManufacturerID);
        return -RT_ERROR;
    }

    return ret;
}

static rt_err_t fm25xx_writeEnable(void)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {FM25xx_CMD_WREN};
    rt_size_t size = rt_spi_send(fm25xx_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("write enable error %d!", ret);
        ret = -RT_EIO;
    }

    return ret;
}

static rt_err_t fm25xx_writeDisable(void)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {FM25xx_CMD_WRDI};
    rt_size_t size = rt_spi_send(fm25xx_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("write disable error %d!", ret);
        ret = -RT_EIO;
    }

    return ret;
}

static rt_err_t fm25xx_readStatus(union FM25xxStatusDef *status)
{
    rt_err_t ret = RT_EOK;

    uint8_t cmd[] = {FM25xx_CMD_RDSR};
    ret = rt_spi_send_then_recv(fm25xx_spidev, cmd, sizeof(cmd), status, 1);
    if (ret != RT_EOK)
    {
        LOG_E("read status error %d!", ret);
        ret = -RT_EIO;
    }
    else
    {
        LOG_D("read status X0 %d, WEL %d, BP0 %d, BP1 %d, X4 %d, X5 %d, X6 %d, WPEN %d"
                , status->X0, status->WEL, status->BP0, status->BP1
                , status->X4, status->X5, status->X6, status->WPEN);
    }

    return ret;
}

static rt_err_t fm25xx_writeStatus(union FM25xxStatusDef status)
{
    rt_err_t ret = RT_EOK;
    status.WEL = 1;

    fm25xx_writeEnable();

    uint8_t cmd[] = {FM25xx_CMD_WRSR, status.status};
    rt_size_t size = rt_spi_send(fm25xx_spidev, cmd, sizeof(cmd));
    if (size != sizeof(cmd))
    {
        LOG_E("write status error %d!", ret);
        ret = -RT_EIO;
    }
    else
    {
        LOG_D("write status X0 %d, WEL %d, BP0 %d, BP1 %d, X4 %d, X5 %d, X6 %d, WPEN %d"
                , status.X0, status.WEL, status.BP0, status.BP1
                , status.X4, status.X5, status.X6, status.WPEN);
    }

    fm25xx_writeDisable();

    return ret;
}

static int fram_spi_device_init(void)
{
    char dev_name[RT_NAME_MAX];
    rt_uint8_t dev_num = 0;
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", BSP_FM25xx_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(BSP_FM25xx_SPI_BUS, dev_name, BSP_FM25xx_SPI_CS_PIN);
    rt_hw_spi_device_attach(BSP_FM25xx_SPI_BUS, dev_name, rt_pin_get(BSP_FM25xx_SPI_CS_PIN));
    fm25xx_spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (fm25xx_spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_FM25xx_SPI_SPEED;
    if (rt_spi_configure(fm25xx_spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(fram_spi_device_init);

static int fal_fram_init(void)
{
    rt_err_t ret = RT_EOK;

    struct FM25xxDeviceIDDef id = {0};
    ret = fm25xx_readDeviceID(&id);

    union FM25xxStatusDef status = {0};
    fm25xx_readStatus(&status);
    status.WEL = 1;
    status.BP0 = 0;
    status.BP1 = 0;
    status.WPEN = 0;
    fm25xx_writeStatus(status);

    return ret;
}

static int fal_fram_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = fm25xx_fram.addr + offset;
    if (addr + size > fm25xx_fram.addr + fm25xx_fram.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    /* 发送读命令 */
    uint8_t cmd[] = { FM25xx_CMD_READ, (uint8_t) ((addr) >> 16U), (uint8_t) ((addr) >> 8U), (uint8_t) ((addr)) };

    /* 读数据 */
    rt_err_t ret = rt_spi_send_then_recv(fm25xx_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("read data error %d!", ret);
        return -RT_EIO;
    }

    LOG_HEX("read", 16, buf, (size > 64)?(64):(size));
    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}

static int fal_fram_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = fm25xx_fram.addr + offset;
    if (addr + size > fm25xx_fram.addr + fm25xx_fram.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    fm25xx_writeEnable();

    /* 发送写命令 */
    uint8_t cmd[] = {FM25xx_CMD_WRITE, (uint8_t)((addr) >> 16U), (uint8_t)((addr) >> 8U), (uint8_t)((addr))};

    /* 写数据 */
    rt_err_t ret = rt_spi_send_then_send(fm25xx_spidev, cmd, sizeof(cmd), buf, size);
    if (ret != RT_EOK)
    {
        LOG_E("write data error %d!", ret);
        return -RT_EIO;
    }

    fm25xx_writeDisable();

    LOG_HEX("write", 16, (uint8_t *)buf, (size > 64)?(64):(size));
    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}

static uint8_t buf[FM25xx_BLK_SIZE];
static int fal_fram_erase(long offset, size_t size)
{
    rt_uint32_t addr = fm25xx_fram.addr + offset;
    if ((addr + size) > fm25xx_fram.addr + fm25xx_fram.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    fm25xx_writeEnable();

    /* 发送写命令 */
    uint8_t cmd[] = {FM25xx_CMD_WRITE, (uint8_t)((addr) >> 16U), (uint8_t)((addr) >> 8U), (uint8_t)((addr))};

    rt_memset(buf, 0xff, sizeof(buf));
    uint32_t nblk = size / fm25xx_fram.blk_size;

    while(nblk --)
    {
        /* 写数据 */
        rt_err_t ret = rt_spi_send_then_send(fm25xx_spidev, cmd, sizeof(cmd), buf, sizeof(buf));
        if (ret != RT_EOK)
        {
            LOG_E("erase data error %d!", ret);
            return -RT_EIO;
        }
    }

    fm25xx_writeDisable();

    LOG_D("erase (0x%p) %d", (void*)(addr), size);

    return size;
}

#endif
#endif
