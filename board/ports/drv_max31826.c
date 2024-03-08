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

#ifdef RT_USING_SENSOR
#include <drivers/sensor.h>
#else
#include <drivers/sensor_v2.h>
#endif

#include "fal.h"

#define DBG_TAG "max31826"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef MAX31826_USING_IO

#define MAX31826_PIN    rt_pin_get(MAX31826_GPIO)
static rt_base_t max31826_pin = 0;

inline static void SET_DQ(void)
{
    if (max31826_pin == 0)
    {
        max31826_pin = MAX31826_PIN;
    }
    rt_pin_write(max31826_pin, PIN_HIGH);
}
inline static void CLR_DQ(void)
{
    if (max31826_pin == 0)
    {
        max31826_pin = MAX31826_PIN;
    }
    rt_pin_write(max31826_pin, PIN_LOW);
}
inline static void OUT_DQ(void)
{
    if (max31826_pin == 0)
    {
        max31826_pin = MAX31826_PIN;
    }
    rt_pin_mode(max31826_pin, PIN_MODE_OUTPUT_OD);
}
inline static void IN_DQ(void)
{
    if (max31826_pin == 0)
    {
        max31826_pin = MAX31826_PIN;
    }
    rt_pin_mode(max31826_pin, PIN_MODE_INPUT);
}
inline static int GET_DQ(void)
{
    if (max31826_pin == 0)
    {
        max31826_pin = MAX31826_PIN;
    }
    return rt_pin_read(max31826_pin);
}

#endif  /* MAX31826_USING_IO */

/* ***************************MAX31826 DEFININS****************************************/
#define MAX31826_CMD_READ_ROM               ((rt_uint8_t)0x33)
#define MAX31826_CMD_SKIP_ROM               ((rt_uint8_t)0xCC)
#define MAX31826_CMD_MATCH_ROM              ((rt_uint8_t)0x55)
#define MAX31826_CMD_SERCH_ROM              ((rt_uint8_t)0xF0)

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

#define TEMP_INVALID                        (-12500)            /*  无效的温度数据 */
#define MAX31826_MAX_NUM                    8

static struct rt_mutex max31826_mux;    /** 读写互斥信号量 */
static struct rt_mutex fal_mux;         /** 读写互斥信号量 */

static int fal_max31826_init(void);
static int fal_max31826_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_max31826_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_max31826_erase(long offset, size_t size);

static void DEV_MAX31826_Write1Wire(rt_uint8_t data);
static rt_uint8_t DEV_MAX31826_Read1Wire(void);

static uint8_t LastDiscrepancy = 0;
static uint8_t LastDeviceFlag = 0;
static uint8_t LastFamilyDiscrepancy = 0;

typedef struct
{
    struct rt_sensor_device sensor;
    const char *dev_name;
    rt_uint8_t rom_id[8];
    rt_uint8_t address;
}MAX31826_SEN_DEV;


#ifdef MAX31826_USING_I2C_DS2484

static MAX31826_SEN_DEV max31826_sen[] = {

#if MAX31826_SEN_ALL > 0
    {
        .dev_name = "max31826_1",
        .address = 0xFF,
    },
#endif

#if MAX31826_SEN_ALL > 1
    {
        .dev_name = "max31826_2",
        .address = 0xFF,
    },
#endif

#if MAX31826_SEN_ALL > 2
    {
        .dev_name = "max31826_3",
        .address = 0xFF,
    },
#endif

#if MAX31826_SEN_ALL > 3
    {
        .dev_name = "max31826_4",
        .address = 0xFF,
    },
#endif

};

const struct fal_flash_dev max31826_flash = {
    .name = "max31826",
    .addr = MAX31826_START_ADRESS,
    .len = MAX31826_SIZE_GRANULARITY_TOTAL*MAX31826_SEN_ALL,
    .blk_size = MAX31826_BLK_SIZE,
    .ops = {fal_max31826_init, fal_max31826_read, fal_max31826_write, fal_max31826_erase},
    .write_gran = 0,
};


#endif


#ifdef MAX31826_USING_IO

const struct fal_flash_dev max31826_flash = {
    .name = "max31826",
    .addr = MAX31826_START_ADRESS,
    .len = MAX31826_SIZE_GRANULARITY_TOTAL,
    .blk_size = MAX31826_BLK_SIZE,
    .ops = {fal_max31826_init, fal_max31826_read, fal_max31826_write, fal_max31826_erase},
    .write_gran = 0,
};

#endif

static rt_uint8_t max31826_id_temp[MAX31826_SEN_ALL][8]={0};

