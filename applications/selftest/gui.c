/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-27     lvhan       the first version
 */

#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <string.h>

#include "selftest.h"

#ifdef PKG_USING_LVGL
#include <lvgl.h>
#endif

#include "sysinfo.h"

#define DBG_TAG "gui "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#if defined(PKG_USING_LVGL) && defined(BSP_USING_NO_LVGL_DEMO)
struct ui_object
{
    lv_style_t bg_style;
    lv_grad_dsc_t bg_grad;

    lv_obj_t * tileview;
    lv_obj_t * tile_state;
    lv_obj_t * tile_main;
    lv_obj_t * tile_file;

    lv_obj_t * label_logo;
    lv_obj_t * label_icon;
    lv_obj_t * icon;

    lv_obj_t * slider_lcd;
    lv_obj_t * slider_lcd_label;

    lv_obj_t * table_result;
    lv_obj_t * btn_enter;
    lv_obj_t * btn_enter_label;

    lv_obj_t * file_explorer;

    lv_obj_t * keyboard;
    lv_obj_t * textarea;

    lv_obj_t * table_monitor;

    lv_obj_t * cont_row_ps;
    lv_obj_t * cont_row_free;

    lv_obj_t * btn_refresh;
    lv_obj_t * btn_refresh_label;

    rt_thread_t thread_selftest;
    rt_device_t lcd_device;
};
static struct ui_object ui = {0};

void gui_display_result(SelftestResult *presult, uint16_t num)
{
    lv_mutex_take();

    if (ui.table_result != NULL)
    {
        lv_obj_clean(ui.table_result);

        for (uint16_t i = 0; i < num; i++)
        {
            uint16_t row = i / 3;
            uint16_t col[2];
            if (i % 3 == 0)
            {
                col[0] = 0;
                col[1] = 1;
            }
            else if (i % 3 == 1)
            {
                col[0] = 2;
                col[1] = 3;
            }
            else if (i % 3 == 2)
            {
                col[0] = 4;
                col[1] = 5;
            }

            lv_table_set_cell_value(ui.table_result, row, col[0], presult[i].name);
            if (presult[i].result == 0)
            {
                lv_table_set_cell_value(ui.table_result, row, col[1], LV_SYMBOL_OK);
            }
            else
            {
                lv_table_set_cell_value(ui.table_result, row, col[1], LV_SYMBOL_CLOSE);
            }
        }
    }

    lv_mutex_release();
}

static void selftest_thread_entry(void *parameter)
{
    system("selftest_start");

    ui.thread_selftest = NULL;
}

static void gesture_event_cb(lv_event_t *event)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

    switch (dir)
    {
    case LV_DIR_BOTTOM:
    {
        char str[512] = {0};
        struct SysInfoDef info = {0};
        sysinfo_get(&info);
        char SN_str[sizeof(info.SN) + 1] = {0};
        rt_memcpy(SN_str, info.SN, sizeof(info.SN));

        rt_snprintf(str, sizeof(str),
                "----------------------------------------\n"
                "- version : %s\n"
                "- type : 0x%X\n"
                "- sn : %s\n"
                "- cpu temp : %d (C * 100) \n"
                "- chip id : %02X %02X %02X %02X %02X %02X %02X %02X\n"
                "- chip temp : %d (C * 100) \n"
                "- times : %d s\n"
                "- count : %d\n"
                "----------------------------------------\n"
                "Powered by hnthinker team\n"
                , info.version, info.type, SN_str, info.cpu_temp
                , info.chip_id[0], info.chip_id[1], info.chip_id[2], info.chip_id[3], info.chip_id[4], info.chip_id[5], info.chip_id[6], info.chip_id[7]
                , info.chip_temp, info.times, info.count);
        lv_obj_t * mbox = lv_msgbox_create(NULL, "SWOS2", str, NULL, true);
        lv_obj_set_width(mbox, LV_HOR_RES_MAX/2);
        lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
    }
        break;
    default:
        break;
    }
}

#define LIST_FIND_OBJ_NR 8

typedef struct
{
    rt_list_t *list;
    rt_list_t **array;
    rt_uint8_t type;
    int nr;             /* input: max nr, can't be 0 */
    int nr_out;         /* out: got nr */
} list_get_next_t;

static void list_find_init(list_get_next_t *p, rt_uint8_t type, rt_list_t **array, int nr)
{
    struct rt_object_information *info;
    rt_list_t *list;

    info = rt_object_get_information((enum rt_object_class_type)type);
    list = &info->object_list;

    p->list = list;
    p->type = type;
    p->array = array;
    p->nr = nr;
    p->nr_out = 0;
}

