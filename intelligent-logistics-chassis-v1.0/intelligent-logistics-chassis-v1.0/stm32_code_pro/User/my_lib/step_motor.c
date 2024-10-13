#include "my_lib/step_motor.h"

//步进电机控制结构体
step_motor_t step = {0};

//temp
static TRAPE_ID_t TrapeAccelId;

/// @brief 步进电机使能/失能
/// @param en 
void StepEnable(uint8_t en)
{
    HAL_GPIO_WritePin(StepEn_GPIO_Port, StepEn_Pin, (GPIO_PinState)(!en));  //控制使能
}

/// @brief 机械臂绝对角度控制
/// @param angle 
void StepMotorAngle(float angle)
{
    HAL_GPIO_WritePin(StepEn_GPIO_Port, StepEn_Pin, GPIO_PIN_RESET);  //控制使能

    step.tar_angle = angle;
    step.change_angle = (step.tar_angle - step.pre_angle);

    if(fabs(step.change_angle) < PULSE_ANGLE){
        return;
    }
    
    step.ctrl_flag = 1;              //使能步进电机控制

    uint32_t counter;

    counter = TrapeAccel_Start(step.trapeID, (uint32_t)(fabs(step.change_angle)/PULSE_ANGLE));

    //设置方向
    if((step.change_angle) > 0){
        step.dir = 0;
    }
    else{
        step.dir = 1;
    }
    HAL_GPIO_WritePin(MotorDir_GPIO_Port, MotorDir_Pin, step.dir);
    

    __HAL_TIM_SET_AUTORELOAD(&STEP_MOTOR_TIM, (uint16_t)counter);
    HAL_TIM_Base_Start_IT(&STEP_MOTOR_TIM);
}

/// @brief 定时器中记录角度变化,控制定时器关断
/// @param  
void StepMotorTIM_Ctrl(void)
{
    uint32_t counerVal;
    trape_sta_enum_t stat;

    static uint8_t count;
    if(step.ctrl_flag)
    {
        step.pre_angle += ((step.dir? (-PULSE_ANGLE):(PULSE_ANGLE)))/2;  //除以2, 两次翻转为一个脉冲

        HAL_GPIO_TogglePin(MotorStep_GPIO_Port, MotorStep_Pin);

        if(++count > 1){
            count = 0;
        }
        else{
            return; //两次中断为一次脉冲
        }
       
        stat = TrapeAccel_CalNextCounterVal(TrapeAccelId, &counerVal);
       
        if(stat == STOP)
        {
            step.ctrl_flag = 0;
           HAL_TIM_Base_Stop_IT(&STEP_MOTOR_TIM);
        }
        else
        {
            __HAL_TIM_SET_AUTORELOAD(&STEP_MOTOR_TIM, (uint16_t)counerVal);
        }
    }
}

/// @brief 判断步进电机是否正在控制中
/// @param  
/// @return 
uint8_t IsStepBusy(void)
{
    return step.ctrl_flag;
}

//cmd test

// static void SetStepAngle(double param)
// {
//     MY_LOGI("step", "set angle %.1f", param);
//     StepMotorAngle(param);
// }

static void SetStepAngle(int argc, char** argv)
{
    if(argc == 3){
        if(strcmp(argv[1], "-e") == 0){  //步进电机使能/失能
            uint8_t en = atoi(argv[2]);
            StepEnable(en);
            MY_LOGI("step", "en sta:%d", en);
            return;
        }
    }


    float angle = atof(argv[1]);
    MY_LOGI("step", "set angle %.1f", angle);
    StepMotorAngle(angle);
}

// static void IsStepEnable(int argc, char** argv)
// {
//     uint8_t en = atoi(argv[1]);
//     MY_LOGI("step", "enable sta[%d]", en);
//     // StepMotorAngle(angle);
//     // HAL_GPIO_WritePin(StepEn_GPIO_Port, StepEn_Pin, en); 
//     StepEnable(en);
// }

/// @brief 控制机械臂回到原点
/// @param  
void StepMotorInit(void)
{

    HAL_GPIO_WritePin(StepEn_GPIO_Port, StepEn_Pin, GPIO_PIN_SET);  //控制失能

    ShellCmdRegister("step",
                "set step angle, param:[angle]",
                SetStepAngle);


    trape_accel_param_t AccelParam =   //配置步进电机梯形加减速参数
    {
        .stepSize = PULSE_ANGLE,
        .ClkFrq = 500000,   //两次中断翻转io产生一次脉冲, 所以中断频率/2
        .accelSpeed = 200,
        .decelSpeed = 200,
        .constSpeed = 200,
        .totalStep = 0,
    };    
    step.trapeID = TrapeAccel_Init(&AccelParam);
}





