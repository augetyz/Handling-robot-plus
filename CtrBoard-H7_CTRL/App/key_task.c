#include "key_task.h"
#include "adc.h"
#include "cmsis_os.h"
#include "vofa.h"
#include "bsp_user_key.h"
#include "bsp_buzzer.h"
#include "ws2812.h"

float vbus = 0;
volatile uint16_t adc_val[2];
extern void AHRS_init(float quat[4]);
extern float imuQuat[4];
/**
  * @brief  按键任务入口函数，用于处理五向按键和板载按键
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void KeyTask_Entry(void const * argument)
{
    /* USER CODE BEGIN KeyTask_Entry */
    BSP_Buzzer_Init();
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_val,2);
    WS2812_Ctrl(0, 10, 0);
    /* Infinite loop */
    for(;;)
    {
        if (BSP_UserKey_Detect() == BUTTON_PRESSED)
        {
            AHRS_init(imuQuat);
        }
        vbus = (adc_val[0]*3.3f/65535)*11.0f;
        osDelay(10);
    }
    /* USER CODE END KeyTask_Entry */
}



