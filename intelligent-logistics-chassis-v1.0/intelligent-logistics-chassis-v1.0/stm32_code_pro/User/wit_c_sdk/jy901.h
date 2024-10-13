#ifndef _JY901_H
#define _JY901_H

#include "wit_c_sdk.h"

#include "my_sys.h"
#include "my_lib/my_log.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"
#include "main.h"
#include "usart.h"


void JY901_Task(void* param);

// void JY901_Init(void);
// void JY901_Loop(uint16_t Cycle);
float GetYaw(void);
uint8_t IsIMU_Start(void);

#endif

