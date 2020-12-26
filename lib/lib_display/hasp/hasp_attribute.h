/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_ATTR_SET_H
#define HASP_ATTR_SET_H

#include "lv_conf.h"
#include "lvgl.h"
//#include "../lv_components/lv_components.h"
//#include "hasp_conf.h"

//#include "hasp.h"

#ifdef __cplusplus
extern "C" {
#endif

// test
lv_chart_series_t * lv_chart_get_series(lv_obj_t * chart, uint8_t ser_num);

void hasp_process_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload, bool update);
void my_printf(uint8_t tag, const char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#define _HASP_ATTRIBUTE(prop_name, func_name, value_type)                                                              \
    static inline void attribute_##func_name(lv_obj_t * obj, uint8_t part, lv_state_t state, bool update,              \
                                             const char * attr, value_type val)                                        \
    {                                                                                                                  \
        if(update) {                                                                                                   \
            return lv_obj_set_style_local_##func_name(obj, part, state, (value_type)val);                              \
        } else {                                                                                                       \
            value_type temp = lv_obj_get_style_##func_name(obj, part);                                                 \
            /*lv_obj_get_style_##func_name(obj, part, state, &temp);*/                                                 \
            return hasp_send_obj_attribute_int(obj, attr, temp);                                                       \
        }                                                                                                              \
    }

