#include "beep.h"

#include "main.h"

static Beep_Type BeepArr[1] = {0};

/// @brief 设置蜂鸣器开关状态
/// @param BeepPointer BeepArr 数组指针
/// @param BeepSta 0/1 开/关
static void BeepCtrlOne(uint8_t BeepPointer, uint8_t BeepSta)
{
    switch (BeepPointer)
    {
    case 0:
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, BeepSta);
        break;
    
    default:
        break;
    }
}

void SetBeepFun(uint8_t BeepPointer, BeepFun_Enum BeepFun, uint16_t CycleOrMement)
{
    BeepArr[BeepPointer].BeepFun = BeepFun;
    BeepArr[BeepPointer].CycleOrMement = CycleOrMement;
    BeepArr[BeepPointer].LastChangeTick = GetSysTick(); 
    BeepArr[BeepPointer].LastLevel = 0;  
}


//cmd test
void BeepTest(int argc, char** argv)
{
    uint16_t tick = atoi(argv[1]);
    MY_LOGI("beep", "beep on for %d ticks", tick);
    SetBeepFun(0, BeepOnMement, tick);
}

void BeepTask(void* param)
{
    //蜂鸣器测试命令
    ShellCmdRegister("beep",
                "beep test, like:200, beep on for 200 ticks",
                BeepTest);

    SetBeepFun(0, BeepOnMement, 300); //上电提示
    while (1)
    {
        for(uint8_t i=0; i<sizeof(BeepArr)/sizeof(BeepArr[0]); i++)
        {
            if(BeepArr[i].BeepFun == BeepOff)
            {
                BeepCtrlOne(i, 0);
            }else if(BeepArr[i].BeepFun == BeepOn)
            {
                BeepCtrlOne(i, 1);
            }else if(BeepArr[i].BeepFun == BeepOnOff)
            {
                if((GetSysTick()-BeepArr[i].LastChangeTick) > BeepArr[i].CycleOrMement)
                {
                    BeepArr[i].LastChangeTick = GetSysTick();
                    BeepArr[i].LastLevel = (!BeepArr[i].LastLevel)&0x01;
                    BeepCtrlOne(i, BeepArr[i].LastLevel);
                }
            }else if(BeepArr[i].BeepFun == BeepOnMement)
            {
                if((GetSysTick()-BeepArr[i].LastChangeTick) < BeepArr[i].CycleOrMement)
                {
                    BeepCtrlOne(i, 1);
                }else
                {
                    BeepCtrlOne(i, 0);
                }
            }
        }
        vTaskDelay(20);
    }
}