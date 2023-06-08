/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-08     lvhan       the first version
 */

#include <board.h>
#include <stdlib.h>

#ifdef BSP_USING_TCA9555

#define DBG_TAG "tca9555"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

enum
{
    TCA9555_Input_port = 0,
    TCA9555_Output_port = 2,
    TCA9555_Polarity_Inversion_port = 4,
    TCA9555_Configuration_port = 6,
};

struct __attribute__ ((packed)) TCAPinDef
{
    union
    {
        struct
        {
            uint8_t i2c;
            uint8_t address;
            uint8_t pin;
            uint8_t not_use;
        };
        uint32_t u32;
    };
};

static struct rt_device_pin _tca9555_pin;
static rt_size_t _pin_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_device_pin_status *status;
    struct rt_device_pin *pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(pin != RT_NULL);

    status = (struct rt_device_pin_status *)buffer;
    if (status == RT_NULL || size != sizeof(*status))
        return 0;

    status->status = pin->ops->pin_read(dev, status->pin);
    return size;
}

static rt_size_t _pin_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_device_pin_status *status;
    struct rt_device_pin *pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(pin != RT_NULL);

    status = (struct rt_device_pin_status *)buffer;
    if (status == RT_NULL || size != sizeof(*status))
        return 0;

    pin->ops->pin_write(dev, (rt_base_t)status->pin, (rt_base_t)status->status);

    return size;
}

static rt_err_t _pin_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_device_pin_mode *mode;
    struct rt_device_pin *pin = (struct rt_device_pin *)dev;

    /* check parameters */
    RT_ASSERT(pin != RT_NULL);

    mode = (struct rt_device_pin_mode *)args;
    if (mode == RT_NULL)
        return -RT_ERROR;

    pin->ops->pin_mode(dev, (rt_base_t)mode->pin, (rt_base_t)mode->mode);

    return 0;
}

static int tca9555_write_registers(char *dev, uint8_t address, uint8_t cmd, uint16_t data)
{
    uint8_t buf[3] = {0};
    struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */

    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(dev);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }

    buf[0] = cmd;
    buf[1] = data;
    buf[2] = data >> 8;
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(i2c_bus, address, 0, buf, 3) != 3)
    {
        LOG_E("i2c_master_send write error!");
        return -RT_EIO;
    }

//    LOG_D("write: %s 0x%x [%d 0x%x %x] 0x%x", dev, address, buf[0], buf[1], buf[2], data);
    return RT_EOK;
}

static int tca9555_read_registers(char *dev, uint8_t address, uint8_t cmd, uint16_t *data)
{
    uint8_t buf[3] = {0};
    struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */

    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(dev);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }

    buf[0] = cmd;
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(i2c_bus, address, 0, buf, 1) != 1)
    {
        LOG_E("i2c_master_send read error!");
        return -RT_EIO;
    }

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_recv(i2c_bus, address, 0, &buf[1], 2) != 2)
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_EIO;
    }

    *data = buf[1] + (((uint16_t)buf[2])<<8);

//    LOG_D("read: %s 0x%x [%d 0x%x %x] 0x%x", dev, address, buf[0], buf[1], buf[2], *data);
    return RT_EOK;
}

static void tca9555_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    struct TCAPinDef tcapin;
    tcapin.u32 = pin;

    if(tcapin.pin < 16)
    {
        char devname[7] = {0};
        rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, tcapin.i2c);

        uint16_t data;
        tca9555_read_registers(devname, tcapin.address, TCA9555_Configuration_port, &data);

        if (mode == PIN_MODE_OUTPUT)
        {
            data &= ~(1<<tcapin.pin);
        }
        else if (mode == PIN_MODE_INPUT)
        {
            data |= (1 << tcapin.pin);
        }
        tca9555_write_registers(devname, tcapin.address, TCA9555_Configuration_port, data);

        LOG_D("pin_mode: [%d 0x%x %d] 0x%x %d", tcapin.i2c, tcapin.address, tcapin.pin, data, mode);
    }
}

static void tca9555_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    struct TCAPinDef tcapin;
    tcapin.u32 = pin;

    if(tcapin.pin < 16)
    {
        char devname[7] = {0};
        rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, tcapin.i2c);

        uint16_t data;
        tca9555_read_registers(devname, tcapin.address, TCA9555_Output_port, &data);

        if (value == PIN_LOW)
        {
            data &= ~(1 << tcapin.pin);
        }
        else if (value == PIN_HIGH)
        {
            data |= (1 << tcapin.pin);
        }
        tca9555_write_registers(devname, tcapin.address, TCA9555_Output_port, data);

        LOG_D("pin_write: [%d 0x%x %d] 0x%x %d", tcapin.i2c, tcapin.address, tcapin.pin, data, value);
    }
}

static int tca9555_pin_read(rt_device_t dev, rt_base_t pin)
{
    int ret = 0;
    struct TCAPinDef tcapin;
    tcapin.u32 = pin;

    if(tcapin.pin < 16)
    {
        char devname[7] = {0};
        rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, tcapin.i2c);

        uint16_t data;
        tca9555_read_registers(devname, tcapin.address, TCA9555_Input_port, &data);

        ret = ((data & (1 << tcapin.pin)) == 0)?(0):(1);

        LOG_D("pin_read: [%d 0x%x %d] 0x%x %d", tcapin.i2c, tcapin.address, tcapin.pin, data, ret);
    }

    return ret;
}

