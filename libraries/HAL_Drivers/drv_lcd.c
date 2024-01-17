/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-08     zylx         first version
 */
#include <string.h>

#include <board.h>

#ifdef BSP_USING_LCD
#include "lv_conf.h"

#define DRV_DEBUG
#define LOG_TAG "drv.lcd"
#include <drv_log.h>

#define LCD_DEVICE(dev)     (struct drv_lcd_device*)(dev)
#define LCD_BUF_SIZE        (LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8)
#define LCD_PIXEL_FORMAT    RTGRAPHIC_PIXEL_FORMAT_RGB565
#define LCD_RGB2BGR(x)      (((x & 0xf800) >> 11) | (x & 0x07e0) | ((x & 0x001f) << 11))

struct drv_lcd_device
{
    struct rt_device parent;

    struct rt_device_graphic_info lcd_info;

    struct rt_semaphore lcd_lock;
    struct rt_completion dma2d_completion;

    /* 0:front_buf is being used 1: back_buf is being used*/
    rt_uint8_t cur_buf;
    rt_uint8_t *front_buf;
    rt_uint8_t *back_buf;

    LTDC_HandleTypeDef LtdcHandle;
    DMA2D_HandleTypeDef hdma2d;
};
static struct drv_lcd_device _lcd;

static rt_err_t drv_lcd_init(struct rt_device *device)
{
    struct drv_lcd_device *lcd = LCD_DEVICE(device);
    /* nothing, right now */
    lcd = lcd;
    return RT_EOK;
}

static void lcd_DMA2D_Copy(void * pSrc, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLineSrc, uint32_t OffLineDst)
{
    struct drv_lcd_device *lcd = &_lcd;

    lcd->hdma2d.Init.OutputOffset = OffLineDst;
    lcd->hdma2d.LayerCfg[1].InputOffset = OffLineSrc;
    HAL_DMA2D_Init(&lcd->hdma2d);
    HAL_DMA2D_ConfigLayer(&lcd->hdma2d, 1);
    HAL_DMA2D_Start_IT(&lcd->hdma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, ySize);
    if (rt_completion_wait(&lcd->dma2d_completion, 1000) != RT_EOK)
    {
        LOG_E("dma2d copy timeout %d %d %d %d!", xSize, ySize, OffLineSrc, OffLineDst);
    }
}

static rt_err_t drv_lcd_control(struct rt_device *device, int cmd, void *args)
{
    struct drv_lcd_device *lcd = LCD_DEVICE(device);

    switch (cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
    {
        /* update */
        if (_lcd.cur_buf)
        {
            /* back_buf is being used */
            lcd_DMA2D_Copy(_lcd.lcd_info.framebuffer, _lcd.front_buf, LCD_WIDTH, LCD_HEIGHT, 0, 0);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
            SCB_InvalidateDCache_by_Addr((uint32_t *)_lcd.front_buf, LCD_BUF_SIZE);
#endif
            /* Configure the color frame buffer start address */
            LTDC_LAYER(&lcd->LtdcHandle, 0)->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
            LTDC_LAYER(&lcd->LtdcHandle, 0)->CFBAR = (uint32_t)(_lcd.front_buf);
            _lcd.cur_buf = 0;
        }
        else
        {
            /* front_buf is being used */
            lcd_DMA2D_Copy(_lcd.lcd_info.framebuffer, _lcd.back_buf, LCD_WIDTH, LCD_HEIGHT, 0, 0);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
            SCB_InvalidateDCache_by_Addr((uint32_t *)_lcd.back_buf, LCD_BUF_SIZE);
#endif
            /* Configure the color frame buffer start address */
            LTDC_LAYER(&lcd->LtdcHandle, 0)->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
            LTDC_LAYER(&lcd->LtdcHandle, 0)->CFBAR = (uint32_t)(_lcd.back_buf);
            _lcd.cur_buf = 1;
        }
        rt_sem_take(&_lcd.lcd_lock, RT_TICK_PER_SECOND / 20);
        HAL_LTDC_Reload(&lcd->LtdcHandle, LTDC_SRCR_VBR);
    }
    break;

    case RTGRAPHIC_CTRL_GET_INFO:
    {
        struct rt_device_graphic_info *info = (struct rt_device_graphic_info *)args;

        RT_ASSERT(info != RT_NULL);
        info->pixel_format = lcd->lcd_info.pixel_format;
        info->bits_per_pixel = 16;
        info->width = lcd->lcd_info.width;
        info->height = lcd->lcd_info.height;
        info->framebuffer = lcd->lcd_info.framebuffer;
    }
    break;

    default:
        return -RT_EINVAL;
    }

    return RT_EOK;
}

