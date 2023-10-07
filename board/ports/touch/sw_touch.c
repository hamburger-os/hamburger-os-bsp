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

#ifdef TOUCH_SW_USING_KVDB
#include "flashdb_port.h"
#endif

#define DBG_TAG "sw_touch"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct SWAdjuct
{
    float A;
    float B;
    float C;
    float D;
    float E;
    float F;
};

static struct SWAdjuct sw_adjuct = {0.23f, 0.00f, -62.91f, 0.01f, 0.15f, 1.03f};

/*
 * slld_Read_IDCmd - Read ID from SPI Flash
 *
 * This function issues the Read_ID command to SPI Flash and reads out the ID.
 * This command sets the target device in software-unprotected state
 * so this function also sets sys_software_protect_status to FLASH_SOFTWARE_UNPROTECTED.
 *
 * RETURNS: Touch_STATE_OK or SLLD_E_HAL_ERROR
 */
static rt_err_t slld_Read_toucX(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd = 0XD0;
    ret = rt_spi_send_then_recv(config->spidev, &cmd, 1, value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read x error %d!", ret);
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
static rt_err_t slld_Read_toucY(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

    uint8_t cmd = 0X90;
    ret = rt_spi_send_then_recv(config->spidev, &cmd, 1, value, 2);
    if (ret != RT_EOK)
    {
        LOG_E("read y error %d!", ret);
        ret = -RT_EIO;
    }
    *value = __SWP16(*value);
    *value = *value >> 3;

    return ret;
}

/* Please calibrate the resistive touch screen before use, it is best to store the calibrated data */
rt_err_t sw_calibration(const char *lcd_name,const char *touch_name)
{
#ifdef TOUCH_SW_USING_KVDB
    struct fdb_blob blob = {0};
    /* GET the KV value */
    kvdb_get_blob("sw_touch", fdb_blob_make(&blob, &sw_adjuct, sizeof(sw_adjuct)));
    /* the blob.saved.len is more than 0 when get the value successful */
    if (blob.saved.len > 0) {
        LOG_I(" Calibration result: %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d"
                , (int)sw_adjuct.A, abs((int)((sw_adjuct.A - (int)sw_adjuct.A) * 100))
                , (int)sw_adjuct.B, abs((int)((sw_adjuct.B - (int)sw_adjuct.B) * 100))
                , (int)sw_adjuct.C, abs((int)((sw_adjuct.C - (int)sw_adjuct.C) * 100))
                , (int)sw_adjuct.D, abs((int)((sw_adjuct.D - (int)sw_adjuct.D) * 100))
                , (int)sw_adjuct.E, abs((int)((sw_adjuct.E - (int)sw_adjuct.E) * 100))
                , (int)sw_adjuct.F, abs((int)((sw_adjuct.F - (int)sw_adjuct.F) * 100)));
        return RT_EOK;
    }
#endif

    uint16_t black = 0b1111100000000000;
    uint16_t white = 0b1111111111111111;

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

    /* Get TFT LCD screen information */
    struct rt_device_graphic_info lcd_info = { 0 };
    rt_device_control(lcd, RTGRAPHIC_CTRL_GET_INFO, &lcd_info);

    /* clear screen */
    LOG_D(" lcd (%d, %d)", lcd_info.width, lcd_info.height);
    for (uint16_t y = 0; y < lcd_info.height; ++y)
    {
        rt_graphix_ops(lcd)->draw_hline((const char *) (&white), 0, lcd_info.width, y);
    }
    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

    /* Set the information for the four points used for calibration */
    uint16_t cross_size = (lcd_info.width > lcd_info.height ? lcd_info.height : lcd_info.width) / 32;

_adjust:
    //rand()%(b - a + 1)+ a    生成[a, b]   范围内的随机数
    srand(UID_BASE * rt_tick_get() + rt_tick_get());
    uint16_t x0 = rand() % (lcd_info.width - cross_size - cross_size + 1) + cross_size;
    uint16_t y0 = rand() % (lcd_info.height - cross_size - cross_size + 1) + cross_size;
    uint16_t x1 = rand() % (lcd_info.width - cross_size - cross_size + 1) + cross_size;
    uint16_t y1 = rand() % (lcd_info.height - cross_size - cross_size + 1) + cross_size;
    uint16_t x2 = rand() % (lcd_info.width - cross_size - cross_size + 1) + cross_size;
    uint16_t y2 = rand() % (lcd_info.height - cross_size - cross_size + 1) + cross_size;
    uint16_t x3 = rand() % (lcd_info.width - cross_size - cross_size + 1) + cross_size;
    uint16_t y3 = rand() % (lcd_info.height - cross_size - cross_size + 1) + cross_size;

    rt_graphix_ops(lcd)->draw_hline((const char *) (&black), x0 - cross_size, x0 + cross_size, y0);
    rt_graphix_ops(lcd)->draw_vline((const char *) (&black), x0, y0 - cross_size, y0 + cross_size);
    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    LOG_I(" %d point capture (%d, %d)", 0, x0, y0);

    uint8_t raw_idx = 0;
    uint16_t adxvalue[4] = { 0 };
    uint16_t adyvalue[4] = { 0 };
    uint16_t posxvalue[4] = { 0 };
    uint16_t posyvalue[4] = { 0 };
    struct rt_touch_data read_data = {0};
    while (1)
    {
        rt_thread_mdelay(20);

        if (rt_device_read((rt_device_t) touch, 0, &read_data, 1) == 1 && read_data.event == RT_TOUCH_EVENT_DOWN)
        {
            LOG_I(" %d point press (%d, %d)", raw_idx, read_data.x, read_data.y);

            adxvalue[raw_idx] = read_data.x;
            adyvalue[raw_idx] = read_data.y;
            raw_idx++;

            /* After processing a point, proceed to clear the screen */
            for (uint16_t y = 0; y < lcd_info.height; ++y)
            {
                rt_graphix_ops(lcd)->draw_hline((const char *) (&white), 0, lcd_info.width, y);
            }
            rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
            rt_thread_delay(500);

            if (raw_idx > 3)
            {
                break;
            }
            switch (raw_idx)
            {
            case 1:
                rt_graphix_ops(lcd)->draw_hline((const char *) (&black), x1 - cross_size, x1 + cross_size, y1);
                rt_graphix_ops(lcd)->draw_vline((const char *) (&black), x1, y1 - cross_size, y1 + cross_size);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x1, y1);
                break;
            case 2:
                rt_graphix_ops(lcd)->draw_hline((const char *) (&black), x2 - cross_size, x2 + cross_size, y2);
                rt_graphix_ops(lcd)->draw_vline((const char *) (&black), x2, y2 - cross_size, y2 + cross_size);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x2, y2);
                break;
            case 3:
                rt_graphix_ops(lcd)->draw_hline((const char *) (&black), x3 - cross_size, x3 + cross_size, y3);
                rt_graphix_ops(lcd)->draw_vline((const char *) (&black), x3, y3 - cross_size, y3 + cross_size);
                LOG_I(" %d point capture (%d, %d)", raw_idx, x3, y3);
                break;
            default:
                break;
            }
            rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
        }
    }

    //计算结果
    float K = (adxvalue[0] - adxvalue[2]) * (adyvalue[1] - adyvalue[2])
            - (adxvalue[1] - adxvalue[2]) * (adyvalue[0] - adyvalue[2]);
    sw_adjuct.A = ((x0 - x2) * (adyvalue[1] - adyvalue[2]) - (x1 - x2) * (adyvalue[0] - adyvalue[2])) / K;
    sw_adjuct.B = ((adxvalue[0] - adxvalue[2]) * (x1 - x2) - (x0 - x2) * (adxvalue[1] - adxvalue[2])) / K;
    sw_adjuct.C = (adyvalue[0] * (adxvalue[2] * x1 - adxvalue[1] * x2)
            + adyvalue[1] * (adxvalue[0] * x2 - adxvalue[2] * x0)
            + adyvalue[2] * (adxvalue[1] * x0 - adxvalue[0] * x1)) / K;
    sw_adjuct.D = ((y0 - y2) * (adyvalue[1] - adyvalue[2]) - (y1 - y2) * (adyvalue[0] - adyvalue[2])) / K;
    sw_adjuct.E = ((adxvalue[0] - adxvalue[2]) * (y1 - y2) - (y0 - y2) * (adxvalue[1] - adxvalue[2])) / K;
    sw_adjuct.F = (adyvalue[0] * (adxvalue[2] * y1 - adxvalue[1] * y2)
            + adyvalue[1] * (adxvalue[0] * y2 - adxvalue[2] * y0)
            + adyvalue[2] * (adxvalue[1] * y0 - adxvalue[0] * y1)) / K;

    //验证校验值正确性
    posxvalue[3] = sw_adjuct.A * adxvalue[3] + sw_adjuct.B * adyvalue[3] + sw_adjuct.C;
    posyvalue[3] = sw_adjuct.D * adxvalue[3] + sw_adjuct.E * adyvalue[3] + sw_adjuct.F;
    if (abs(posxvalue[3] - x3) > cross_size || abs(posyvalue[3] - y3) > cross_size)
    {
        LOG_E("check: (%d, %d) (%d, %d) (%d, %d)", adxvalue[3], adyvalue[3], posxvalue[3], posyvalue[3], x3, y3);
        goto _adjust;
    }

#ifdef TOUCH_SW_USING_KVDB
    /* CHANGE the KV value */
    kvdb_set_blob("sw_touch", fdb_blob_make(&blob, &sw_adjuct, sizeof(sw_adjuct)));
#endif
    LOG_I(" Calibration result: %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d"
            , (int)sw_adjuct.A, abs((int)((sw_adjuct.A - (int)sw_adjuct.A) * 100))
            , (int)sw_adjuct.B, abs((int)((sw_adjuct.B - (int)sw_adjuct.B) * 100))
            , (int)sw_adjuct.C, abs((int)((sw_adjuct.C - (int)sw_adjuct.C) * 100))
            , (int)sw_adjuct.D, abs((int)((sw_adjuct.D - (int)sw_adjuct.D) * 100))
            , (int)sw_adjuct.E, abs((int)((sw_adjuct.E - (int)sw_adjuct.E) * 100))
            , (int)sw_adjuct.F, abs((int)((sw_adjuct.F - (int)sw_adjuct.F) * 100)));

    rt_device_close(lcd);
    rt_device_close(touch);
    return RT_EOK;
}

