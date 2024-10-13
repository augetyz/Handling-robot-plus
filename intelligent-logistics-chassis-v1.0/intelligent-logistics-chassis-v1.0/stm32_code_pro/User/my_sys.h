#ifndef _MY_SYS_H
#define _MY_SYS_H

#include "stdio.h"
#include "stdint.h"


#define USE_GUI  1


void AddSysTick(uint16_t PassTick);
uint32_t GetSysTick(void);
void USER_ASSERT(uint8_t judge);


void MyRTOS_Init(void);
// void MainLoop(void);



#endif


