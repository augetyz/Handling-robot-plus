#ifndef __STEER_H__
#define __STEER_H__

#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"

#include "my_lib/my_log.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"
#include "math.h"

// #include "step_trape.h"

#define STEER_NUM  4  //舵机数量

//机械臂杆舵机为270度,抓手舵机为180度, 即抓手舵机0-270度控制实际为0-180

#define MAIN_STEER_TIM      htim12
#define MAIN_STEER_CHANNEL  TIM_CHANNEL_2
#define SECOND_STEER_TIM      htim12
#define SECOND_STEER_CHANNEL  TIM_CHANNEL_1
#define TONG_STEER_TIM      htim14          
#define TONG_STEER_CHANNEL  TIM_CHANNEL_1
#define TURN_STEER_TIM      htim9          
#define TURN_STEER_CHANNEL  TIM_CHANNEL_1

#define STEER_ANGLE         270    //舵机(大小臂)类型
#define NEGATIVE_ANGLE135  (0.5*1000)
#define POSITIVE_ANGLE135  (2.5*1000)

//角度步进变化
// #define STEP_ADD_ANGLE  1
// #define ANGLE_ADD_TIME  15    //角度步进变化间隔时间
//舵机属性
#define MAIN_STEER_INIT_ANGLE  182
#define MAIN_STEER_MIN_ANGLE   120
#define MAIN_STEER_MAX_ANGLE   230
#define MAIN_STEER_STEP_ANGLE  0.7
#define MAIN_STEER_STEP_TICK   10

#define SECOND_STEER_INIT_ANGLE  85
#define SECOND_STEER_MIN_ANGLE   55
#define SECOND_STEER_MAX_ANGLE   200
#define SECOND_STEER_STEP_ANGLE  0.7
#define SECOND_STEER_STEP_TICK   10

#define TONG_STEER_INIT_ANGLE  (150)  //抓手为180舵机
#define TONG_STEER_MIN_ANGLE   (120)
#define TONG_STEER_MAX_ANGLE   (157)
#define TONG_STEER_STEP_ANGLE  1.3
#define TONG_STEER_STEP_TICK   10


#define TURN_STEER_INIT_ANGLE  (112)  
#define TURN_STEER_MIN_ANGLE   (0)
#define TURN_STEER_MAX_ANGLE   (270)
#define TURN_STEER_STEP_ANGLE  1.3
#define TURN_STEER_STEP_TICK   10

#define TURN_STEER_RED       135
#define TURN_STEER_GREEN     (TURN_STEER_RED+120)
#define TURN_STEER_BLUE     (TURN_STEER_RED-120)

typedef enum
{
    MainSteer = 0,
    SecSteer,
    TongSteer,
    TurnSteer,
}steer_id_enum_t;

typedef struct 
{
    float tar_angle;
    float pre_angle;
    // float init_angle;
    float min_angle;
    float max_angle;

    float step_angle;  //角度步进变化值
    uint32_t step_tick;  //角度步进变化时间
    uint8_t state;  //舵机是否需要工作标志位
    uint32_t last_tick;

    // TRAPE_ID_t trapeID;
}steer_t;



void SteerTask(void* param);
// void SteerInit(void);
void SetSteerAngle(steer_id_enum_t steer_id, double angle);
uint8_t IsSteerBusy(steer_id_enum_t id);
void SteerEnable(uint8_t en);

#endif

