/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 */

#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#ifdef BSP_USING_TOUCH
#include "drv_touch.h"
#include "sw_touch.h"
#include "xpt2046.h"

#ifdef PKG_USING_FLASHDB_PORT
#include "flashdb_port.h"
#endif

#define DBG_TAG "drv.touch"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#if defined(PKG_USING_LVGL)
#include <lvgl.h>
#endif /* PKG_USING_LVGL */

#define TOUCH_ADJUCT_KEY "touch_adjuct"

static struct _rt_drv_touch drv_touch;

__weak rt_err_t touch_Read_toucX(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;

    return ret;
}

__weak rt_err_t touch_Read_toucY(struct rt_touch_device *touch, uint16_t *value)
{
    rt_err_t ret = RT_EOK;

    return ret;
}
static rt_size_t touch_readpoint(struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
    struct _rt_drv_touch *config = (struct _rt_drv_touch *)touch->parent.user_data;

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
            touch_Read_toucX(touch, &xa[i]);
            touch_Read_toucY(touch, &ya[i]);
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

        result->x_coordinate = config->adjuct.A * result->x + config->adjuct.B * result->y + config->adjuct.C;
        result->y_coordinate = config->adjuct.D * result->x + config->adjuct.E * result->y + config->adjuct.F;

        LOG_D("press: %d (%d, %d) (%d, %d)", result->event, result->x, result->y, result->x_coordinate, result->y_coordinate);
        return touch_num;
    }
    else
    {
        return 0;
    }
}

static rt_err_t touch_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(touch != RT_NULL);

    /* If necessary, please implement this control function yourself */

    return result;
}

static struct rt_touch_ops drv_touch_ops =
{
    .touch_readpoint = touch_readpoint,
    .touch_control = touch_control,
};

__weak int drv_touch_bus_init(struct _rt_drv_touch *config)
{
    return RT_EOK;
}

int drv_touch_hw_init(void)
{
    rt_err_t result = RT_EOK;

    drv_touch.dev.info.range_x = LCD_WIDTH;
    drv_touch.dev.info.range_y = LCD_HEIGHT;
#ifdef TOUCH_USING_XPT2046
    drv_touch.dev.info.point_num = 1;
    drv_touch.dev.info.type = RT_TOUCH_TYPE_RESISTANCE;
    drv_touch.dev.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
#endif
#ifdef TOUCH_USING_SW
    drv_touch.dev.info.point_num = 1;
    drv_touch.dev.info.type = RT_TOUCH_TYPE_RESISTANCE;
    drv_touch.dev.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
#endif
    drv_touch.dev.config.user_data = &drv_touch;
#ifdef RT_TOUCH_PIN_IRQ
    drv_touch.dev.config.irq_pin.pin = rt_pin_get(TOUCH_IRQ_PIN);
    drv_touch.dev.config.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    rt_pin_mode(drv_touch.dev.config.irq_pin.pin, drv_touch.dev.config.irq_pin.mode);
#endif /* RT_TOUCH_PIN_IRQ */
    drv_touch.dev.ops = &drv_touch_ops;

    drv_touch_bus_init(&drv_touch);
    /* register lcd device */
    result = rt_hw_touch_register(&drv_touch.dev, "touch", RT_DEVICE_FLAG_INT_RX, &drv_touch);

    return result;
}
INIT_DEVICE_EXPORT(drv_touch_hw_init);

#ifdef TOUCH_USING_CALIBRATION

