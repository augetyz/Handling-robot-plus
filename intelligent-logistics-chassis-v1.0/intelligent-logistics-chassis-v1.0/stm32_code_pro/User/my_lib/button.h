#ifndef _BUTTON_H
#define _BUTTON_H

#include "my_sys.h"
#include "my_lib/my_log.h"

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

typedef enum
{
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT,
    BTN_EVENT_LONG,
}button_event_enum_t;

typedef struct 
{
    uint32_t StartPressTick;
    uint8_t StartPressFlag;
    button_event_enum_t PressEvent;
}ButtonType;

void ButtonTask(void* param);
button_event_enum_t AskButtonEvent(uint8_t ButtonNum, uint8_t isClean);




#endif


