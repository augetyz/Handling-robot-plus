#include "button.h"

ButtonType gButtons[3] = {0};

/// @brief 获取按键电平
/// @param ButtonNum 0/1
/// @return 
static uint8_t GetButtonLevel(uint8_t ButtonNum)
{
    switch (ButtonNum)
    {
    case 0:
        return HAL_GPIO_ReadPin(B0_GPIO_Port, B0_Pin);
    case 1:
        return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
    case 2:
        return HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin);
    
    default:
        return 1;
    }
}

void ButtonTask(void* param)
{
    while(1)
    {
        for(uint8_t i=0; i<(sizeof(gButtons)/sizeof(ButtonType)); i++)
        {
            if(!GetButtonLevel(i)){
                if(!gButtons[i].StartPressFlag){
                    gButtons[i].StartPressFlag = 1;
                    gButtons[i].StartPressTick = GetSysTick();
                }
            }else{
                if(gButtons[i].StartPressFlag){
                    gButtons[i].StartPressFlag = 0;
                    if((GetSysTick()-gButtons[i].StartPressTick) > 700){
                        gButtons[i].PressEvent = BTN_EVENT_LONG;
                    }
                    else if((GetSysTick()-gButtons[i].StartPressTick) > 20){
                        gButtons[i].PressEvent = BTN_EVENT_SHORT;
                    }
                }
            }
        }
        // //temp test
        // if(AskButtonEvent(0))
        // {
        //     MY_LOGI("button", "button 0 click");
        // }
        vTaskDelay(5);
    }
}

button_event_enum_t AskButtonEvent(uint8_t ButtonNum, uint8_t isClean)
{
    button_event_enum_t re = gButtons[ButtonNum].PressEvent;
    if(isClean){
        gButtons[ButtonNum].PressEvent = BTN_EVENT_NONE;
    }

    return re;
}


