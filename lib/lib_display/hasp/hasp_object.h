/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_OBJECT_H
#define HASP_OBJECT_H

#include "ArduinoJson.h"
using namespace ArduinoJson;

#define MY_PRINTF(a,...) {char cad[512]; snprintf(cad,sizeof(cad), __VA_ARGS__);  OutputDebugString(cad);}

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

    enum {
        LV_HASP_BTNMATRIX = 1,
        LV_HASP_TABLE = 2,

        LV_HASP_BUTTON = 10,
        LV_HASP_CHECKBOX = 11,
        LV_HASP_LABEL = 12,

        LV_HASP_CPICKER = 20,
        LV_HASP_PRELOADER = 21,
        LV_HASP_ARC = 22,

        LV_HASP_SLIDER = 30,
        LV_HASP_GAUGE = 31,
        LV_HASP_BAR = 32,
        LV_HASP_LMETER = 33,

        LV_HASP_SWITCH = 40,
        LV_HASP_LED = 41,

        LV_HASP_DDLIST = 50,
        LV_HASP_ROLLER = 51,

        LV_HASP_IMAGE = 60,
        LV_HASP_IMGBTN = 61,
        LV_HASP_CANVAS = 62,

        LV_HASP_TILEVIEW = 70,
        LV_HASP_TABVIEW = 71,
        LV_HASP_TAB = 72,

        LV_HASP_CHART = 80,
        LV_HASP_CALENDER = 81,

        LV_HASP_CONTAINER = 90,
        LV_HASP_OBJECT = 91,
        LV_HASP_PAGE = 92,
        LV_HASP_MSGBOX = 93,
        LV_HASP_WINDOW = 94,
    };
    typedef uint8_t lv_hasp_obj_type_t;

    enum { // even = released, odd = pressed
        HASP_EVENT_OFF = 0,
        HASP_EVENT_ON = 1,
        HASP_EVENT_UP = 2,
        HASP_EVENT_DOWN = 3,

        HASP_EVENT_SHORT = 4,
        HASP_EVENT_LONG = 5,
        HASP_EVENT_LOST = 6,
        HASP_EVENT_HOLD = 7,
        HASP_EVENT_DOUBLE = 8
    };
    typedef uint8_t hasp_event_t;

    void hasp_new_object(const JsonObject & config, uint8_t & saved_page_id);

    lv_obj_t* hasp_find_obj_from_parent_id(lv_obj_t* parent, uint8_t objid);
    // lv_obj_t * hasp_find_obj_from_page_id(uint8_t pageid, uint8_t objid);
    bool hasp_find_id_from_obj(lv_obj_t* obj, uint8_t* pageid, uint8_t* objid);
    bool check_obj_type_str(const char* lvobjtype, lv_hasp_obj_type_t haspobjtype);
        bool check_obj_type(lv_obj_t* obj, lv_hasp_obj_type_t haspobjtype);

    void hasp_object_tree(lv_obj_t* parent, uint8_t pageid, uint16_t level);

    void hasp_send_obj_attribute_str(lv_obj_t* obj, const char* attribute, const char* data);
    void hasp_send_obj_attribute_int(lv_obj_t* obj, const char* attribute, int32_t val);
    void hasp_send_obj_attribute_color(lv_obj_t* obj, const char* attribute, lv_color_t color);
    void hasp_process_attribute(uint8_t pageid, uint8_t objid, const char* attr, const char* payload);

    void IRAM_ATTR btn_event_handler(lv_obj_t* obj, lv_event_t event);
    void IRAM_ATTR toggle_event_handler(lv_obj_t* obj, lv_event_t event);
    void wakeup_event_handler(lv_obj_t* obj, lv_event_t event);

    void testparse();

#ifdef __cplusplus
}
#endif

#endif