#ifdef MAX31826_USING_I2C_DS2484

#include "drv_ds2484.h"

typedef union __attribute__ ((packed))
{
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
    __packed struct
#elif defined ( __GNUC__ ) /* GNU Compiler */
    struct
#endif
    {
        uint8_t FamilyCode;
        uint8_t SerialCode[6];
        uint8_t CRC_byte;
    } Code;
    uint8_t u8p_Data[8];
} U_DS31820ID;

typedef union __attribute__ ((packed))
{
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
    __packed struct
#elif defined ( __GNUC__ ) /* GNU Compiler */
    struct
#endif
    {
        uint8_t TempL; /*温度低字节*/
        uint8_t TempH; /*温度的高字节*/
        uint8_t User1; /*用户存储区1*/
        uint8_t User2; /*用户存储区2*/
        uint8_t Confg; /*配置存储区*/
        uint8_t ResFF; /*预留*/
        uint8_t Res; /*预留*/
        uint8_t Res10; /*预留*/
        uint8_t CRC_byte; /*以上字节的CRC8 */
    } s_Memory;
    uint8_t a_data[9]; /*数据*/
} U_MAX31826Memory;

static rt_device_t ds2484_dev = NULL;

static rt_err_t max31826_init_by_ds2484(void)
{
    ds2484_dev = rt_device_find("ds2484");
    if (NULL == ds2484_dev)
    {
        LOG_E("ds2484 find NULL.");
        return -RT_ERROR;
    }

    if (rt_device_open(ds2484_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("ds2484 open fail.");
        return -RT_ERROR;
    }

    if (ds2484_dev->init(ds2484_dev) != RT_EOK)
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
    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    rt_int32_t ret = 0xFF;

#ifdef MAX31826_USING_IO
//    rt_base_t level;
//    level = rt_hw_interrupt_disable();/* 关中断*/
    rt_enter_critical();

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
    rt_hw_us_delay(300);
//    rt_hw_interrupt_enable(level); /* 开中断 */
    rt_exit_critical();
#endif  /* MAX31826_USING_IO */

#ifdef MAX31826_USING_I2C_DS2484
    if (ds2484_dev != NULL)
    {
        if (ds2484_dev->control(ds2484_dev, DS2484_Control_Reset, NULL) != RT_EOK)
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

    rt_mutex_release(&max31826_mux);
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
    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);

#ifdef MAX31826_USING_IO
//    rt_base_t level;
//    level = rt_hw_interrupt_disable();/* 关中断*/
    rt_enter_critical();

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(2);
    if (sendbit == 0x01)
    {
        SET_DQ();
    }
    rt_hw_us_delay(65);
    SET_DQ();
    rt_hw_us_delay(15);
//    rt_hw_interrupt_enable(level); /* 开中断 */
    rt_exit_critical();
#endif /* MAX31826_USING_IO */

    rt_mutex_release(&max31826_mux);
}

/****************************
 函 数 名: DEV_MAX31826_ReadBit
 功    能:MAX31820读出1位
 参    数: -
 返    回: -
 *****************************/
static rt_uint8_t DEV_MAX31826_ReadBit(void)
{
    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    rt_uint8_t readbit = 0;

#ifdef MAX31826_USING_IO
//    rt_base_t level;
//    level = rt_hw_interrupt_disable();/* 关中断*/
    rt_enter_critical();

    OUT_DQ();
    CLR_DQ();
    rt_hw_us_delay(2);
    IN_DQ();
    rt_hw_us_delay(2);
    readbit = GET_DQ();
    rt_hw_us_delay(60);
//    rt_hw_interrupt_enable(level); /* 开中断 */
    rt_exit_critical();
#endif /* MAX31826_USING_IO */

    rt_mutex_release(&max31826_mux);
    return readbit;
}

#define CRC8INIT    0x00
#define CRC8POLY    0x18              //0X18 = X^8+X^5+X^4+X^0

static rt_uint8_t crc8_create(rt_uint8_t *data, rt_uint16_t number_of_bytes_in_data)
{
    uint8_t crc;
    uint16_t loop_count;
    uint8_t bit_counter;
    uint8_t b;
    uint8_t feedback_bit;

    crc = CRC8INIT;
    for (loop_count = 0; loop_count != number_of_bytes_in_data; loop_count++)
    {
        b = data[loop_count];

        bit_counter = 8;
        do
        {
            feedback_bit = (crc ^ b) & 0x01;
            if (feedback_bit == 0x01)
            {
                crc = crc ^ CRC8POLY;
            }
            crc = (crc >> 1) & 0x7F;
            if (feedback_bit == 0x01)
            {
                crc = crc | 0x80;
            }
            b = b >> 1;
            bit_counter--;
        } while (bit_counter > 0);
    }
    return crc;
}

#ifdef MAX31826_USING_I2C_DS2484

/* 挂载单个才能使用 */
static rt_int32_t DEV_MAX31826_ReadID(rt_uint8_t * u8p_SensorID)
{
    rt_uint8_t i;
    rt_err_t ret = -1;

    U_DS31820ID u_SensorID;

    if (DEV_MAX31826_Reset1Wire())
    {
        DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_ROM);

        for (i = 0; i < 8; i++)
        {
            u_SensorID.u8p_Data[i] = DEV_MAX31826_Read1Wire();
        }

        if (crc8_create((uint8_t *) u_SensorID.u8p_Data, 8) != 0)
        {
            LOG_E("read id err\r\n");
            ret = -1;
        }
        else
        {
            LOG_E("read id ok\r\n");
            LOG_E("ID = %x %x %x %x %x %x %x %x", u_SensorID.u8p_Data[0], u_SensorID.u8p_Data[1], u_SensorID.u8p_Data[2], u_SensorID.u8p_Data[3], u_SensorID.u8p_Data[4], u_SensorID.u8p_Data[5], u_SensorID.u8p_Data[6], u_SensorID.u8p_Data[7]);
            rt_memcpy(u8p_SensorID, u_SensorID.u8p_Data, 8);
            ret = 0;
        }
    }
    else
    {
        ret = -1;
    }

    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_BEGINS_COVERSION);
    DEV_MAX31826_Reset1Wire();

    return ret;
}

