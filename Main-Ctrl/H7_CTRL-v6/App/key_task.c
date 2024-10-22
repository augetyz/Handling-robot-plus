#include "key_task.h"
#include "adc.h"
#include "cmsis_os.h"
#include "vofa.h"
#include "bsp_user_key.h"
#include "bsp_buzzer.h"
#include "ws2812.h"
#include "bsp_motor_ctrl.h"
float vbus = 0;
volatile uint16_t adc_val[2];
extern void AHRS_init(float quat[4]);
extern float imuQuat[4];
extern uint8_t key_up;
extern uint8_t key_down;
extern uint8_t key_mid;
extern uint8_t key_left;
extern uint8_t key_right;
extern QueueHandle_t Move_task_queue;
/**
  * @brief  按键任务入口函数，用于处理五向按键和板载按键
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void KeyTask_Entry(void const * argument)
{
    /* USER CODE BEGIN KeyTask_Entry */
    uint32_t RGB_num=0;
    
    BSP_Buzzer_Init();
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_val,2);
    WS2812_Ctrl(100, 100, 100);
    position_order Move_order={0};
    Move_order.x=600;
    Move_order.y=200;
    Move_order.angle=0;
  /* Infinite loop */
    
    /* Infinite loop */
    for(;;)
    {
        vbus = (adc_val[0]*3.3f/4096)*11.0f;
        if(vbus<11.3)
        {
            BSP_Buzzer_On();
        }
        get_key_adc();
        
        if (BSP_UserKey_Detect() == BUTTON_PRESSED)
        {
            AHRS_init(imuQuat);
        }
        
        if (key_up==0)
        {
            WS2812_Ctrl(20,0,0);
            sync_ctrl_speed(0,0,0,0);
        }
        if (key_down==0)
        {
            WS2812_Ctrl(0,20,20);//存在问题，易触发
        }
        if (key_mid==0)
        {
            WS2812_Ctrl(0,0,20);

        }
        if (key_left==0)
        {
            WS2812_Ctrl(20,20,0);
        }
        if (key_right==0)
        {
            WS2812_Ctrl(20,0,20);
        }
//        if(RGB_num>=0XFFFFFF)
//            RGB_num=0;
//        WS2812_Ctrl((RGB_num>>16)&0XFF,(RGB_num>>8)&0XFF,RGB_num&0XFF);
//        if(RGB_num>0XFFFF)
//            RGB_num+=0X10000;
//        else if(RGB_num>0XFF)
//            RGB_num+=0X100;
//        else
//            RGB_num++;
        
        osDelay(10);
    }
    /* USER CODE END KeyTask_Entry */
}



