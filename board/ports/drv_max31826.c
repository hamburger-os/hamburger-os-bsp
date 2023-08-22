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
#include "fal.h"

#define DBG_TAG "max31826"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef MAX31826_USING_IO

#define MAX31826_PIN    rt_pin_get(MAX31826_GPIO)
#define SET_DQ()        rt_pin_write(MAX31826_PIN, PIN_HIGH)
#define CLR_DQ()        rt_pin_write(MAX31826_PIN, PIN_LOW)
#define OUT_DQ()        rt_pin_mode(MAX31826_PIN, PIN_MODE_OUTPUT)
#define IN_DQ()         rt_pin_mode(MAX31826_PIN, PIN_MODE_INPUT)
#define GET_DQ()        rt_pin_read(MAX31826_PIN)

#endif  /* MAX31826_USING_IO */

/* ***************************MAX31826 DEFININS****************************************/

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
#define MAX31826_CMD_TOKEN ((rt_uint8_t)0xA5)

#define WIRE_RST_TIMEOUT                    (60*480)            /* 复位访问超时阈值 */
#define WIRE_RD_TIMEOUT                     (60*80)             /* 读访问超时阈值 */

#define TEMP_INVALID                        (-12500)            /*  无效的温度数据 */

#define BD_HW_INF_ADDR                      (0)                 /*  板卡信息起始地址 */
#define DEV_HW_INF_ADDR                     (0x10)              /*  整机信息起始地址 */

static int fal_max31826_init(void);
static int fal_max31826_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_max31826_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_max31826_erase(long offset, size_t size);

const struct fal_flash_dev max31826_flash = {
    .name = "max31826",
    .addr = MAX31826_START_ADRESS,
    .len = MAX31826_SIZE_GRANULARITY_TOTAL,
    .blk_size = MAX31826_BLK_SIZE,
    .ops = {fal_max31826_init, fal_max31826_read, fal_max31826_write, fal_max31826_erase},
    .write_gran = 0,
};

#ifdef MAX31826_USING_I2C_DS2484

#include "drv_ds2484.h"

#define CRC8INIT    0x00
#define CRC8POLY    0x18              //0X18 = X^8+X^5+X^4+X^0

typedef __packed union
{
  __packed struct _code
  {
    uint8_t FamilyCode;
    uint8_t SerialCode[6];
    uint8_t CRC_byte;
  }Code;
  uint8_t  u8p_Data[8];
}U_DS31820ID;

typedef union
{
  /*数据*/
  struct
  {
    uint8_t TempL;     /*温度低字节*/
    uint8_t TempH;     /*温度的高字节*/
    uint8_t User1;     /*用户存储区1*/
    uint8_t User2;     /*用户存储区2*/
    uint8_t Confg;     /*配置存储区*/
    uint8_t ResFF;     /*预留*/
    uint8_t Res;        /*预留*/
    uint8_t Res10;     /*预留*/
    uint8_t CRC_byte;   /*以上字节的CRC8 */
  }s_Memory;
  uint8_t  a_data[9];   /*数据*/
}U_MAX31826Memory;

static rt_device_t ds2484_dev = NULL;

static rt_uint8_t crc8_create(rt_uint8_t *data, rt_uint16_t number_of_bytes_in_data)
{
  uint8_t  crc;
  uint16_t loop_count;
  uint8_t  bit_counter;
  uint8_t  b;
  uint8_t  feedback_bit;

  crc = CRC8INIT;
  for (loop_count = 0; loop_count != number_of_bytes_in_data; loop_count++)
  {
    b = data[loop_count];

    bit_counter = 8;
    do
    {
      feedback_bit = (crc ^ b) & 0x01;
      if ( feedback_bit == 0x01 )
      {
        crc = crc ^ CRC8POLY;
      }
      crc = (crc >> 1) & 0x7F;
      if ( feedback_bit == 0x01 )
      {
        crc = crc | 0x80;
      }
      b = b >> 1;
      bit_counter--;
    } while (bit_counter > 0);
  }
  return crc;
}

