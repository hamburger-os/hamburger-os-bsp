#include "board.h"

#ifdef BSP_USING_LTC2991

#define DBG_TAG "ltc2991"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
typedef enum
{
    LTC2991_REG_STA_LOW = 0X00,
    LTC2991_REG_STA_HIGH,
    LTC2991_REG_RSV1,
    LTC2991_REG_RSV2,
    LTC2991_REG_RSV3,
    LTC2991_REG_RSV4,
    LTC2991_REG_CTRL_V1_4,
    LTC2991_REG_CTRL_V5_8,
    LTC2991_REG_PWM_LSB,
    LTC2991_REG_PWM_MSB,
    LTC2991_REG_V1_MSB,
    LTC2991_REG_V1_LSB,
    LTC2991_REG_V2_MSB,
    LTC2991_REG_V2_LSB,
    LTC2991_REG_V3_MSB,
    LTC2991_REG_V3_LSB,
    LTC2991_REG_V4_MSB,
    LTC2991_REG_V4_LSB,
    LTC2991_REG_V5_MSB,
    LTC2991_REG_V5_LSB,
    LTC2991_REG_V6_MSB,
    LTC2991_REG_V6_LSB,
    LTC2991_REG_V7_MSB,
    LTC2991_REG_V7_LSB,
    LTC2991_REG_V8_MSB,
    LTC2991_REG_V8_LSB,
    LTC2991_REG_T_INT_MSB,
    LTC2991_REG_T_INT_LSB,
    LTC2991_REG_VCC_MSB,
    LTC2991_REG_VCC_LSB,
} eLTC2991_REG;

typedef enum
{
    LTC2991_CH_V1 = 0x00,
    LTC2991_CH_V2 = 0x01,
    LTC2991_CH_V3,
    LTC2991_CH_V4,
    LTC2991_CH_V5,
    LTC2991_CH_V6,
    LTC2991_CH_V7,
    LTC2991_CH_V8,
    LTC2991_CH_VCC,
    LTC2991_CH_T,
} LTC2991ChannelNameTable;

