#ifndef __TEST_TASK_H__
#define __TEST_TASK_H__

// #include "iwdg.h"

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"

#include "my_lib/my_log.h"
#include "my_lib/led.h"
#include "my_lib/button.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"
// #include "my_gui/my_gui.h"
// #include "my_lib/adc_wave.h"
// #include "SEEKFREE_MPU6050/SEEKFREE_MPU6050.h"
// #include "ZTJS/imulib.h"



void TestTask(void* param);
float CheckBatteryVoltage(void);

//temp test
// void QR_CodeGetChar(uint8_t info);

#endif

