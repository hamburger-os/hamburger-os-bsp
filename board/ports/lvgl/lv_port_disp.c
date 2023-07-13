/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */
#include <lvgl.h>
#include <board.h>

#define DRV_DEBUG
#define LOG_TAG "drv.lv"
#include <drv_log.h>

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

static rt_device_t lcd_device = RT_NULL;
static struct rt_device_graphic_info info;

static lv_disp_drv_t disp_drv;  /*Descriptor of a display driver*/

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void lcd_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /* color_p is a buffer pointer; the buffer is provided by LVGL */
    extern void lcd_fill_array(rt_uint16_t x_start, rt_uint16_t y_start, rt_uint16_t x_end, rt_uint16_t y_end, void *pcolor);
    lcd_fill_array(area->x1, area->y1, area->x2, area->y2, color_p);

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    rt_err_t result;
    lv_color_t *fbuf1, *fbuf2;

    lcd_device = rt_device_find("lcd");
    if (lcd_device == RT_NULL)
    {
        LOG_E("find lcd error!");
        return;
    }
    result = rt_device_open(lcd_device, 0);
    if (result != RT_EOK)
    {
        LOG_E("open lcd error!");
        return;
    }
    /* get framebuffer address */
    result = rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_INFO, &info);
    if (result != RT_EOK)
    {
        LOG_E("get lcd info error!");
        /* get device information failed */
        return;
    }

    RT_ASSERT(info.bits_per_pixel == 8 || info.bits_per_pixel == 16 ||
              info.bits_per_pixel == 24 || info.bits_per_pixel == 32);

    fbuf1 = rt_malloc(info.width * info.height * sizeof(lv_color_t));
    if (fbuf1 == RT_NULL)
    {
        LOG_E("alloc disp buf fail!");
        return;
    }

    fbuf2 = rt_malloc(info.width * info.height * sizeof(lv_color_t));
    if (fbuf2 == RT_NULL)
    {
        LOG_E("alloc disp buf fail!");
        rt_free(fbuf1);
        return;
    }

    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    lv_disp_draw_buf_init(&disp_buf, fbuf1, fbuf2, info.width * info.height);

    lv_disp_drv_init(&disp_drv); /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = info.width;
    disp_drv.ver_res = info.height;

    /*Set a display buffer*/
    disp_drv.draw_buf = &disp_buf;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = lcd_flush_cb;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}