static rt_list_t *list_get_next(rt_list_t *current, list_get_next_t *arg)
{
    int first_flag = 0;
    rt_base_t level;
    rt_list_t *node, *list;
    rt_list_t **array;
    int nr;

    arg->nr_out = 0;

    if (!arg->nr || !arg->type)
    {
        return (rt_list_t *)RT_NULL;
    }

    list = arg->list;

    if (!current) /* find first */
    {
        node = list;
        first_flag = 1;
    }
    else
    {
        node = current;
    }

    level = rt_hw_interrupt_disable();

    if (!first_flag)
    {
        struct rt_object *obj;
        /* The node in the list? */
        obj = rt_list_entry(node, struct rt_object, list);
        if ((obj->type & ~RT_Object_Class_Static) != arg->type)
        {
            rt_hw_interrupt_enable(level);
            return (rt_list_t *)RT_NULL;
        }
    }

    nr = 0;
    array = arg->array;
    while (1)
    {
        node = node->next;

        if (node == list)
        {
            node = (rt_list_t *)RT_NULL;
            break;
        }
        nr++;
        *array++ = node;
        if (nr == arg->nr)
        {
            break;
        }
    }

    rt_hw_interrupt_enable(level);
    arg->nr_out = nr;
    return node;
}

static long list_thread(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    list_find_init(&find_arg, RT_Object_Class_Thread, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_thread thread_info, *thread;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();

                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }
                /* copy info */
                rt_memcpy(&thread_info, obj, sizeof thread_info);
                rt_hw_interrupt_enable(level);

                thread = (struct rt_thread *)obj;
                {
                    rt_uint8_t stat;
                    rt_uint8_t *ptr;

                    stat = (thread->stat & RT_THREAD_STAT_MASK);

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
                    ptr = (rt_uint8_t *)thread->stack_addr + thread->stack_size - 1;
                    while (*ptr == '#')ptr --;
#else
                    ptr = (rt_uint8_t *)thread->stack_addr;
                    while (*ptr == '#') ptr ++;

                    lv_obj_t * cont_line = lv_obj_create(ui.cont_row_ps);
                    lv_obj_set_size(cont_line, LV_HOR_RES_MAX*19/48, LV_VER_RES_MAX/10);
                    lv_obj_align(cont_line, LV_ALIGN_CENTER, 0, 0);
                    lv_obj_set_flex_flow(cont_line, LV_FLEX_FLOW_ROW);

                    lv_obj_t * label = lv_label_create(cont_line);
                    lv_obj_set_size(label, LV_HOR_RES_MAX*4/48, LV_VER_RES_MAX/36);
                    lv_label_set_text_fmt(label, "%s", thread->name);

                    label = lv_label_create(cont_line);
                    lv_obj_set_size(label, LV_HOR_RES_MAX*3/48, LV_VER_RES_MAX/36);
                    if (stat == RT_THREAD_READY)        lv_label_set_text_fmt(label, LV_SYMBOL_PREV"%2d", thread->current_priority);
                    else if (stat == RT_THREAD_SUSPEND) lv_label_set_text_fmt(label, LV_SYMBOL_PAUSE"%2d", thread->current_priority);
                    else if (stat == RT_THREAD_INIT)    lv_label_set_text_fmt(label, LV_SYMBOL_OK"%2d", thread->current_priority);
                    else if (stat == RT_THREAD_CLOSE)   lv_label_set_text_fmt(label, LV_SYMBOL_CLOSE"%2d", thread->current_priority);
                    else if (stat == RT_THREAD_RUNNING) lv_label_set_text_fmt(label, LV_SYMBOL_PLAY"%2d", thread->current_priority);

                    uint16_t value = (thread->stack_size - ((rt_ubase_t) ptr - (rt_ubase_t) thread->stack_addr)) * 100 / thread->stack_size;
                    lv_obj_t * bar = lv_bar_create(cont_line);
                    lv_obj_set_size(bar, LV_HOR_RES_MAX*4/48, LV_VER_RES_MAX/36);
                    lv_bar_set_range(bar, 0, 100);
                    lv_bar_set_value(bar, value, LV_ANIM_ON);
                    lv_bar_set_mode(bar, LV_BAR_MODE_RANGE);

                    label = lv_label_create(cont_line);
                    lv_obj_set_size(label, LV_HOR_RES_MAX*3/48, LV_VER_RES_MAX/36);
                    lv_label_set_text_fmt(label, "%2u%%", value);
#endif
                }
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}

