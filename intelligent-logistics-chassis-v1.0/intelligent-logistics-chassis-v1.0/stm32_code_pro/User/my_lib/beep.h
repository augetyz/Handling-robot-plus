#ifndef _BEEP_H
#define _BEEP_H

#include "FreeRTOS.h"
#include "task.h"

#include "my_lib/beep.h"
#include "my_lib/m_shell.h"
#include "my_lib/my_log.h"

typedef enum
{
    BeepOff = 0,
    BeepOn,
    BeepOnOff,
    BeepOnMement,
}BeepFun_Enum;

typedef struct 
{
    BeepFun_Enum BeepFun;
    uint16_t CycleOrMement;
    uint32_t LastChangeTick;
    uint8_t LastLevel;
}Beep_Type;

void BeepTask(void* param);
void SetBeepFun(uint8_t BeepPointer, BeepFun_Enum BeepFun, uint16_t OnOffCycle);


#endif

