#include "my_lib/remote_ctrl.h"


static SemaphoreHandle_t blue_receive_sem = NULL;

BluePack_t bluePack = {0};
// RemoteCtrl_t remoteCtrl = {0};
static uint8_t receiveLen = 0, startFlag =0;
static uint8_t receiveBuffer[TOTAL_LENTH];  //接收缓存
static  uint8_t tempBuffer[TOTAL_LENTH];   //临时处理缓存

static float motorSpeed[4] ={0};

static void CarRemote(void)
{
    if(bluePack.mInt8[0])  //是否使能遥控模式
    {
        //遥控模式下必须关闭距离闭环,角度闭环以及速度闭环控制
        IsEnableDistenceCtrl(0);
        IsEnableAngleCtrl(0);
        MotorCtrlEnable(0);

        //电机速度控制
        motorSpeed[0] = bluePack.mInt8[1]+bluePack.mInt8[2]+bluePack.mInt8[3];
        motorSpeed[1] = bluePack.mInt8[1]-bluePack.mInt8[2]+bluePack.mInt8[3];
        motorSpeed[2] = bluePack.mInt8[1]+bluePack.mInt8[2]-bluePack.mInt8[3];
        motorSpeed[3] = bluePack.mInt8[1]-bluePack.mInt8[2]-bluePack.mInt8[3];
        MotorSetPwm(1, motorSpeed[0]*REMOTE_SPEED_RATIO);
        MotorSetPwm(2, motorSpeed[1]*REMOTE_SPEED_RATIO);
        MotorSetPwm(3, motorSpeed[2]*REMOTE_SPEED_RATIO);
        MotorSetPwm(4, motorSpeed[3]*REMOTE_SPEED_RATIO);
        //抓手
        // if(bluePack.mInt8[4]){
        //     SetSteerAngle(TongSteer, TONG_STEER_MAX_ANGLE);
        // }
        // else{
        //     SetSteerAngle(TongSteer, TONG_STEER_MIN_ANGLE);
        // }
        // //大小臂舵机
        // SetSteerAngle(MainSteer, MAIN_STEER_MIN_ANGLE+bluePack.mInt8[5]*REMOTE_MAIN_STEER_ANGLE_RATIO);
        // SetSteerAngle(SecSteer, SECOND_STEER_MIN_ANGLE+bluePack.mInt8[6]*REMOTE_SEC_STEER_ANGLE_RATIO);
        // CtrlSteer();
        // //步进电机角度
        // StepMotorAngle(bluePack.mInt16[0]);

        SetArmState(bluePack.mInt16[0]/10.0f, bluePack.mInt16[1]/10.0f, bluePack.mInt16[2]/10.0f, bluePack.mInt8[4]);
    }
    else{
        IsEnableDistenceCtrl(1);
        IsEnableAngleCtrl(1);
        MotorCtrlEnable(1);
    }
}



uint8_t BlueGetData(uint8_t info)
{
    uint8_t re=1;
    if(info == PACK_HEAD){
        receiveLen = 0;
        startFlag = 1;
    }
    else if(info == PACK_TAIL){
        startFlag =0;
        // receiveFlag = 1;
        xSemaphoreGiveFromISR(blue_receive_sem, NULL);
    }
    else if(startFlag)
    {
        if(receiveLen >= TOTAL_LENTH){  //保护措施, 防止越界
            return 0;
        }
        receiveBuffer[receiveLen++] = info;
    }
    else
    {
        re =0; //无效数据
    }
    return re;
}

uint8_t BlueCheck(void)
{
    uint32_t checkCode=0;
    for(uint16_t i=0; i<(TOTAL_LENTH-1); i++)
    {
        checkCode += tempBuffer[i];
    }

    if(tempBuffer[TOTAL_LENTH-1] == (uint8_t)(checkCode&0xff)){
        return 1;
    }
    else{
        return 0;
    }
}

void BlueRemoteTask(void* param)
{
    blue_receive_sem = xSemaphoreCreateBinary();  //信号量创建

    while(1)
    {
        xSemaphoreTake(blue_receive_sem, portMAX_DELAY);

        memcpy(tempBuffer, receiveBuffer, TOTAL_LENTH); //转移接收信息
        if(BlueCheck())  //校验
        {
            //解析数据
            // uint8_t* pBool = (uint8_t*)tempBuffer;
            // for(uint8_t i=0; i<BOOL_NUM; i++)
            // {
            //     bluePack.mBool[i] = *pBool;
            //     pBool++;
            // }

            int8_t* pByte = (int8_t*)tempBuffer;
            for(uint8_t i=0; i<BYTE_NUM; i++)
            {
                bluePack.mInt8[i] = *pByte;
                pByte++;
            }
            int16_t* pShort = (int16_t*)pByte;
            for(uint8_t i=0; i<SHORT_NUM; i++)
            {
                bluePack.mInt16[i] = *pShort;
                pShort++;
            }

            // int32_t* pInt = (int32_t*)pShort;
            // for(uint8_t i=0; i<INT_NUM; i++)
            // {
            //     bluePack.mInt32[i] = *pInt;
            //     pInt++;
            // }

            // float* pFloat = (float*)pInt;
            // for(uint8_t i=0; i<FLOAT_NUM; i++)
            // {
            //     bluePack.mFloat[i] = *pFloat;
            //     pFloat++;
            // }


            CarRemote(); //执行
        }

        // MY_LOGI("blue", "en:%d fSp:%d sSp:%d tSp:%d tong:%d big:%d sml:%d stp:%d", bluePack.mInt8[0],bluePack.mInt8[1],bluePack.mInt8[2],bluePack.mInt8[3],bluePack.mInt8[4],bluePack.mInt8[5],bluePack.mInt8[6],bluePack.mInt8[7],bluePack.mInt16[0]);
        vTaskDelay(20);
    }    
}



