#include "led.h"

#include "main.h"

static uint8_t gLedBit =0xff;
static LedType gLeds[2];


static void LedBitCtrl(uint8_t LedNum, uint8_t LedSta)
{
    if(LedSta){
        gLedBit |= 1<<(LedNum);
    }else{
        gLedBit &= ~(1<<(LedNum));
    }

    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, gLedBit&(1<<0));
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, gLedBit&(1<<1));
}

void LedSetParam(uint8_t LedNum, LedSta_Enum LedSta, uint16_t Tick)
{
    gLeds[LedNum].sta = LedSta;
    gLeds[LedNum].tick = Tick;
    gLeds[LedNum].LastChangeTick = GetSysTick();
    gLeds[LedNum].LastLevel = 0;
}

static void LedInit(void)
{
    LedSetParam(0, LedOff, 0);
    LedSetParam(1, LedOff, 0);;
}

void LedTask(void* param)
{
    LedInit();
    LedSetParam(0, LedOnOff, 200);
    LedSetParam(1, LedOnOff, 500);
    while(1)
    {
        for(uint8_t i=0; i<3; i++)
        {
            switch (gLeds[i].sta)
            {
            case LedOff:
                LedBitCtrl(i, 1);
                break;
            case LedOn:
                LedBitCtrl(i, 0);
                break;
            case LedOnOff:
                if((GetSysTick()-gLeds[i].LastChangeTick) > gLeds[i].tick)
                {
                    gLeds[i].LastChangeTick = GetSysTick();
                    gLeds[i].LastLevel = ~gLeds[i].LastLevel;
                    LedBitCtrl(i, gLeds[i].LastLevel);
                }
                break;
            
            default:
                break;
            }
        }
        vTaskDelay(10);
    }
}

