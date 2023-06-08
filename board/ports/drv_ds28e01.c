/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-15     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_DS28E01
#include "sensor.h"

#define DBG_TAG "DS28E01"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DS28E01_PIN rt_pin_get(DS28E01_GPIO)
#define SET_DQ() rt_pin_write(DS28E01_PIN, PIN_HIGH)
#define CLR_DQ() rt_pin_write(DS28E01_PIN, PIN_LOW)
#define OUT_DQ() rt_pin_mode(DS28E01_PIN, PIN_MODE_OUTPUT)
#define IN_DQ() rt_pin_mode(DS28E01_PIN, PIN_MODE_INPUT)
#define GET_DQ() rt_pin_read(DS28E01_PIN)
/****************************DS28E01 DEFININS****************************************/
#define WIRE_RST_TIMEOUT (60 * 480) /* 复位访问超时阈值 */
#define WIRE_RD_TIMEOUT (60 * 80)   /* 读访问超时阈值 */

#define DS28E01_FAMILY_CODE ((rt_uint8_t)0x2F)

#define DS28E01_RD_ROM ((rt_uint8_t)0x33) /*  读ROM */
#define DS28E01_MATCH_ROM (0x55)          /*  匹配ROM */
#define DS28E01_SEARCH_ROM (0xF0)         /*  搜索ROM */
#define DS28E01_SKIP_ROM (0xCC)           /*  跳过ROM */

#define DS28E01_WR_SCRATCH (0x0F)    /*  写至暂存器 */
#define DS28E01_RD_SCRATCH (0xAA)    /*  读暂存器 */
#define DS28E01_COPY_SCRATCH (0x55)  /*  复制暂存器数据到存储区 */
#define DS28E01_RD_MEMORY (0xF0)     /*  读内存 */
#define DS28E01_LD_FST_SECRET (0x5A) /*  装载首个密钥 */

static const rt_uint8_t st_a_crctab8[256] =
{
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83, 0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x9D, 0xC3, 0x21, 0x7F, 0xFC, 0xA2, 0x40, 0x1E, 0x5F, 0x01, 0xE3, 0xBD, 0x3E, 0x60, 0x82, 0xDC,
    0x23, 0x7D, 0x9F, 0xC1, 0x42, 0x1C, 0xFE, 0xA0, 0xE1, 0xBF, 0x5D, 0x03, 0x80, 0xDE, 0x3C, 0x62,
    0xBE, 0xE0, 0x02, 0x5C, 0xDF, 0x81, 0x63, 0x3D, 0x7C, 0x22, 0xC0, 0x9E, 0x1D, 0x43, 0xA1, 0xFF,
    0x46, 0x18, 0xFA, 0xA4, 0x27, 0x79, 0x9B, 0xC5, 0x84, 0xDA, 0x38, 0x66, 0xE5, 0xBB, 0x59, 0x07,
    0xDB, 0x85, 0x67, 0x39, 0xBA, 0xE4, 0x06, 0x58, 0x19, 0x47, 0xA5, 0xFB, 0x78, 0x26, 0xC4, 0x9A,
    0x65, 0x3B, 0xD9, 0x87, 0x04, 0x5A, 0xB8, 0xE6, 0xA7, 0xF9, 0x1B, 0x45, 0xC6, 0x98, 0x7A, 0x24,
    0xF8, 0xA6, 0x44, 0x1A, 0x99, 0xC7, 0x25, 0x7B, 0x3A, 0x64, 0x86, 0xD8, 0x5B, 0x05, 0xE7, 0xB9,
    0x8C, 0xD2, 0x30, 0x6E, 0xED, 0xB3, 0x51, 0x0F, 0x4E, 0x10, 0xF2, 0xAC, 0x2F, 0x71, 0x93, 0xCD,
    0x11, 0x4F, 0xAD, 0xF3, 0x70, 0x2E, 0xCC, 0x92, 0xD3, 0x8D, 0x6F, 0x31, 0xB2, 0xEC, 0x0E, 0x50,
    0xAF, 0xF1, 0x13, 0x4D, 0xCE, 0x90, 0x72, 0x2C, 0x6D, 0x33, 0xD1, 0x8F, 0x0C, 0x52, 0xB0, 0xEE,
    0x32, 0x6C, 0x8E, 0xD0, 0x53, 0x0D, 0xEF, 0xB1, 0xF0, 0xAE, 0x4C, 0x12, 0x91, 0xCF, 0x2D, 0x73,
    0xCA, 0x94, 0x76, 0x28, 0xAB, 0xF5, 0x17, 0x49, 0x08, 0x56, 0xB4, 0xEA, 0x69, 0x37, 0xD5, 0x8B,
    0x57, 0x09, 0xEB, 0xB5, 0x36, 0x68, 0x8A, 0xD4, 0x95, 0xCB, 0x29, 0x77, 0xF4, 0xAA, 0x48, 0x16,
    0xE9, 0xB7, 0x55, 0x0B, 0x88, 0xD6, 0x34, 0x6A, 0x2B, 0x75, 0x97, 0xC9, 0x4A, 0x14, 0xF6, 0xA8,
    0x74, 0x2A, 0xC8, 0x96, 0x15, 0x4B, 0xA9, 0xF7, 0xB6, 0xE8, 0x0A, 0x54, 0xD7, 0x89, 0x6B, 0x35,
};

