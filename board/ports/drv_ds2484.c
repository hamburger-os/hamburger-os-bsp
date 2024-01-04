/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-25     zm       the first version
 */
#include "drv_ds2484.h"

#ifdef MAX31826_USING_I2C_DS2484

#define DBG_TAG "ds2484"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <string.h>

//定义DS2484的状态位
typedef enum
{
  DIR        =(uint8_t)0x80,
  TSB        =(uint8_t)0x40,
  SBR        =(uint8_t)0x20,
  RST        =(uint8_t)0x10,
  LL         =(uint8_t)0x08,
  SD         =(uint8_t)0x04,
  PPD        =(uint8_t)0x02,
  OWB        =(uint8_t)0x01
} Status_Bit;

//定义DS2484的配置寄存器
typedef enum
{
  Enable_Active_Pullup         =(uint8_t)0x01,
  Disable_Active_Pullup        =(uint8_t)0x0E,
  OW_Power_Down                =(uint8_t)0x02,
  OW_Power_On                  =(uint8_t)0x0D,
  Enable_Strong_Pullup         =(uint8_t)0x04,
  Disable_Strong_Pullup        =(uint8_t)0x0B,
  OW_Overdrive_Speed           =(uint8_t)0x08,
  OW_Standard_Speed            =(uint8_t)0x07
} Device_Config;

//定义DS2483的命令
typedef enum
{
  Device_RESET_Command         =(uint8_t)0xF0,
  Set_Read_Pointer_Command     =(uint8_t)0xE1,
  Write_Device_Conif_Command   =(uint8_t)0xD2,
  Adj_OW_Port_Command          =(uint8_t)0xC3,
  OW_RESET_Command             =(uint8_t)0xB4,
  OW_Single_Bit_Command        =(uint8_t)0x87,
  OW_Write_Byte_Command        =(uint8_t)0xA5,
  OW_Read_Byte_Command         =(uint8_t)0x96,
  OW_Triplet_Command           =(uint8_t)0x78
} Command_Def;

//定义DS2483的寄存器指针
typedef enum
{
  Device_Config_Register       =(uint8_t)0xC3,
  Status_Register              =(uint8_t)0xF0,
  Read_Data_Register           =(uint8_t)0xE1,
  Port_Config_Register         =(uint8_t)0xB4
} Register_Pointer;

typedef struct {
  struct rt_device dev;                 /* 设备 */
  struct rt_i2c_bus_device *i2c_bus;    /* I2C总线设备句柄 */
  rt_uint8_t temp;
  rt_uint8_t device_status;

} Ds2484_Dev;

static Ds2484_Dev ds2484_dev;

static rt_err_t write_data(rt_uint8_t command, rt_uint8_t *buf, rt_size_t len)
{
  rt_uint8_t send_buf[16+1];
  rt_uint8_t send_len;

  if(len > 16)
  {
    LOG_E("write data exceeding 16");
    return -RT_ERROR;
  }

  send_buf[0] = command;
  memcpy(send_buf + 1, buf, len);
  send_len = len + 1;

  if(send_len != rt_i2c_master_send(ds2484_dev.i2c_bus, DS2484_I2C_ADD, 0, send_buf, send_len))
  {
    LOG_E("i2c_master_send write error!");
    return -RT_ERROR;
  }
  return RT_EOK;
}

