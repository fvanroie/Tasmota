/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DISPATCH_H
#define HASP_DISPATCH_H

#include "ArduinoJson.h"
#include "lvgl.h"

/* ===== Default Event Processors ===== */
void dispatchSetup(void);
void IRAM_ATTR dispatchLoop(void);
void dispatchEverySecond(void);
void dispatchStart(void);
void dispatchStop(void);

/* ===== Special Event Processors ===== */
void dispatch_topic_payload(const char * topic, const char * payload);
void dispatch_text_line(const char * cmnd);
void dispatch_parse_jsonl(Stream & stream);
void dispatch_clear_page(const char * page);

// void dispatchPage(uint8_t page);
void dispatch_page_next();
void dispatch_page_prev();

void dispatch_dim(const char * level);
void dispatch_backlight(const char * payload);

void dispatch_web_update(const char * espOtaUrl);
void dispatch_reboot(bool saveConfig);

void dispatch_output_idle_state(uint8_t state);
void dispatch_output_statusupdate(const char *, const char *);
void dispatch_output_current_page();

void dispatch_gpio_event(uint8_t pin, uint8_t group, uint8_t eventid);
void dispatch_object_event(lv_obj_t * obj, uint8_t eventid);
bool dispatch_get_event_state(uint8_t eventid);

bool is_true(const char * s);
void IRAM_ATTR dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute,
                                               const char * data);

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */

/* ===== Structs and Constants ===== */
struct haspCommand_t
{
    void (*func)(const char *, const char *);
    const char * p_cmdstr;
};

#endif