static rt_size_t sw_touch_readpoint(struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
    if (touch_num > 0)
    {
        struct rt_touch_data *result = (struct rt_touch_data *)buf;
        result->event = RT_TOUCH_EVENT_DOWN;

#ifdef RT_TOUCH_PIN_IRQ
        if (rt_pin_read(touch->config.irq_pin.pin))
        {
            result->event = RT_TOUCH_EVENT_NONE;
            return 0;
        }
#endif /* RT_TOUCH_PIN_IRQ */
        uint8_t i = 0;
        uint16_t xa[9] = { 0 };
        uint16_t ya[9] = { 0 };
        uint16_t xb[3] = { 0 };
        uint16_t yb[3] = { 0 };
        uint16_t mx[3] = { 0 };
        uint16_t my[3] = { 0 };

        for (i = 0; i < 9; i++)
        {
            slld_Read_toucX(touch, &xa[i]);
            slld_Read_toucY(touch, &ya[i]);
            LOG_D("press: read (%d, %d)", xa[i], ya[i]);
        }

        for (i = 0; i < 3; i++)
        {
            xb[i] = (uint16_t) ((xa[i * 3] + xa[i * 3 + 1] + xa[i * 3 + 2]) / 3.0f + 0.5f);
            yb[i] = (uint16_t) ((ya[i * 3] + ya[i * 3 + 1] + ya[i * 3 + 2]) / 3.0f + 0.5f);
            LOG_D("press: aver (%d, %d)", xb[i], yb[i]);
        }

        for (i = 0; i < 3; i++)
        {
            mx[i] = abs(xb[i] - xb[(i + 1) % 3]);
            my[i] = abs(yb[i] - yb[(i + 1) % 3]);

            if (mx[i] > 10 || my[i] > 10)
            {
                result->event = RT_TOUCH_EVENT_MOVE;
            }
        }

        result->timestamp = rt_tick_get();
        if (mx[0] < mx[1])
        {
            if (mx[2] < mx[0])
                result->x = (xb[0] + xb[2]) / 2;
            else
                result->x = (xb[0] + xb[1]) / 2;
        }
        else if (mx[2] < mx[1])
            result->x = (xb[0] + xb[2]) / 2;
        else
            result->x = (xb[1] + xb[2]) / 2;

        if (my[0] < my[1])
        {
            if (my[2] < my[0])
                result->y = (yb[0] + yb[2]) / 2;
            else
                result->y = (yb[0] + yb[1]) / 2;
        }
        else if (my[2] < my[1])
            result->y = (yb[0] + yb[2]) / 2;
        else
            result->y = (yb[1] + yb[2]) / 2;

        result->x_coordinate = sw_adjuct.A * result->x + sw_adjuct.B * result->y + sw_adjuct.C;
        result->y_coordinate = sw_adjuct.D * result->x + sw_adjuct.E * result->y + sw_adjuct.F;

        LOG_D("press: %d (%d, %d) (%d, %d)", result->event, result->x, result->y, result->x_coordinate, result->y_coordinate);
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