static long list_memheap(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    rt_list_t *next = (rt_list_t *)RT_NULL;

    list_find_init(&find_arg, RT_Object_Class_MemHeap, obj_list, sizeof(obj_list) / sizeof(obj_list[0]));

    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_memheap *mh;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);
                level = rt_hw_interrupt_disable();
                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    rt_hw_interrupt_enable(level);
                    continue;
                }

                rt_hw_interrupt_enable(level);

                mh = (struct rt_memheap *)obj;

                lv_obj_t * cont_line = lv_obj_create(ui.cont_row_free);
                lv_obj_set_size(cont_line, LV_HOR_RES_MAX*19/48, LV_VER_RES_MAX/10);
                lv_obj_align(cont_line, LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_flex_flow(cont_line, LV_FLEX_FLOW_ROW);

                lv_obj_t * label = lv_label_create(cont_line);
                lv_obj_set_size(label, LV_HOR_RES_MAX*4/48, LV_VER_RES_MAX/36);
                lv_label_set_text_fmt(label, "%s", mh->parent.name);

                label = lv_label_create(cont_line);
                lv_obj_set_size(label, LV_HOR_RES_MAX*4/48, LV_VER_RES_MAX/36);
                if (mh->available_size > 1024 * 1024) lv_label_set_text_fmt(label, "%dMB", mh->available_size/1024/1024);
                else lv_label_set_text_fmt(label, "%dKB", mh->available_size/1024);

                lv_obj_t * bar = lv_bar_create(cont_line);
                lv_obj_set_size(bar, LV_HOR_RES_MAX*3/48, LV_VER_RES_MAX/36);
                lv_bar_set_range(bar, 0, mh->pool_size);
                lv_bar_set_value(bar, mh->pool_size - mh->available_size, LV_ANIM_ON);
                lv_bar_set_mode(bar, LV_BAR_MODE_RANGE);

                label = lv_label_create(cont_line);
                lv_obj_set_size(label, LV_HOR_RES_MAX*3/48, LV_VER_RES_MAX/36);
                lv_label_set_text_fmt(label, "%2u%%", (mh->pool_size - mh->available_size)*100/mh->pool_size);
            }
        }
    }
    while (next != (rt_list_t *)RT_NULL);

    return 0;
}

static void slider_event_cb(lv_event_t * event)
{
    lv_obj_t * slider = lv_event_get_target(event);
    uint32_t value = lv_slider_get_value(slider);

    lv_label_set_text_fmt(ui.slider_lcd_label, "%d%%", (int)value);

    if (ui.lcd_device != NULL)
    {
        rt_device_control(ui.lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &value);
    }
}

