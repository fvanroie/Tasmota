/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "ArduinoJson.h"
#include "ArduinoLog.h"

#include "lvgl.h"
#include "lv_conf.h"
#if LVGL_VERSION_MAJOR != 7
#include "../lv_components.h"
#endif
#include "hasp_conf.h"

#include "hasp.h"
#include "hasp_dispatch.h"  // for is_true
#include "hasp_object.h"
#include "hasp_attribute.h"

LV_FONT_DECLARE(unscii_8_icon);
extern lv_font_t * haspFonts[8];

static inline bool only_digits(const char * s);
static inline void hasp_out_int(lv_obj_t * obj, const char * attr, uint32_t val);
static inline void hasp_out_str(lv_obj_t * obj, const char * attr, const char * data);
static inline void hasp_out_color(lv_obj_t * obj, const char * attr, lv_color_t color);

/* 16-bit hashing function http://www.cse.yorku.ca/~oz/hash.html */
/* all possible attributes are hashed and checked if they are unique */
static uint16_t sdbm(const char * str)
{
    uint16_t hash = 0;
    char c;

    // while(c = *str++) hash = c + (hash << 6) + (hash << 16) - hash;
    while((c = *str++)) {
        hash = tolower(c) + (hash << 6) - hash;
    }

    return hash;
}

// OK - this function is missing in lvgl
static uint8_t lv_roller_get_visible_row_count(lv_obj_t * roller)
{
    const lv_font_t * font    = lv_obj_get_style_text_font(roller, LV_ROLLER_PART_BG);
    lv_style_int_t line_space = lv_obj_get_style_text_line_space(roller, LV_ROLLER_PART_BG);
    lv_coord_t h              = lv_obj_get_height(roller);

    if((lv_font_get_line_height(font) + line_space) != 0)
        return (uint8_t)(h / (lv_font_get_line_height(font) + line_space));
    else
        return 0;
}

// OK - this function is missing in lvgl
static inline int16_t lv_chart_get_min_value(lv_obj_t * chart)
{
    lv_chart_ext_t * ext = (lv_chart_ext_t *)lv_obj_get_ext_attr(chart);
    return ext->ymin[LV_CHART_AXIS_PRIMARY_Y];
}

// OK - this function is missing in lvgl
static inline int16_t lv_chart_get_max_value(lv_obj_t * chart)
{
    lv_chart_ext_t * ext = (lv_chart_ext_t *)lv_obj_get_ext_attr(chart);
    return ext->ymax[LV_CHART_AXIS_PRIMARY_Y];
}

lv_chart_series_t * lv_chart_get_series(lv_obj_t * chart, uint8_t ser_num)
{
    lv_chart_ext_t * ext    = (lv_chart_ext_t *)lv_obj_get_ext_attr(chart);
    lv_chart_series_t * ser = (lv_chart_series_t *)_lv_ll_get_tail(&ext->series_ll);
    while(ser_num > 0 && ser) {
        ser = (lv_chart_series_t *)_lv_ll_get_prev(&ext->series_ll, ser);
        ser_num--;
    }
    return ser;
}

// OK
static inline lv_color_t haspLogColor(lv_color_t color)
{
    // uint8_t r = (LV_COLOR_GET_R(color) * 263 + 7) >> 5;
    // uint8_t g = (LV_COLOR_GET_G(color) * 259 + 3) >> 6;
    // uint8_t b = (LV_COLOR_GET_B(color) * 263 + 7) >> 5;
    // Log.verbose(F("Color: R%u G%u B%u"), r, g, b);
    return color;
}