void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc)
{
    /* enable line interupt */
    __HAL_LTDC_ENABLE_IT(&_lcd.LtdcHandle, LTDC_IER_LIE);
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc)
{
    rt_sem_release(&_lcd.lcd_lock);
}

void HAL_DMA2D_LineEventCallback(DMA2D_HandleTypeDef *hdma2d)
{

}

void HAL_DMA2D_CLUTLoadingCpltCallback(DMA2D_HandleTypeDef *hdma2d)
{

}

/**
  * @brief  DMA2D Transfer completed callback
  * @param  hdma2d: DMA2D handle.
  * @note   This example shows a simple way to report end of DMA2D transfer, and
  *         you can add your own implementation.
  * @retval None
  */
static void HAL_DMA2D_TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
    rt_completion_done(&_lcd.dma2d_completion);
}

/**
  * @brief  DMA2D error callbacks
  * @param  hdma2d: DMA2D handle
  * @note   This example shows a simple way to report DMA2D transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
static void HAL_DMA2D_TransferError(DMA2D_HandleTypeDef *hdma2d)
{
    rt_completion_done(&_lcd.dma2d_completion);
    LOG_E("DMA2D Transfer Error!");
}

void LTDC_IRQHandler(void)
{
    rt_interrupt_enter();

    HAL_LTDC_IRQHandler(&_lcd.LtdcHandle);

    rt_interrupt_leave();
}

void DMA2D_IRQHandler(void)
{
    rt_interrupt_enter();

    HAL_DMA2D_IRQHandler(&_lcd.hdma2d);

    rt_interrupt_leave();
}

static rt_err_t stm32_lcd_init(struct drv_lcd_device *lcd)
{
    //初始化dma2d
    lcd->hdma2d.Instance = DMA2D;
    lcd->hdma2d.Init.Mode = DMA2D_M2M;
    lcd->hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
    lcd->hdma2d.Init.OutputOffset = 0;
    lcd->hdma2d.LayerCfg[1].InputOffset = 0;
    lcd->hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
    /* Pixel Format configuration*/
    if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_RGB565)
    {
        lcd->hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
    }
    else if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_ARGB888)
    {
        lcd->hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    }
    else if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_RGB888)
    {
        lcd->hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
    }
    lcd->hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    lcd->hdma2d.LayerCfg[1].InputAlpha = 0xFF;
#ifdef SOC_SERIES_STM32H7
    lcd->hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
#ifdef LCD_USING_RGB2BGR
    lcd->hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_SWAP;
#else
    lcd->hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
#endif
    lcd->hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
