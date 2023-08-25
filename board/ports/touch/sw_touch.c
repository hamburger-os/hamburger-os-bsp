/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-30     lvhan       the first version
 */
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "drv_soft_spi.h"

#ifdef TOUCH_USING_SW
#include "drv_touch.h"
#include "sw_touch.h"

#define DBG_TAG "sw_touch"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct SWAdjuct
{
    float xfac;
    float xoff;
    float yfac;
    float yoff;
};
static struct SWAdjuct sw_adjuct = {0.24f, -102.12f, 0.17f, -64.94f};

/*
 * slld_Read_IDCmd - Read ID from SPI Flash
 *
 * This function issues the Read_ID command to SPI Flash and reads out the ID.
 * This command sets the target device in software-unprotected state
 * so this function also sets sys_software_protect_status to FLASH_SOFTWARE_UNPROTECTED.
 *
 * RETURNS: Touch_STATE_OK or SLLD_E_HAL_ERROR
 */
rt_err_t slld_Read_toucX(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd = 0XD0;
    ret = rt_spi_send_then_recv(config->spidev, &cmd, 1, value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    *value = __SWP16(*value);
    *value = *value >> 3;

    return ret;
}

/*
 * slld_Read_IDCmd - Read ID from SPI Flash
 *
 * This function issues the Read_ID command to SPI Flash and reads out the ID.
 * This command sets the target device in software-unprotected state
 * so this function also sets sys_software_protect_status to FLASH_SOFTWARE_UNPROTECTED.
 *
 * RETURNS: Touch_STATE_OK or SLLD_E_HAL_ERROR
 */
rt_err_t slld_Read_toucY(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd = 0X90;
    ret = rt_spi_send_then_recv(config->spidev, &cmd, 1, value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    *value = __SWP16(*value);
    *value = *value >> 3;

    return ret;
}

/* Please calibrate the resistive touch screen before use, it is best to store the calibrated data */
rt_err_t sw_calibration(const char *lcd_name,const char *touch_name)
{
    /* Find the TFT LCD device */
    rt_device_t lcd = rt_device_find(lcd_name);
    if (lcd == RT_NULL)
    {
        LOG_E(" cannot find lcd device named %s", lcd_name);
        return -RT_ERROR;
    }
    if (rt_device_open(lcd, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open lcd device named %s", lcd_name);
        return -RT_ERROR;
    }

    /* Find the Touch device */
    rt_device_t touch = rt_device_find(touch_name);
    if (touch == RT_NULL)
    {
        LOG_E(" cannot find touch device named %s", touch_name);
        return -RT_ERROR;
    }
    if (rt_device_open(touch, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open touch device named %s", touch_name);
        return -RT_ERROR;
    }

    /* 重置校准值 */
    sw_adjuct.xfac = 1;
    sw_adjuct.xoff = 0;
    sw_adjuct.yfac = 1;
    sw_adjuct.yoff = 0;

    /* Get TFT LCD screen information */
    struct rt_device_graphic_info lcd_info = {0};
    rt_device_control(lcd, RTGRAPHIC_CTRL_GET_INFO, &lcd_info);

__adjust:
    /* clear screen */
    LOG_I(" lcd (%d, %d)", lcd_info.width, lcd_info.height);
    for (rt_uint32_t y = 0; y < lcd_info.height; ++y)
    {
        const uint32_t white = 0xFFFFFFFF;
        rt_graphix_ops(lcd)->draw_hline((const char *)(&white), 0, lcd_info.width, y);
    }
    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    /* Set the information for the four points used for calibration */
    rt_uint32_t cross_size = (lcd_info.width > lcd_info.height ? lcd_info.height : lcd_info.width) / 10;
    rt_uint32_t x0 = cross_size;
    rt_uint32_t y0 = cross_size;
    rt_uint32_t x1 = lcd_info.width - cross_size;
    rt_uint32_t y1 = cross_size;
    rt_uint32_t x2 = lcd_info.width - cross_size;
    rt_uint32_t y2 = lcd_info.height - cross_size;
    rt_uint32_t x3 = cross_size;
    rt_uint32_t y3 = lcd_info.height - cross_size;
    const rt_uint32_t black = 0x0;
    /* Upper left cross */
    rt_graphix_ops(lcd)->draw_hline((const char *)(&black), 0, x0 + cross_size, y0);
    rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x0, 0, y0 + cross_size);
    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    LOG_I(" %d point capture (%d, %d)", 0, x0, y0);

    rt_uint8_t raw_idx = 0;
    rt_uint16_t x_raw[4] = {0};
    rt_uint16_t y_raw[4] = {0};
    while (1)
    {
        rt_thread_mdelay(10);

        struct rt_touch_data read_data;
        rt_memset(&read_data, 0, sizeof(struct rt_touch_data));
        if (rt_device_read((rt_device_t)touch, 0, &read_data, 1) == 1)
        {
            LOG_I(" %d point press (%d, %d)", raw_idx, read_data.x_coordinate, read_data.y_coordinate);

            x_raw[raw_idx] = read_data.x_coordinate;
            y_raw[raw_idx++] = read_data.y_coordinate;

            /* After processing a point, proceed to clear the screen */
            for (rt_uint32_t y = 0; y < lcd_info.height; ++y)
            {
                const uint32_t white = 0xFFFFFFFF;
                rt_graphix_ops(lcd)->draw_hline((const char *)(&white), 0, lcd_info.width, y);
            }
            rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
            rt_thread_mdelay(400);
            if (raw_idx >= 4)
            {
                break;
            }
            switch (raw_idx)
            {
            case 1:
                /* Upper right cross */
                rt_graphix_ops(lcd)->draw_hline((const char *)(&black), x1 - cross_size, lcd_info.width, y1);
                rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x1, 0, y1 + cross_size);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x1, y1);
                break;
            case 2:
                /* lower right cross */
                rt_graphix_ops(lcd)->draw_hline((const char *)(&black), x2 - cross_size, lcd_info.width, y2);
                rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x2, y2 - cross_size, lcd_info.height);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x2, y2);
                break;
            case 3:
                /* lower left cross */
                rt_graphix_ops(lcd)->draw_hline((const char *)(&black), 0, x3 + cross_size, y3);
                rt_graphix_ops(lcd)->draw_vline((const char *)(&black), x3, y3 - cross_size, lcd_info.height);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x3, y3);
                break;
            default:
                break;
            }
            rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
        }
    }
    //校验触摸结果
    if (abs(x_raw[0] - x_raw[3]) > 20 && abs(x_raw[1] - x_raw[2]) > 20
            && abs(y_raw[0] - y_raw[1]) > 20 && abs(y_raw[2] - y_raw[3]) > 20)
    {
        LOG_W(" press warning %d %d %d %d"
                , abs(x_raw[0] - x_raw[3]), abs(x_raw[1] - x_raw[2])
                , abs(y_raw[0] - y_raw[1]), abs(y_raw[2] - y_raw[3]));
//        goto __adjust;
    }
    //计算结果
    sw_adjuct.xfac=(float)(lcd_info.width - cross_size*2)/(x_raw[1]-x_raw[0]);//得到xfac
    sw_adjuct.xoff=(lcd_info.width-sw_adjuct.xfac*(x_raw[1]+x_raw[0]))/2;//得到xoff

    sw_adjuct.yfac=(float)(lcd_info.height - cross_size*2)/(y_raw[2]-y_raw[0]);//得到yfac
    sw_adjuct.yoff=(lcd_info.height-sw_adjuct.yfac*(y_raw[2]+y_raw[0]))/2;//得到yoff

    LOG_I(" Calibration result, xfac:%d.%02d, xoff:%d.%02d, yfac:%d.%02d, yoff:%d.%02d"
            , (int)sw_adjuct.xfac, abs((int)((sw_adjuct.xfac - (int)sw_adjuct.xfac) * 100)), (int)sw_adjuct.xoff, abs((int)((sw_adjuct.xoff - (int)sw_adjuct.xoff) * 100))
            , (int)sw_adjuct.yfac, abs((int)((sw_adjuct.yfac - (int)sw_adjuct.yfac) * 100)), (int)sw_adjuct.yoff, abs((int)((sw_adjuct.yoff - (int)sw_adjuct.yoff) * 100)));

    rt_device_close(lcd);
    rt_device_close(touch);
    return RT_EOK;
}

