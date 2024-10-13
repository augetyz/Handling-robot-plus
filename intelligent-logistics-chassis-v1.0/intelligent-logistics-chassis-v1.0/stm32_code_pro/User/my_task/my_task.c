#include "my_task/my_task.h"

#include "main.h"
#include "stdlib.h"

//temp test
#include "usart.h"
#include "adc.h"

#include "lvgl.h"
#include "ui.h"

// #include "lcd.h"
#include "button.h"
#include "communication.h"

#include "my_gui.h"
#include "lcd.h"
#include "beep.h"
#include "rgb.h"

#include "route.h"
#include "steer.h"
#include "file_info.h"

// #include "display_3D.h"



//temp test
uint8_t pcWriteBuffer[500];
/// @brief 串口打印所有任务信息
/// @param param 
void ShowTaskInfo(int argc, char** argv)
{
    vTaskList((char*)&pcWriteBuffer);
    printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
    printf("%s\r\n", pcWriteBuffer);  
}


/// @brief 软件复位
/// @param param 
void SoftResetSystem(int argc, char** argv)
{
    HAL_NVIC_SystemReset();
}


float CheckBatteryVoltage(void)
{
    uint32_t sum = 0;
    for(uint16_t i=0; i<16; i++)
    {
        HAL_ADC_Start(&hadc1);
        vTaskDelay(3);
        sum += HAL_ADC_GetValue(&hadc1);
    }
    float vol = (sum>>4)/4096.0f*3.3f*6;
    return vol;
}

/// @brief 检测电池电压
/// @param argc 
/// @param argv 
static void GetBattryVoltage(int argc, char** argv)
{
    float vol = CheckBatteryVoltage();
    MY_LOGI("battery", "voltage:%.1f", vol);
}


/// @brief openmv屏幕开关控制
/// @param argc 
/// @param argv 
void OvDisplayCtrl(int argc, char** argv)
{
    uint8_t en = atoi(argv[1]);
    
    CommunicationTrans(UI_BTN_SWITCH_EVENT, &en, 1);
    MY_LOGI("ov", "ov display sta:%d", en);
}



/// @brief 测试任务
/// @param param 
void TestTask(void* param)
{
    //注册任务信息打印命令
    ShellCmdRegister("task",
                "show all task infor",
                ShowTaskInfo);

    //软件复位
    ShellCmdRegister("reboot",
                "reset system",
                SoftResetSystem);


    //电池电压
    ShellCmdRegister("vbat",
                "measure battery voltage",
                GetBattryVoltage);


    //openmv屏幕显示使能
    ShellCmdRegister("ov_disp",
                "ov display on/off, param:[0/1]",
                OvDisplayCtrl);


    while(1)
    {
        if(AskButtonEvent(0, 0) == BTN_EVENT_SHORT){
            AskButtonEvent(0,1); //清除标志位
            MY_LOG_Print("\r\nB0 short");

            static uint8_t uiFlag=1;
            uiFlag = !uiFlag;
            if(uiFlag)
            {
                IsEnableLvgl(0);        //退出旋转立方体显示,解锁ui控制界面
            }
            else
            {
                IsEnableLvgl(1);
                lv_disp_load_scr( ui_ScreenMenu);
            }

        }
        else if(AskButtonEvent(0,1) == BTN_EVENT_LONG){
            MY_LOG_Print("\r\nB0 long");
        }

        if(AskButtonEvent(1, 0) == BTN_EVENT_SHORT){
             AskButtonEvent(1,1); //清除标志位
            MY_LOG_Print("\r\nB1"); 

            SetSteerAngle(TurnSteer, TURN_STEER_INIT_ANGLE);  //B1短按, 调整车辆至待发车状态
	        ExcuteArmActionArr(ARM_START_POS);
        }
        else if(AskButtonEvent(1,1) == BTN_EVENT_LONG){
            MY_LOG_Print("\r\nB1 long");
            com_infor.id = OV_STOP;      //模拟发送openmv通信终止信息
            xSemaphoreGive(com_receive_sem);
        }


        if(AskButtonEvent(2, 0) == BTN_EVENT_SHORT){
            AskButtonEvent(2,1); //清除标志位
            MY_LOG_Print("\r\nB2 short");

            SetBeepFun(0, BeepOnMement, 800);
            TestEnable(1);
        }
        else if(AskButtonEvent(2, 1) == BTN_EVENT_LONG){
            MY_LOG_Print("\r\nB2 Long");
            MY_LOGI("mode", "specil route");
            RouteExtract_Specil();

        SetBeepFun(0, BeepOnMement, 100);
        vTaskDelay(400);
        SetBeepFun(0, BeepOnMement, 100);
        }

        // static uint8_t bat_cnt =0;
        // if(CheckBatteryVoltage() < 10.5)   //电池电压检测
        // {
        //     if(!bat_cnt){
        //         bat_cnt++;
        //         MY_LOGW("bat", "voltage low");
        //         SetBeepFun(0, BeepOnOff, 100);
        //     }
        // }
        // else if(bat_cnt)    //防止错误检测
        // {
        //     bat_cnt = 0;
        //     SetBeepFun(0, BeepOff, 0);
        // }

        vTaskDelay(10);
    }
}





