#include "motor.h"
#include "vofa_communication.h"

#include "math.h"

#include "main.h"
#include "tim.h"


//temp test
static uint8_t Motor_VofaTranFlag =0;  //使能/失能 vofa传输
static uint8_t MotorCtrlFlag = 1;
Motor_Type Motors[4] = {0};  //装载四个电机信息

/// @brief 设置电机速度pid参数
/// @param PID_P 
/// @param PID_I 
/// @param PID_D 
/// @param Limit 
void SetSpeedPID(float PID_P, float PID_I, float PID_D, float Limit)
{
    for(uint8_t i=0; i<4; i++)
    {
        Motors[i].PID.Kp = PID_P;
        Motors[i].PID.Ki = PID_I;
        Motors[i].PID.Kd = PID_D;
        Motors[i].PID.OutLimit = Limit;
    }
}


static void MotorParamInit(void)
{
    if(!IsFileReady()){
        vTaskDelay(500);
        if(!IsFileReady()){
            SetSpeedPID(SpeedKp, SpeedKi, SpeedKd, SpeedOutMax);  //初始化写入默认参数
            MY_LOGI("motor", "load default param");
            return;  
        }
    }
    SetSpeedPID(car_param.motor_pid.kp, car_param.motor_pid.ki, car_param.motor_pid.kd, car_param.motor_pid.out_max);
    MY_LOGI("motor", "load param from file");
}
static void MotorInit(void)
{
    MotorParamInit(); //参数初始化   

    Motors[0].pEncoderTim = &htim2;
    Motors[0].pPwmTim = &htim8;
    Motors[0].TIM_Channel = TIM_CHANNEL_1;
    Motors[0].MotorId = 1;

    Motors[1].pEncoderTim = &htim3;
    Motors[1].pPwmTim = &htim8;
    Motors[1].TIM_Channel = TIM_CHANNEL_2;
    Motors[1].MotorId = 2;

    Motors[2].pEncoderTim = &htim4;
    Motors[2].pPwmTim = &htim1;
    Motors[2].TIM_Channel = TIM_CHANNEL_1;
    Motors[2].MotorId = 3;

    Motors[3].pEncoderTim = &htim5;
    Motors[3].pPwmTim = &htim8;
    Motors[3].TIM_Channel = TIM_CHANNEL_4;
    Motors[3].MotorId = 4;
    
    for(uint8_t i=0; i<4; i++)
    {
        __HAL_TIM_SET_COMPARE(Motors[i].pPwmTim, Motors[i].TIM_Channel, 0);
    }
    
    for(uint8_t i=0; i<4; i++)
    {
        HAL_TIM_PWM_Start(Motors[i].pPwmTim, Motors[i].TIM_Channel);
    }

    for(uint8_t i=0; i<4; i++)
    {
        HAL_TIM_Encoder_Start(Motors[i].pEncoderTim, TIM_CHANNEL_ALL);
    }
}

/// @brief 计算每个电机的速度,里程信息
/// @param PassTick_ms 处理时间间隔
static void MotorReadOne(Motor_Type* Motor, uint16_t PassTick_ms)
{
    int16_t  Change;
    uint32_t Temp = (uint32_t)(__HAL_TIM_GET_COUNTER(Motor->pEncoderTim));//获取定时器的值

    if((Temp - Motor->LastTimCount) > EncoderCountMax/2) //反向突变
    {
        Change = -(Motor->LastTimCount + (EncoderCountMax - Temp));
    }else if((int32_t)(Temp - Motor->LastTimCount) < (int32_t)(-(EncoderCountMax/2)))
    {
        Change = (EncoderCountMax - Motor->LastTimCount) + Temp;
    }else
    {
        Change = Temp - Motor->LastTimCount;
    }

    if(Motor->MotorId==1 || Motor->MotorId==2) //安装反向
    {
        Motor->LastTimCount = Temp;
        Motor->Speed = -(float)Change*PulseToMeter*1000/PassTick_ms;  //meter/s
        Motor->Mileage -= Change * PulseToMeter;
    }
    else
    {
        Motor->LastTimCount = Temp;
        Motor->Speed = (float)Change*PulseToMeter*1000/PassTick_ms;  //meter/s
        Motor->Mileage += Change * PulseToMeter;
    }
}