static rt_err_t max31826_init_by_ds2484(void)
{
  ds2484_dev = rt_device_find("ds2484");
  if(NULL == ds2484_dev)
  {
    LOG_E("ds2484 find NULL.");
    return -RT_ERROR;
  }

  if(rt_device_open(ds2484_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
  {
    LOG_E("ds2484 open fail.");
    return -RT_ERROR;
  }

  if(ds2484_dev->init(ds2484_dev) != RT_EOK)
  {
    LOG_E("ds2484 init fail.");
    rt_device_close(ds2484_dev);
    return -RT_ERROR;
  }
  return RT_EOK;
}

#endif /* MAX31826_USING_I2C_DS2484 */

/*
 * @brief 复位
 * @param 无
 */
static rt_int32_t DEV_MAX31826_Reset1Wire(void)
{
    rt_int32_t ret = 0xFF;

#ifdef MAX31826_USING_IO
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
#endif  /* MAX31826_USING_IO */

#ifdef MAX31826_USING_I2C_DS2484
    if(ds2484_dev != NULL)
    {
      if(ds2484_dev->control(ds2484_dev, DS2484_Control_Reset, NULL) != RT_EOK)
      {
        LOG_E("MAX31826 reset fail.");
        ret = 0;
      }
      else
      {
        ret = 1;
      }
    }
    else
    {
      LOG_E("MAX31826 reset ds2484_dev is null.");
      ret = 0;
    }
#endif /* MAX31826_USING_I2C_DS2484 */

    return ret;
}

/*****************************
函 数 名: DEV_MAX31826_WriteBit
功    能:MAX31826写入1位
参    数: -
返    回: -
******************************/
static void DEV_MAX31826_WriteBit(rt_uint8_t sendbit)
{
#ifdef MAX31826_USING_IO
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
#endif /* MAX31826_USING_IO */

#ifdef MAX31826_USING_I2C_DS2484
    rt_uint8_t send_data;

    send_data = sendbit;
    if(ds2484_dev != NULL)
    {
      if(ds2484_dev->control(ds2484_dev, DS2484_Control_Write_Byte, (void *)&send_data) != RT_EOK)
    }
    else
    {
      LOG_E("MAX31826 write ds2484_dev is null.");
    }
#endif /* MAX31826_USING_I2C_DS2484 */
}

/****************************
函 数 名: DEV_MAX31826_ReadBit
功    能:MAX31820读出1位
参    数: -
返    回: -
*****************************/
static rt_uint8_t DEV_MAX31826_ReadBit(void)
{
    rt_uint8_t readbit = 0;

#ifdef MAX31826_USING_IO

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(8);
    SET_DQ();
    rt_hw_us_delay(2);

    IN_DQ();
    rt_hw_us_delay(2);
    readbit = GET_DQ();
    rt_hw_us_delay(60);

#endif /* MAX31826_USING_IO */

#ifdef MAX31826_USING_I2C_DS2484
    if(ds2484_dev != NULL)
    {
      if(ds2484_dev->control(ds2484_dev, DS2484_Control_Read_Byte, (void *)&readbit) != RT_EOK)
      {
        LOG_E("MAX31826 read fail.");
      }
    }
    else
    {
      LOG_E("MAX31826 read ds2484_dev is null.");
    }
#endif /* MAX31826_USING_I2C_DS2484 */
    return readbit;
}

#ifdef MAX31826_USING_I2C_DS2484

static rt_int32_t DEV_MAX31826_ReadID(rt_uint8_t  * u8p_SensorID)
{
  rt_uint8_t i;
  rt_err_t ret = -1;

  U_DS31820ID u_SensorID;

  if(DEV_MAX31826_Reset1Wire())
  {
    DEV_MAX31826_WriteBit(MAX31826_CMD_READ_ROM);

    for(i = 0; i < 8; i ++)
    {
      u_SensorID.u8p_Data[i] = DEV_MAX31826_ReadBit();
    }

    if(crc8_create((char*)u_SensorID.u8p_Data,8) != 0)
    {
      ret = -1;
    }
    else
    {
      memcpy(u8p_SensorID,(char*)u_SensorID.u8p_Data,8);
      ret = 0;
    }
  }
  else
  {
    ret = -1;
  }

  DEV_MAX31826_WriteBit(MAX31826_CMD_SKIP_ROM);
  DEV_MAX31826_WriteBit(MAX31826_CMD_BEGINS_COVERSION);
  DEV_MAX31826_Reset1Wire();

  return ret;
}

int Max3182x_ReadTemp(rt_uint16_t * u16p_temp_value)
{
  rt_err_t e_return = -RT_ERROR;

  rt_uint8_t i;
  rt_uint8_t  u8_tempL = 0;
  rt_uint8_t  u8_tempH = 0;
  rt_uint16_t u16_Temp = 0;
  U_MAX31826Memory u_DS_mem;

  rt_thread_delay(118);

  /* 跳过读序号列号的操作 */
  DEV_MAX31826_WriteBit(MAX31826_CMD_SKIP_ROM);
  DEV_MAX31826_WriteBit(MAX31826_CMD_READ_SCRATCHPAD);

  /*
   *             Scratchpad Memory Layout
   *             Byte  Register
   *             0     Temperature_LSB
   *             1     Temperature_MSB
   *             2     Temp Alarm High / User Byte 1
   *             3     Temp Alarm Low / User Byte 2
   *             4     Reserved
   *             5     Reserved
   *             6     Count_Remain
   *             7     Count_per_C
   *             8     CRC
   */
  for(i = 0; i < 9; i++)
  { /* Scratchpad Memory */
    u_DS_mem.a_data[i] = DEV_MAX31826_ReadBit();
  }
  if(crc8_create(u_DS_mem.a_data, 9) != 0)
  {
    e_return = -RT_ERROR;
  }
  else
  {
    u8_tempL = u_DS_mem.s_Memory.TempL;
    u8_tempH = u_DS_mem.s_Memory.TempH;

    u16_Temp = (uint16_t)(u8_tempH & ((uint8_t)0x0F));
    u16_Temp <<= 8;
    u16_Temp |= (uint16_t)u8_tempL;
  /*
     *             Temperature calculation for MAX31826 (Family Code 0x28):
     *             =======================================================
     *                      bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
     *             LSB      2^3    2^2    2^1    2^0    2^-1   2^-2   2^-3   2^-4
     *                      bit15  bit14  bit13  bit12  bit3   bit2   bit1   bit0
     *             MSB      S      S      S      S      S      2^6    2^5    2^4
     */
    if((u8_tempH & 0x80) != 0x00)
    {/* 温度 为负值,取出温度值 */
      u16_Temp = (~(u16_Temp - 1)) & 0xFFF;
    }
    else
    {
        /* 空 */
    }


     if(u16p_temp_value != NULL)
     {
        *u16p_temp_value = u16_Temp*100/16;   //实际值*100
     }

    if((u8_tempH & 0x80) != 0x00)
    {/* 温度 为负值,将高位置1 */
      if(u16p_temp_value != NULL)
      {
        *u16p_temp_value |= 0x8000;
      }
    }
    else
    {
        /* 空  */
    }

    e_return = RT_EOK;
  }

  DEV_MAX31826_Reset1Wire();
  DEV_MAX31826_WriteBit(MAX31826_CMD_SKIP_ROM);
  DEV_MAX31826_WriteBit(MAX31826_CMD_BEGINS_COVERSION);
  DEV_MAX31826_Reset1Wire();
  return e_return;
}

/* RT-Thread Device Driver Interface */
static rt_size_t _max31826_polling_get_data(struct rt_sensor_device *sensor, struct rt_sensor_data *data)
{
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        rt_uint16_t s32Tmp = 0;

        Max3182x_ReadTemp(&s32Tmp);

        LOG_D("temp : %d", s32Tmp);
        data->data.temp = s32Tmp;
        data->timestamp = rt_tick_get_millisecond();
    }
    return 1;
}

#endif /* MAX31826_USING_I2C_DS2484 */

#ifdef MAX31826_USING_IO
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

/* crc8计算 ,多项式= x8+x6+x4+x3++x2+x1,0x5E,高位在前*/
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
返  回:成功:1   失败:返回其他值
*******************************************/
static rt_uint8_t DEV_MAX31826_ReadAck(void)
{
    rt_uint8_t readbit;

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(10); /* 短暂延时后拉高,设置为输入,采集温度传感器输出的电平 */
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
 * @param *id - 存放芯片ID的数据缓冲区指针.
 */
static rt_int32_t DEV_MAX31826_ReadID(rt_uint8_t *bufferid)
{
    rt_int32_t ret = -1;
    rt_uint8_t p_id[8] = {0};
    rt_uint8_t i, *p;

    /* 读ID */
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
            rt_hw_us_delay(100);//此处增加延时后,则可读取成功
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
    rt_int32_t temperature_x100;
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        rt_int32_t s32Tmp = TEMP_INVALID;
        float f32Max31826Tmp = 0.0f;

        s32Tmp = DEV_MAX31826_ReadTemp();
        f32Max31826Tmp = (0.0625f * s32Tmp);//单位:℃

        temperature_x100 = f32Max31826Tmp * 100;
        LOG_D("temp : %d", temperature_x100);
        data->data.temp = temperature_x100;
        data->timestamp = rt_tick_get_millisecond();
    }
    return 1;
}
#endif /* MAX31826_USING_IO */

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

/*******************************************************
 *
 * @brief  向温度传感器内部存储区域写入8个字节数据
 *
 * @param  offset: 内部存储区的起始偏移
 *         buf: 写缓冲区.
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int max31826_write_8bytes(rt_uint8_t offset, rt_uint8_t *buf)
{
  int e_return = -1;
  rt_uint8_t i;
  rt_uint8_t access_buffer[MAX31826_BLK_SIZE+3] = {0};

  /* 将要写入的字节放到数组中  */
  access_buffer[0] = MAX31826_WRITE_SCRATCHPAD2;
  access_buffer[1] = offset;
  rt_memcpy((void *)&access_buffer[2], (void *)buf, (uint16_t)8);

  /* 复位传感器,按照时序进行操作 */
  DEV_MAX31826_Reset1Wire();
  DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);

  for (i = (rt_uint8_t)0; i < (rt_uint8_t)MAX31826_BLK_SIZE+3-1; i++)
  {
    DEV_MAX31826_Write1Wire(access_buffer[i]);
  }
  /* 第十一个字节是传感器输出刚写入的10个字节的CRC */
  access_buffer[MAX31826_BLK_SIZE+3-1] = DEV_MAX31826_Read1Wire();

  /* 校验序列 */
  if (crc8_create(access_buffer, (uint16_t)MAX31826_BLK_SIZE+3, 0) == (rt_uint8_t)0)
  {
    /* 发送写入命令 */
    DEV_MAX31826_Reset1Wire();
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_COPY_SCRATCHPAD2);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_TOKEN);
    e_return = 0;
  }
  else
  {
    e_return = -1;
  }
  rt_thread_delay(20);/* 20ms */
  return (e_return);
}