int Max3182x_ReadTemp(rt_uint16_t * u16p_temp_value, struct rt_sensor_device *sensor)
{
    rt_err_t e_return = -RT_ERROR;

    rt_uint8_t i;
    rt_uint8_t u8_tempL = 0;
    rt_uint8_t u8_tempH = 0;
    rt_uint16_t u16_Temp = 0;
    U_MAX31826Memory u_DS_mem;

    MAX31826_SEN_DEV *pSenDev = RT_NULL;
    pSenDev = (MAX31826_SEN_DEV*)sensor;

    rt_thread_delay(118);

    DEV_MAX31826_Write1Wire (MAX31826_CMD_MATCH_ROM);
    for(i = 0; i < 8; i++)
    {
        DEV_MAX31826_Write1Wire(pSenDev->rom_id[i]);
    }
    DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_SCRATCHPAD);


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
    for (i = 0; i < 9; i++)
    { /* Scratchpad Memory */
        u_DS_mem.a_data[i] = DEV_MAX31826_Read1Wire();
    }
    if (crc8_create(u_DS_mem.a_data, 9) != 0)
    {
        e_return = -RT_ERROR;
    }
    else
    {
        u8_tempL = u_DS_mem.s_Memory.TempL;
        u8_tempH = u_DS_mem.s_Memory.TempH;

        u16_Temp = (uint16_t) (u8_tempH & ((uint8_t) 0x0F));
        u16_Temp <<= 8;
        u16_Temp |= (uint16_t) u8_tempL;
        /*
         *             Temperature calculation for MAX31826 (Family Code 0x28):
         *             =======================================================
         *                      bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
         *             LSB      2^3    2^2    2^1    2^0    2^-1   2^-2   2^-3   2^-4
         *                      bit15  bit14  bit13  bit12  bit3   bit2   bit1   bit0
         *             MSB      S      S      S      S      S      2^6    2^5    2^4
         */
        if ((u8_tempH & 0x80) != 0x00)
        {/* 温度 为负值,取出温度值 */
            u16_Temp = (~(u16_Temp - 1)) & 0xFFF;
        }
        else
        {
            /* 空 */
        }

        if (u16p_temp_value != NULL)
        {
            *u16p_temp_value = u16_Temp * 100 / 16;   //实际值*100
        }

        if ((u8_tempH & 0x80) != 0x00)
        {/* 温度 为负值,将高位置1 */
            if (u16p_temp_value != NULL)
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

    DEV_MAX31826_Write1Wire(MAX31826_CMD_MATCH_ROM);
    for(i = 0; i < 8; i++)
    {
        DEV_MAX31826_Write1Wire(pSenDev->rom_id[i]);
    }

    DEV_MAX31826_Write1Wire(MAX31826_CMD_BEGINS_COVERSION);

    DEV_MAX31826_Reset1Wire();
    return e_return;
}

/* RT-Thread Device Driver Interface */
static rt_size_t _max31826_polling_get_data(struct rt_sensor_device *sensor, struct rt_sensor_data *data)
{
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        rt_uint16_t s32Tmp = 0;

        Max3182x_ReadTemp(&s32Tmp, sensor);

        LOG_D("temp : %d", s32Tmp);
        data->data.temp = s32Tmp;
        data->timestamp = rt_tick_get_millisecond();
    }
    return 1;
}

