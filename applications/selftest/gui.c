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

#include "selftest.h"

#ifdef PKG_USING_LVGL
#include <lvgl.h>
#endif

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

    lv_obj_t * table_result;
    lv_obj_t * btn_enter;
    lv_obj_t * btn_label;

    lv_obj_t * file_explorer;
};
static struct ui_object ui = {0};

void gui_display_result(SelftestResult *presult, uint16_t num)
{
    lv_mutex_take();

    if (ui.table_result != NULL)
    {
        lv_obj_clean(ui.table_result);

        lv_table_set_cell_value(ui.table_result, 0, 0, "Project");
        lv_table_set_cell_value(ui.table_result, 0, 1, "");
        lv_table_set_cell_value(ui.table_result, 0, 2, "Project");
        lv_table_set_cell_value(ui.table_result, 0, 3, "");
        for (uint16_t i = 0; i < num; i++)
        {
            uint16_t row = (i % 2 == 0)?(i + 1 - i/2):(i - i/2);
            lv_table_set_cell_value(ui.table_result, row, (i % 2 == 0)?(0):(2), presult[i].name);
            if (presult[i].result == 0)
            {
                lv_table_set_cell_value(ui.table_result, row, (i % 2 == 0)?(1):(3), LV_SYMBOL_OK);
            }
            else
            {
                lv_table_set_cell_value(ui.table_result, row, (i % 2 == 0)?(1):(3), LV_SYMBOL_CLOSE);
            }
        }
    }

    lv_mutex_release();
}

static void selftest_thread_entry(void *parameter)
{
    system("selftest_start");
}

static void btn_pressed_event_cb(lv_event_t *event)
{
    lv_obj_t * btn = lv_event_get_target(event);
    lv_obj_t * label = lv_obj_get_child(btn, 0);

    if (rt_strcmp(lv_label_get_text(label), LV_SYMBOL_PLAY) == 0)
    {
        /* 创建 自测 线程 */
        rt_thread_t thread = rt_thread_create("selftest", selftest_thread_entry, NULL, 4096, 22, 10);
        /* 创建成功则启动线程 */
        if (thread != RT_NULL)
        {
            rt_thread_startup(thread);
        }
        else
        {
            LOG_E("thread startup error!");
        }
    }
}

void lv_user_gui_init(void)
{
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
    lv_table_set_cell_value(ui.table_result, 0, 0, "Project");
    lv_table_set_cell_value(ui.table_result, 0, 1, "");
    lv_table_set_cell_value(ui.table_result, 0, 2, "Project");
    lv_table_set_cell_value(ui.table_result, 0, 3, "");
    lv_table_set_col_width(ui.table_result, 0, LV_HOR_RES_MAX*2/5);
    lv_table_set_col_width(ui.table_result, 1, LV_HOR_RES_MAX/12);
    lv_table_set_col_width(ui.table_result, 2, LV_HOR_RES_MAX*2/5);
    lv_table_set_col_width(ui.table_result, 3, LV_HOR_RES_MAX/12);

    //添加启动按钮
    ui.btn_enter = lv_btn_create(ui.tile_main);
    lv_obj_set_size(ui.btn_enter, LV_HOR_RES_MAX/8, LV_HOR_RES_MAX/8);
    lv_obj_align(ui.btn_enter, LV_ALIGN_BOTTOM_MID, 0, -LV_HOR_RES_MAX/64);
    lv_obj_add_event_cb(ui.btn_enter, btn_pressed_event_cb, LV_EVENT_PRESSED, NULL);

    ui.btn_label = lv_label_create(ui.btn_enter);
    lv_obj_set_style_text_align(ui.btn_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(ui.btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui.btn_label, LV_SYMBOL_PLAY);

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
}
#endif