static int fal_max31826_init(void)
{
    LOG_I("check succeed %d byte", max31826_flash.len);
    /* do nothing */
    return RT_EOK;
}
static int fal_max31826_erase(long offset, size_t size)
{
    /* do nothing */
    return RT_EOK;
}


/*******************************************************
 *
 * @brief  读取温度传感器内部存储区
 *
 * @param  offset:内部存储区的起始偏移;
 *         len:读取长度;
 *         buf:读取到数据存储的缓冲区
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fal_max31826_read(long offset,  rt_uint8_t *buf, size_t len)
{
  rt_uint8_t i;

  /* 复位传感器,按照时序进行操作 */
  DEV_MAX31826_Reset1Wire();
  DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
  DEV_MAX31826_Write1Wire(MAX31826_READ_MEMORY);
  DEV_MAX31826_Write1Wire((rt_uint8_t)offset);

  for (i = (rt_uint8_t)0; i < len; i++)
  {
    buf[i] = DEV_MAX31826_Read1Wire();
  }
  rt_thread_delay(20);
  return 0;
}
/*******************************************************
 *
 * @brief  向温度传感器内部存储区域写入指定长度数据
 *
 * @param  offset:内部存储区的起始偏移;
 *         len:缓冲区长度;
 *         buf:写缓冲区起始地址
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fal_max31826_write(long offset, const rt_uint8_t *buf, size_t len)
{
  rt_uint8_t a_temp_buf[MAX31826_BLK_SIZE];
  int e_return = -1;
  rt_uint8_t remain_len, write_len, offset_tmp, error_times = (rt_uint8_t)0;

  /* 起始地址是8的整数倍   */
  if ((offset % (rt_uint8_t)MAX31826_BLK_SIZE) != (rt_uint8_t)0)
  {
    return -1;
  }

  remain_len = len;
  offset_tmp = (rt_uint8_t)0;
  /* 关中断  */
  while (remain_len > (rt_uint8_t)0)
  {
    /* 每次拷贝8个字节 */
    memset(a_temp_buf, (rt_uint8_t)0, (uint16_t)8);
    if (remain_len > (rt_uint8_t)MAX31826_BLK_SIZE)
    {
      write_len = (rt_uint8_t)MAX31826_BLK_SIZE;
      memcpy((void *)a_temp_buf, (void *)&buf[offset_tmp], (uint16_t)MAX31826_BLK_SIZE);
    }
    else
    {
      /* 不足8个字节的补零 */
      write_len = remain_len;
      memcpy((void *)a_temp_buf, (void *)&buf[offset_tmp], (uint16_t)write_len);
    }

    if (max31826_write_8bytes((rt_uint8_t)(offset + offset_tmp), a_temp_buf) == 0/*ok*/)
    {
      remain_len = (rt_uint8_t)(remain_len - write_len);
      offset_tmp = (rt_uint8_t)(offset_tmp + write_len);
      error_times = (rt_uint8_t)0;
      e_return = 0;
      rt_hw_us_delay(30 * 1000); // 必要的延迟.
    }
    else
    {
      error_times++;
      /* 连续3次写不成功认为失败 */
      if (error_times > (rt_uint8_t)3)
      {
        e_return = -1;
        break;
      }
      else
      {
        /* 空 */
      }
    }
  }
  /* 开中断  */
  return (e_return);
}

