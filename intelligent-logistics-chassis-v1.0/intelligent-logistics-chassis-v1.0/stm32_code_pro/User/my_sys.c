#include "my_sys.h"

#include "main.h"
#include "usart.h"
#include "tim.h"
#include "iwdg.h"


#include "my_lib/my_log.h"
#include "my_task/my_task.h"
#include "my_gui/my_gui.h"
#include "my_lib/button.h"
#include "my_lib/led.h"
#include "my_lib/motor.h"
#include "my_lib/car_ctrl.h"
#include "wit_c_sdk/jy901.h"
#include "my_lib/file_info.h"
#include "my_lib/route.h"
#include "my_lib/arm.h"

#include "my_lib/rgb.h"
#include "my_lib/communication.h"
#include "my_lib/remote_ctrl.h"

#include "my_lib/m_shell.h"
#include "QR_code.h"
#include "beep.h"



//uart2 字符接收暂存变量
static uint8_t UartReceive2, UartReceive3, UartReceive4;


/// @brief ????//////////////////////////////////////////////////////////////////////////////
static uint32_t gSysTick =0;

void AddSysTick(uint16_t PassTick)
{
    gSysTick += PassTick;

    if(gSysTick%10 == 0){
        HAL_IWDG_Refresh(&hiwdg);  //100ms内喂狗一次
    }
}
uint32_t GetSysTick(void)
{
    return gSysTick;
}

/// @brief ??
/// @param judge ????????
void USER_ASSERT(uint8_t judge)
{
    if(!judge)                            
    {                                     
        while(1)
        {
            MY_LOG_Print("User Assert Failed!\r\n");
            vTaskDelay(1000);
        }                        
    }    
}

/// @brief printf
/// @param ch 
/// @param f 
/// @return 
int fputc(int ch, FILE* f)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 1000);
    return ch;
}
/////////////////系统锟斤拷锟斤拷锟斤拷锟?////////////////////////////////////////////////////////////////////////



void LogPrint(void)
{
    printf("\r\n\r\n");
	printf("    ___                   _    ___  \r\n");
	printf("   \/   \\  ___   __ _   __| |  \/ _ \\  ___    ___  _ __ ___  \r\n");
	printf("  \/ \/\\ \/ \/ _ \\ \/ _` | \/ _` | \/ \/_)\/ \/ _ \\  \/ _ \\| '_ ` _ \\  \r\n");
	printf(" \/ \/_\/\/ |  __\/| (_| || (_| |\/ ___\/ | (_) ||  __\/| | | | | |		\r\n");
	printf("\/___,'   \\___| \\__,_| \\__,_|\\\/      \\___\/  \\___||_| |_| |_|			\r\n");
    printf("\r\n");
    printf("Copy Right:     DeadPoem(2819731924@qq.com)\r\n");
    // printf("\r\n->");
}


static void CreateTask(void* param)
{
    BaseType_t re;

    //print infor
    LogPrint();

    taskENTER_CRITICAL();
    //涉及到log打印互斥量, 应该尽早初始化
    MyLogInit();
    //测试任务
    re = xTaskCreate(TestTask, "test task", 512, NULL, 1, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "test task create failed");
    }
    //sd card infor ctrl task
    re = xTaskCreate(FileInfoTask, "file task", 512, NULL, 6, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "file task create failed");
    }
    //闪灯任务
    re = xTaskCreate(LedTask, "led task", 64, NULL, 1, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "led task create failed");
    }
    //beep任务
    re = xTaskCreate(BeepTask, "beep task", 128, NULL, 2, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "beep task create failed");
    }
    //按键任务
    re = xTaskCreate(ButtonTask, "button task", 128, NULL, 5, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "button task create failed");
    }
    //命令任务
    re = xTaskCreate(ShellTask, "shell task", 512, NULL, 6, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "shell task create failed");
    }
    //gui task
    #if USE_GUI
    re = xTaskCreate(GUI_Task, "gui task", 1024, NULL, 2, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "gui task create failed");
    }
    #endif

    //motor task
    re = xTaskCreate(MotorTask, "motor task", 256, NULL, 3, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "motor task create failed");
    }
    //motor task
    re = xTaskCreate(SteerTask, "steer task", 256, NULL, 3, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "steer task create failed");
    }
    //jy901 task
    re = xTaskCreate(JY901_Task, "jy901 task", 256, NULL, 4, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "jy901 task create failed");
    }
    //car task
    re = xTaskCreate(CarTask, "car task", 256, NULL, 3, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "car task create failed");
    }
    //route task
    re = xTaskCreate(RouteTask, "route task", 512, NULL, 5, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "route task create failed");
    }
    //arm task
    re = xTaskCreate(ArmTask, "arm task", 256, NULL, 5, NULL);
    if(re != pdPASS){
        MY_LOGE("rtos", "arm task create failed");
    }
    // //remote blue task
    // re = xTaskCreate(BlueRemoteTask, "remote task", 256, NULL, 4, NULL);
    // if(re != pdPASS){
    //     MY_LOGE("rtos", "remote task create failed");
    // }
    // //rgb task
    // re = xTaskCreate(RGB_Task, "car task", 128, NULL, 1, NULL);
    // if(re != pdPASS){
    //     MY_LOGE("rtos", "RGB_Task task create failed");
    // }

    CommunicationInit();
    CodeScanInit();

    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}



void MyRTOS_Init(void)
{
    //printf uart2 
    HAL_UART_Receive_IT(&huart2,&UartReceive2,1);
    // __HAL_UART_ENABLE_IT(&huart2, UART_IT_ERR);
    //jy901 uart3
    HAL_UART_Receive_IT(&huart3,&UartReceive3,1);
    //openmv uart4
    HAL_UART_Receive_IT(&huart4,&UartReceive4,1);
    // //task timer
    // HAL_TIM_Base_Start_IT(&htim14);


    //在任务里进行其它相关初始化
    xTaskCreate(CreateTask, "create task", 128, NULL, 6, NULL);
}   





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // printf("uart test1\n");
	if (huart->Instance == huart1.Instance)
    {

    }
    else if(huart->Instance == huart2.Instance)
    {
        if(!BlueGetData(UartReceive2));
        {
            ShellGetLine(UartReceive2);
        }
		HAL_UART_Receive_IT(&huart2,&UartReceive2,1);
    }
    else if(huart->Instance == huart3.Instance)
    {
        WitSerialDataIn(UartReceive3);
		HAL_UART_Receive_IT(&huart3,&UartReceive3,1);
    }
    else if(huart->Instance == huart4.Instance)
    {
        CommunicationReceive(UartReceive4);  //openmv
        ReceiveCodeInfo(UartReceive4);  //二维码
		HAL_UART_Receive_IT(&huart4,&UartReceive4,1);
    }
}