#endif
    /* DMA2D Callbacks */
    lcd->hdma2d.XferCpltCallback = HAL_DMA2D_TransferComplete;
    lcd->hdma2d.XferErrorCallback = HAL_DMA2D_TransferError;
    if (HAL_DMA2D_Init(&lcd->hdma2d) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_DMA2D_ConfigLayer(&lcd->hdma2d, 1) != HAL_OK)
    {
        Error_Handler();
    }

    LTDC_LayerCfgTypeDef pLayerCfg = {0};

    /* LTDC Initialization -------------------------------------------------------*/

    /* Polarity configuration */
    /* Initialize the horizontal synchronization polarity as active low */
    lcd->LtdcHandle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    /* Initialize the vertical synchronization polarity as active low */
    lcd->LtdcHandle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    /* Initialize the data enable polarity as active low */
    lcd->LtdcHandle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    /* Initialize the pixel clock polarity as input pixel clock */
    lcd->LtdcHandle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    /* Timing configuration */
    /* Horizontal synchronization width = Hsync - 1 */
    lcd->LtdcHandle.Init.HorizontalSync = LCD_HSYNC_WIDTH - 1;
    /* Vertical synchronization height = Vsync - 1 */
    lcd->LtdcHandle.Init.VerticalSync = LCD_VSYNC_HEIGHT - 1;
    /* Accumulated horizontal back porch = Hsync + HBP - 1 */
    lcd->LtdcHandle.Init.AccumulatedHBP = LCD_HSYNC_WIDTH + LCD_HBP - 1;
    /* Accumulated vertical back porch = Vsync + VBP - 1 */
    lcd->LtdcHandle.Init.AccumulatedVBP = LCD_VSYNC_HEIGHT + LCD_VBP - 1;
    /* Accumulated active width = Hsync + HBP + Active Width - 1 */
    lcd->LtdcHandle.Init.AccumulatedActiveW = LCD_HSYNC_WIDTH + LCD_HBP + lcd->lcd_info.width - 1;
    /* Accumulated active height = Vsync + VBP + Active Heigh - 1 */
    lcd->LtdcHandle.Init.AccumulatedActiveH = LCD_VSYNC_HEIGHT + LCD_VBP + lcd->lcd_info.height - 1;
    /* Total height = Vsync + VBP + Active Heigh + VFP - 1 */
    lcd->LtdcHandle.Init.TotalHeigh = lcd->LtdcHandle.Init.AccumulatedActiveH + LCD_VFP;
    /* Total width = Hsync + HBP + Active Width + HFP - 1 */
    lcd->LtdcHandle.Init.TotalWidth = lcd->LtdcHandle.Init.AccumulatedActiveW + LCD_HFP;

    /* Configure R,G,B component values for LCD background color */
    lcd->LtdcHandle.Init.Backcolor.Blue = 0;
    lcd->LtdcHandle.Init.Backcolor.Green = 0;
    lcd->LtdcHandle.Init.Backcolor.Red = 0;

    lcd->LtdcHandle.Instance = LTDC;

    /* Layer1 Configuration ------------------------------------------------------*/

    /* Windowing configuration */
    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowX1 = lcd->lcd_info.width;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowY1 = lcd->lcd_info.height;

    /* Pixel Format configuration*/
    if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_RGB565)
    {
        pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    }
    else if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_ARGB888)
    {
        pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
    }
    else if (lcd->lcd_info.pixel_format == RTGRAPHIC_PIXEL_FORMAT_RGB888)
    {
        pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
    }
    else
    {
        LOG_E("unsupported pixel format");
        return -RT_ERROR;
    }

    /* Start Address configuration : frame buffer is located at FLASH memory */
    pLayerCfg.FBStartAdress = (uint32_t)lcd->front_buf;

    /* Alpha constant (255 totally opaque) */
    pLayerCfg.Alpha = 255;

    /* Default Color configuration (configure A,R,G,B component values) */
    pLayerCfg.Alpha0 = 0;
    pLayerCfg.Backcolor.Blue = 0;
    pLayerCfg.Backcolor.Green = 0;
    pLayerCfg.Backcolor.Red = 0;

    /* Configure blending factors */
    /* Constant Alpha value:  pLayerCfg.Alpha / 255
       C: Current Layer Color
       Cs: Background color
       BC = Constant Alpha x C + (1 - Constant Alpha ) x Cs */
    /* BlendingFactor1: Pixel Alpha x Constant Alpha */
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    /* BlendingFactor2: 1 - (Pixel Alpha x Constant Alpha) */
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

    /* Configure the number of lines and number of pixels per line */
    pLayerCfg.ImageWidth = lcd->lcd_info.width;
    pLayerCfg.ImageHeight = lcd->lcd_info.height;

    /* Configure the LTDC */
    if (HAL_LTDC_Init(&lcd->LtdcHandle) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure the Background Layer*/
    if (HAL_LTDC_ConfigLayer(&lcd->LtdcHandle, &pLayerCfg, 0) != HAL_OK)
    {
        Error_Handler();
    }

    /* enable interrupt */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
    HAL_NVIC_SetPriority(LTDC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    LOG_D("LTDC init success");
    return RT_EOK;
}
#if defined(LCD_BACKLIGHT_USING_PWM)
static void turn_on_lcd_backlight(void)
{
    struct rt_device_pwm *pwm_dev;

    /* turn on the LCD backlight */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(LCD_PWM_DEV_NAME);
    /* pwm frequency:100K = 10000ns */
    rt_pwm_set(pwm_dev, LCD_PWM_DEV_CHANNEL, 10000, 10000);
    rt_pwm_enable(pwm_dev, LCD_PWM_DEV_CHANNEL);
}
#elif defined(LCD_BACKLIGHT_USING_GPIO)
static void turn_on_lcd_backlight(void)
{
    /* turn on the LCD backlight */
    rt_base_t ctl_pin = rt_pin_get(LCD_BKLT_CTL_GPIO);
    rt_pin_mode(ctl_pin, PIN_MODE_OUTPUT);

    rt_pin_write(ctl_pin, PIN_LOW);
}
#else
static void turn_on_lcd_backlight(void)
{
    /* turn on the LCD backlight */
}
#endif

void lcd_fill_array(rt_uint16_t x_start, rt_uint16_t y_start, rt_uint16_t x_end, rt_uint16_t y_end, void *pcolor)
{
    struct drv_lcd_device *lcd = &_lcd;
    uint16_t *pixel = (uint16_t *)pcolor;
    uint16_t *framebuffer = (uint16_t *)&lcd->lcd_info.framebuffer[2 * (y_start * lcd->lcd_info.width + x_start)];

    lcd_DMA2D_Copy(pixel, framebuffer, x_end - x_start + 1, y_end - y_start + 1, 0, lcd->lcd_info.width - x_end + x_start - 1);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr((uint32_t *)lcd->lcd_info.framebuffer, LCD_BUF_SIZE);
#endif

    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops lcd_ops =
{
    drv_lcd_init,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    drv_lcd_control
};
#endif

//画点
//x,y:坐标
//color:颜色
static void drv_lcd_set_pixel(const char *pixel, int x, int y)
{
    uint16_t color = *((uint16_t *)pixel);
#ifdef LCD_USING_RGB2BGR
    color = LCD_RGB2BGR(color);
#endif
    uint16_t *framebuffer;

    struct drv_lcd_device *lcd = &_lcd;

    framebuffer = (uint16_t *)&lcd->lcd_info.framebuffer[2 * (y * lcd->lcd_info.width + x)];
    *framebuffer = color;
}

//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
static void drv_lcd_get_pixel(char *pixel, int x, int y)
{
    uint16_t *color = (uint16_t *)pixel;
    uint16_t *framebuffer;

    struct drv_lcd_device *lcd = &_lcd;

    framebuffer = (uint16_t *)&lcd->lcd_info.framebuffer[2 * (y * lcd->lcd_info.width + x)];
    *color = *framebuffer;
#ifdef LCD_USING_RGB2BGR
    *color = LCD_RGB2BGR(*color);
#endif
}

//画横线
static void drv_lcd_draw_hline(const char *pixel, int x1, int x2, int y)
{
    uint16_t color = *((uint16_t *)pixel);
#ifdef LCD_USING_RGB2BGR
    color = LCD_RGB2BGR(color);
#endif
    uint16_t *framebuffer;

    struct drv_lcd_device *lcd = &_lcd;
    uint16_t x_offset = 0;

    for(x_offset = 0;x1 + x_offset <= x2; x_offset++)
    {
        framebuffer = (uint16_t *)&lcd->lcd_info.framebuffer[2 * (y * lcd->lcd_info.width + x_offset + x1)];
        *framebuffer = color;
    }
}

//画竖线
static void drv_lcd_draw_vline(const char *pixel, int x, int y1, int y2)
{
    uint16_t color = *((uint16_t *)pixel);
#ifdef LCD_USING_RGB2BGR
    color = LCD_RGB2BGR(color);
#endif
    uint16_t *framebuffer;

    struct drv_lcd_device *lcd = &_lcd;
    uint16_t cycle_y = 0;

    for(cycle_y = y1; cycle_y <= y2; cycle_y++)
    {
        framebuffer = (uint16_t *)&lcd->lcd_info.framebuffer[2 * (cycle_y * lcd->lcd_info.width + x)];
        *framebuffer = color;
    }
}

//画线
static void drv_lcd_blit_line(const char *pixel, int x, int y, rt_size_t size)
{
}

static struct rt_device_graphic_ops drv_lcd_ops =
{
    drv_lcd_set_pixel,
    drv_lcd_get_pixel,
    drv_lcd_draw_hline,
    drv_lcd_draw_vline,
    drv_lcd_blit_line,
};

static int drv_lcd_hw_init(void)
{
    rt_err_t result = RT_EOK;
    struct rt_device *device = &_lcd.parent;

    /* memset _lcd to zero */
    rt_memset(&_lcd, 0x00, sizeof(_lcd));

    /* init lcd_lock semaphore */
    result = rt_sem_init(&_lcd.lcd_lock, "lcd_lock", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("init semaphore failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    /* init dma2d completion */
    rt_completion_init(&_lcd.dma2d_completion);

    /* config LCD dev info */
    _lcd.lcd_info.height = LCD_HEIGHT;
    _lcd.lcd_info.width = LCD_WIDTH;
    _lcd.lcd_info.bits_per_pixel = LCD_BITS_PER_PIXEL;
    _lcd.lcd_info.pixel_format = LCD_PIXEL_FORMAT;

    /* malloc memory for Triple Buffering */
    _lcd.lcd_info.framebuffer = rt_malloc_align(LCD_BUF_SIZE, RT_ALIGN_SIZE);
    _lcd.back_buf = rt_malloc_align(LCD_BUF_SIZE, RT_ALIGN_SIZE);
    _lcd.front_buf = rt_malloc_align(LCD_BUF_SIZE, RT_ALIGN_SIZE);
    if (_lcd.lcd_info.framebuffer == RT_NULL || _lcd.back_buf == RT_NULL || _lcd.front_buf == RT_NULL)
    {
        LOG_E("init frame buffer failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* memset buff to 0xFF */
    rt_memset(_lcd.lcd_info.framebuffer, 0xFF, LCD_BUF_SIZE);
    rt_memset(_lcd.back_buf, 0xFF, LCD_BUF_SIZE);
    rt_memset(_lcd.front_buf, 0xFF, LCD_BUF_SIZE);

    device->type = RT_Device_Class_Graphic;
#ifdef RT_USING_DEVICE_OPS
    device->ops = &lcd_ops;
#else
    device->init = drv_lcd_init;
    device->control = drv_lcd_control;
#endif
    device->user_data = &drv_lcd_ops;

    /* init stm32 LTDC */
    if (stm32_lcd_init(&_lcd) != RT_EOK)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    turn_on_lcd_backlight();

    /* register lcd device */
    rt_device_register(device, "lcd", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

__exit:
    if (result != RT_EOK)
    {
        LOG_E("init lcd failed!");
        rt_sem_detach(&_lcd.lcd_lock);

        if (_lcd.lcd_info.framebuffer)
        {
            rt_free(_lcd.lcd_info.framebuffer);
        }

        if (_lcd.back_buf)
        {
            rt_free(_lcd.back_buf);
        }

        if (_lcd.front_buf)
        {
            rt_free(_lcd.front_buf);
        }
    }
    return result;
}
INIT_DEVICE_EXPORT(drv_lcd_hw_init);

#ifdef BSP_USING_LCD_TEST
static int lcd_test(void)
{
    uint16_t *pixel;    //核心板的lvds转接板将RGB接为了BGR
    struct drv_lcd_device *lcd = &_lcd;

    /* red */
    for (int i = 0; i < LCD_BUF_SIZE / 2; i++)
    {
        pixel = (uint16_t *)&lcd->lcd_info.framebuffer[2 * i];
        *pixel = 0b0000000000011111;
    }
    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    rt_thread_mdelay(1000);
    /* green */
    for (int i = 0; i < LCD_BUF_SIZE / 2; i++)
    {
        pixel = (uint16_t *)&lcd->lcd_info.framebuffer[2 * i];
        *pixel = 0b0000011111100000;
    }
    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    rt_thread_mdelay(1000);
    /* blue */
    for (int i = 0; i < LCD_BUF_SIZE / 2; i++)
    {
        pixel = (uint16_t *)&lcd->lcd_info.framebuffer[2 * i];
        *pixel = 0b1111100000000000;
    }
    lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    rt_thread_mdelay(1000);

    return 0;
}
MSH_CMD_EXPORT(lcd_test, lcd_test);
#endif
#endif /* BSP_USING_LCD */
