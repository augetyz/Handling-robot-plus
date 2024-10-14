#include "bsp_user_key.h"
#include "cmsis_os.h"
KeyState keyState = NO_DETECT;
uint32_t overTick = 0;
uint16_t key_value;
uint8_t key_up=0;
uint8_t key_down=0;
uint8_t key_mid=0;
uint8_t key_left=0;
uint8_t key_right=0;
extern volatile uint16_t adc_val[2];


int8_t BSP_UserKey_Detect(void)
{
    GPIO_PinState pinSta;
    uint32_t nowTick = HAL_GetTick();
    pinSta = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15);
    if (keyState == NO_DETECT)
    {
        if (pinSta == GPIO_PIN_RESET)
        {
            keyState = DETECTING;
            #warning "TODO: 需要解决溢出的情况"
            overTick = nowTick + 100;//0.1s
            
        }
        return BUTTON_NOT_PRESSED;
    }
    else if (keyState == DETECTING)
    {
        if (nowTick >= overTick)
        {
            if (pinSta == GPIO_PIN_RESET)
            {
                keyState = DETECTED;
                return BUTTON_PRESSED;
            }
            else
            {
                keyState = NO_DETECT;
                return BUTTON_NOT_PRESSED;
            }
        }
        return BUTTON_NOT_PRESSED;
    }
    else if (keyState == DETECTED)
    {
        if (pinSta == GPIO_PIN_SET)
        {
            keyState = NO_DETECT;
        }
        return BUTTON_NOT_PRESSED;
    }
    return BUTTON_NOT_PRESSED;
}


/**
***********************************************************************
* @brief:      get_key_adc
* @param:			 void
* @retval:     void
* @details:    获取按键adc建值并转化为 0 1 信号
***********************************************************************
**/
void get_key_adc(void)
{
	key_value = (float)adc_val[1];
	
	if (key_value>=0 && key_value<200)
    {
        key_mid = 0;
        osDelay(50);
        return;
    }
	else
		key_mid = 1;
	if (key_value>2200 && key_value<2600)
    {
        key_up = 0;
        osDelay(50);
        return;
    }
	else
		key_up = 1;
	if (key_value>3000 && key_value<3600)
    {
        key_down = 0;
        osDelay(50);
        return;
    }
	else
		key_down = 1;
	if (key_value>1500 && key_value<1800)
	{
        key_left = 0;
        osDelay(50);
        return;
    }
	else
		key_left = 1;
	if (key_value>700 && key_value<1000)
    {
        key_right = 0;
        osDelay(50);
        return;   
    }
	else
		key_right = 1;
}