static rt_err_t read_data(rt_uint8_t *buf, rt_uint8_t len)
{
  if(len == rt_i2c_master_recv(ds2484_dev.i2c_bus, DS2484_I2C_ADD, 0, buf, len))
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

static rt_err_t ds2484_set_up(Device_Config device_config)
{
  uint8_t Get_Data = 0;
  uint8_t Data_Buffer[8];

  Data_Buffer[0] = Device_Config_Register;
  if(RT_EOK != write_data(Set_Read_Pointer_Command, Data_Buffer, 1))
  {
    LOG_E("ds2484_set_up write data error!");
    return -RT_ERROR;
  }

  if(RT_EOK != read_data(Data_Buffer, 1))
  {
    LOG_E("ds2484_set_up read data error!");
    return -RT_ERROR;
  }

  Get_Data = Data_Buffer[0];
  if(device_config == 0x07)
  {
    Get_Data &= 0x07;
    ds2484_dev.temp =~ Get_Data;
    Get_Data |= (ds2484_dev.temp << 4);
  }

  if(device_config == 0x08)
  {
    Get_Data |= 0x08;
    Get_Data &= 0x0F;
    ds2484_dev.temp =~ Get_Data;
    Get_Data |= (ds2484_dev.temp << 4);
  }

  if(device_config < 0x07)
  {
    Get_Data |= device_config;
    Get_Data &= 0x0F;
    ds2484_dev.temp =~ Get_Data;
    Get_Data |= (ds2484_dev.temp << 4);
  }

  if(device_config > 0x08)
  {
    Get_Data &= device_config;
    ds2484_dev.temp =~ Get_Data;
    Get_Data |= (ds2484_dev.temp << 4);
  }

  Data_Buffer[0] = Get_Data;
  if(RT_EOK != write_data(Write_Device_Conif_Command, Data_Buffer, 1))
  {
    LOG_E("ds2484_set_up %d error!", (rt_uint8_t)device_config);
    return -RT_ERROR;
  }
  else
  {
    return RT_EOK;
  }
}

/*******************************************************************************
  * @函数名称   rt_uint8_t ds2484_get_status(Status_Bit status_bit)
  * @函数说明   获取DS2484状态信息
  * @输入参数   status_bit：状态位
  * @输出参数   无
  * @返回参数   RT_EOK：获取成功，-RT_ERROR：获取失败
*******************************************************************************/
static rt_err_t ds2484_get_status(Status_Bit status_bit)
{
  uint8_t data_buffer[8];

  data_buffer[0] = Status_Register;
  if(RT_EOK != write_data(Set_Read_Pointer_Command, data_buffer, 1))
  {
    LOG_E("ds2484_get_status Set_Read_Pointer_Command error");
    return -RT_ERROR;
  }

  if(RT_EOK != read_data(data_buffer, 1))
  {
    LOG_E("ds2484_get_status read data error");
    return -RT_ERROR;
  }

  ds2484_dev.device_status = data_buffer[0];
  if((ds2484_dev.device_status & status_bit) == status_bit)
  {
//    LOG_E("status_bit = device status");
    return -RT_ERROR;
  }
  else
  {
    return RT_EOK;
  }
}

/*******************************************************************************
  * @函数名称   rt_uint8_t ds2484_ow_reset(void)
  * @函数说明   单总线复位
  * @输入参数   无
  * @输出参数   无
  * @返回参数   RT_EOK：获取成功，-RT_ERROR：获取失败
*******************************************************************************/
static rt_err_t ds2484_ow_reset(void)
{
  uint8_t data_buffer[8];

  while(ds2484_get_status(OWB));

  if(RT_EOK != write_data(OW_RESET_Command, data_buffer, 0))
  {
    LOG_E("ds2484_ow_reset OW_RESET_Command error");
    return -RT_ERROR;
  }

  while(ds2484_get_status(OWB));

  if(RT_EOK != read_data(data_buffer, 1))
  {
    LOG_E("ds2484_ow_reset read_data error");
    return -RT_ERROR;
  }

  ds2484_dev.temp = data_buffer[0];
  if(((ds2484_dev.temp & PPD) == PPD) && (!((ds2484_dev.temp & SD) == SD)))
  {
    return RT_EOK;
  }
  else
  {
    LOG_E("ds2484_ow_reset error");
    return -RT_ERROR;
  }
}

/*******************************************************************************
  * @函数名称   rt_err_t ds2484_ow_write_byte(uint8_t data)
  * @函数说明   单总线写操作
  * @输入参数   data：待写入数据
  * @输出参数   无
  * @返回参数   RT_EOK：获取成功，-RT_ERROR：获取失败
*******************************************************************************/
static rt_err_t ds2484_ow_write_byte(uint8_t data)
{
  uint8_t data_buffer[8];

  while(ds2484_get_status(OWB));

  data_buffer[0] = data;
  if(RT_EOK != write_data(OW_Write_Byte_Command, data_buffer, 1))
  {
    LOG_E("ds2484_ow_write_byte error");
    return -RT_ERROR;
  }

  while(ds2484_get_status(OWB));
  return RT_EOK;
}

/*******************************************************************************
  * @函数名称   rt_err_t ds2484_ow_read_byte(uint8_t *buff)
  * @函数说明   单总线读操作
  * @输入参数   无
  * @输出参数   buff：读出数据缓存
  * @返回参数   RT_EOK：获取成功，-RT_ERROR：获取失败
*******************************************************************************/
static rt_err_t ds2484_ow_read_byte(uint8_t *buff)
{
  uint8_t data_buffer[8];

  while(ds2484_get_status(OWB));

  if(RT_EOK != write_data(OW_Read_Byte_Command, data_buffer, 0))
  {
    LOG_E("OW_Read_Byte_Command write error");
    return -RT_ERROR;
  }

  while(ds2484_get_status(OWB));

  data_buffer[0] = Read_Data_Register;
  if(RT_EOK != write_data(Set_Read_Pointer_Command, data_buffer, 1))
  {
    LOG_E("Set_Read_Pointer_Command write error");
    return -RT_ERROR;
  }

  if(RT_EOK != read_data(data_buffer, 1))
  {
    LOG_E("ds2484_ow_read_byte error");
    return -RT_ERROR;
  }

  *buff = data_buffer[0];

  return RT_EOK;
}

static rt_err_t rt_ds2484_init(rt_device_t dev)
{
  rt_uint8_t data_buffer[5];

  /* 查找I2C总线设备，获取I2C总线设备句柄 */
  ds2484_dev.i2c_bus = rt_i2c_bus_device_find(DS2484_I2C_DEV);
  if (ds2484_dev.i2c_bus == NULL)
  {
    LOG_E("i2c bus is not find!");
    return -RT_EIO;
  }

  if(RT_EOK != write_data(Device_RESET_Command, data_buffer, 0))       /* 复位DS2484 */
  {
    LOG_E("ds2484 reset error");
    return -RT_ERROR;
  }

  if(RT_EOK != ds2484_set_up(OW_Standard_Speed))
  {
    LOG_E("ds2484 OW_Standard_Speed error");
    return -RT_ERROR;
  }

  if(RT_EOK != ds2484_set_up(Disable_Strong_Pullup))
  {
    LOG_E("ds2484 Disable_Strong_Pullup error");
    return -RT_ERROR;
  }

  if(RT_EOK != ds2484_set_up(OW_Power_On))
  {
    LOG_E("ds2484 OW_Power_On error");
    return -RT_ERROR;
  }

  data_buffer[0] = 0x06;
  data_buffer[1] = 0x26;
  data_buffer[2] = 0x46;
  data_buffer[3] = 0x66;
  data_buffer[4] = 0x86;

  if(RT_EOK != write_data(Adj_OW_Port_Command, data_buffer, 5))
  {
    LOG_E("ds2484 Adj_OW_Port_Command error");
    return -RT_ERROR;
  }

  return RT_EOK;
}

static rt_err_t rt_ds2484_control(rt_device_t dev, int cmd, void *args)
{
  rt_err_t ret = -RT_ERROR;

  if(NULL == dev)
  {
    LOG_E("ds2484 control dev is null");
    return -RT_ERROR;
  }

  switch(cmd)
  {
    case DS2484_Control_Reset:         /* 复位 */
      ret = ds2484_ow_reset();
      break;

    case DS2484_Control_Write_Byte:   /* 写一个字节数据 */
      ret = ds2484_ow_write_byte((*(uint8_t *)args));
      break;

    case DS2484_Control_Read_Byte:   /* 读一个字节数据 */
      ret = ds2484_ow_read_byte((uint8_t *)args);
      break;

    default:
      LOG_E("ds2484 control dev is null");
      ret = -RT_ERROR;
      break;
  }
  return ret;
}

int rt_hw_ds2484_init(void)
{
  memset(&ds2484_dev, 0, sizeof(Ds2484_Dev));

  ds2484_dev.dev.type = RT_Device_Class_I2CBUS;
  ds2484_dev.dev.init = rt_ds2484_init;
  ds2484_dev.dev.control = rt_ds2484_control;
  if(rt_device_register(&ds2484_dev.dev, "ds2484", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
  {
    LOG_I("ds2484 dev init successfully!");
  }
  else
  {
    LOG_E("ds2484 dev init failed!");
    return -RT_ERROR;
  }
  return RT_EOK;
}

#endif
