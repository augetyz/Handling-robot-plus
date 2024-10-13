#ifndef _MOTOR_H
#define _MOTOR_H

#include "my_sys.h"
#include "main.h"
#include "tim.h"

#include "my_lib/file_info.h"
#include "m_shell.h"

// #include "my_lib/file_info.h"

//pwm define
#define TIM_ARR 100
//encoder define
#define EncoderCountMax  65535UL
#define PulseToMeter    0.001F  //脉冲数转换成米数对应转换系数
//default Speed PID define
#define SpeedKp    200.0f
#define SpeedKi    300.0f
#define SpeedKd    0.0f
#define SpeedOutMax  100  //满占空比为100

//PID控制结构体
typedef struct 
{
    uint8_t PIDCtrl_Flag;
    float Kp;
    float Ki;
    float Kd;
    float Bias;
    float LastBias;
    float BiasSum;
    float Target;
    float Present;
    float Output;
    float OutLimit;

    float BiasAllow;  //误差允许(绝对值),避免静态振荡
}PID_Type;



typedef struct 
{
    TIM_HandleTypeDef* pEncoderTim;  //编码器tim句柄
    TIM_HandleTypeDef* pPwmTim;  //编码器tim句柄
    uint32_t TIM_Channel;   //pwm定时器通道
    uint8_t MotorId;

    uint32_t LastTimCount;

    float Mileage;  //meter
    float Speed;    //meter/s

    PID_Type PID;   //速度PID控制
}Motor_Type;


//temp test
extern Motor_Type Motors[4];


void MotorTask(void* param);

void SetSpeedPID(float PID_P, float PID_I, float PID_D, float Limit);
void SetMotorParam(uint8_t MotorId, float SpeedTar, uint8_t IsEnable);
float GetMotorSpeed(uint8_t MotorId);
float GetMotorMileage(uint8_t MotorId);
void ClearMotorMileage(uint8_t MotorId);

void MotorSetPwm(uint8_t MotorNum, int8_t PwmDuty);
void MotorCtrlEnable(uint8_t enable);


#endif