/* Please calibrate the resistive touch screen before use, it is best to store the calibrated data */
rt_err_t touch_calibration(const char *lcd_name,const char *touch_name)
{
#ifdef TOUCH_USING_KVDB
    struct fdb_blob blob = {0};
    /* GET the KV value */
    kvdb_get_blob(TOUCH_ADJUCT_KEY, fdb_blob_make(&blob, &drv_touch.adjuct, sizeof(drv_touch.adjuct)));
    /* the blob.saved.len is more than 0 when get the value successful */
    if (blob.saved.len > 0) {
        LOG_I(" Calibration result: %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d"
                , (int)drv_touch.adjuct.A, abs((int)((drv_touch.adjuct.A - (int)drv_touch.adjuct.A) * 100))
                , (int)drv_touch.adjuct.B, abs((int)((drv_touch.adjuct.B - (int)drv_touch.adjuct.B) * 100))
                , (int)drv_touch.adjuct.C, abs((int)((drv_touch.adjuct.C - (int)drv_touch.adjuct.C) * 100))
                , (int)drv_touch.adjuct.D, abs((int)((drv_touch.adjuct.D - (int)drv_touch.adjuct.D) * 100))
                , (int)drv_touch.adjuct.E, abs((int)((drv_touch.adjuct.E - (int)drv_touch.adjuct.E) * 100))
                , (int)drv_touch.adjuct.F, abs((int)((drv_touch.adjuct.F - (int)drv_touch.adjuct.F) * 100)));
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
    drv_touch.adjuct.A = ((x0 - x2) * (adyvalue[1] - adyvalue[2]) - (x1 - x2) * (adyvalue[0] - adyvalue[2])) / K;
    drv_touch.adjuct.B = ((adxvalue[0] - adxvalue[2]) * (x1 - x2) - (x0 - x2) * (adxvalue[1] - adxvalue[2])) / K;
    drv_touch.adjuct.C = (adyvalue[0] * (adxvalue[2] * x1 - adxvalue[1] * x2)
            + adyvalue[1] * (adxvalue[0] * x2 - adxvalue[2] * x0)
            + adyvalue[2] * (adxvalue[1] * x0 - adxvalue[0] * x1)) / K;
    drv_touch.adjuct.D = ((y0 - y2) * (adyvalue[1] - adyvalue[2]) - (y1 - y2) * (adyvalue[0] - adyvalue[2])) / K;
    drv_touch.adjuct.E = ((adxvalue[0] - adxvalue[2]) * (y1 - y2) - (y0 - y2) * (adxvalue[1] - adxvalue[2])) / K;
    drv_touch.adjuct.F = (adyvalue[0] * (adxvalue[2] * y1 - adxvalue[1] * y2)
            + adyvalue[1] * (adxvalue[0] * y2 - adxvalue[2] * y0)
            + adyvalue[2] * (adxvalue[1] * y0 - adxvalue[0] * y1)) / K;

    //验证校验值正确性
    posxvalue[3] = drv_touch.adjuct.A * adxvalue[3] + drv_touch.adjuct.B * adyvalue[3] + drv_touch.adjuct.C;
    posyvalue[3] = drv_touch.adjuct.D * adxvalue[3] + drv_touch.adjuct.E * adyvalue[3] + drv_touch.adjuct.F;
    if (abs(posxvalue[3] - x3) > cross_size || abs(posyvalue[3] - y3) > cross_size)
    {
        LOG_E("check: (%d, %d) (%d, %d) (%d, %d)", adxvalue[3], adyvalue[3], posxvalue[3], posyvalue[3], x3, y3);
        goto _adjust;
    }

#ifdef TOUCH_USING_KVDB
    /* CHANGE the KV value */
    kvdb_set_blob(TOUCH_ADJUCT_KEY, fdb_blob_make(&blob, &drv_touch.adjuct, sizeof(drv_touch.adjuct)));
#endif
    LOG_I(" Calibration result: %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d %d.%02d"
            , (int)drv_touch.adjuct.A, abs((int)((drv_touch.adjuct.A - (int)drv_touch.adjuct.A) * 100))
            , (int)drv_touch.adjuct.B, abs((int)((drv_touch.adjuct.B - (int)drv_touch.adjuct.B) * 100))
            , (int)drv_touch.adjuct.C, abs((int)((drv_touch.adjuct.C - (int)drv_touch.adjuct.C) * 100))
            , (int)drv_touch.adjuct.D, abs((int)((drv_touch.adjuct.D - (int)drv_touch.adjuct.D) * 100))
            , (int)drv_touch.adjuct.E, abs((int)((drv_touch.adjuct.E - (int)drv_touch.adjuct.E) * 100))
            , (int)drv_touch.adjuct.F, abs((int)((drv_touch.adjuct.F - (int)drv_touch.adjuct.F) * 100)));

    rt_device_close(lcd);
    rt_device_close(touch);
    return RT_EOK;
}

int drv_touch_calibration(void)
{
    touch_calibration("lcd", "touch");

    return RT_EOK;
}
INIT_ENV_EXPORT(drv_touch_calibration);
#endif

#ifdef BSP_USING_NO_LVGL_DEMO
static void touch_show_thread_entry(void* parameter)
{
    uint16_t black = 0b1111100000000000;
    uint16_t white = 0b1111111111111111;
    char *lcd_name = "lcd";
    char *touch_name = "touch";

    /* Find the TFT LCD device */
    rt_device_t lcd = rt_device_find(lcd_name);
    if (lcd == RT_NULL)
    {
        LOG_E(" cannot find lcd device named %s", lcd_name);
        return;
    }
    if (rt_device_open(lcd, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open lcd device named %s", lcd_name);
        return;
    }

    /* Find the Touch device */
    rt_device_t touch = rt_device_find(touch_name);
    if (touch == RT_NULL)
    {
        LOG_E(" cannot find touch device named %s", touch_name);
        return;
    }
    if (rt_device_open(touch, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        LOG_E(" cannot open touch device named %s", touch_name);
        return;
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

    uint8_t raw_idx = 0;
    struct rt_touch_data read_data = {0};
    while (1)
    {
        rt_thread_mdelay(100);

        if (rt_device_read((rt_device_t) touch, 0, &read_data, 1) == 1)
        {
            LOG_I(" %d point press (%d, %d)(%d, %d)", raw_idx, read_data.x, read_data.y,
                    read_data.x_coordinate, read_data.y_coordinate);
            if (read_data.x_coordinate < lcd_info.width - cross_size || read_data.y_coordinate < lcd_info.height - cross_size)
            {
                if (raw_idx % 16 == 0)
                {
                    /* After processing a point, proceed to clear the screen */
                    for (uint16_t y = 0; y < lcd_info.height; ++y)
                    {
                        rt_graphix_ops(lcd)->draw_hline((const char *) (&white), 0, lcd_info.width, y);
                    }
                    rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
                }

                rt_graphix_ops(lcd)->draw_hline((const char *) (&black), read_data.x_coordinate - cross_size,
                        read_data.x_coordinate + cross_size, read_data.y_coordinate);
                rt_graphix_ops(lcd)->draw_vline((const char *) (&black), read_data.x_coordinate,
                        read_data.y_coordinate - cross_size, read_data.y_coordinate + cross_size);
                rt_device_control(lcd, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

                raw_idx++;
            }
        }
    }

    rt_device_close(lcd);
    rt_device_close(touch);
}

int drv_touch_show(void)
{
    rt_thread_t touch_show_thread = rt_thread_create( "touch_show",
                                    touch_show_thread_entry,
                                    NULL,
                                    2048,
                                    24,
                                    10);
    if ( touch_show_thread != RT_NULL)
    {
        rt_thread_startup(touch_show_thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(drv_touch_show);
#endif

#ifdef PKG_USING_FLASHDB_PORT
int touch_adjust_del(void)
{
    return kvdb_del(TOUCH_ADJUCT_KEY);
}
MSH_CMD_EXPORT(touch_adjust_del, touch adjust del);
#endif

#endif
