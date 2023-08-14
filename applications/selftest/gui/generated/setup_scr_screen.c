/*
* Copyright 2023 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"


void setup_scr_screen(lv_ui *ui)
{
	//Write codes screen
	ui->screen = lv_obj_create(NULL);
	ui->g_kb_screen = lv_keyboard_create(ui->screen);
	lv_obj_add_event_cb(ui->g_kb_screen, kb_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(ui->g_kb_screen, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_text_font(ui->g_kb_screen, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_size(ui->screen, 800, 600);
	lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_logo
	ui->screen_logo = lv_img_create(ui->screen);
	lv_obj_add_flag(ui->screen_logo, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_logo, &_swlogo_alpha_240x204);
	lv_img_set_pivot(ui->screen_logo, 50,50);
	lv_img_set_angle(ui->screen_logo, 0);
	lv_obj_set_pos(ui->screen_logo, 280, -208);
	lv_obj_set_size(ui->screen_logo, 240, 204);
	lv_obj_set_scrollbar_mode(ui->screen_logo, LV_SCROLLBAR_MODE_OFF);
	//Update pos for widget screen_logo
	lv_obj_update_layout(ui->screen_logo);
	//Write animation: screen_logo move in x direction
	lv_anim_t screen_logo_x;
	lv_anim_init(&screen_logo_x);
	lv_anim_set_var(&screen_logo_x, ui->screen_logo);
	lv_anim_set_time(&screen_logo_x, 1000);
	lv_anim_set_exec_cb(&screen_logo_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
	lv_anim_set_values(&screen_logo_x, lv_obj_get_x(ui->screen_logo), 280);
	lv_anim_set_path_cb(&screen_logo_x, lv_anim_path_ease_in_out);
	lv_anim_start(&screen_logo_x);
	//Write animation: screen_logo move in y direction
	lv_anim_t screen_logo_y;
	lv_anim_init(&screen_logo_y);
	lv_anim_set_var(&screen_logo_y, ui->screen_logo);
	lv_anim_set_time(&screen_logo_y, 1000);
	lv_anim_set_exec_cb(&screen_logo_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
	lv_anim_set_values(&screen_logo_y, lv_obj_get_y(ui->screen_logo), 90);
	lv_anim_set_path_cb(&screen_logo_y, lv_anim_path_ease_in_out);
	lv_anim_start(&screen_logo_y);

	//Write style for screen_logo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_logo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_lab_logo
	ui->screen_lab_logo = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_lab_logo, "标准化核心板自动检测系统");
	lv_label_set_long_mode(ui->screen_lab_logo, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_lab_logo, 200, 327);
	lv_obj_set_size(ui->screen_lab_logo, 400, 32);
	lv_obj_set_scrollbar_mode(ui->screen_lab_logo, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_lab_logo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_lab_logo, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_lab_logo, &lv_font_simsun_28, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_lab_logo, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_lab_logo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_lab_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_ok
	ui->screen_btn_ok = lv_btn_create(ui->screen);
	ui->screen_btn_ok_label = lv_label_create(ui->screen_btn_ok);
	lv_label_set_text(ui->screen_btn_ok_label, "ok");
	lv_label_set_long_mode(ui->screen_btn_ok_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_ok_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_ok, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_ok, 180, 425);
	lv_obj_set_size(ui->screen_btn_ok, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_btn_ok, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_btn_ok, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_ok, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_ok, lv_color_hex(0x0a4082), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_ok, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_ok, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_ok, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_ok, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_ok, &lv_font_simsun_22, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_ok, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_cancel
	ui->screen_btn_cancel = lv_btn_create(ui->screen);
	ui->screen_btn_cancel_label = lv_label_create(ui->screen_btn_cancel);
	lv_label_set_text(ui->screen_btn_cancel_label, "cancel");
	lv_label_set_long_mode(ui->screen_btn_cancel_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_cancel_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_cancel, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_cancel, 520, 425);
	lv_obj_set_size(ui->screen_btn_cancel, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_btn_cancel, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_btn_cancel, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_cancel, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_cancel, lv_color_hex(0x0a4082), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_cancel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_cancel, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_cancel, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_cancel, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_cancel, &lv_font_simsun_22, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_cancel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_ta
	ui->screen_ta = lv_textarea_create(ui->screen);
	lv_textarea_set_text(ui->screen_ta, "");
	#if LV_USE_KEYBOARD != 0 || LV_USE_ZH_KEYBOARD != 0
		lv_obj_add_event_cb(ui->screen_ta, ta_event_cb, LV_EVENT_ALL, ui->g_kb_screen);
	#endif
	lv_obj_set_pos(ui->screen_ta, 0, -4);
	lv_obj_set_size(ui->screen_ta, 800, 30);
	lv_obj_set_scrollbar_mode(ui->screen_ta, LV_SCROLLBAR_MODE_OFF);

	//Write style for screen_ta, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_text_color(ui->screen_ta, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_ta, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_ta, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_ta, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_ta, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_ta, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_ta, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->screen_ta, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->screen_ta, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_ta, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_ta, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_ta, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_ta, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_ta, 4, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style for screen_ta, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_ta, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_ta, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_ta, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen);

	
}