static rt_int32_t DEV_MAX31826_Triplet1Wire(rt_uint8_t data)
{
    rt_uint8_t send_data;

    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    send_data = data;
    if (ds2484_dev != NULL)
    {
        if (ds2484_dev->control(ds2484_dev, DS2484_Control_Triplet, (void *)&send_data) != RT_EOK)
        {
            LOG_E("MAX31826 triplet cmd fail.");
            return -1;
        }
    }
    else
    {
        LOG_E("MAX31826 write ds2484_dev is null.");
        return -1;
    }
    rt_mutex_release(&max31826_mux);
    return 0;
}

/* 只读取数据,用于遍历id */
static rt_uint8_t DEV_MAX31826_TripletRead1Wire(void)
{
    rt_uint8_t readdata = 0;

    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    if (ds2484_dev != NULL)
    {
        if (ds2484_dev->control(ds2484_dev, DS2484_Control_Read, (void *)&readdata) != RT_EOK)
        {
            LOG_E("MAX31826 triplet cmd fail.");
        }
    }
    else
    {
        LOG_E("MAX31826 write ds2484_dev is null.");
    }
    rt_mutex_release(&max31826_mux);
    return readdata;
}

/* 遍历所有MAX31826的ID */
static rt_int32_t max31826_search_rom(void)
{
    uint8_t i,j;
    uint8_t Data_Buffer;
    uint8_t Sensor_ID[8];
    uint8_t id_bit_number;
    uint8_t last_zero,rom_byte_number;
    uint8_t rom_byte_mask,search_direction;
    uint8_t id_bit, cmp_id_bit;

    LastDiscrepancy = 0;
    LastDeviceFlag = 0;
    LastFamilyDiscrepancy = 0;

    for(i = 0; i < MAX31826_SEN_ALL; i++)
    {
        id_bit_number = 1;
        last_zero = 0;
        rom_byte_number = 0;
        rom_byte_mask = 1;

        if (DEV_MAX31826_Reset1Wire())
        {
            DEV_MAX31826_Write1Wire(MAX31826_CMD_SERCH_ROM);
            do
            {
                //read a bit and its complement
                if(id_bit_number < LastDiscrepancy)
                {
                  search_direction = (((Sensor_ID[rom_byte_number]) & rom_byte_mask) > 0);
                }
                else
                {
                  search_direction = (id_bit_number == LastDiscrepancy);
                }
                if(search_direction)
                    Data_Buffer = 0x80;
                else
                    Data_Buffer = 0x00;
                if(0 != DEV_MAX31826_Triplet1Wire(Data_Buffer))
                    return -1;
                Data_Buffer = DEV_MAX31826_TripletRead1Wire();

                id_bit = ((Data_Buffer & 0x20) > 0);
                cmp_id_bit = ((Data_Buffer & 0x40) > 0);
                search_direction = ((Data_Buffer & 0x80) > 0);
                if((id_bit == 1) && (cmp_id_bit == 1))
                    return -1;
                //if 0 was picked then record this position in LastZero
                if((!id_bit) && (!cmp_id_bit) && (search_direction == 0))
                {
                    last_zero = id_bit_number;
                    //check for Last discrepancy in family
                    if(last_zero < 9)
                        LastFamilyDiscrepancy = last_zero;
                }
                if(search_direction == 1)
                    Sensor_ID[rom_byte_number] |= rom_byte_mask;
                else
                    Sensor_ID[rom_byte_number] &=~rom_byte_mask;
                //increment the byte counter id_bit_number;
                //and shift the mask rom_byte_mask;
                id_bit_number++;
                rom_byte_mask <<= 1;

                //if the mask is 0 then go to new byte rom_byte_number and reset mask
                if(rom_byte_mask == 0)
                {
                  rom_byte_number++;
                  rom_byte_mask = 1;
                }

            }
            //loop untill through all rom byte 0~7
            while(rom_byte_number < 8);

            if(!(id_bit_number < 65))
            {
              LastDiscrepancy = last_zero;
              if(LastDiscrepancy == 0)
                  LastDeviceFlag = 1;
            }

            if (Sensor_ID[7] != crc8_create((rt_uint8_t *)Sensor_ID, 7))
            {
                LOG_E("STEM1 ERR, i = %d\r\n", i);
                LOG_E("ID = %x %x %x %x %x %x %x %x", Sensor_ID[0], Sensor_ID[1], Sensor_ID[2], Sensor_ID[3], Sensor_ID[4], Sensor_ID[5], Sensor_ID[6], Sensor_ID[7]);
            }
            else
            {
                for(j = 0; j < i; j++)
                {
                    if(0 == rt_memcmp(Sensor_ID, max31826_id_temp[j], 8))
                    {
                        LOG_E("STEM NUMBER ERR\r\n");
                        return -1;
                    }
                }
                rt_memcpy((void*)max31826_id_temp[i], (void*)Sensor_ID, 8);
            }
        }
        else
        {
            LOG_E("STEM ERR\r\n");
            LastDiscrepancy = 0;
            LastDeviceFlag = 0;
            LastFamilyDiscrepancy = 0;
            return -1;
        }
    }
    return 0;
}


