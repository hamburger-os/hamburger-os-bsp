#include "board.h"

#ifdef BSP_USING_LTC2945

#define DBG_TAG "ltc2945"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/********************************************************************************************************
**  LTC2945 Registers定义
********************************************************************************************************/
#define LTC2945_CONTROL_REG 0x00
#define LTC2945_ALERT_REG 0x01
#define LTC2945_STATUS_REG 0x02
#define LTC2945_FAULT_REG 0x03
#define LTC2945_FAULT_CoR_REG 0x04

#define LTC2945_POWER_MSB2_REG 0x05
#define LTC2945_POWER_MSB1_REG 0x06
#define LTC2945_POWER_LSB_REG 0x07
#define LTC2945_MAX_POWER_MSB2_REG 0x08
#define LTC2945_MAX_POWER_MSB1_REG 0x09
#define LTC2945_MAX_POWER_LSB_REG 0x0A
#define LTC2945_MIN_POWER_MSB2_REG 0x0B
#define LTC2945_MIN_POWER_MSB1_REG 0x0C
#define LTC2945_MIN_POWER_LSB_REG 0x0D
#define LTC2945_MAX_POWER_THRESHOLD_MSB2_REG 0x0E
#define LTC2945_MAX_POWER_THRESHOLD_MSB1_REG 0x0F
#define LTC2945_MAX_POWER_THRESHOLD_LSB_REG 0x10
#define LTC2945_MIN_POWER_THRESHOLD_MSB2_REG 0x11
#define LTC2945_MIN_POWER_THRESHOLD_MSB1_REG 0x12
#define LTC2945_MIN_POWER_THRESHOLD_LSB_REG 0x13

#define LTC2945_DELTA_SENSE_MSB_REG 0x14 // SENSE MSB
#define LTC2945_DELTA_SENSE_LSB_REG 0x15 // SENSE LSB
#define LTC2945_MAX_DELTA_SENSE_MSB_REG 0x16
#define LTC2945_MAX_DELTA_SENSE_LSB_REG 0x17
#define LTC2945_MIN_DELTA_SENSE_MSB_REG 0x18
#define LTC2945_MIN_DELTA_SENSE_LSB_REG 0x19
#define LTC2945_MAX_DELTA_SENSE_THRESHOLD_MSB_REG 0x1A
#define LTC2945_MAX_DELTA_SENSE_THRESHOLD_LSB_REG 0x1B
#define LTC2945_MIN_DELTA_SENSE_THRESHOLD_MSB_REG 0x1C
#define LTC2945_MIN_DELTA_SENSE_THRESHOLD_LSB_REG 0x1D

#define LTC2945_VIN_MSB_REG 0x1E // VIN MSB
#define LTC2945_VIN_LSB_REG 0x1F // VIN LSB
#define LTC2945_MAX_VIN_MSB_REG 0x20
#define LTC2945_MAX_VIN_LSB_REG 0x21
#define LTC2945_MIN_VIN_MSB_REG 0x22
#define LTC2945_MIN_VIN_LSB_REG 0x23
#define LTC2945_MAX_VIN_THRESHOLD_MSB_REG 0x24
#define LTC2945_MAX_VIN_THRESHOLD_LSB_REG 0x25
#define LTC2945_MIN_VIN_THRESHOLD_MSB_REG 0x26
#define LTC2945_MIN_VIN_THRESHOLD_LSB_REG 0x27

#define LTC2945_ADIN_MSB_REG 0x28     // ADIN MSB
#define LTC2945_ADIN_LSB_REG_REG 0x29 // ADIN LSB
#define LTC2945_MAX_ADIN_MSB_REG 0x2A
#define LTC2945_MAX_ADIN_LSB_REG 0x2B
#define LTC2945_MIN_ADIN_MSB_REG 0x2C
#define LTC2945_MIN_ADIN_LSB_REG 0x2D
#define LTC2945_MAX_ADIN_THRESHOLD_MSB_REG 0x2E
#define LTC2945_MAX_ADIN_THRESHOLD_LSB_REG 0x2F
#define LTC2945_MIN_ADIN_THRESHOLD_MSB_REG 0x30
#define LTC2945_MIN_ADIN_THRESHOLD_LSB_REG 0x31