typedef struct
{
    rt_uint16_t vot_val;
    rt_uint16_t current_Val;
} ltc2945_5vot_current;
static struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */
static rt_err_t ltc2991_write_byte(rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd
    buf[1] = data;

    /* 调用I2C设备接口传输数据LTC2991_I2C_ADD:0x90*/
    if (rt_i2c_master_send(i2c_bus, LTC2991_I2C_ADD, 0, buf, 2) != 2)
    {
        LOG_E("i2c_master_send write error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ltc2991_read_byte(rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[1];
    buf[0] = reg; // cmd

    /* 调用I2C设备接口传输数据 LTC2991_I2C_ADD:0x90*/
    if (rt_i2c_master_send(i2c_bus, LTC2991_I2C_ADD, reg, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        return -RT_ERROR;
    }

    /* 调用I2C设备接口传输数*/
    if (rt_i2c_master_recv(i2c_bus, LTC2991_I2C_ADD, reg, buf, 1) == 1)
    {
        *data = buf[0];
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

/**************************************************************************************************************
 * 函数名:BspLtc2991ReadReg16
 * 描     述:LTC2991读取从某个地址
 * 输     入: LTC2991_REG--要读取的LTC2991内部寄存器地址；

 * 输     出: ER:    操作失败
 *            OK： 操作成功
 * 作     者:
 * 日     期:
 * 备     注:
 *************************************************************************************************************
 * 修     改:
 *                   无
************************************************************************************************************* */
static rt_err_t ltc2991_read_word(rt_uint8_t reg, rt_uint16_t *data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd

    if (0 != (reg % 2))
    {
        /*地址非法，退出*/
        return -RT_ERROR;
    }
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(i2c_bus, LTC2991_I2C_ADD, reg, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        return -RT_ERROR;
    }
    /* 调用I2C设备接口传输数*/
    if (rt_i2c_master_recv(i2c_bus, LTC2991_I2C_ADD, reg, buf, 2) == 2)
    {
        if (reg < LTC2991_REG_V1_MSB)
        {
            *data = buf[0] + ((rt_uint16_t)buf[1] << 8);
        }
        else
        {
            *data = buf[1] + ((rt_uint16_t)buf[0] << 8);
        }
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

/**************************************************************************************************************
 * 函数名:BspLtc2991StaGet
 * 描     述:获取LTC2991的状态寄存器值
 * 输     入: *pSta--状态值。
 * 输     出: ER:    操作失败
 *            OK：   操作成功
 * 作     者:
 * 日     期:
 * 备     注:
 *************************************************************************************************************
 * 修     改:
 *                   无
 ************************************************************************************************************* */
rt_uint8_t BspLtc2991StaGet(rt_uint16_t *pSta)
{
    rt_uint16_t Tmp;

    if ((-RT_ERROR) == ltc2991_read_word(LTC2991_REG_STA_LOW, &Tmp))
    {
        /*读取状态寄存器失败*/
        return (-RT_ERROR);
    }
    // 读取成功
    else
    {
        *pSta = Tmp & 0x303;
        return RT_EOK;
    }
}
/*通道寄存器地址*/
static eLTC2991_REG Ltc2991Ch2Reg[] =
    {
        LTC2991_REG_V1_MSB,
        LTC2991_REG_V2_MSB,
        LTC2991_REG_V3_MSB,
        LTC2991_REG_V4_MSB,
        LTC2991_REG_V5_MSB,
        LTC2991_REG_V6_MSB,
        LTC2991_REG_V7_MSB,
        LTC2991_REG_V8_MSB,
        LTC2991_REG_VCC_MSB,
        LTC2991_REG_T_INT_MSB,
};
/**************************************************************************************************************
 * 函数名:ReadVoltReg
 * 描     述:获取单个通道的寄存器值并返回
 * 输     入: channel  通道 。
              *pWord  通道的输入电压值
 * 输     出: OK --操作成功；
              ER --操作失败
 * 作     者:
 * 日     期:
 * 备     注:
 *************************************************************************************************************/

rt_int32_t ReadVoltReg(rt_uint8_t Channel, rt_uint16_t *pWord)
{
    rt_uint16_t Tmp = 0;
    rt_int16_t Value = 0;
    float tmpvalue = 0.0f;
    if ((-RT_ERROR) == ltc2991_read_word(Ltc2991Ch2Reg[Channel], &Tmp))
    {
        /*单个通道读取失败直接退出读取*/
        return (-RT_ERROR);
    }
    else
    {
        if (Channel < LTC2991_CH_VCC)
        {
            /*通道V1-V8寄存器值转化为mV电压值*/
            if (Tmp & (1 << 15))
            {
                /*nothing*/
            }
            else
            {
                /*数据无效返回错误*/
                return (-RT_ERROR);
            }
            if (Tmp & (1 << 14))
            {
                /*符号位的处理*/
#if 0
        Tmp |= (0x01 << 15);
        Value = (rt_int16_t)Tmp;
#else
                /*芯片可以测量范围-0.3-VC，运行到此处说明输入电压为负 */
                return (-RT_ERROR);
#endif
            }
            Tmp &= 0x7fff;
            /*LSB = 305.18μV*/
            Value = (rt_int16_t)Tmp;
            tmpvalue = (Value * 305.18f) / 1000.0f;
            Tmp = (rt_uint16_t)tmpvalue;
        }

        else if (Channel == LTC2991_CH_VCC)
        {
            /*VCC通道格式转化 VCC= Result + 2.5V
                LSB = 305.18μV */
            Tmp &= 0x3fff;
            Tmp = (Tmp * 305.18) / 1000 + 2500;
        }
        else
        {
            /*温度通道数据，数据格式未转换*/
            Tmp &= 0x0fff;
        }
        *pWord = Tmp;
        return RT_EOK;
    }
}
/**************************************************************************************************************
 * 函数名:BspLtc2991ChannelRead
 * 描     述:获取LTC2991所有通道的电压值和VCC通道电压值
 * 输     入: channel--总的获取的通道数。
 * 输     入: *p_volt--返回的9个通道量化值:单位，mv。
                      V1-V8  VCC
 * 输     出:   OK--操作成功；
                ER --操作失败
 * 作     者:
 * 日     期:
 * 备     注:读取一个通道的值会导致芯片内部重新开始转化，所以最好全部读取出来
 *************************************************************************************************************/
rt_uint8_t BspLtc2991ChannelRead(rt_uint16_t all_channel, rt_uint16_t *p_volt)
{

    rt_uint16_t Tmp = 0;

    /*获取数据状态*/
    if (RT_EOK != BspLtc2991StaGet(&Tmp))
    {
        return -RT_ERROR;
    }
    else
    {
        // nothing
    }

    if ((Tmp & 0x303) != 0x303)
    {
        /*并非所有通道都有有效数据，是否两次读数时间间隔太短*/
        return -RT_ERROR;
    }

    for (int i = 0; i < all_channel; i++)
    {
        if (0 == ReadVoltReg(i, &Tmp))
        {
            *(p_volt + i) = Tmp;
        }
        else
        {
            return -RT_ERROR;
        }
    }
    return RT_EOK;
}

static rt_size_t rt_ltc2991_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return BspLtc2991ChannelRead(pos, (rt_uint16_t *)buffer);
}

/* RT-Thread Device Driver Interface */
static rt_err_t rt_ltc2991_init(rt_device_t dev)
{
    rt_uint8_t Byte = 0;
    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(LTC2991_I2C_DEV);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }
    /*使能V1 - V4，使能温度和VCC*/
    Byte = 0x38;
    ltc2991_write_byte(LTC2991_REG_STA_HIGH, Byte);
    /*V1-V4 配置，单端电压采样，使能滤波*/
    Byte = 0x88;
    ltc2991_write_byte(LTC2991_REG_CTRL_V1_4, Byte);
    /*V5-V8 配置，单端电压采样，使能滤波*/
    //    Byte = 0x88;
    //    ltc2991_write_byte(LTC2991_REG_CTRL_V5_8, Byte);
    /*禁止PWM，使能温度，重复采样模式*/
    Byte = 0x18;
    ltc2991_write_byte(LTC2991_REG_PWM_LSB, Byte);
    /** power vol */
    return RT_EOK;
}

static int rt_hw_ltc2991_init(void)
{
    // 注册设备
    rt_device_t ltc2991_device = rt_malloc(sizeof(struct rt_device));
    if (ltc2991_device)
    {
        ltc2991_device->type = RT_Device_Class_Char;
        ltc2991_device->init = rt_ltc2991_init;
        ltc2991_device->open = NULL;
        ltc2991_device->close = NULL;
        ltc2991_device->read = rt_ltc2991_read;
        ltc2991_device->write = NULL;
        ltc2991_device->control = NULL;
        if (rt_device_register(ltc2991_device, "ltc2991", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
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
INIT_DEVICE_EXPORT(rt_hw_ltc2991_init);
#endif
