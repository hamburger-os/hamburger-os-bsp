/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-22     lvhan       the first version
 */

#include "board.h"

#ifdef PKG_USING_LVGL
#include <lvgl.h>

RTM_EXPORT(lv_font_get_glyph_dsc_fmt_txt);
RTM_EXPORT(lv_font_get_bitmap_fmt_txt);

RTM_EXPORT(lv_anim_init);
RTM_EXPORT(lv_anim_start);
RTM_EXPORT(lv_anim_path_ease_in_out);

RTM_EXPORT(lv_obj_update_layout);
RTM_EXPORT(lv_obj_set_style_pad_right);
RTM_EXPORT(lv_obj_align);
RTM_EXPORT(lv_obj_set_style_text_font);
RTM_EXPORT(lv_obj_set_style_pad_left);
RTM_EXPORT(lv_obj_add_event_cb);
RTM_EXPORT(lv_obj_get_y);
RTM_EXPORT(lv_obj_get_child_cnt);
RTM_EXPORT(lv_obj_set_style_text_line_space);
RTM_EXPORT(lv_obj_set_pos);
RTM_EXPORT(lv_obj_set_style_border_width);
RTM_EXPORT(lv_obj_add_flag);
RTM_EXPORT(lv_obj_set_style_pad_top);
RTM_EXPORT(lv_obj_set_style_border_opa);
RTM_EXPORT(lv_obj_set_style_bg_opa);
RTM_EXPORT(lv_obj_set_style_pad_bottom);
RTM_EXPORT(lv_obj_get_x);
RTM_EXPORT(lv_obj_set_style_text_letter_space);
RTM_EXPORT(lv_obj_get_parent);
RTM_EXPORT(lv_obj_set_style_text_align);
RTM_EXPORT(lv_obj_set_style_shadow_width);
RTM_EXPORT(lv_obj_set_scrollbar_mode);
RTM_EXPORT(lv_obj_set_style_shadow_ofs_y);
RTM_EXPORT(lv_obj_set_style_shadow_color);
RTM_EXPORT(lv_obj_set_style_border_color);
RTM_EXPORT(lv_obj_set_style_bg_color);
RTM_EXPORT(lv_obj_set_size);
RTM_EXPORT(lv_obj_set_style_radius);
RTM_EXPORT(lv_obj_clear_flag);
RTM_EXPORT(lv_obj_set_style_shadow_ofs_x);
RTM_EXPORT(lv_obj_move_to_index);
RTM_EXPORT(lv_obj_set_style_shadow_opa);
RTM_EXPORT(lv_obj_set_x);
RTM_EXPORT(lv_obj_create);
RTM_EXPORT(lv_obj_set_y);
RTM_EXPORT(lv_obj_set_style_shadow_spread);
RTM_EXPORT(lv_obj_set_style_img_opa);
RTM_EXPORT(lv_obj_set_style_text_color);

RTM_EXPORT(lv_textarea_create);
RTM_EXPORT(lv_textarea_set_text);

RTM_EXPORT(lv_img_create);
RTM_EXPORT(lv_img_set_angle);
RTM_EXPORT(lv_img_set_pivot);
RTM_EXPORT(lv_img_set_src);

RTM_EXPORT(lv_style_init);
RTM_EXPORT(lv_style_reset);

RTM_EXPORT(lv_label_create);
RTM_EXPORT(lv_label_set_text);
RTM_EXPORT(lv_label_set_long_mode);

RTM_EXPORT(lv_keyboard_create);
RTM_EXPORT(lv_keyboard_set_textarea);

RTM_EXPORT(lv_event_get_code);
RTM_EXPORT(lv_event_get_target);
RTM_EXPORT(lv_event_get_user_data);

RTM_EXPORT(lv_disp_load_scr);

RTM_EXPORT(lv_btn_create);

#endif