/* crc8计算*/
rt_uint8_t crc8_create(const rt_uint8_t *p_dat_u8, rt_uint16_t len_u16, rt_uint8_t crc_init_val_u8)
{
    rt_uint8_t crc_u8 = crc_init_val_u8;
    rt_uint16_t i;

    for (i = 0U; i < len_u16; i++)
    {
        crc_u8 = st_a_crctab8[crc_u8 ^ p_dat_u8[i]];
    }
    return crc_u8;
}
/*
 * @brief 复位
 * @param 无
 */
static void DEV_DS28E01_Reset1Wire(void)
{
    rt_int32_t i = 0;
    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(750);
    SET_DQ();
    IN_DQ();

    /* 等待数据变0 */
    while (GET_DQ() != (rt_uint8_t)0)
    {
        i++;

        if (i > WIRE_RST_TIMEOUT)
        {
            break;
        }
        else
        {
            /* nothing to do */
        }
    };

    /* 等待数据变高 */
    while (!(GET_DQ()))
    {
        i++;

        if (i > WIRE_RST_TIMEOUT)
        {
            break;
        }
        else
        {
            /* nothing to do */
        }
    }

    OUT_DQ();
}

/*
 * @brief 读数据
 */
static rt_uint8_t read_ds28e01(void)
{
    rt_uint8_t data = (rt_uint8_t)0, i = (rt_uint8_t)0;
    rt_int32_t n;

    /* 读一个字节数据 */
    for (i = (rt_uint8_t)0; i < (rt_uint8_t)8; i++)
    {
        data = (rt_uint8_t)(data >> 1U);
        OUT_DQ();
        CLR_DQ();
        rt_hw_us_delay(5U);
        SET_DQ();
        rt_hw_us_delay(15U);
        IN_DQ();

        if (GET_DQ() != (rt_uint8_t)0)
        {
            data |= (rt_uint8_t)0x80;
        }
        else
        {
            /* 读0时，等位结束 */
            n = 0;

            while (!(GET_DQ()))
            {
                n++;

                /* 判断循环次数 */
                if (n > WIRE_RD_TIMEOUT)
                {
                    break;
                }
                else
                {
                    /* nothing to do */
                }
            };
        }

        rt_hw_us_delay(60U);
        SET_DQ();
    }

    return (data);
}

/*
 * @brief 写数据
 */
static void write_ds28e01(rt_uint8_t data_u8)
{
    rt_uint8_t i = (rt_uint8_t)0;

    OUT_DQ();
    for (i = (rt_uint8_t)0; i < (rt_uint8_t)8; i++)
    {
        if ((data_u8 & (rt_uint8_t)0x01) != (rt_uint8_t)0)
        {
            CLR_DQ();
            rt_hw_us_delay(5U);
            SET_DQ();
            rt_hw_us_delay(85U); /* 65 */
        }
        else
        {
            CLR_DQ();
            rt_hw_us_delay(90U); /* 65 */
            SET_DQ();
            rt_hw_us_delay(5U);
        }

        data_u8 = (rt_uint8_t)(data_u8 >> 1U);
    }
}

/*
 * @brief 读芯片ID。
 */
static rt_int32_t DEV_DS28E01_ReadID(rt_uint8_t *pid_u8)
{
    rt_uint8_t i, *p;
    DEV_DS28E01_Reset1Wire();
    write_ds28e01(DS28E01_RD_ROM);
    p = pid_u8;

    for (i = (rt_uint8_t)0; i < (rt_uint8_t)8; i++)
    {
        *p = read_ds28e01();
        p++;
    }

    i = crc8_create((const rt_uint8_t *)pid_u8, (rt_uint16_t)7, (rt_uint8_t)0);

    if (pid_u8[7] == i)
    {
        if (pid_u8[0] == DS28E01_FAMILY_CODE)
        {
            return (rt_int32_t)0;
        }
        else
        {
            /* nothing to do */
        }
    }
    else
    {
        /* nothing to do */
    }

    return (rt_int32_t)-1;
}

static rt_size_t ds28e01_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return DEV_DS28E01_ReadID(buf);
    }
    else
        return 0;
}

static rt_err_t ds28e01_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;
    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        if (args)
        {
            if (DEV_DS28E01_ReadID(args) != 0)
            {
                ret = -RT_EIO;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}

static struct rt_sensor_ops sensor_ops =
    {
        ds28e01_fetch_data,
        ds28e01_control};

static int rt_hw_ds28e01_init()
{
    rt_int8_t result;
    rt_sensor_t sensor_sha = RT_NULL;

    /* temperature sensor register */
    sensor_sha = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_sha == RT_NULL)
        return -RT_ERROR;

    sensor_sha->info.type = RT_SENSOR_CLASS_MAG;
    sensor_sha->info.vendor = RT_SENSOR_VENDOR_SHARP;
    sensor_sha->info.model = "ds28e01";
    sensor_sha->info.unit = RT_SENSOR_UNIT_NONE;
    sensor_sha->info.intf_type = RT_SENSOR_INTF_ONEWIRE;
    sensor_sha->info.range_max = 125;
    sensor_sha->info.range_min = -55;
    sensor_sha->info.period_min = 150;

    sensor_sha->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_sha, "ds28e01", RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    return RT_EOK;

__exit:
    rt_free(sensor_sha);
    return -RT_ERROR;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_ds28e01_init);

#endif