_HASP_ATTRIBUTE(RADIUS, radius, lv_style_int_t)
_HASP_ATTRIBUTE(CLIP_CORNER, clip_corner, bool)
_HASP_ATTRIBUTE(SIZE, size, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSFORM_WIDTH, transform_width, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSFORM_HEIGHT, transform_height, lv_style_int_t)
_HASP_ATTRIBUTE(OPA_SCALE, opa_scale, lv_opa_t)
_HASP_ATTRIBUTE(PAD_TOP, pad_top, lv_style_int_t)
_HASP_ATTRIBUTE(PAD_BOTTOM, pad_bottom, lv_style_int_t)
_HASP_ATTRIBUTE(PAD_LEFT, pad_left, lv_style_int_t)
_HASP_ATTRIBUTE(PAD_RIGHT, pad_right, lv_style_int_t)
//_HASP_ATTRIBUTE(PAD_INNER, pad_inner, lv_style_int_t)
#if LV_USE_BLEND_MODES
_HASP_ATTRIBUTE(BG_BLEND_MODE, bg_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(BORDER_BLEND_MODE, border_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(OUTLINE_BLEND_MODE, outline_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(SHADOW_BLEND_MODE, shadow_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(PATTERN_BLEND_MODE, pattern_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(VALUE_BLEND_MODE, value_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(TEXT_BLEND_MODE, text_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(LINE_BLEND_MODE, line_blend_mode, lv_blend_mode_t)
_HASP_ATTRIBUTE(IMAGE_BLEND_MODE, image_blend_mode, lv_blend_mode_t)
#endif
_HASP_ATTRIBUTE(BG_MAIN_STOP, bg_main_stop, lv_style_int_t)
_HASP_ATTRIBUTE(BG_GRAD_STOP, bg_grad_stop, lv_style_int_t)
_HASP_ATTRIBUTE(BG_GRAD_DIR, bg_grad_dir, lv_grad_dir_t)
//_HASP_ATTRIBUTE(BG_COLOR, bg_color, lv_color_t, _color, nonscalar)
//_HASP_ATTRIBUTE(BG_GRAD_COLOR, bg_grad_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(BG_OPA, bg_opa, lv_opa_t)
_HASP_ATTRIBUTE(BORDER_WIDTH, border_width, lv_style_int_t)
_HASP_ATTRIBUTE(BORDER_SIDE, border_side, lv_border_side_t)
_HASP_ATTRIBUTE(BORDER_POST, border_post, bool)
//_HASP_ATTRIBUTE(BORDER_COLOR, border_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(BORDER_OPA, border_opa, lv_opa_t)
_HASP_ATTRIBUTE(OUTLINE_WIDTH, outline_width, lv_style_int_t)
_HASP_ATTRIBUTE(OUTLINE_PAD, outline_pad, lv_style_int_t)
//_HASP_ATTRIBUTE(OUTLINE_COLOR, outline_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(OUTLINE_OPA, outline_opa, lv_opa_t)
#if LV_USE_SHADOW
_HASP_ATTRIBUTE(SHADOW_WIDTH, shadow_width, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_OFS_X, shadow_ofs_x, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_OFS_Y, shadow_ofs_y, lv_style_int_t)
_HASP_ATTRIBUTE(SHADOW_SPREAD, shadow_spread, lv_style_int_t)
//_HASP_ATTRIBUTE(SHADOW_COLOR, shadow_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(SHADOW_OPA, shadow_opa, lv_opa_t)
#endif
_HASP_ATTRIBUTE(PATTERN_REPEAT, pattern_repeat, bool)
//_HASP_ATTRIBUTE(PATTERN_RECOLOR, pattern_recolor, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(PATTERN_OPA, pattern_opa, lv_opa_t)
_HASP_ATTRIBUTE(PATTERN_RECOLOR_OPA, pattern_recolor_opa, lv_opa_t)
//_HASP_ATTRIBUTE(PATTERN_IMAGE, pattern_image, const void *, _data_ptr, scalar)
_HASP_ATTRIBUTE(VALUE_LETTER_SPACE, value_letter_space, lv_style_int_t)
_HASP_ATTRIBUTE(VALUE_LINE_SPACE, value_line_space, lv_style_int_t)
_HASP_ATTRIBUTE(VALUE_OFS_X, value_ofs_x, lv_style_int_t)
_HASP_ATTRIBUTE(VALUE_OFS_Y, value_ofs_y, lv_style_int_t)
_HASP_ATTRIBUTE(VALUE_ALIGN, value_align, lv_align_t)
//_HASP_ATTRIBUTE(VALUE_COLOR, value_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(VALUE_OPA, value_opa, lv_opa_t)
//_HASP_ATTRIBUTE(VALUE_FONT, value_font, const lv_font_t *, _data_ptr, scalar)
//_HASP_ATTRIBUTE(VALUE_STR, value_str, const char *, _data_ptr, scalar)
_HASP_ATTRIBUTE(TEXT_LETTER_SPACE, text_letter_space, lv_style_int_t)
_HASP_ATTRIBUTE(TEXT_LINE_SPACE, text_line_space, lv_style_int_t)
_HASP_ATTRIBUTE(TEXT_DECOR, text_decor, lv_text_decor_t)
//_HASP_ATTRIBUTE(TEXT_COLOR, text_color, lv_color_t, _color, nonscalar)
//_HASP_ATTRIBUTE(TEXT_SEL_COLOR, text_sel_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(TEXT_OPA, text_opa, lv_opa_t)
//_HASP_ATTRIBUTE(TEXT_FONT, text_font, const lv_font_t *, _data_ptr, scalar)
_HASP_ATTRIBUTE(LINE_WIDTH, line_width, lv_style_int_t)
_HASP_ATTRIBUTE(LINE_DASH_WIDTH, line_dash_width, lv_style_int_t)
_HASP_ATTRIBUTE(LINE_DASH_GAP, line_dash_gap, lv_style_int_t)
_HASP_ATTRIBUTE(LINE_ROUNDED, line_rounded, bool)
//_HASP_ATTRIBUTE(LINE_COLOR, line_color, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(LINE_OPA, line_opa, lv_opa_t)
//_HASP_ATTRIBUTE(IMAGE_RECOLOR, image_recolor, lv_color_t, _color, nonscalar)
_HASP_ATTRIBUTE(IMAGE_OPA, image_opa, lv_opa_t)
_HASP_ATTRIBUTE(IMAGE_RECOLOR_OPA, image_recolor_opa, lv_opa_t)
_HASP_ATTRIBUTE(TRANSITION_TIME, transition_time, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_DELAY, transition_delay, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_1, transition_prop_1, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_2, transition_prop_2, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_3, transition_prop_3, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_4, transition_prop_4, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_5, transition_prop_5, lv_style_int_t)
_HASP_ATTRIBUTE(TRANSITION_PROP_6, transition_prop_6, lv_style_int_t)

_HASP_ATTRIBUTE(SCALE_WIDTH, scale_width, lv_style_int_t)
_HASP_ATTRIBUTE(SCALE_BORDER_WIDTH, scale_border_width, lv_style_int_t)
_HASP_ATTRIBUTE(SCALE_END_BORDER_WIDTH, scale_end_border_width, lv_style_int_t)
_HASP_ATTRIBUTE(SCALE_END_LINE_WIDTH, scale_end_line_width, lv_style_int_t)
//_HASP_ATTRIBUTE(SCALE_GRAD_COLOR, scale_grad_color, lv_color_t, _color, nonscalar)
//_HASP_ATTRIBUTE(SCALE_END_COLOR, scale_end_color, lv_color_t, _color, nonscalar)

/* attribute hashes */
/* Object Part Attributes */
#define ATTR_SIZE 16417
#define ATTR_RADIUS 20786
#define ATTR_CLIP_CORNER 9188
#define ATTR_OPA_SCALE 64875
#define ATTR_TRANSFORM_HEIGHT 55994
#define ATTR_TRANSFORM_WIDTH 48627

/* Background Attributes */
#define ATTR_BG_OPA 48966
#define ATTR_BG_COLOR 64969
#define ATTR_BG_GRAD_DIR 41782
#define ATTR_BG_GRAD_STOP 4025
#define ATTR_BG_MAIN_STOP 63118
#define ATTR_BG_BLEND_MODE 31147
#define ATTR_BG_GRAD_COLOR 44140

/* Padding Attributes */
#define ATTR_PAD_TOP 59081
#define ATTR_PAD_LEFT 43123
#define ATTR_PAD_INNER 9930
#define ATTR_PAD_RIGHT 65104
#define ATTR_PAD_BOTTOM 3767

/* Text Attributes */
#define ATTR_TEXT_OPA 37166
#define ATTR_TEXT_FONT 22465
#define ATTR_TEXT_COLOR 23473
#define ATTR_TEXT_DECOR 1971
#define ATTR_TEXT_LETTER_SPACE 62079
#define ATTR_TEXT_SEL_COLOR 32076
#define ATTR_TEXT_LINE_SPACE 54829
#define ATTR_TEXT_BLEND_MODE 32195

/* Border Attributes */
#define ATTR_BORDER_OPA 2061
#define ATTR_BORDER_SIDE 53962
#define ATTR_BORDER_POST 49491
#define ATTR_BORDER_BLEND_MODE 23844
#define ATTR_BORDER_WIDTH 24531
#define ATTR_BORDER_COLOR 21264

/* Outline Attributes */
#define ATTR_OUTLINE_OPA 23011
#define ATTR_OUTLINE_PAD 26038
#define ATTR_OUTLINE_COLOR 6630
#define ATTR_OUTLINE_BLEND_MODE 25038
#define ATTR_OUTLINE_WIDTH 9897

/* Shadow Attributes */
#define ATTR_SHADOW_OPA 38401
#define ATTR_SHADOW_WIDTH 13255
#define ATTR_SHADOW_OFS_X 44278
#define ATTR_SHADOW_OFS_Y 44279
#define ATTR_SHADOW_SPREAD 21138
#define ATTR_SHADOW_BLEND_MODE 64048
#define ATTR_SHADOW_COLOR 9988

/* Line Attributes */
#define ATTR_LINE_OPA 24501
#define ATTR_LINE_WIDTH 25467
#define ATTR_LINE_COLOR 22200
#define ATTR_LINE_DASH_WIDTH 32676
#define ATTR_LINE_ROUNDED 15042
#define ATTR_LINE_DASH_GAP 49332
#define ATTR_LINE_BLEND_MODE 60284

/* Value Attributes */
#define ATTR_VALUE_OPA 50482
#define ATTR_VALUE_STR 1091
#define ATTR_VALUE_FONT 9405
#define ATTR_VALUE_ALIGN 27895
#define ATTR_VALUE_COLOR 52661
#define ATTR_VALUE_OFS_X 21415
#define ATTR_VALUE_OFS_Y 21416
#define ATTR_VALUE_LINE_SPACE 26921
#define ATTR_VALUE_BLEND_MODE 4287
#define ATTR_VALUE_LETTER_SPACE 51067

/* Pattern attributes */
#define ATTR_PATTERN_BLEND_MODE 43456
#define ATTR_PATTERN_RECOLOR_OPA 35074
#define ATTR_PATTERN_RECOLOR 7745
#define ATTR_PATTERN_REPEAT 31338
#define ATTR_PATTERN_OPA 43633
#define ATTR_PATTERN_IMAGE 61292

#define ATTR_TRANSITION_PROP_1 49343
#define ATTR_TRANSITION_PROP_2 49344
#define ATTR_TRANSITION_PROP_3 49345
#define ATTR_TRANSITION_PROP_4 49346
#define ATTR_TRANSITION_PROP_5 49347
#define ATTR_TRANSITION_PROP_6 49348
#define ATTR_TRANSITION_TIME 26263
#define ATTR_TRANSITION_PATH 43343
#define ATTR_TRANSITION_DELAY 64537

#define ATTR_IMAGE_OPA 58140
#define ATTR_IMAGE_RECOLOR 52204
#define ATTR_IMAGE_BLEND_MODE 11349
#define ATTR_IMAGE_RECOLOR_OPA 43949

#define ATTR_SCALE_END_LINE_WIDTH 30324
#define ATTR_SCALE_END_BORDER_WIDTH 34380
#define ATTR_SCALE_BORDER_WIDTH 2440
#define ATTR_SCALE_GRAD_COLOR 47239
#define ATTR_SCALE_WIDTH 36017
#define ATTR_SCALE_END_COLOR 44074

/* Object Attributes */
#define ATTR_X 120
#define ATTR_Y 121
#define ATTR_W 119
#define ATTR_H 104
#define ATTR_OPTIONS 29886
#define ATTR_ENABLED 28193
#define ATTR_OPACITY 10155
#define ATTR_TOGGLE 38580
#define ATTR_HIDDEN 11082
#define ATTR_VIS 16320
#define ATTR_MODE 45891
// #define ATTR_RECT 11204
#define ATTR_ROWS 52153
#define ATTR_COLS 36307
#define ATTR_MIN 46130
#define ATTR_MAX 45636
#define ATTR_VAL 15809
#define ATTR_TXT 9328
#define ATTR_SRC 4964
#define ATTR_ID 6715
#define ATTR_DELETE 50027

// Gauge
#define ATTR_CRITICAL_VALUE 39281
#define ATTR_ANGLE 2387
#define ATTR_LABEL_COUNT 20356
#define ATTR_LINE_COUNT 57860
#define ATTR_FORMAT 38871

// Buttonmatric
#define ATTR_MAP 45628
#endif