// OK
static lv_color_t haspPayloadToColor(const char * payload)
{
    switch(strlen(payload)) {
        case 3:
            if(!strcasecmp_P(payload, PSTR("red"))) return haspLogColor(LV_COLOR_RED);
            break;
        case 4:
            if(!strcasecmp_P(payload, PSTR("blue"))) return haspLogColor(LV_COLOR_BLUE);
            if(!strcasecmp_P(payload, PSTR("cyan"))) return haspLogColor(LV_COLOR_CYAN);
            if(!strcasecmp_P(payload, PSTR("gray"))) return haspLogColor(LV_COLOR_GRAY);
            //          if(!strcmp_P(payload, PSTR("aqua"))) return haspLogColor(LV_COLOR_AQUA);
            //          if(!strcmp_P(payload, PSTR("lime"))) return haspLogColor(LV_COLOR_LIME);
            //          if(!strcmp_P(payload, PSTR("teal"))) return haspLogColor(LV_COLOR_TEAL);
            //          if(!strcmp_P(payload, PSTR("navy"))) return haspLogColor(LV_COLOR_NAVY);
            break;
        case 5:
            if(!strcasecmp_P(payload, PSTR("green"))) return haspLogColor(LV_COLOR_GREEN);
            if(!strcasecmp_P(payload, PSTR("white"))) return haspLogColor(LV_COLOR_WHITE);
            if(!strcasecmp_P(payload, PSTR("black"))) return haspLogColor(LV_COLOR_BLACK);
            //            if(!strcmp_P(payload, PSTR("olive"))) return haspLogColor(LV_COLOR_OLIVE);
            break;
        case 6:
            if(!strcasecmp_P(payload, PSTR("yellow"))) return haspLogColor(LV_COLOR_YELLOW);
            if(!strcasecmp_P(payload, PSTR("orange"))) return haspLogColor(LV_COLOR_ORANGE);
            if(!strcasecmp_P(payload, PSTR("purple"))) return haspLogColor(LV_COLOR_PURPLE);
            if(!strcasecmp_P(payload, PSTR("silver"))) return haspLogColor(LV_COLOR_SILVER);
            //            if(!strcmp_P(payload, PSTR("maroon"))) return haspLogColor(LV_COLOR_MAROON);
            break;
        case 7:
            if(!strcasecmp_P(payload, PSTR("magenta"))) return haspLogColor(LV_COLOR_MAGENTA);

        default:
            //            if(!strcmp_P(payload, PSTR("darkblue"))) return haspLogColor(LV_COLOR_MAKE(0, 51, 102));
            //            if(!strcmp_P(payload, PSTR("lightblue"))) return haspLogColor(LV_COLOR_MAKE(46, 203,
            //            203));
            break;
    }

    /* HEX format #rrggbb or #rrggbbaa */
    char pattern[4];
    snprintf_P(pattern, sizeof(pattern), PSTR(" 2x")); // % cannot be escaped, so we build our own pattern
    pattern[0] = '%';
    char buffer[13];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s%s%s%s"), pattern, pattern, pattern, pattern);
    int r, g, b, a;

    if(*payload == '#' && sscanf(payload + 1, buffer, &r, &g, &b, &a) == 4) {
      //  return haspLogColor(LV_COLOR_MAKE(r, g, b));
    } else if(*payload == '#' && sscanf(payload + 1, buffer, &r, &g, &b) == 3) {
      //  return haspLogColor(LV_COLOR_MAKE(r, g, b));
    }

    /* 16-bit RGB565 Color Scheme*/
    if(only_digits(payload)) {
        uint16_t c = atoi(payload);
        /* Initial colors */
        uint8_t R5 = ((c >> 11) & 0b11111);
        uint8_t G6 = ((c >> 5) & 0b111111);
        uint8_t B5 = (c & 0b11111);
        /* Remapped colors */
        uint8_t R8 = (R5 * 527 + 23) >> 6;
        uint8_t G8 = (G6 * 259 + 33) >> 6;
        uint8_t B8 = (B5 * 527 + 23) >> 6;
        return lv_color_make(R8, G8, B8);
    }

    /* Unknown format */
    Log.warning(TAG_ATTR, F("Invalid color %s"), payload);
    return LV_COLOR_BLACK;
}

static lv_font_t * haspPayloadToFont(const char * payload)
{
    uint8_t var = atoi(payload);

    switch(var) {
        case 0:
        case 1:
        case 2:
        case 3:
           // return hasp_get_font(var);

        case 8:
            // return &unscii_8_icon;

#if ESP32

#if LV_FONT_MONTSERRAT_12 > 0
        case 12:
            return &lv_font_montserrat_12;
#endif

#if LV_FONT_MONTSERRAT_16 > 0
        case 16:
            return &lv_font_montserrat_16;
#endif

#if LV_FONT_MONTSERRAT_22 > 0
        case 22:
            return &lv_font_montserrat_22;
#endif

#if LV_FONT_MONTSERRAT_28 > 0
        case 28:
            return &lv_font_montserrat_28_compressed;
#endif

#endif

        default:
            return nullptr;
    }
}

static void hasp_process_label_long_mode(lv_obj_t * obj, const char * payload, bool update)
{
    if(update) {
        lv_label_long_mode_t mode = LV_LABEL_LONG_EXPAND;
        if(!strcasecmp_P(payload, PSTR("expand"))) {
            mode = LV_LABEL_LONG_EXPAND;
        } else if(!strcasecmp_P(payload, PSTR("break"))) {
            mode = LV_LABEL_LONG_BREAK;
        } else if(!strcasecmp_P(payload, PSTR("dots"))) {
            mode = LV_LABEL_LONG_DOT;
        } else if(!strcasecmp_P(payload, PSTR("scroll"))) {
            mode = LV_LABEL_LONG_SROLL;
        } else if(!strcasecmp_P(payload, PSTR("loop"))) {
            mode = LV_LABEL_LONG_SROLL_CIRC;
        } else {
             return Log.verbose(0,F("Invalid long mode"));
        }
        lv_label_set_long_mode(obj, mode);
    } else {
        // Getter needed
    }
}

// OK
lv_obj_t * FindButtonLabel(lv_obj_t * btn)
{
    if(btn) {
        lv_obj_t * label = lv_obj_get_child_back(btn, NULL);
        if(label) {
            lv_obj_type_t list;
            lv_obj_get_type(label, &list);
            const char * objtype = list.type[0];

            if(check_obj_type_str(objtype, LV_HASP_LABEL)) {
                return label;
            }

        } else {
            Log.error(TAG_ATTR, F("FindButtonLabel NULL Pointer encountered"));
        }
    } else {
        Log.warning(TAG_ATTR, F("Button not defined"));
    }
    return NULL;
}

// OK
static inline void haspSetLabelText(lv_obj_t * obj, const char * value)
{
    lv_obj_t * label = FindButtonLabel(obj);
    if(label) {
        lv_label_set_text(label, value);
    }
}

// OK
static bool haspGetLabelText(lv_obj_t * obj, char * text)
{
    if(!obj) {
        Log.warning(TAG_ATTR, F("Button not defined"));
        return false;
    }

    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    if(label) {
        lv_obj_type_t list;
        lv_obj_get_type(label, &list);

        if(check_obj_type_str(list.type[0], LV_HASP_LABEL)) {
            text = lv_label_get_text(label);
            return true;
        }

    } else {
        Log.warning(TAG_ATTR, F("haspGetLabelText NULL Pointer encountered"));
    }

    return false;
}