static rt_err_t tca9555_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    return RT_EOK;
}

static rt_err_t tca9555_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    return RT_EOK;
}

static rt_err_t tca9555_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled)
{
    return RT_EOK;
}

//example: P01.02.03
static rt_base_t tca9555_pin_get(const char *name)
{
    char num[3] = {0};
    struct TCAPinDef pin = {0};
    int name_len;

    name_len = rt_strlen(name);

    if (name_len != 9)
    {
        return -RT_EINVAL;
    }
    if ((name[0] != 'P'))
    {
        return -RT_EINVAL;
    }
    if ((name[3] != '.'))
    {
        return -RT_EINVAL;
    }
    if ((name[6] != '.'))
    {
        return -RT_EINVAL;
    }

    num[0] = name[1];
    num[1] = name[2];
    pin.i2c = strtol(num, NULL, 10);

    num[0] = name[4];
    num[1] = name[5];
    pin.address = strtol(num, NULL, 16);

    num[0] = name[7];
    num[1] = name[8];
    pin.pin = strtol(num, NULL, 10);

    LOG_D("pin_get: [%d 0x%x %d]", pin.i2c, pin.address, pin.pin);
    return pin.u32;
}

static const struct rt_pin_ops tca9555_pin_ops =
{
    tca9555_pin_mode,
    tca9555_pin_write,
    tca9555_pin_read,
    tca9555_pin_attach_irq,
    tca9555_pin_dettach_irq,
    tca9555_pin_irq_enable,
    tca9555_pin_get,
};

static int rt_tca9555_init(void)
{
    _tca9555_pin.parent.type         = RT_Device_Class_Pin;
    _tca9555_pin.parent.rx_indicate  = RT_NULL;
    _tca9555_pin.parent.tx_complete  = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    _tca9555_pin.parent.ops          = &tca9555_pin_ops;
#else
    _tca9555_pin.parent.init         = RT_NULL;
    _tca9555_pin.parent.open         = RT_NULL;
    _tca9555_pin.parent.close        = RT_NULL;
    _tca9555_pin.parent.read         = _pin_read;
    _tca9555_pin.parent.write        = _pin_write;
    _tca9555_pin.parent.control      = _pin_control;
#endif

    _tca9555_pin.ops                 = &tca9555_pin_ops;
    _tca9555_pin.parent.user_data    = NULL;

    /* register a character device */
    rt_device_register(&_tca9555_pin.parent, "tca9555", RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_tca9555_init);

/* RT-Thread Hardware PIN APIs */
void rt_tca9555_pin_mode(rt_base_t pin, rt_base_t mode)
{
    RT_ASSERT(_tca9555_pin.ops != RT_NULL);
    _tca9555_pin.ops->pin_mode(&_tca9555_pin.parent, pin, mode);
}
void rt_tca9555_fast_mode(uint8_t i2c, uint8_t address, uint16_t mode)
{
    char devname[7] = {0};
    rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, i2c);
    tca9555_write_registers(devname, address, TCA9555_Configuration_port, mode);

    LOG_D("fast_mode: %s 0x%x 0x%x", devname, address, mode);
}

void rt_tca9555_pin_write(rt_base_t pin, rt_base_t value)
{
    RT_ASSERT(_tca9555_pin.ops != RT_NULL);
    _tca9555_pin.ops->pin_write(&_tca9555_pin.parent, pin, value);
}
void rt_tca9555_fast_write(uint8_t i2c, uint8_t address, uint16_t data)
{
    char devname[7] = {0};
    rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, i2c);
    tca9555_write_registers(devname, address, TCA9555_Output_port, data);

    LOG_D("fast_write: %s 0x%x 0x%x", devname, address, data);
}

int rt_tca9555_pin_read(rt_base_t pin)
{
    RT_ASSERT(_tca9555_pin.ops != RT_NULL);
    return _tca9555_pin.ops->pin_read(&_tca9555_pin.parent, pin);
}
void rt_tca9555_fast_read(uint8_t i2c, uint8_t address, uint16_t *data)
{
    char devname[7] = {0};
    rt_sprintf(devname, "%s%d", TCA9555_I2C_DEVNAME, i2c);
    tca9555_read_registers(devname, address, TCA9555_Input_port, data);

    LOG_D("fast_read: %s 0x%x 0x%x", devname, address, *data);
}

rt_base_t rt_tca9555_pin_get(const char *name)
{
    RT_ASSERT(_tca9555_pin.ops != RT_NULL);

    if (name[0] != 'P' && name[0] != 'p')
    {
        return -RT_EINVAL;
    }
    if (_tca9555_pin.ops->pin_get == RT_NULL)
    {
        return -RT_ENOSYS;
    }
    return _tca9555_pin.ops->pin_get(name);
}

#ifdef RT_USING_FINSH
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <finsh.h>
#include <msh_parse.h>

