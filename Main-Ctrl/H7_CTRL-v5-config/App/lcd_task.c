#include "lcd_task.h"
#include "cmsis_os.h"
#include "lcd.h"
#include "pic.h"
#include "typedef_user.h"
extern float vbus;
extern float imuAngle[3];
extern QueueHandle_t qrQueue;
void show_QR(QR_code_date date);

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
//    LCD_ShowString(10, 72,(uint8_t *)"123321", BRRED, BLACK, 32, 0);
//    LCD_ShowChinese(84, 100, (uint8_t *)"达妙科技", WHITE, BLACK, 32, 0);
    
    LCD_ShowString(200, 0,(uint8_t *)"YAW", LIGHTBLUE, BLACK, 24, 0);
    LCD_ShowString(200, 50,(uint8_t *)"vbus", BRED, BLACK, 24, 0);
    
    
    QR_code_date QR_num ={2,1,3,1,3,2};
    show_QR(QR_num);
    
    /* Infinite loop */
    for(;;)
    {
        
        LCD_ShowFloatNum(200, 25, imuAngle[0], 3, 2, LIGHTBLUE, BLACK, 24);
        LCD_ShowFloatNum(200, 75, vbus, 3, 2, BRED, BLACK, 24);
        osDelay(10);
        
        if (xQueueReceive(qrQueue, &QR_num, 0) == pdPASS) {
            // 处理接收到的QR码数据
            show_QR(QR_num);
        }
    }
    /* USER CODE END LcdTask_Entry */
}
void show_bignum(uint16_t x,uint16_t y,uint8_t num)
{
    if(num==1)LCD_ShowPicture(x, y, 60, 80, gImage_1);
    else if(num==2)LCD_ShowPicture(x, y, 60, 80, gImage_2);
    else if(num==3)LCD_ShowPicture(x, y, 60, 80, gImage_3);
}
void show_QR(QR_code_date date)
{
    show_bignum(10,0,date.QR_date[0]);
    show_bignum(10+60,0,date.QR_date[1]);
    show_bignum(10+120,0,date.QR_date[2]);
    show_bignum(10,0+80,date.QR_date[3]);
    show_bignum(10+60,0+80,date.QR_date[4]);
    show_bignum(10+120,0+80,date.QR_date[5]);
}