/// @brief 设值电机pwm输出
/// @param MotorNum 电机编号1~4
/// @param PwmDuty 占空比-100~100
void MotorSetPwm(uint8_t MotorNum, int8_t PwmDuty)
{
    float fDuty = PwmDuty/100.0;
    if(fDuty > 1){
        fDuty = 1;
    }
    else if(fDuty < -1){
        fDuty = -1;
    }

    switch (MotorNum)
    {
    case 1:  //安装反向
        if(fDuty < 0){
            HAL_GPIO_WritePin(CH1_B_GPIO_Port, CH1_B_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(CH1_A_GPIO_Port, CH1_A_Pin, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)((-fDuty)*TIM_ARR));
        }else{
            HAL_GPIO_WritePin(CH1_B_GPIO_Port, CH1_B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CH1_A_GPIO_Port, CH1_A_Pin, GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)(fDuty*TIM_ARR));  
        }
        break;
    case 2: //安装反向
        if(fDuty < 0){
            HAL_GPIO_WritePin(CH2_B_GPIO_Port, CH2_B_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(CH2_A_GPIO_Port, CH2_A_Pin, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)((-fDuty)*TIM_ARR));
        }else{
            HAL_GPIO_WritePin(CH2_B_GPIO_Port, CH2_B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CH2_A_GPIO_Port, CH2_A_Pin, GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)(fDuty*TIM_ARR));   
        }
        break;
    case 3:
        if(fDuty < 0){
            HAL_GPIO_WritePin(CH3_A_GPIO_Port, CH3_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(CH3_B_GPIO_Port, CH3_B_Pin, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)((-fDuty)*TIM_ARR));
        }else{
            HAL_GPIO_WritePin(CH3_A_GPIO_Port, CH3_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CH3_B_GPIO_Port, CH3_B_Pin, GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)(fDuty*TIM_ARR));   
        }
        break;
    case 4:
        if(fDuty < 0){
            HAL_GPIO_WritePin(CH4_A_GPIO_Port, CH4_A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(CH4_B_GPIO_Port, CH4_B_Pin, GPIO_PIN_RESET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)((-fDuty)*TIM_ARR));
        }else{
            HAL_GPIO_WritePin(CH4_A_GPIO_Port, CH4_A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(CH4_B_GPIO_Port, CH4_B_Pin, GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(Motors[MotorNum-1].pPwmTim, Motors[MotorNum-1].TIM_Channel, (uint16_t)(fDuty*TIM_ARR));   
        }
        break;

    default:
        break;
    }
}


/// @brief 增量式PID速度控制
/// @param pMotor 
static void SpeedControlOne(Motor_Type* pMotor, uint16_t PassTick_ms)
{
    if(pMotor->PID.PIDCtrl_Flag)
    {
        pMotor->PID.Present = pMotor->Speed;
        pMotor->PID.Bias = pMotor->PID.Target - pMotor->PID.Present;
        //除以采样时间, 减小采样时间变化的影响
        pMotor->PID.Output += (pMotor->PID.Kp*(pMotor->PID.Bias-pMotor->PID.LastBias) + pMotor->PID.Ki*pMotor->PID.Bias)/PassTick_ms;
        pMotor->PID.LastBias = pMotor->PID.Bias;
        //限幅
        if(pMotor->PID.Output < -pMotor->PID.OutLimit){
            pMotor->PID.Output = -pMotor->PID.OutLimit;
        }else if(pMotor->PID.Output > pMotor->PID.OutLimit){
            pMotor->PID.Output = pMotor->PID.OutLimit;
        }

        MotorSetPwm(pMotor->MotorId, (int8_t)pMotor->PID.Output);
    }else
    {
        MotorSetPwm(pMotor->MotorId, 0);
    }
}

