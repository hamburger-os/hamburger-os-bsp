/**
 * @file lv_demo_music.h
 *
 */

#ifndef LV_DEMO_MUSIC_H
#define LV_DEMO_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_demos.h"

#if LV_USE_DEMO_MUSIC

/*********************
 *      DEFINES
 *********************/

#if LV_DEMO_MUSIC_LARGE
#  define LV_DEMO_MUSIC_HANDLE_SIZE  40
#else
#  define LV_DEMO_MUSIC_HANDLE_SIZE  20
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_demo_music(void) LV_SECTION;
void lv_demo_music_close(void) LV_SECTION;

const char * _lv_demo_music_get_title(uint32_t track_id) LV_SECTION;
const char * _lv_demo_music_get_artist(uint32_t track_id) LV_SECTION;
const char * _lv_demo_music_get_genre(uint32_t track_id) LV_SECTION;
uint32_t _lv_demo_music_get_track_length(uint32_t track_id) LV_SECTION;

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DEMO_MUSIC*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_MUSIC_H*/