typedef struct
{
    rt_uint16_t vot_val;
    rt_uint16_t current_Val;
} ltc2945_5vot_current;
static struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */
static rt_err_t ltc2945_write_byte(rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd
    buf[1] = data;

    /* 调用I2C设备接口传输数据LTC2945_I2C_ADD:0xD8*/
    if (rt_i2c_master_send(i2c_bus, LTC2945_I2C_ADD, reg, buf, 2) != 2)
    {
        LOG_E("i2c_master_send write error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ltc2945_read_byte(rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[1];
    buf[0] = reg; // cmd

    /* 调用I2C设备接口传输数据LTC2945_I2C_ADD:0xD8*/
    if (rt_i2c_master_send(i2c_bus, LTC2945_I2C_ADD, reg, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        return -RT_ERROR;
    }

    /* 调用I2C设备接口传输数据LTC2945_I2C_ADD:0xD8*/
    if (rt_i2c_master_recv(i2c_bus, LTC2945_I2C_ADD, reg, data, 32) == 32)
    {
        return RT_EOK;
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_size_t rt_ltc2945_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    ltc2945_5vot_current *pthis = NULL;
    if (size < sizeof(ltc2945_5vot_current))
    {
        LOG_E("size < %d", sizeof(ltc2945_5vot_current));
    }
    else
    {
        rt_uint8_t data[32], i;
        pthis = buffer;
        for (i = 0; i < 32; i++)
        {
            data[i] = 0xff;
        }
        if (ltc2945_read_byte(LTC2945_CONTROL_REG, data) == RT_EOK)
        {
            rt_uint16_t v_u16 = (rt_uint16_t)(((rt_uint16_t)data[0x1e] << 8 | data[0x1f]) >> 4);
            rt_uint16_t i_u16 = (rt_uint16_t)(((rt_uint16_t)data[0x14] << 8 | data[0x15]) >> 4);
            rt_uint32_t power_u32 = (rt_uint32_t)data[5] << 16 | (rt_uint32_t)data[6] << 8 | data[7];
            if (power_u32 == (rt_uint32_t)(v_u16 * i_u16))
            {
                pthis->vot_val = (rt_uint16_t)((rt_uint32_t)v_u16 * 25 / 10U);             // 0.01V
                pthis->current_Val = (rt_uint16_t)((rt_uint32_t)i_u16 * 25 * 50 / 10000U); // 0.01A
                //               LOG_D ("get_5v_voltage_u16 = %d.%dV\n" , pthis->vot_val/100 , pthis->vot_val%100);
                //               LOG_D ("get_5v_current_u16 = %d.%dA\n" ,pthis->current_Val/100 ,pthis->current_Val%100);
            }
            else
            {
                LOG_E("rt_ltc2945_read power_u32 VAL ERR");
            }
        }
        else
        {
            LOG_E("rt_ltc2945_read err");
        }
    }

    return 0;
}

/* RT-Thread Device Driver Interface */
static rt_err_t rt_ltc2945_init(rt_device_t dev)
{
    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(LTC2945_I2C_DEV);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }

    /** power vol */
    //    ltc2945_write_byte(LTC2945_CONTROL_REG, 0x15);
    //    ltc2945_write_byte(LTC2945_CONTROL_REG, 0x15);
    return RT_EOK;
}

static int rt_hw_ltc2945_init(void)
{
    // 注册设备
    rt_device_t ltc2945_device = rt_malloc(sizeof(struct rt_device));
    if (ltc2945_device)
    {
        ltc2945_device->type = RT_Device_Class_Char;
        ltc2945_device->init = rt_ltc2945_init;
        ltc2945_device->open = NULL;
        ltc2945_device->close = NULL;
        ltc2945_device->read = rt_ltc2945_read;
        ltc2945_device->write = NULL;
        ltc2945_device->control = NULL;
        if (rt_device_register(ltc2945_device, "ltc2945", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
        {
            LOG_I("device created successfully!");
        }
        else
        {
            LOG_E("device created failed!");
            return -RT_ERROR;
        }
    }
    else
    {
        LOG_E("no memory for create device");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_ltc2945_init);
#endif
