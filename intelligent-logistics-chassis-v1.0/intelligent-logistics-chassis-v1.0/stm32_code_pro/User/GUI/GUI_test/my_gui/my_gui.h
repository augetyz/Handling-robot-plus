#ifndef _GUI_TEST_H
#define _GUI_TEST_H

#include "my_sys.h"
#include "lcd.h"
#include "xpt2046.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

// void GUI_Init(void);
void GUI_Task(void* param);

void IsEnableLvgl(uint8_t en);
void IsGuiTaskHighPriority(uint8_t en);


#endif