static rt_size_t sw_touch_readpoint(struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
    if (touch_num > 0)
    {
        struct rt_touch_data *result = (struct rt_touch_data *)buf;
#ifdef RT_TOUCH_PIN_IRQ
        if (rt_pin_read(touch->config.irq_pin.pin))
        {
            result->event = RT_TOUCH_EVENT_NONE;
            return 0;
        }
#endif /* RT_TOUCH_PIN_IRQ */
        rt_uint8_t x_count = 0;
        rt_uint8_t y_count = 0;
        rt_uint32_t x_cum = 0;
        rt_uint32_t y_cum = 0;
        for (rt_uint8_t i = 0; i < TOUCH_SW_SAMPLE_TIMES; i ++)
        {
            rt_uint16_t temp = 0;
            if (slld_Read_toucX(touch, &temp) == RT_EOK && temp >= 10 && temp <= 4096)
            {
                ++x_count;
                x_cum += temp;
            }
            if (slld_Read_toucY(touch, &temp) == RT_EOK && temp >= 10 && temp <= 4096)
            {
                ++y_count;
                y_cum += temp;
            }
        }
        if (!x_count || !y_count)
        {
            result->event = RT_TOUCH_EVENT_NONE;
            return 0;
        }
        result->event = RT_TOUCH_EVENT_DOWN;
        result->timestamp = rt_tick_get();
        result->x_coordinate = sw_adjuct.xfac * ((float)x_cum / x_count) + sw_adjuct.xoff;
        result->y_coordinate = sw_adjuct.yfac * ((float)y_cum / y_count) + sw_adjuct.yoff;
        LOG_D("press: (%d, %d) (%d, %d)"
                , x_cum / x_count, y_cum / y_count
                , result->x_coordinate, result->y_coordinate);
        return touch_num;
    }
    else
    {
        return 0;
    }
}

static rt_err_t sw_touch_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(touch != RT_NULL);

    /* If necessary, please implement this control function yourself */

    return result;
}

int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", TOUCH_SW_SPI_BUS, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(TOUCH_SW_SPI_BUS, dev_name, TOUCH_SW_CS_PIN);
    rt_hw_spi_device_attach(TOUCH_SW_SPI_BUS, dev_name, rt_pin_get(TOUCH_SW_CS_PIN));
    config->spidev = (struct rt_spi_device *) rt_device_find(dev_name);
    if (config->spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = { 0 };
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = TOUCH_SW_SPI_SPEED;
    if (rt_spi_configure(config->spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return -RT_EIO;
    }

    return RT_EOK;
}

struct rt_touch_ops drv_touch_ops =
{
    .touch_readpoint = sw_touch_readpoint,
    .touch_control = sw_touch_control,
};

#endif
