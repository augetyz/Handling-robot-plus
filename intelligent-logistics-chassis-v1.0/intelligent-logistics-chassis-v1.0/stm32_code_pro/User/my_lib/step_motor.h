#ifndef __STEP_MOTOR_H__
#define __STEP_MOTOR_H__

#include "math.h"

#include "main.h"
#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"

#include "my_sys.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"
#include "my_lib/my_log.h"

#include "my_lib/step_trape.h"
#include "my_lib/led.h"
#include "vofa_communication.h"

#define STEP_MOTOR_TIM        htim10

#define STEP_ANGLE 1.8f
#define DRIVE_RATIO  (5.18*16)   //步进电机驱动4分频
#define PULSE_ANGLE  (STEP_ANGLE/DRIVE_RATIO)

// #define ANGLE_BIAS_ALLOW    (PULSE_ANGLE/2)  //运行误差,一单位步进角度

typedef enum
{
    BACKWARD = 0,
    FORWARD,
}step_motor_dir_enum_t;

typedef struct 
{
    float tar_angle;
    float pre_angle;
    float change_angle;
    step_motor_dir_enum_t dir;
    uint8_t ctrl_flag;

    TRAPE_ID_t trapeID;
}step_motor_t;



// void StepMotorTask(void* param);
void StepMotorInit(void);
void StepMotorAngle(float angle);
void StepMotorTIM_Ctrl(void);
uint8_t IsStepBusy(void);

void StepEnable(uint8_t en);


#endif


