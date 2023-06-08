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

#ifdef BSP_USING_MAX31826
#include "sensor.h"

#define DBG_TAG "max31826"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define MAX31826_PIN    rt_pin_get(MAX31826_GPIO)
#define SET_DQ()        rt_pin_write(MAX31826_PIN, PIN_HIGH)
#define CLR_DQ()        rt_pin_write(MAX31826_PIN, PIN_LOW)
#define OUT_DQ()        rt_pin_mode(MAX31826_PIN, PIN_MODE_OUTPUT)
#define IN_DQ()         rt_pin_mode(MAX31826_PIN, PIN_MODE_INPUT)
#define GET_DQ()        rt_pin_read(MAX31826_PIN)

/* ***************************MAX31826 DEFININS****************************************/
#define MAX31826_EEPROM_SIZE                (0x80U)             /*  EEPROM容量，单位:字节 */

#define MAX31826_CMD_READ_ROM               ((rt_uint8_t)0x33)
#define MAX31826_CMD_SKIP_ROM               ((rt_uint8_t)0xCC)
#define MAX31826_CMD_READ_SCRATCHPAD        ((rt_uint8_t)0xBE)
#define MAX31826_CMD_COPY_SCRATCHPAD2       ((rt_uint8_t)0x55)       /*  复制scratchpad2 数据到EEPROM */
#define MAX31826_CMD_READ_EE_SCRATCHPAD     ((rt_uint8_t)0xAA)       /*  读EEPROM scratchpad数据 */
#define MAX31826_CMD_BEGINS_COVERSION       ((rt_uint8_t)0x44)

#define MAX31826_WRITE_SCRATCHPAD2          ((rt_uint8_t)0x0F)       /*  写scratchpad2 数据 */
#define MAX31826_WRITE_FRM_LEN              ((rt_uint8_t)10)         /*  写scratchpad2命令帧长度 */
#define MAX31826_EEPROM_LINE_LEN            ((rt_uint8_t)8)          /*  EEPROM行长度 */
#define MAX31826_READ_MEMORY                ((rt_uint8_t)0xF0)       /*  读EEPROM数据 */
#define MAX31826_PROG_EEPROM                ((rt_uint8_t)0xA5)       /*  编程EEPROM */

#define WIRE_RST_TIMEOUT                    (60*480)            /* 复位访问超时阈值 */
#define WIRE_RD_TIMEOUT                     (60*80)             /* 读访问超时阈值 */

#define TEMP_INVALID                        (-12500)            /*  无效的温度数据 */

#define BD_HW_INF_ADDR                      (0)                 /*  板卡信息起始地址 */
#define DEV_HW_INF_ADDR                     (0x10)              /*  整机信息起始地址 */

/*
 * @brief 复位
 * @param 无
 */
static rt_int32_t DEV_MAX31826_Reset1Wire(void)
{
    rt_int32_t ret = 0xFF;

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(750);
    SET_DQ();
    rt_hw_us_delay(2);

    IN_DQ();
    rt_hw_us_delay(200);
    if(GET_DQ() == 0x01)/* 复位之后从机将低电平状态释放 */
        ret = 1;
    else
        ret = 0;

    return ret;
}

/*****************************
函 数 名: DEV_MAX31826_WriteBit
功    能：MAX31826写入1位
参    数： -
返    回： -
******************************/
static void DEV_MAX31826_WriteBit(rt_uint8_t sendbit)
{
    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(2);
    if (sendbit == 0x01)
    {
        SET_DQ();
    }
    rt_hw_us_delay(55);
    SET_DQ();
    rt_hw_us_delay(15);
}

/****************************
函 数 名: DEV_MAX31826_ReadBit
功    能：MAX31820读出1位
参    数： -
返    回： -
*****************************/
static rt_uint8_t DEV_MAX31826_ReadBit(void)
{
    rt_uint8_t readbit = 0;

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(8);
    SET_DQ();
    rt_hw_us_delay(2);

    IN_DQ();
    rt_hw_us_delay(2);
    readbit = GET_DQ();
    rt_hw_us_delay(60);

    return readbit;
}

/*
 * @brief 读数据
 * @param 无
 */
static rt_uint8_t DEV_MAX31826_Read1Wire(void)
{
    rt_uint8_t   readdata = 0x00;
    rt_uint16_t  setcontrol = 0x0001;

    while (setcontrol <= 0x0080)
    {
        if (DEV_MAX31826_ReadBit())
        {
            readdata |=  setcontrol;
        }
        setcontrol = setcontrol << 1;
    }
    return readdata;
}

/*
 * @brief 写数据
 * @param data - 待写入数据
 */
static void DEV_MAX31826_Write1Wire(rt_uint8_t data)
{
    rt_uint8_t  i;
    rt_uint8_t  temp;

    for (i = 0x00; i < 0x08; i++)
    {
        temp = data >> i;
        DEV_MAX31826_WriteBit(temp & 0x01);
    }
}

/* crc8计算 ，多项式= x8+x6+x4+x3++x2+x1,0x5E,高位在前*/
static rt_uint8_t crc8_create(const rt_uint8_t *p_dat_u8, rt_uint16_t len_u16, rt_uint8_t crc_init_val_u8)
{
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
    rt_uint8_t crc_u8 = crc_init_val_u8;
    rt_uint16_t i;

    for(i = 0U; i < len_u16; i++)
    {
        crc_u8 = st_a_crctab8[crc_u8 ^ p_dat_u8[i]];
    }
    return crc_u8;
}

