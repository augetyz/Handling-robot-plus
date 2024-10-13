#ifndef _LED_H
#define _LED_H

#include "my_sys.h"

#include "FreeRTOS.h"
#include "task.h"

typedef enum 
{
    LedOn = 0,
    LedOff,
    LedOnOff,
}LedSta_Enum;


typedef struct 
{
    LedSta_Enum sta;  
    uint16_t tick;
    uint32_t LastChangeTick;
    uint8_t LastLevel;
}LedType;




// void LedInit(void);
void LedTask(void* param);
void LedSetParam(uint8_t LedNum, LedSta_Enum LedSta, uint16_t Tick);
#endif