/*
 * convert function for port name
 * support P01.20.00 - P30.23.15
 */
static rt_base_t _pin_cmd_conv(const char *name)
{
    return rt_tca9555_pin_get(name);
}

static void _pin_cmd_print_usage(void)
{
    rt_kprintf("tca9555 [option]\n");
    rt_kprintf("  num: get pin number from hardware pin\n");
    rt_kprintf("    num can be P01.20.00 - P30.23.15\n");
    rt_kprintf("    e.g. MSH >tca9555 num P01.20.00\n");
    rt_kprintf("  mode: set pin mode to output/input\n    e.g. MSH >tca9555 mode P01.20.00 output\n");
    rt_kprintf("  read: read pin level of hardware pin\n    e.g. MSH >tca9555 read P01.20.00\n");
    rt_kprintf("  write: write pin level(high/low or on/off) to hardware pin\n    e.g. MSH >tca9555 write P01.20.00 high\n");
    rt_kprintf("  help: this help list\n");
}

static void _pin_cmd_get(int argc, char *argv[])
{
    rt_base_t pin;
    if (argc < 3)
    {
        _pin_cmd_print_usage();
        return;
    }
    pin = _pin_cmd_conv(argv[2]);
    if (pin < 0)
    {
        rt_kprintf("Parameter invalid : %s!\n", argv[2]);
        _pin_cmd_print_usage();
        return ;
    }
    rt_kprintf("%s : %d\n", argv[2], pin);
}

static void _pin_cmd_mode(int argc, char *argv[])
{
    rt_base_t pin;
    rt_base_t mode;
    if (argc < 4)
    {
        _pin_cmd_print_usage();
        return;
    }
    if (!msh_isint(argv[2]))
    {
        pin = _pin_cmd_conv(argv[2]);
        if (pin < 0)
        {
            rt_kprintf("Parameter invalid : %s!\n", argv[2]);
            _pin_cmd_print_usage();
            return;
        }
    }
    else
    {
        pin = atoi(argv[2]);
    }
    if (0 == rt_strcmp("output", argv[3]))
    {
        mode = PIN_MODE_OUTPUT;
    }
    else if (0 == rt_strcmp("input", argv[3]))
    {
        mode = PIN_MODE_INPUT;
    }
    else
    {
        _pin_cmd_print_usage();
        return;
    }

    rt_tca9555_pin_mode(pin, mode);
}

static void _pin_cmd_read(int argc, char *argv[])
{
    rt_base_t pin;
    rt_base_t value;
    if (argc < 3)
    {
        _pin_cmd_print_usage();
        return;
    }
    if (!msh_isint(argv[2]))
    {
        pin = _pin_cmd_conv(argv[2]);
        if (pin < 0)
        {
            rt_kprintf("Parameter invalid : %s!\n", argv[2]);
            _pin_cmd_print_usage();
            return;
        }
    }
    else
    {
        pin = atoi(argv[2]);
    }
    value = rt_tca9555_pin_read(pin);
    if (value == PIN_HIGH)
    {
        rt_kprintf("pin[%d] = on\n", pin);
    }
    else
    {
        rt_kprintf("pin[%d] = off\n", pin);
    }
}

/* e.g. MSH >pin write PA.16 high */
static void _pin_cmd_write(int argc, char *argv[])
{
    rt_base_t pin;
    rt_base_t value;
    if (argc < 4)
    {
        _pin_cmd_print_usage();
        return;
    }
    if (!msh_isint(argv[2]))
    {
        pin = _pin_cmd_conv(argv[2]);
        if (pin < 0)
        {
            rt_kprintf("Parameter invalid : %s!\n", argv[2]);
            _pin_cmd_print_usage();
            return;
        }
    }
    else
    {
        pin = atoi(argv[2]);
    }
    if ((0 == rt_strcmp("high", argv[3])) || (0 == rt_strcmp("on", argv[3])))
    {
        value = PIN_HIGH;
    }
    else if ((0 == rt_strcmp("low", argv[3])) || (0 == rt_strcmp("off", argv[3])))
    {
        value = PIN_LOW;
    }
    else
    {
        _pin_cmd_print_usage();
        return;
    }
    rt_tca9555_pin_write(pin, value);
}

static void _pin_cmd(int argc, char *argv[])
{
    if (argc < 3)
    {
        _pin_cmd_print_usage();
        return ;
    }
    if (0 == rt_strcmp("num", argv[1]))
    {
        _pin_cmd_get(argc, argv);
    }
    else if (0 == rt_strcmp("mode", argv[1]))
    {
        _pin_cmd_mode(argc, argv);
    }
    else if (0 == rt_strcmp("read", argv[1]))
    {
        _pin_cmd_read(argc, argv);
    }
    else if (0 == rt_strcmp("write", argv[1]))
    {
        _pin_cmd_write(argc, argv);
    }
    else
    {
        _pin_cmd_print_usage();
        return;
    }
}
MSH_CMD_EXPORT_ALIAS(_pin_cmd, tca9555, tca9555 [option]);

#endif /* RT_USING_FINSH */

#endif