static void btn_pressed_event_cb(lv_event_t *event)
{
    lv_obj_t * btn = lv_event_get_target(event);
    lv_obj_t * label = lv_obj_get_child(btn, 0);

    if (rt_strcmp(lv_label_get_text(label), LV_SYMBOL_PLAY) == 0)
    {
        if (ui.thread_selftest == NULL)
        {
            /* 创建 自测 线程 */
            ui.thread_selftest = rt_thread_create("selftest", selftest_thread_entry, NULL, 4096, 22, 10);
            /* 创建成功则启动线程 */
            if (ui.thread_selftest != RT_NULL)
            {
                rt_thread_startup(ui.thread_selftest);
            }
            else
            {
                LOG_E("thread startup error!");
            }
        }
    }
    else if (rt_strcmp(lv_label_get_text(label), LV_SYMBOL_REFRESH) == 0)
    {
        lv_obj_clean(ui.cont_row_ps);
        lv_obj_clean(ui.cont_row_free);

        //线程监控
        list_thread();
        //内存监控
        list_memheap();
    }
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

void lv_user_gui_init(void)
{
    ui.lcd_device = rt_device_find("lcd");
    if (ui.lcd_device == RT_NULL)
    {
        LOG_E("find lcd error!");
        return;
    }

    lv_style_init(&ui.bg_style);

    /*Make a gradient*/
    lv_style_set_bg_opa(&ui.bg_style, LV_OPA_50);
    ui.bg_grad.dir = LV_GRAD_DIR_VER;
    ui.bg_grad.stops_count = 2;
    ui.bg_grad.stops[0].color = lv_color_hex(0xADD8E6);
    ui.bg_grad.stops[1].color = lv_color_hex(0x4169E1);

    /*Shift the gradient to the bottom*/
    ui.bg_grad.stops[0].frac  = 1;
    ui.bg_grad.stops[1].frac  = 255;

    lv_style_set_bg_grad(&ui.bg_style, &ui.bg_grad);

    //创建滑动页面
    ui.tileview = lv_tileview_create(lv_scr_act());
    lv_obj_add_style(ui.tileview, &ui.bg_style, 0);
    lv_obj_set_style_bg_opa(ui.tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui.tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    ui.tile_state = lv_tileview_add_tile(ui.tileview, 0, 0, LV_DIR_RIGHT);
    ui.tile_main = lv_tileview_add_tile(ui.tileview, 1, 0, LV_DIR_LEFT  | LV_DIR_RIGHT);
    ui.tile_file = lv_tileview_add_tile(ui.tileview, 2, 0, LV_DIR_LEFT);
    lv_obj_set_tile_id(ui.tileview, 1, 0, LV_ANIM_ON);

    //添加全局的左右手势
    lv_obj_add_event_cb(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);

    //添加logo
    LV_IMG_DECLARE(swlogo);
    /*Now create the actual image*/
    ui.icon = lv_img_create(ui.tile_main);
    lv_img_set_src(ui.icon, &swlogo);
    lv_obj_align(ui.icon, LV_ALIGN_TOP_LEFT, 0, 0);

    //添加logo文字显示
    ui.label_logo = lv_label_create(ui.tile_main);
    lv_label_set_long_mode(ui.label_logo, LV_LABEL_LONG_WRAP);
    lv_label_set_recolor(ui.label_logo, true);
    lv_label_set_text_fmt(ui.label_logo,
            "SWOS2 %s\n"
            "Thread Operating System\n"
            "%d.%d.%d build %s %s\n"
            "Powered by RT-Thread\n"
            "2006 - 2023 Copyright\n"
            "by hnthinker team\n"
            , SYS_VERSION
            , RT_VERSION, RT_SUBVERSION, RT_REVISION, __DATE__, __TIME__);
    lv_obj_set_width(ui.label_logo, LV_HOR_RES_MAX/3);
    lv_obj_set_style_text_align(ui.label_logo, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(ui.label_logo, LV_ALIGN_TOP_LEFT, 96, 0);

    //添加结果表格
    ui.table_result = lv_table_create(ui.tile_main);
    lv_obj_set_height(ui.table_result, LV_VER_RES_MAX*10/17);
    lv_obj_align(ui.table_result, LV_ALIGN_CENTER, 0, 0);
    lv_table_set_col_width(ui.table_result, 0, LV_HOR_RES_MAX*2/9);
    lv_table_set_col_width(ui.table_result, 1, LV_HOR_RES_MAX/12);
    lv_table_set_col_width(ui.table_result, 2, LV_HOR_RES_MAX*2/9);
    lv_table_set_col_width(ui.table_result, 3, LV_HOR_RES_MAX/12);
    lv_table_set_col_width(ui.table_result, 4, LV_HOR_RES_MAX*2/9);
    lv_table_set_col_width(ui.table_result, 5, LV_HOR_RES_MAX/12);

    //添加启动按钮
    ui.btn_enter = lv_btn_create(ui.tile_main);
    lv_obj_set_size(ui.btn_enter, LV_HOR_RES_MAX/12, LV_HOR_RES_MAX/12);
    lv_obj_align(ui.btn_enter, LV_ALIGN_BOTTOM_MID, 0, -LV_HOR_RES_MAX/36);
    lv_obj_add_event_cb(ui.btn_enter, btn_pressed_event_cb, LV_EVENT_PRESSED, NULL);

    ui.btn_enter_label = lv_label_create(ui.btn_enter);
    lv_obj_set_style_text_align(ui.btn_enter_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(ui.btn_enter_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui.btn_enter_label, LV_SYMBOL_PLAY);

    //添加屏幕亮度控制
    /*Create a slider in the center of the display*/
    ui.slider_lcd = lv_slider_create(ui.tile_main);
    lv_slider_set_range(ui.slider_lcd, 10, 100);
    lv_slider_set_value(ui.slider_lcd, 100, LV_ANIM_ON);
    lv_obj_set_size(ui.slider_lcd, LV_HOR_RES_MAX/4, LV_VER_RES_MAX/32);
    lv_obj_align(ui.slider_lcd, LV_ALIGN_TOP_RIGHT, -LV_HOR_RES_MAX/8, LV_VER_RES_MAX/12);
    lv_obj_add_event_cb(ui.slider_lcd, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    /*Create a label below the slider*/
    ui.slider_lcd_label = lv_label_create(ui.tile_main);
    lv_label_set_text(ui.slider_lcd_label, "100%");
    lv_obj_align_to(ui.slider_lcd_label, ui.slider_lcd, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    //添加文件浏览器
    ui.file_explorer = lv_file_explorer_create(ui.tile_file);
    lv_file_explorer_set_sort(ui.file_explorer, LV_EXPLORER_SORT_KIND);

    /* linux */
    lv_file_explorer_open_dir(ui.file_explorer, "A:/");

#if LV_FILE_EXPLORER_QUICK_ACCESS
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_HOME_DIR, "A:/");
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_VIDEO_DIR, "A:/usr");
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_PICTURES_DIR, "A:/usr");
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_MUSIC_DIR, "A:/usr");
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_DOCS_DIR, "A:/etc");
    lv_file_explorer_set_quick_access_path(ui.file_explorer, LV_EXPLORER_FS_DIR, "A:/");
#endif

    //添加线程监控和内存监控
    /*Create a container with ROW flex direction*/
    ui.cont_row_ps = lv_obj_create(ui.tile_state);
    lv_obj_set_size(ui.cont_row_ps, LV_HOR_RES_MAX*11/24, LV_VER_RES_MAX*10/17);
    lv_obj_align(ui.cont_row_ps, LV_ALIGN_LEFT_MID, LV_HOR_RES_MAX/36, 0);
    lv_obj_set_flex_flow(ui.cont_row_ps, LV_FLEX_FLOW_COLUMN);

    lv_obj_t * label_name = lv_label_create(ui.tile_state);
    lv_label_set_text(label_name, "Thread Monitor");
    lv_obj_set_width(label_name, LV_HOR_RES_MAX*11/24);
    lv_obj_set_style_text_align(label_name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(label_name, ui.cont_row_ps, LV_ALIGN_OUT_TOP_MID, 0, 0);
    /*Create a container with ROW flex direction*/
    ui.cont_row_free = lv_obj_create(ui.tile_state);
    lv_obj_set_size(ui.cont_row_free, LV_HOR_RES_MAX*11/24, LV_VER_RES_MAX*10/17);
    lv_obj_align(ui.cont_row_free, LV_ALIGN_RIGHT_MID, -LV_HOR_RES_MAX/36, 0);
    lv_obj_set_flex_flow(ui.cont_row_free, LV_FLEX_FLOW_COLUMN);

    label_name = lv_label_create(ui.tile_state);
    lv_label_set_text(label_name, "Memory Monitor");
    lv_obj_set_width(label_name, LV_HOR_RES_MAX*11/24);
    lv_obj_set_style_text_align(label_name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(label_name, ui.cont_row_free, LV_ALIGN_OUT_TOP_MID, 0, 0);

    //添加刷新按钮
    ui.btn_refresh = lv_btn_create(ui.tile_state);
    lv_obj_set_size(ui.btn_refresh, LV_HOR_RES_MAX/12, LV_HOR_RES_MAX/12);
    lv_obj_align(ui.btn_refresh, LV_ALIGN_BOTTOM_MID, 0, -LV_HOR_RES_MAX/36);
    lv_obj_add_event_cb(ui.btn_refresh, btn_pressed_event_cb, LV_EVENT_PRESSED, NULL);

    ui.btn_refresh_label = lv_label_create(ui.btn_refresh);
    lv_obj_set_style_text_align(ui.btn_refresh_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(ui.btn_refresh_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui.btn_refresh_label, LV_SYMBOL_REFRESH);

    //添加键盘测试
    /*Create a keyboard to use it with an of the text areas*/
    ui.keyboard = lv_keyboard_create(ui.tile_state);

    /*Create a text area. The keyboard will write here*/
    ui.textarea = lv_textarea_create(ui.tile_state);
    lv_obj_align(ui.textarea, LV_ALIGN_TOP_MID, 0, LV_VER_RES_MAX/36);
    lv_obj_add_event_cb(ui.textarea, ta_event_cb, LV_EVENT_ALL, ui.keyboard);
    lv_textarea_set_placeholder_text(ui.textarea, "Hello World!");
    lv_obj_set_size(ui.textarea, LV_HOR_RES_MAX*3/4, LV_VER_RES_MAX/8);
    lv_keyboard_set_textarea(ui.keyboard, NULL);
    lv_obj_add_flag(ui.keyboard, LV_OBJ_FLAG_HIDDEN);
}
#endif