/****************************************
功  能:判断max31826是否存在,然后在读取温度值
参  数:无
返  回:成功:1   失败：返回其他值
*******************************************/
static rt_uint8_t DEV_MAX31826_ReadAck(void)
{
    rt_uint8_t readbit;

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(10); /* 短暂延时后拉高，设置为输入，采集温度传感器输出的电平 */
    SET_DQ();

    IN_DQ();
    rt_hw_us_delay(10);
    if(GET_DQ() == 1)
        readbit = 1;
    else
        readbit = 0;

    rt_hw_us_delay(70);
    OUT_DQ();
    SET_DQ();

    return readbit;
}

/*
 * @brief 读芯片ID
 * @param *id - 存放芯片ID的数据缓冲区指针。
 */
static rt_int32_t DEV_MAX31826_ReadID(rt_uint8_t *bufferid)
{
    rt_int32_t ret = -1;
    rt_uint8_t p_id[8] = {0};
    rt_uint8_t i, *p;

    /* 发送命令 */
    if(DEV_MAX31826_Reset1Wire())
    {
        DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_ROM);
        p = p_id;

        /* 读ID */
        for(i = (rt_uint8_t)0; i < (rt_uint8_t)8; i++)
        {
            *p = DEV_MAX31826_Read1Wire();
            *bufferid = *p;
            p++;
            bufferid++;
            rt_hw_us_delay(100);//此处增加延时后，则可读取成功
        }

        /* 检查结果 */
        i = p_id[7];
        if(i == crc8_create((const rt_uint8_t *)p_id, (rt_uint16_t)7, (rt_uint8_t)0x00))
        {
           LOG_D("id : %x %x %x %x %x %x"
                 , p_id[0], p_id[1], p_id[2], p_id[3], p_id[4], p_id[5], p_id[6]);
           ret = (rt_int32_t)0;
        }
        else
        {
           LOG_E("id : %x %x %x %x %x %x"
                 , p_id[0], p_id[1], p_id[2], p_id[3], p_id[4], p_id[5], p_id[6]);
           ret = (rt_int32_t)-1;
        }
    }
    return ret;
}

/*
 * @brief MAX31820转换温度
 * @param 无
 */
static void DEV_MAX31826_Convert(void)
{
    DEV_MAX31826_Reset1Wire();
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_BEGINS_COVERSION);
}

/*
 * @brief MAX31826读温度
 * @param 无
 */
static rt_int32_t DEV_MAX31826_ReadTemp(void)
{
    rt_uint8_t buf[12];
    rt_uint8_t temp1, temp2;
    rt_int16_t tmpval;
    rt_int32_t i;

    rt_hw_us_delay(20U);
    /* 复位并发送读命令 */
    DEV_MAX31826_Reset1Wire();
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_SCRATCHPAD);
    /* 读温度 */
    for(i = 0; i < 9; i++)
    {
        buf[i] = DEV_MAX31826_Read1Wire();
    }

    temp1 = buf[8];
    /* 校验数据 */
    if(temp1 == crc8_create((const rt_uint8_t *)buf, (rt_uint16_t)8, (rt_uint8_t)0x00))
    {
        temp1 = buf[0];
        temp2 = buf[1];
        tmpval = (rt_int16_t)(((rt_uint32_t)temp2 << 8U) | (rt_uint32_t)temp1);
    }
    else
    {
        tmpval= (rt_int32_t)TEMP_INVALID;
    }
    DEV_MAX31826_Convert();

    return (rt_int32_t)tmpval;
}

/* RT-Thread Device Driver Interface */
static rt_size_t _max31826_polling_get_data(struct rt_sensor_device *sensor, struct rt_sensor_data *data)
{
    rt_int32_t temperature_x10;
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        rt_int32_t s32Tmp = TEMP_INVALID;
        float f32Max31826Tmp = 0.0f;

        s32Tmp = DEV_MAX31826_ReadTemp();
        f32Max31826Tmp = (0.0625f * s32Tmp);//单位：℃

        temperature_x10 = f32Max31826Tmp * 10;
        LOG_D("temp : %d", temperature_x10);
        data->data.temp = temperature_x10;
        data->timestamp = rt_tick_get_millisecond();
    }
    return 1;
}

static rt_size_t max31826_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _max31826_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static rt_err_t max31826_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;
    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        if (args)
        {
            if (DEV_MAX31826_ReadID(args) != 0)
            {
                ret = -RT_EIO;
            }
        }
        break;
    default:
        break;
    }

    LOG_D("max31826_control %d 0x%x", ret, cmd);
    return ret;
}

static struct rt_sensor_ops sensor_ops =
{
    max31826_fetch_data,
    max31826_control
};

static int rt_hw_max31826_init()
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL;

    /* temperature sensor register */
    sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -RT_ERROR;

    sensor_temp->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_MAXIM;
    sensor_temp->info.model      = "max31826";
    sensor_temp->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
    sensor_temp->info.range_max  = 125;
    sensor_temp->info.range_min  = -55;
    sensor_temp->info.period_min = 150;

    sensor_temp->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_temp, "max31826", RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    DEV_MAX31826_ReadTemp();
    return RT_EOK;

__exit:
    rt_free(sensor_temp);
    return -RT_ERROR;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_max31826_init);

#endif
