#include "jy901.h"


#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80
static volatile char s_cDataUpdate = 0, s_cCmd = 0xff;

//获取数据
static float fAcc[3], fGyro[3], fAngle[3];

static uint8_t gIMU_StartFlag =0;  //IMU开始上报数据标志位

// static SemaphoreHandle_t jy901_sem = NULL;  //jy901 互斥锁

/// @brief 判断是否已经开始IMU解析
/// @param  
/// @return 
uint8_t IsIMU_Start(void)
{
    return gIMU_StartFlag;
}

void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    HAL_UART_Transmit(&huart3, p_data, uiSize, 1000);
}
void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    // s_cDataUpdate =0;
    for(i = 0; i < uiRegNum; i++)
    {
        switch(uiReg)
        {
            case AZ:
				s_cDataUpdate |= ACC_UPDATE;
                break;

            case GZ:
				s_cDataUpdate |= GYRO_UPDATE;
                break;

            case HZ:
				s_cDataUpdate |= MAG_UPDATE;
                break;

            case Yaw:
				s_cDataUpdate |= ANGLE_UPDATE;
                break;
            default:
				s_cDataUpdate |= READ_UPDATE;
			break;
        }
		uiReg++;
    }
}


static void JY901_Init(void)
{
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitSerialWriteRegister(SensorUartSend);
    WitRegisterCallBack(SensorDataUpdata);
    WitDelayMsRegister(HAL_Delay);
}


//获取yaw角度
float GetYaw(void)
{
    return fAngle[2];
}

/////////////////////////////////////////for test///////////////////////////
static void JY901_Test(int argc, char** argv)
{
    MY_LOGI("jy901", "yaw->%.1f", GetYaw());
}
static void JY901_TestInit(void)
{
    ShellCmdRegister("yaw",
                "print yaw angle",
                JY901_Test);
}
/////////////////////////////////////////for test///////////////////////////


void JY901_Task(void* param)
{
    JY901_Init();
    //temp test
    JY901_TestInit();

    while (1)
    {
        if(s_cDataUpdate & ANGLE_UPDATE)  //角度更新
        {
            gIMU_StartFlag = 1;
            for(uint8_t i = 0; i < 3; i++)
            {
                fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
                fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
                fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
            }
        }
        vTaskDelay(10);
    }
}


