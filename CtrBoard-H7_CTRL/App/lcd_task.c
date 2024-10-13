#include "lcd_task.h"
#include "cmsis_os.h"
#include "lcd.h"
#include "pic.h"

extern float vbus;
/* USER CODE BEGIN Header_LcdTask_Entry */
/**
  * @brief  LCD显示任务入口函数，用于控制LCD屏幕的显示内容
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void LcdTask_Entry(void const * argument)
{
    /* USER CODE BEGIN LcdTask_Entry */
    LCD_Init();//LCD初始化
    LCD_Fill(0,0,LCD_W, LCD_H,BLACK);
    
    /* Infinite loop */
    for(;;)
    {
        LCD_ShowString(120, 72,(uint8_t *)"dmBot", BRRED, BLACK, 24, 0);
        LCD_ShowChinese(84, 100, (uint8_t *)"达妙科技", WHITE, BLACK, 32, 0);
        LCD_DrawLine(10, 0, 10,  280,WHITE);
        LCD_DrawLine(270,0, 270, 280,WHITE);
//        LCD_ShowIntNum(50, 170, adc_val[0], 5, WHITE, BLACK, 32);
        LCD_ShowFloatNum(50, 170, vbus, 3, 2, WHITE, BLACK, 32);
        LCD_ShowPicture(180, 150, 80, 80, gImage_1);
        osDelay(10);
    }
    /* USER CODE END LcdTask_Entry */
}



