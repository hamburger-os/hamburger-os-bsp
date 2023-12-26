/**
 * @file lv_templ.h
 *
 */

#ifndef LV_TABVIEW_H
#define LV_TABVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../../lvgl.h"

#if LV_USE_TABVIEW

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    const char ** map;
    uint16_t tab_cnt;
    uint16_t tab_cur;
    lv_dir_t tab_pos;
} lv_tabview_t;

extern const lv_obj_class_t lv_tabview_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t * lv_tabview_create(lv_obj_t * parent, lv_dir_t tab_pos, lv_coord_t tab_size) LV_SECTION;

lv_obj_t * lv_tabview_add_tab(lv_obj_t * tv, const char * name) LV_SECTION;

void lv_tabview_rename_tab(lv_obj_t * obj, uint32_t tab_id, const char * new_name) LV_SECTION;

lv_obj_t * lv_tabview_get_content(lv_obj_t * tv) LV_SECTION;

lv_obj_t * lv_tabview_get_tab_btns(lv_obj_t * tv) LV_SECTION;

void lv_tabview_set_act(lv_obj_t * obj, uint32_t id, lv_anim_enable_t anim_en) LV_SECTION;

uint16_t lv_tabview_get_tab_act(lv_obj_t * tv) LV_SECTION;

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_TABVIEW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TABVIEW_H*/