/* ID与设备地址匹配 */
static rt_int32_t max31826_match_rom(void)
{
    rt_uint8_t i,j,k;
    rt_err_t ret = 0;
    rt_uint8_t address = 0xff;
    rt_uint8_t data[9];
    
    for(i = 0; i < MAX31826_SEN_ALL; i++)
    {
        if (DEV_MAX31826_Reset1Wire())
        {
            DEV_MAX31826_Write1Wire(MAX31826_CMD_MATCH_ROM);

            for (j = 0; j < 8; j++)
            {
                DEV_MAX31826_Write1Wire(max31826_id_temp[i][j]);
            }

            DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_SCRATCHPAD);

            for(j = 0; j < 9; j++)
            {
                data[j] = DEV_MAX31826_Read1Wire();
            }

            if (crc8_create((rt_uint8_t *)data, 9) != 0)
            {
                ret = -1;
            }
            else
            {
                address = data[4] & 0x0f;//地址只有低四位有效
                LOG_I("Address is %d", address);
                LOG_I("ID = %x %x %x %x %x %x %x %x", max31826_id_temp[i][0], max31826_id_temp[i][1], max31826_id_temp[i][2], max31826_id_temp[i][3], max31826_id_temp[i][4], max31826_id_temp[i][5], max31826_id_temp[i][6], max31826_id_temp[i][7]);

                if(i == 0)
                {
                    max31826_sen[0].address = address;
                    rt_memcpy(max31826_sen[0].rom_id, max31826_id_temp[0], 8);
                }

                for(j = 0; j < i; j++)
                {
                    if(address < max31826_sen[j].address)
                    {
                        for(k = i; k != j; k--)
                        {
                            max31826_sen[k].address = max31826_sen[k-1].address;
                            rt_memcpy(max31826_sen[k].rom_id, max31826_sen[k-1].rom_id, 8);
                        }
                        max31826_sen[j].address = address;
                        rt_memcpy(max31826_sen[j].rom_id, max31826_id_temp[i], 8);
                        break;
                    }
                    else if(address > max31826_sen[j].address)
                    {
                        if((j + 1) == i)
                        {
                            max31826_sen[j+1].address = address;
                            rt_memcpy(max31826_sen[j+1].rom_id, max31826_id_temp[i], 8);
                        }
                    }
                    else
                    {
                        return -1;
                    }
                }
            }
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}

static void DEV_MAX31826_Convert(void)
{
    DEV_MAX31826_Reset1Wire();
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
    DEV_MAX31826_Write1Wire(MAX31826_CMD_BEGINS_COVERSION);
}


#endif /* MAX31826_USING_I2C_DS2484 */
/*
 * @brief 读数据
 * @param 无
 */
static rt_uint8_t DEV_MAX31826_Read1Wire(void)
{
    rt_uint8_t readdata = 0x00;
    rt_uint16_t setcontrol = 0x0001;

#ifdef MAX31826_USING_IO
    while (setcontrol <= 0x0080)
    {
        if (DEV_MAX31826_ReadBit())
        {
            readdata |= setcontrol;
        }
        setcontrol = setcontrol << 1;
    }
#endif

#ifdef MAX31826_USING_I2C_DS2484

    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    if (ds2484_dev != NULL)
    {
        if (ds2484_dev->control(ds2484_dev, DS2484_Control_Read_Byte, (void *) &readdata) != RT_EOK)
        {
            LOG_E("MAX31826 read fail.");
        }
    }
    else
    {
        LOG_E("MAX31826 read ds2484_dev is null.");
    }
    rt_mutex_release(&max31826_mux);
#endif /* MAX31826_USING_I2C_DS2484 */
    return readdata;
}

/*
 * @brief 写数据
 * @param data - 待写入数据
 */
static void DEV_MAX31826_Write1Wire(rt_uint8_t data)
{
    rt_uint8_t i;
    rt_uint8_t temp;

#ifdef MAX31826_USING_IO
    for (i = 0x00; i < 0x08; i++)
    {
        temp = data >> i;
        DEV_MAX31826_WriteBit(temp & 0x01);
    }
#endif

#ifdef MAX31826_USING_I2C_DS2484
    rt_uint8_t send_data;

    rt_mutex_take(&max31826_mux, RT_WAITING_FOREVER);
    send_data = data;
    if (ds2484_dev != NULL)
    {
        if (ds2484_dev->control(ds2484_dev, DS2484_Control_Write_Byte, (void *) &send_data) != RT_EOK)
        {
            LOG_E("MAX31826 write fail.");
        }
    }
    else
    {
        LOG_E("MAX31826 write ds2484_dev is null.");
    }
    rt_mutex_release(&max31826_mux);

#endif /* MAX31826_USING_I2C_DS2484 */
}

#ifdef MAX31826_USING_IO
/*
 * @brief 读芯片ID
 * @param *id - 存放芯片ID的数据缓冲区指针.
 */
static rt_int32_t DEV_MAX31826_ReadID(rt_uint8_t *bufferid)
{
    rt_int32_t ret = -1;
    rt_uint8_t i;

    /* 读ID */
    if(DEV_MAX31826_Reset1Wire())
    {
        DEV_MAX31826_Write1Wire(MAX31826_CMD_READ_ROM);

        /* 读ID */
        for(i = 0; i < 8; i++)
        {
            bufferid[i] = DEV_MAX31826_Read1Wire();
            rt_hw_us_delay(100);   //此处增加延时后,则可读取成功
        }

        /* 检查结果 */
        i = bufferid[7];
        if(i == crc8_create(bufferid, 7))
        {
            ret = (rt_int32_t)0;
        }
        else
        {
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
    if(temp1 == crc8_create(buf, 8))
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
        f32Max31826Tmp = (0.0625f * s32Tmp);   //单位:℃

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
#ifdef MAX31826_USING_IO
            if (DEV_MAX31826_ReadID(args) != 0)
            {
                ret = -RT_EIO;
            }
#endif

#ifdef MAX31826_USING_I2C_DS2484
            MAX31826_SEN_DEV *pSenDev = RT_NULL;
            pSenDev = (MAX31826_SEN_DEV*)sensor;
            rt_memcpy(args, &pSenDev->rom_id, 8);
#endif
        }
        break;
    default:
        break;
    }

    LOG_D("control %d 0x%x", ret, cmd);
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
int max31826_write_8bytes(rt_uint32_t offset, rt_uint8_t *buf)
{
    int e_return = -1;
    rt_uint8_t i;
    rt_uint8_t chip;
    rt_uint8_t access_buffer[MAX31826_BLK_SIZE + 3] = { 0 };

    /* 将要写入的字节放到数组中  */
    access_buffer[0] = MAX31826_WRITE_SCRATCHPAD2;
#ifdef MAX31826_USING_IO
    access_buffer[1] = offset;
#endif

#ifdef MAX31826_USING_I2C_DS2484
    chip = offset/128;
    access_buffer[1] = offset%128;
#endif
    rt_memcpy((void *) &access_buffer[2], (void *) buf, (uint16_t) 8);

    /* 复位传感器,按照时序进行操作 */
    DEV_MAX31826_Reset1Wire();
#ifdef MAX31826_USING_IO
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
#endif

#ifdef MAX31826_USING_I2C_DS2484
    DEV_MAX31826_Write1Wire(MAX31826_CMD_MATCH_ROM);
    for (i = 0; i < 8; i++)
    {
        DEV_MAX31826_Write1Wire(max31826_sen[chip].rom_id[i]);
    }
#endif

    for (i = (rt_uint8_t) 0; i < (rt_uint8_t) MAX31826_BLK_SIZE + 3 - 1; i++)
    {
        DEV_MAX31826_Write1Wire(access_buffer[i]);
    }
    /* 第十一个字节是传感器输出刚写入的10个字节的CRC */
    access_buffer[MAX31826_BLK_SIZE + 3 - 1] = DEV_MAX31826_Read1Wire();

    /* 校验序列 */
    if (crc8_create(access_buffer, (uint16_t) MAX31826_BLK_SIZE + 3) == (rt_uint8_t) 0)
    {
        /* 发送写入命令 */
        DEV_MAX31826_Reset1Wire();
#ifdef MAX31826_USING_IO
        DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
#endif

#ifdef MAX31826_USING_I2C_DS2484
    DEV_MAX31826_Write1Wire(MAX31826_CMD_MATCH_ROM);
    for (i = 0; i < 8; i++)
    {
        DEV_MAX31826_Write1Wire(max31826_sen[chip].rom_id[i]);
    }
#endif
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
    uint8_t p_id[7];
#ifdef MAX31826_USING_IO
    if (DEV_MAX31826_ReadID(p_id) == 0)
    {
        LOG_I("check succeed %d byte", max31826_flash.len);
        return RT_EOK;
    }
    else
    {
        LOG_E("check failed id : %x %x %x %x %x %x"
                ,p_id[0], p_id[1], p_id[2], p_id[3], p_id[4], p_id[5], p_id[6]);
        return -RT_ERROR;
    }
#endif

#ifdef MAX31826_USING_I2C_DS2484
    return RT_EOK;
#endif
}

int fal_max31826_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr;
    uint8_t chip;
    uint8_t findflag = RT_FALSE;

    rt_mutex_take(&fal_mux, RT_WAITING_FOREVER);

#ifdef MAX31826_USING_I2C_DS2484

    if(MAX31826_SEN_ALL != 0)
    {
        if((offset < 128*MAX31826_SEN_ALL) && (offset/128 == (offset + size -1)/128))
        {
            addr = offset%128;
            findflag = RT_TRUE;
            chip = offset/128;
        }
    }
    if(findflag != RT_TRUE)
        return -RT_EINVAL;

#endif

#ifdef MAX31826_USING_IO
    addr = max31826_flash.addr + offset;
    if (addr + size > max31826_flash.addr + max31826_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        rt_mutex_release(&fal_mux);
        return -RT_EINVAL;
    }
#endif

    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        rt_mutex_release(&fal_mux);
        return 0;
    }

    /* 复位传感器,按照时序进行操作 */
    DEV_MAX31826_Reset1Wire();
    rt_thread_delay(100);/* 非常有必要 */
#ifdef MAX31826_USING_IO
    DEV_MAX31826_Write1Wire(MAX31826_CMD_SKIP_ROM);
#endif

#ifdef MAX31826_USING_I2C_DS2484
    DEV_MAX31826_Write1Wire(MAX31826_CMD_MATCH_ROM);
    for (uint8_t k = 0; k < 8; k++)
    {
        DEV_MAX31826_Write1Wire(max31826_sen[chip].rom_id[k]);
    }
#endif
    DEV_MAX31826_Write1Wire(MAX31826_READ_MEMORY);
    DEV_MAX31826_Write1Wire((rt_uint8_t) addr);

    for (uint8_t i = 0; i < size; i++)
    {
        buf[i] = DEV_MAX31826_Read1Wire();
    }
    DEV_MAX31826_Reset1Wire();

    LOG_HEX("rd", 16, buf, size);
    LOG_D("read (0x%p) %d", (void*)(addr), size);
    rt_mutex_release(&fal_mux);
    return size;
}

int fal_max31826_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr;
    uint8_t findflag = RT_FALSE;

    rt_mutex_take(&fal_mux, RT_WAITING_FOREVER);
#ifdef MAX31826_USING_I2C_DS2484

    if(MAX31826_SEN_ALL != 0)
    {
        if((offset < 128*MAX31826_SEN_ALL) && (offset/128 == (offset + size -1)/128))
        {
            addr = offset;
            findflag = RT_TRUE;
        }
    }
    if(findflag != RT_TRUE)
        return -RT_EINVAL;

#endif

#ifdef MAX31826_USING_IO
    addr = max31826_flash.addr + offset;
    if (addr + size > max31826_flash.addr + max31826_flash.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        rt_mutex_release(&fal_mux);
        return -RT_EINVAL;
    }
#endif
    if (addr % 8 != 0)
    {
        LOG_E("write addr must be 8-byte alignment (0x%p) %d %d", (void*)(addr), addr % 8, size);
        rt_mutex_release(&fal_mux);
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        rt_mutex_release(&fal_mux);
        return 0;
    }

    rt_uint8_t a_temp_buf[MAX31826_BLK_SIZE];
    int e_return = -1;
    rt_uint32_t remain_len, write_len, offset_tmp, error_times = (rt_uint32_t) 0;

    remain_len = size;
    offset_tmp = (rt_uint32_t) 0;

    while (remain_len > (rt_uint32_t) 0)
    {
        /* 每次拷贝8个字节 */
        rt_memset(a_temp_buf, (rt_uint8_t) 0, (uint16_t) 8);
        if (remain_len > (rt_uint8_t) MAX31826_BLK_SIZE)
        {
            write_len = (rt_uint8_t) MAX31826_BLK_SIZE;
            rt_memcpy((void *) a_temp_buf, (void *) &buf[offset_tmp], (uint16_t) MAX31826_BLK_SIZE);
        }
        else
        {
            /* 不足8个字节的补零 */
            write_len = remain_len;
            rt_memcpy((void *) a_temp_buf, (void *) &buf[offset_tmp], (uint16_t) write_len);
        }

        if (max31826_write_8bytes((rt_uint32_t) (addr + offset_tmp), a_temp_buf) == 0/*ok*/)
        {
            remain_len = (rt_uint32_t) (remain_len - write_len);
            offset_tmp = (rt_uint32_t) (offset_tmp + write_len);
            error_times = (rt_uint8_t) 0;
            e_return = 0;
            rt_thread_delay(20);
        }
        else
        {
            error_times++;
            /* 连续3次写不成功认为失败 */
            if (error_times > (rt_uint8_t) 3)
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

    LOG_HEX("wr", 16, (rt_uint8_t *)buf, size);
    LOG_D("write (0x%p) %d", (void*)(addr), size);
    if (e_return == 0)
    {
        rt_mutex_release(&fal_mux);
        return size;
    }
    rt_mutex_release(&fal_mux);
    return 0;
}

static int fal_max31826_erase(long offset, size_t size)
{
    rt_uint32_t addr;
#ifdef MAX31826_USING_I2C_DS2484
    rt_uint8_t findflag = RT_FALSE;

    if(MAX31826_SEN_ALL != 0)
    {
        if((offset < 128*MAX31826_SEN_ALL) && (offset/128 == (offset + size -1)/128))
        {
            addr = offset%128;
            findflag = RT_TRUE;
        }
    }
    if(findflag != RT_TRUE)
        return -RT_EINVAL;

#endif

#ifdef MAX31826_USING_IO
    addr = max31826_flash.addr + offset;

    if ((addr + size) > max31826_flash.addr + max31826_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
#endif
    if (size < 1)
    {
//        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return 0;
    }

    return size;
}

static struct rt_sensor_ops sensor_ops = { max31826_fetch_data, max31826_control };

static int rt_hw_max31826_init()
{
    rt_uint8_t i;
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL;

    /* 初始化 读写 互斥 */
    result = rt_mutex_init(&max31826_mux, "max31826", RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        LOG_E("mutex init error %d!", result);
        return -RT_ERROR;
    }
    result = rt_mutex_init(&fal_mux, "fal_mux", RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        LOG_E("mutex init error %d!", result);
        return -RT_ERROR;
    }

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

    if(max31826_search_rom() != RT_EOK)
    {
        LOG_E("max31826 search rom err\r\n");
        return -RT_ERROR;
    }
    if(max31826_match_rom() != RT_EOK)
    {
        LOG_E("max31826 match rom err\r\n");
        return -RT_ERROR;
    }
    DEV_MAX31826_Convert();


    for(i = 0; i < MAX31826_SEN_ALL; i++)
    {
        /* temperature sensor register */
        max31826_sen[i].sensor.info.type = RT_SENSOR_CLASS_TEMP;
        max31826_sen[i].sensor.info.vendor = RT_SENSOR_VENDOR_MAXIM;
        max31826_sen[i].sensor.info.model = "max31826";
        max31826_sen[i].sensor.info.unit = RT_SENSOR_UNIT_DCELSIUS;
        max31826_sen[i].sensor.info.intf_type = RT_SENSOR_INTF_ONEWIRE;
        max31826_sen[i].sensor.info.range_max = 125;
        max31826_sen[i].sensor.info.range_min = -55;
        max31826_sen[i].sensor.info.period_min = 150;

        max31826_sen[i].sensor.ops = &sensor_ops;

        result = rt_hw_sensor_register(&max31826_sen[i].sensor, max31826_sen[i].dev_name, RT_DEVICE_FLAG_RDONLY, RT_NULL);

        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }
    DEV_MAX31826_Convert();
#endif /* MAX31826_USING_I2C_DS2484 */


#ifdef MAX31826_USING_IO

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

    DEV_MAX31826_ReadTemp();
#endif /* MAX31826_USING_IO */
    return RT_EOK;

__exit:
    rt_free(sensor_temp);
    return -RT_ERROR;
}
/* 导出到自动初始化 */
INIT_PREV_EXPORT(rt_hw_max31826_init);

#endif