static struct rt_sensor_ops sensor_ops =
    {
        max31826_fetch_data,
        max31826_control};

static int rt_hw_max31826_init()
{
  rt_int8_t result;
  rt_sensor_t sensor_temp = RT_NULL;

#ifdef MAX31826_USING_I2C_DS2484
  if (rt_hw_ds2484_init() != RT_EOK)
  {
    return -RT_ERROR;
  }
  if (max31826_init_by_ds2484() != RT_EOK)
  {
    LOG_E("max31826_init_by_ds2484 fail");
    return -RT_ERROR;
  }
#endif /* MAX31826_USING_I2C_DS2484 */

  /* temperature sensor register */
  sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
  if (sensor_temp == RT_NULL)
    return -RT_ERROR;

  sensor_temp->info.type = RT_SENSOR_CLASS_TEMP;
  sensor_temp->info.vendor = RT_SENSOR_VENDOR_MAXIM;
  sensor_temp->info.model = "max31826";
  sensor_temp->info.unit = RT_SENSOR_UNIT_DCELSIUS;
  sensor_temp->info.intf_type = RT_SENSOR_INTF_ONEWIRE;
  sensor_temp->info.range_max = 125;
  sensor_temp->info.range_min = -55;
  sensor_temp->info.period_min = 150;

  sensor_temp->ops = &sensor_ops;

  result = rt_hw_sensor_register(sensor_temp, "max31826", RT_DEVICE_FLAG_RDONLY, RT_NULL);
  if (result != RT_EOK)
  {
    LOG_E("device register err code: %d", result);
    goto __exit;
  }

#ifdef MAX31826_USING_IO
    DEV_MAX31826_ReadTemp();
#endif /* MAX31826_USING_IO */
    return RT_EOK;

__exit:
    rt_free(sensor_temp);
    return -RT_ERROR;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_max31826_init);

#endif