void MotorProcess(uint16_t PassTick_ms)
{
    for(uint8_t i=0; i<4; i++)
    {
        MotorReadOne(&Motors[i], PassTick_ms);
        SpeedControlOne(&Motors[i], PassTick_ms);
    }
}

/// @brief 设置电机参数
/// @param MotorId 电机编号1-4
/// @param SpeedTar 目标速度值 米/秒
/// @param IsEnable 电机使能 0/1
void SetMotorParam(uint8_t MotorId, float SpeedTar, uint8_t IsEnable)
{
    USER_ASSERT(MotorId <=4 );
    
    Motors[MotorId-1].PID.Target = SpeedTar;
    Motors[MotorId-1].PID.PIDCtrl_Flag = IsEnable;
}

/// @brief 获取电机速度 米/秒
/// @param MotorId 电机编号1-4
/// @return 
float GetMotorSpeed(uint8_t MotorId)
{
    return Motors[MotorId-1].Speed;
}

/// @brief 获取电机转动里程 米
/// @param MotorId 电机编号1-4
/// @return 
float GetMotorMileage(uint8_t MotorId)
{
    return Motors[MotorId-1].Mileage;
}

/// @brief 清零电机转动里程
/// @param MotorId 电机编号1-4
void ClearMotorMileage(uint8_t MotorId)
{
    Motors[MotorId-1].Mileage = 0;
}

/// @brief 电机速度闭环使能
/// @param enable 
void MotorCtrlEnable(uint8_t enable)
{
    MotorCtrlFlag = enable;
}


/////////////////测试功能函数///////////////////////////////////////////////

// static void SpeedTest(double Speed)
// {
//     printf("new speed->%f\r\n", Speed);
//     SetMotorParam(1, Speed, 1);
//     SetMotorParam(2, Speed, 1);
//     SetMotorParam(3, Speed, 1);
//     SetMotorParam(4, Speed, 1);
// }

static void SpeedTest(int argc, char** argv)
{
    if(argc == 3){
        uint8_t id = atoi(argv[1]);
        float speed = atof(argv[2]);
        MY_LOGI("motor", "set motor[%d] speed[%.1f]", id, speed);
        SetMotorParam(id, speed, 1);
    }
    else if(argc == 2)
    {
        float speed = atof(argv[1]);
        MY_LOGI("motor", "set all motor speed[%.1f]", speed);
        SetMotorParam(1, speed, 1);
        SetMotorParam(2, speed, 1);
        SetMotorParam(3, speed, 1);
        SetMotorParam(4, speed, 1);
    }
    else
    {
        MY_LOGW("motor", "cmd param num invalid");
    }
}

// static void IsEnableMotorTrans(double param)
// {
//     Motor_VofaTranFlag = (uint8_t)param;
//     MY_LOGI("motor", "motor transfer state:%d", Motor_VofaTranFlag);
// }

static void IsEnableMotorTrans(int argc, char** argv)
{
    uint8_t en = atoi(argv[1]);
    Motor_VofaTranFlag = en;
    MY_LOGI("motor", "motor transfer state:%d", en);
}


static void MotorTestInit(void)
{
    ShellCmdRegister("mt_spd",
                "set motor speed, param:[id(1-4)] [speed] or [speed]",
                 SpeedTest);
    ShellCmdRegister("mt_tsf",
                "enable motor trans, param[en]",
                 IsEnableMotorTrans);
}
//////////////////////////////////////////////////////////////////////////////

void MotorTask(void* param)
{
    MotorInit();
    //temp test
    MotorTestInit();
    static uint32_t TimeTick;
    while (1)
    {
        uint16_t TimePass = (GetSysTick()-TimeTick);

        TimeTick = GetSysTick();

        if(MotorCtrlFlag){
            MotorProcess(TimePass); //pid处理
        }

        //temp test
        if(Motor_VofaTranFlag)
        {
            float Info[4];
            Info[0] = Motors[0].PID.Present;
            Info[1] = Motors[0].PID.Target;
            Info[2] = Motors[0].PID.Bias;
            Info[3] = Motors[0].PID.Output;
            VofaFloatSend(4,Info);
        }
        vTaskDelay(10);
    }
}