static void hasp_attribute_get_part_state(lv_obj_t * obj, const char * attr_in, char * attr_out, uint8_t & part,
                                          uint8_t & state)
{
    int len = strlen(attr_in);
    if(len <= 0 || len >= 32) {
        attr_out[0] = 0; // empty string
        part        = LV_OBJ_PART_MAIN;
        state       = LV_STATE_DEFAULT;
        return;
    }
    int index = atoi(&attr_in[len - 1]);

    // Drop Trailing partnumber
    if(attr_in[len - 1] == '0' || index > 0) {
        part = LV_TABLE_PART_BG;
        len--;
    }
    strncpy(attr_out, attr_in, len);
    attr_out[len] = 0;

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type_str(objtype, LV_HASP_BUTTON)) {
        switch(index) {
            case 1:
                state = LV_BTN_STATE_PRESSED;
                break;
            case 2:
                state = LV_BTN_STATE_DISABLED;
                break;
            case 3:
                state = LV_BTN_STATE_CHECKED_RELEASED;
                break;
            case 4:
                state = LV_BTN_STATE_CHECKED_PRESSED;
                break;
            case 5:
                state = LV_BTN_STATE_CHECKED_DISABLED;
                break;
            default:
                state = LV_BTN_STATE_RELEASED;
        }
        part = LV_BTN_PART_MAIN;
        return;
    }

    if(check_obj_type_str(objtype, LV_HASP_BAR)) {
        if(index == 1) {
            part = LV_BAR_PART_INDIC;
        } else {
            part = LV_BAR_PART_BG;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    if(check_obj_type_str(objtype, LV_HASP_CHECKBOX)) {
        if(index == 1) {
            part = LV_CHECKBOX_PART_BULLET;
        } else {
            part = LV_CHECKBOX_PART_BG;
        }
        state = LV_STATE_DEFAULT;
        return;
    }

    if(check_obj_type_str(objtype, LV_HASP_CPICKER)) {
        if(index == 1) {
            part = LV_CPICKER_PART_KNOB;
        } else {
            part = LV_CPICKER_PART_MAIN;
        }
        state = LV_STATE_DEFAULT;
        return;
    }
}

/**
 * Change or Retrieve the value of a local attribute of an object PART
 * @param obj lv_obj_t*: the object to get/set the attribute
 * @param attr_p char*: the attribute name (with or without leading ".")
 * @param attr_hash uint16_t: the sbdm hash of the attribute name without leading "."
 * @param payload char*: the new value of the attribute
 * @param update  bool: change/set the value if true, dispatch/get value if false
 * @note setting a value won't return anything, getting will dispatch the value
 */
static void hasp_local_style_attr(lv_obj_t * obj, const char * attr_p, uint16_t attr_hash, const char * payload,
                                  bool update)
{
    char attr[32];
    uint8_t part  = LV_OBJ_PART_MAIN;
    uint8_t state = LV_STATE_DEFAULT;
    int16_t var   = atoi(payload);

    hasp_attribute_get_part_state(obj, attr_p, attr, part, state);
    attr_hash = sdbm(attr); // attribute name without the index number

    /* ***** WARNING ****************************************************
     * when using hasp_out use attr_p for the original attribute name
     * *************************************************************** */

    switch(attr_hash) {

/* 1: Use other blend modes than normal (`LV_BLEND_MODE_...`)*/
#if LV_USE_BLEND_MODES
        case ATTR_BG_BLEND_MODE:
            return attribute_bg_blend_mode(obj, part, state, update, attr_p, (lv_blend_mode_t)var);
        case ATTR_TEXT_BLEND_MODE:
            return lv_obj_set_style_local_text_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_BORDER_BLEND_MODE:
            return lv_obj_set_style_local_border_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_OUTLINE_BLEND_MODE:
            return lv_obj_set_style_local_outline_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_SHADOW_BLEND_MODE:
            return lv_obj_set_style_local_shadow_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_LINE_BLEND_MODE:
            return lv_obj_set_style_local_line_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_VALUE_BLEND_MODE:
            return lv_obj_set_style_local_value_blend_mode(obj, part, state, (lv_blend_mode_t)var);
        case ATTR_PATTERN_BLEND_MODE:
            return lv_obj_set_style_local_pattern_blend_mode(obj, part, state, (lv_blend_mode_t)var);
#endif

        case ATTR_SIZE:
            return attribute_size(obj, part, state, update, attr_p, var);
        case ATTR_RADIUS:
            return attribute_radius(obj, part, state, update, attr_p, var);
        case ATTR_CLIP_CORNER:
            return attribute_clip_corner(obj, part, state, update, attr_p, var);
        case ATTR_OPA_SCALE:
            return attribute_opa_scale(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_TRANSFORM_WIDTH:
            return attribute_transform_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TRANSFORM_HEIGHT:
            return attribute_transform_height(obj, part, state, update, attr_p, (lv_style_int_t)var);

            /* Background attributes */
        case ATTR_BG_MAIN_STOP:
            return attribute_bg_main_stop(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BG_GRAD_STOP:
            return attribute_bg_grad_stop(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BG_GRAD_DIR:
            return attribute_bg_grad_dir(obj, part, state, update, attr_p, (lv_grad_dir_t)var);
        case ATTR_BG_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            if(part != 64) return lv_obj_set_style_local_bg_color(obj, part, state, color);
            // else
            //     return lv_obj_set_style_local_bg_color(obj, LV_PAGE_PART_SCROLLBAR, LV_STATE_CHECKED, color);
        }
        case ATTR_BG_GRAD_COLOR:
            return lv_obj_set_style_local_bg_grad_color(obj, part, state, haspPayloadToColor(payload));
        case ATTR_BG_OPA:
            return attribute_bg_opa(obj, part, state, update, attr_p, (lv_opa_t)var);

        /* Padding attributes */
        case ATTR_PAD_TOP:
            return attribute_pad_top(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_BOTTOM:
            return attribute_pad_bottom(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_LEFT:
            return attribute_pad_left(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_PAD_RIGHT:
            return attribute_pad_right(obj, part, state, update, attr_p, (lv_style_int_t)var);
#if LVGL_VERSION_MAJOR == 7
        case ATTR_PAD_INNER:
            return; //attribute_pad_inner(obj, part, state, update, attr_p, (lv_style_int_t)var);
#endif

        /* Text attributes */
        case ATTR_TEXT_LETTER_SPACE:
            return attribute_text_letter_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TEXT_LINE_SPACE:
            return attribute_text_line_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_TEXT_DECOR:
            return attribute_text_decor(obj, part, state, update, attr_p, (lv_text_decor_t)var);
        case ATTR_TEXT_OPA:
            return attribute_text_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_TEXT_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_text_color(obj, part, state, color);
        }
        case ATTR_TEXT_SEL_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_text_sel_color(obj, part, state, color);
        }
        case ATTR_TEXT_FONT: {
            lv_font_t * font = haspPayloadToFont(payload);
            if(font) {
                return lv_obj_set_style_local_text_font(obj, part, state, font);
            } else {
                 return Log.verbose( 0, F("Unknown Font ID %s"), payload);
            }
        }

        /* Border attributes */
        case ATTR_BORDER_WIDTH:
            return attribute_border_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_BORDER_SIDE:
            return attribute_border_side(obj, part, state, update, attr_p, (lv_border_side_t)var);
        case ATTR_BORDER_POST:
            return attribute_border_post(obj, part, state, update, attr_p, is_true(payload));
        case ATTR_BORDER_OPA:
            return attribute_border_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_BORDER_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_border_color(obj, part, state, color);
        }

        /* Outline attributes */
        case ATTR_OUTLINE_WIDTH:
            return attribute_outline_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_OUTLINE_PAD:
            return attribute_outline_pad(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_OUTLINE_OPA:
            return attribute_outline_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_OUTLINE_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_outline_color(obj, part, state, color);
        }

        /* Shadow attributes */
#if LV_USE_SHADOW
        case ATTR_SHADOW_WIDTH:
            return attribute_shadow_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OFS_X:
            return attribute_shadow_ofs_x(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OFS_Y:
            return attribute_shadow_ofs_y(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_SPREAD:
            return attribute_shadow_spread(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_SHADOW_OPA:
            return attribute_shadow_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_SHADOW_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_shadow_color(obj, part, state, color);
        }
#endif

        /* Line attributes */
        case ATTR_LINE_WIDTH:
            return attribute_line_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_DASH_WIDTH:
            return attribute_line_dash_width(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_DASH_GAP:
            return attribute_line_dash_gap(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_LINE_ROUNDED:
            return attribute_line_rounded(obj, part, state, update, attr_p, is_true(payload));
        case ATTR_LINE_OPA:
            return attribute_line_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_LINE_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_line_color(obj, part, state, color);
        }

        /* Value attributes */
        case ATTR_VALUE_LETTER_SPACE:
            return attribute_value_letter_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_LINE_SPACE:
            return attribute_value_line_space(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_OFS_X:
            return attribute_value_ofs_x(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_OFS_Y:
            return attribute_value_ofs_y(obj, part, state, update, attr_p, (lv_style_int_t)var);
        case ATTR_VALUE_ALIGN:
            return attribute_value_align(obj, part, state, update, attr_p, (lv_align_t)var);
        case ATTR_VALUE_OPA:
            return attribute_value_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_VALUE_STR: {
            if(update) {

                size_t len = strlen(payload);
                if(len > 0) {
                    // Free previous string
                    const char * str = lv_obj_get_style_value_str(obj, part);

                    // Create new string
                    len++;
                    char * str_p = (char *)lv_mem_alloc(len);
                    memset(str_p, 0, len);
                    memccpy(str_p, payload, 0, len);
                    lv_obj_set_style_local_value_str(obj, part, state, str_p);

                    // if(strlen(str) > 0) lv_mem_free(str); // BIG : Memory Leak ! / crashes
                }
            } else {
                hasp_out_str(obj, attr, lv_obj_get_style_value_str(obj, part));
            }
            return;
        }
        case ATTR_VALUE_COLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_value_color(obj, part, state, color);
        }
        case ATTR_VALUE_FONT: {
            lv_font_t * font = haspPayloadToFont(payload);
            if(font) {
                return lv_obj_set_style_local_value_font(obj, part, state, font);
            } else {
                 return Log.verbose(0, F("Unknown Font ID %s"), attr_p);
            }
        }

        /* Pattern attributes */
        case ATTR_PATTERN_REPEAT:
            return attribute_pattern_repeat(obj, part, state, update, attr_p, is_true(payload));
        case ATTR_PATTERN_OPA:
            return attribute_pattern_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_PATTERN_RECOLOR_OPA:
            return attribute_pattern_recolor_opa(obj, part, state, update, attr_p, (lv_opa_t)var);
        case ATTR_PATTERN_IMAGE:
            //   return lv_obj_set_style_local_pattern_image(obj, part, state, (constvoid *)var);
            break;
        case ATTR_PATTERN_RECOLOR: {
            lv_color_t color = haspPayloadToColor(payload);
            return lv_obj_set_style_local_pattern_recolor(obj, part, state, color);
        }

            /* Image attributes */
            // Todo

            /* Scale attributes */
            // Todo

            /* Transition attributes */
            // Todo
    }
    Log.warning(TAG_ATTR, F("Unknown property %s"), attr_p);
}

static void hasp_process_gauge_attribute(lv_obj_t * obj, const char * attr_p, uint16_t attr_hash, const char * payload,
                                         bool update)
{
    // We already know it's a gauge object
    int16_t intval = atoi(payload);
    uint16_t val   = atoi(payload);

    uint8_t label_count = lv_gauge_get_label_count(obj);
    uint16_t line_count = lv_gauge_get_line_count(obj);
    uint16_t angle      = lv_gauge_get_scale_angle(obj);

    char * attr = (char *)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_CRITICAL_VALUE:
            return (update) ? lv_gauge_set_critical_value(obj, intval)
                            : hasp_out_int(obj, attr, lv_gauge_get_critical_value(obj));

        case ATTR_ANGLE:
            return (update) ? lv_gauge_set_scale(obj, val, line_count, label_count) : hasp_out_int(obj, attr, angle);

        case ATTR_LINE_COUNT:
            return (update) ? lv_gauge_set_scale(obj, angle, val, label_count) : hasp_out_int(obj, attr, line_count);

        case ATTR_LABEL_COUNT:
            return (update) ? lv_gauge_set_scale(obj, angle, line_count, val) : hasp_out_int(obj, attr, label_count);
    }

    Log.warning(TAG_ATTR, F("Unknown property %s"), attr_p);
}

static void hasp_process_btnmatrix_attribute(lv_obj_t * obj, const char * attr_p, uint16_t attr_hash,
                                             const char * payload, bool update)
{
    char * attr = (char *)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    switch(attr_hash) {
        case ATTR_MAP: {
            const char ** map_p = lv_btnmatrix_get_map_array(obj);
            if(update) {
                // Free previous map
                // lv_mem_free(*map_p);

                // if(map_p != lv_btnmatrix_def_map) {
                // }

                // Create new map

                // Reserve memory for JsonDocument
                size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 256;
                DynamicJsonDocument map_doc(maxsize);
                DeserializationError jsonError = deserializeJson(map_doc, payload);

                if(jsonError) { // Couldn't parse incoming JSON payload
                     return Log.verbose(0, F("JSON: Failed to parse incoming button map with error: %s"),
                                       jsonError.c_str());
                }

                JsonArray arr = map_doc.as<JsonArray>(); // Parse payload

                size_t tot_len        = sizeof(char *) * (arr.size() + 1);
                const char ** map_arr = (const char **)lv_mem_alloc(tot_len);
                if(map_arr == NULL) {
                     return Log.verbose(0, F("Out of memory while creating button map"));
                }
                memset(map_arr, 0, tot_len);

                // Create buffer
                tot_len    = 0;
                size_t pos = 0;
                for(JsonVariant btn : arr) {
                    tot_len += btn.as<std::string>().length() + 1;
                }
                tot_len++; // trailing '\0'
                Log.verbose(TAG_ATTR, F("Array Size = %d, Map Length = %d"), arr.size(), tot_len);

                char * buffer = (char *)lv_mem_alloc(tot_len);
                if(map_arr == NULL) {
                    lv_mem_free(map_arr);
                     return Log.verbose(0, F("Out of memory while creating button map"));
                }
                memset(buffer, 0, tot_len); // Important, last index needs to be 0

                // Fill buffer
                size_t index = 0;
                for(JsonVariant btn : arr) {
                    size_t len = btn.as<std::string>().length() + 1;
                    Log.verbose(TAG_ATTR, F("    * Adding button: %s (%d bytes)"), btn.as<std::string>().c_str(), len);
                    memccpy(buffer + pos, btn.as<std::string>().c_str(), 0, len); // Copy the labels into the buffer
                    map_arr[index++] = buffer + pos;                         // save pointer to start of the label
                    pos += len;
                }
                map_arr[index] = buffer + pos; // save pointer to the last \0 byte

                lv_btnmatrix_set_map(obj, map_arr);

                // TO DO : free & destroy previous buttonmap!

            } else {
                hasp_out_str(obj, attr, *map_p);
            }
            return;
        } // ATTR_MAP
    }

    Log.warning(TAG_ATTR, F("Unknown property %s"), attr_p);
}

// ##################### Common Attributes ########################################################

// OK
static void hasp_process_obj_attribute_txt(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type_str(objtype, LV_HASP_BUTTON)) {
        if(update) {
            haspSetLabelText(obj, payload);
        } else {
            char * text = NULL;
            if(haspGetLabelText(obj, text)) hasp_out_str(obj, attr, text);
        }
        return;
    }
    if(check_obj_type_str(objtype, LV_HASP_LABEL)) {
        return update ? lv_label_set_text(obj, payload) : hasp_out_str(obj, attr, lv_label_get_text(obj));
    }
    if(check_obj_type_str(objtype, LV_HASP_CHECKBOX)) {
        return update ? lv_checkbox_set_text(obj, payload) : hasp_out_str(obj, attr, lv_checkbox_get_text(obj));
    }
    if(check_obj_type_str(objtype, LV_HASP_DDLIST)) {
        char buffer[128];
        lv_dropdown_get_selected_str(obj, buffer, sizeof(buffer));
        return hasp_out_str(obj, attr, buffer);
    }
    if(check_obj_type_str(objtype, LV_HASP_ROLLER)) {
        char buffer[128];
        lv_roller_get_selected_str(obj, buffer, sizeof(buffer));
        return hasp_out_str(obj, attr, buffer);
    }

    Log.warning(TAG_ATTR, F("Unknown property %s"), attr);
}

static void hasp_process_obj_attribute_val(lv_obj_t * obj, const char * attr, const char * payload, bool update)
{
    int16_t intval = atoi(payload);
    uint16_t val   = atoi(payload);

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type_str(objtype, LV_HASP_BUTTON) && lv_btn_get_checkable(obj)) {
        if(update) {
            lv_btn_state_t state;
            switch(val) {
                case 0:
                    state = LV_BTN_STATE_RELEASED;
                    break;
                case 1:
                    state = LV_BTN_STATE_CHECKED_RELEASED;
                    break;
                case 3:
                    state = LV_BTN_STATE_CHECKED_DISABLED;
                    break;
                default:
                    state = LV_BTN_STATE_DISABLED;
            };
            return lv_btn_set_state(obj, state);
        } else {
            lv_btn_state_t state = lv_btn_get_state(obj);
            switch(state) {
                case LV_BTN_STATE_RELEASED:
                case LV_BTN_STATE_PRESSED:
                    return hasp_out_int(obj, attr, 0);
                case LV_BTN_STATE_CHECKED_RELEASED:
                case LV_BTN_STATE_CHECKED_PRESSED:
                    return hasp_out_int(obj, attr, 1);
                case LV_BTN_STATE_DISABLED:
                    return hasp_out_int(obj, attr, 2);
                case LV_BTN_STATE_CHECKED_DISABLED:
                    return hasp_out_int(obj, attr, 3);
            }
        }
    }

    if(check_obj_type_str(objtype, LV_HASP_CHECKBOX)) {
        return update ? lv_checkbox_set_checked(obj, is_true(payload))
                      : hasp_out_int(obj, attr, lv_checkbox_is_checked(obj));
    }
    if(check_obj_type_str(objtype, LV_HASP_SWITCH)) {
        if(update) {
            return is_true(payload) ? lv_switch_on(obj, LV_ANIM_ON) : lv_switch_off(obj, LV_ANIM_ON);
        } else {
            return hasp_out_int(obj, attr, lv_switch_get_state(obj));
        }

    } else if(check_obj_type_str(objtype, LV_HASP_DDLIST)) {
        lv_dropdown_set_selected(obj, val);
        return;

    } else if(check_obj_type_str(objtype, LV_HASP_LMETER)) {
        return update ? lv_linemeter_set_value(obj, intval) : hasp_out_int(obj, attr, lv_linemeter_get_value(obj));

    } else if(check_obj_type_str(objtype, LV_HASP_SLIDER)) {
        return update ? lv_slider_set_value(obj, intval, LV_ANIM_ON)
                      : hasp_out_int(obj, attr, lv_slider_get_value(obj));

    } else if(check_obj_type_str(objtype, LV_HASP_LED)) {
        return update ? lv_led_set_bright(obj, (uint8_t)val) : hasp_out_int(obj, attr, lv_led_get_bright(obj));

    } else if(check_obj_type_str(objtype, LV_HASP_ARC)) {
        return update ? lv_arc_set_value(obj, intval) : hasp_out_int(obj, attr, lv_arc_get_value(obj));

    } else if(check_obj_type_str(objtype, LV_HASP_GAUGE)) {
        return update ? lv_gauge_set_value(obj, 0, intval) : hasp_out_int(obj, attr, lv_gauge_get_value(obj, 0));

    } else if(check_obj_type_str(objtype, LV_HASP_ROLLER)) {
        lv_roller_set_selected(obj, val, LV_ANIM_ON);
        return;

    } else if(check_obj_type_str(objtype, LV_HASP_BAR)) {
        return update ? lv_bar_set_value(obj, intval, LV_ANIM_ON) : hasp_out_int(obj, attr, lv_bar_get_value(obj));

    } else if(check_obj_type_str(objtype, LV_HASP_CPICKER)) {
        return update ? (void)lv_cpicker_set_color(obj, haspPayloadToColor(payload))
                      : hasp_out_color(obj, attr, lv_cpicker_get_color(obj));
    }

    Log.warning(TAG_ATTR, F("Unknown property %s"), attr);
}

// OK
static void hasp_process_obj_attribute_range(lv_obj_t * obj, const char * attr, const char * payload, bool update,
                                             bool set_min, bool set_max)
{
    int16_t val   = atoi(payload);
    int32_t val32 = strtol(payload, nullptr, DEC);

    /* Attributes depending on objecttype */
    lv_obj_type_t list;
    lv_obj_get_type(obj, &list);
    const char * objtype = list.type[0];

    if(check_obj_type_str(objtype, LV_HASP_SLIDER)) {
        int16_t min = lv_slider_get_min_value(obj);
        int16_t max = lv_slider_get_max_value(obj);
        if(update && (set_min ? val : min) >= (set_max ? val : max)) return; // prevent setting min>=max
        return update ? lv_slider_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type_str(objtype, LV_HASP_GAUGE)) {
        int32_t min = lv_gauge_get_min_value(obj);
        int32_t max = lv_gauge_get_max_value(obj);
        if(update && (set_min ? val32 : min) >= (set_max ? val32 : max)) return; // prevent setting min>=max
        return update ? lv_gauge_set_range(obj, set_min ? val32 : min, set_max ? val32 : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type_str(objtype, LV_HASP_ARC)) {
        int16_t min = lv_arc_get_min_value(obj);
        int16_t max = lv_arc_get_max_value(obj);
        if(update && (set_min ? val : min) >= (set_max ? val : max)) return; // prevent setting min>=max
        return update ? lv_arc_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type_str(objtype, LV_HASP_BAR)) {
        int16_t min = lv_bar_get_min_value(obj);
        int16_t max = lv_bar_get_max_value(obj);
        if(update && (set_min ? val : min) >= (set_max ? val : max)) return; // prevent setting min>=max
        return update ? lv_bar_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type_str(objtype, LV_HASP_LMETER)) {
        int32_t min = lv_linemeter_get_min_value(obj);
        int32_t max = lv_linemeter_get_max_value(obj);
        if(update && (set_min ? val32 : min) >= (set_max ? val32 : max)) return; // prevent setting min>=max
        return update ? lv_linemeter_set_range(obj, set_min ? val32 : min, set_max ? val32 : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    if(check_obj_type_str(objtype, LV_HASP_CHART)) {
        int16_t min = lv_chart_get_min_value(obj);
        int16_t max = lv_chart_get_max_value(obj);
        if(update && (set_min ? val : min) >= (set_max ? val : max)) return; // prevent setting min>=max
        return update ? lv_chart_set_range(obj, set_min ? val : min, set_max ? val : max)
                      : hasp_out_int(obj, attr, set_min ? min : max);
    }

    Log.warning(TAG_ATTR, F("Unknown property %s"), attr);
}

// ##################### Default Attributes ########################################################

/**
 * Change or Retrieve the value of the attribute of an object
 * @param obj lv_obj_t*: the object to get/set the attribute
 * @param attr_p char*: the attribute name (with or without leading ".")
 * @param payload char*: the new value of the attribute
 * @param update  bool: change/set the value if true, dispatch/get value if false
 * @note setting a value won't return anything, getting will dispatch the value
 */
void hasp_process_obj_attribute(lv_obj_t * obj, const char * attr_p, const char * payload, bool update)
{
    // unsigned long start = millis();
    if(!obj)  return Log.verbose(0, F("Unknown object"));
    int16_t val = atoi(payload);

    char * attr = (char *)attr_p;
    if(*attr == '.') attr++; // strip leading '.'

    uint16_t attr_hash = sdbm(attr);
    Log.verbose(TAG_ATTR,"%s => %d", attr, attr_hash);

    /* 16-bit Hash Lookup Table */
    switch(attr_hash) {
        case ATTR_X:
            return update ? lv_obj_set_x(obj, val) : hasp_out_int(obj, attr, lv_obj_get_x(obj));

        case ATTR_Y:
            return update ? lv_obj_set_y(obj, val) : hasp_out_int(obj, attr, lv_obj_get_y(obj));

        case ATTR_W: {
            if(update) {
                lv_obj_set_width(obj, val);
                if(check_obj_type(obj, LV_HASP_CPICKER)) {
#if LVGL_VERSION_MAJOR == 7
                    lv_cpicker_set_type(obj, lv_obj_get_width(obj) == lv_obj_get_height(obj) ? LV_CPICKER_TYPE_DISC
                                                                                             : LV_CPICKER_TYPE_RECT);
#endif
                }
            } else {
                hasp_out_int(obj, attr, lv_obj_get_width(obj));
            }
            return;
        }

        case ATTR_H: {
            if(update) {
                lv_obj_set_height(obj, val);
                if(check_obj_type(obj, LV_HASP_CPICKER)) {
#if LVGL_VERSION_MAJOR == 7
                    lv_cpicker_set_type(obj, lv_obj_get_width(obj) == lv_obj_get_height(obj) ? LV_CPICKER_TYPE_DISC
                                                                                             : LV_CPICKER_TYPE_RECT);
#endif
                }
            } else {
                hasp_out_int(obj, attr, lv_obj_get_height(obj));
            }
            return;
        }

        case ATTR_ID:
            return update ? (void)(obj->user_data.id = (uint8_t)val) : hasp_out_int(obj, attr, obj->user_data.id);

        case ATTR_VIS:
            return update ? lv_obj_set_hidden(obj, !is_true(payload))
                          : hasp_out_int(obj, attr, !lv_obj_get_hidden(obj));

        case ATTR_TXT:
            return hasp_process_obj_attribute_txt(obj, attr, payload, update);

        case ATTR_VAL:
            return hasp_process_obj_attribute_val(obj, attr, payload, update);

        case ATTR_MIN:
            return hasp_process_obj_attribute_range(obj, attr, payload, update, true, false);

        case ATTR_MAX:
            return hasp_process_obj_attribute_range(obj, attr, payload, update, false, true);

        case ATTR_HIDDEN:
            return update ? lv_obj_set_hidden(obj, is_true(payload)) : hasp_out_int(obj, attr, lv_obj_get_hidden(obj));

        case ATTR_SRC:
            if(check_obj_type(obj, LV_HASP_IMAGE)) {
                if(update) {
                    return lv_img_set_src(obj, payload);
                } else {
                    switch(lv_img_src_get_type(obj)) {
                        case LV_IMG_SRC_FILE:
                            return hasp_out_str(obj, attr, lv_img_get_file_name(obj));
                        case LV_IMG_SRC_SYMBOL:
                            return hasp_out_str(obj, attr, (char *)lv_img_get_src(obj));
                        default:
                            return;
                    }
                }
            }
            break;

        case ATTR_ROWS:
            if(check_obj_type(obj, LV_HASP_ROLLER)) {
                return update ? lv_roller_set_visible_row_count(obj, (uint8_t)val)
                              : hasp_out_int(obj, attr, lv_roller_get_visible_row_count(obj));
            }

            if(check_obj_type(obj, LV_HASP_TABLE)) {
                return update ? lv_table_set_row_cnt(obj, (uint8_t)val)
                              : hasp_out_int(obj, attr, lv_table_get_row_cnt(obj));
            }
            break;

        case ATTR_COLS:
            if(check_obj_type(obj, LV_HASP_TABLE)) {
                return update ? lv_table_set_col_cnt(obj, (uint8_t)val)
                              : hasp_out_int(obj, attr, lv_table_get_col_cnt(obj));
            }
            break;

            // case ATTR_RECT:
            //     if(check_obj_type(obj, LV_HASP_CPICKER)) {
            //         lv_cpicker_set_type(obj, is_true(payload) ? LV_CPICKER_TYPE_RECT : LV_CPICKER_TYPE_DISC);
            //         return;
            //     }
            //     break;

        case ATTR_MODE:
            if(check_obj_type(obj, LV_HASP_BUTTON)) {
                lv_obj_t * label = FindButtonLabel(obj);
                if(label) {
                    hasp_process_label_long_mode(label, payload, update);
                    lv_obj_set_width(label, lv_obj_get_width(obj));
                }
                return;
            }

            if(check_obj_type(obj, LV_HASP_LABEL)) {
                hasp_process_label_long_mode(obj, payload, update);
                return;
            }
            break;

        case ATTR_TOGGLE:
            if(check_obj_type(obj, LV_HASP_BUTTON)) {
                if(update) {
                    bool toggle = is_true(payload);
                    lv_btn_set_checkable(obj, toggle);
                    lv_obj_set_event_cb(obj, toggle ? toggle_event_handler : btn_event_handler);
                } else {
                    hasp_out_int(obj, attr, lv_btn_get_checkable(obj));
                }
                return;
            }
            break; // not a toggle object

        case ATTR_OPACITY:
            return update ? lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, val)
                          : hasp_out_int(obj, attr, lv_obj_get_style_opa_scale(obj, LV_OBJ_PART_MAIN));

        case ATTR_ENABLED:
            return update ? lv_obj_set_click(obj, is_true(payload)) : hasp_out_int(obj, attr, lv_obj_get_click(obj));

        case ATTR_OPTIONS:
            if(check_obj_type(obj, LV_HASP_DDLIST)) {
                if(update) {
                    lv_dropdown_set_options(obj, payload);
                } else {
                    hasp_out_str(obj, attr, lv_dropdown_get_options(obj));
                }
                return;
            }

            if(check_obj_type(obj, LV_HASP_ROLLER)) {
                if(update) {
                    lv_roller_ext_t * ext = (lv_roller_ext_t *)lv_obj_get_ext_attr(obj);
                    lv_roller_set_options(obj, payload, ext->mode);
                } else {
                    hasp_out_str(obj, attr, lv_roller_get_options(obj));
                }
                return;
            }
            break; // not a options object

        case ATTR_CRITICAL_VALUE:
        case ATTR_ANGLE:
        case ATTR_LABEL_COUNT:
        case ATTR_LINE_COUNT:
        case ATTR_FORMAT:
            if(check_obj_type(obj, LV_HASP_GAUGE)) {
                return hasp_process_gauge_attribute(obj, attr_p, attr_hash, payload, update);
            }

        case ATTR_DELETE:
            return lv_obj_del_async(obj);

        case ATTR_MAP:
            if(check_obj_type(obj, LV_HASP_BTNMATRIX)) {
                return hasp_process_btnmatrix_attribute(obj, attr_p, attr_hash, payload, update);
            }
            // default:
            // hasp_local_style_attr(obj, attr, payload, update);
    }

    hasp_local_style_attr(obj, attr, attr_hash, payload, update);
    Log.verbose(TAG_ATTR, F("%s (%d)"), attr_p, attr_hash);
    // Log.verbose( F("%s (%d) took %d ms."), attr_p, attr_hash, millis() - start);
}

/* **************************
 * Static Inline functions
 * **************************/
static inline bool only_digits(const char * s)
{
    size_t digits = 0;
    while(*(s + digits) != '\0' && isdigit(*(s + digits))) {
        digits++;
    }
    return strlen(s) == digits;
}

// ##################### Value Senders ########################################################

static inline void hasp_out_int(lv_obj_t * obj, const char * attr, uint32_t val)
{
    hasp_send_obj_attribute_int(obj, attr, val);
}

static inline void hasp_out_str(lv_obj_t * obj, const char * attr, const char * data)
{
    hasp_send_obj_attribute_str(obj, attr, data);
}

static inline void hasp_out_color(lv_obj_t * obj, const char * attr, lv_color_t color)
{
    hasp_send_obj_attribute_color(obj, attr, color);
}
