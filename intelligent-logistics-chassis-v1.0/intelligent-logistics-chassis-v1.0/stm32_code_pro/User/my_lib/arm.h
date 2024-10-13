#ifndef __ARM_H__
#define __ARM_H__

#include "math.h"
#include "FreeRTOS.h"
#include "task.h"
#include "my_lib/steer.h"
#include "my_lib/step_motor.h"
#include "my_lib/my_log.h"

#include "m_shell.h"


//单位使用cm
//单位使用cm
//单位使用cm



#define PI  3.1415926f
#define PI2AG(x)     (x/PI*180)  //弧度转为角度

#define MAIN_ARM_LEN      14
#define SECOND_ARM_LEN    14
#define BASE_HIGH         15
#define ADD_DISTENCE      7

#define X_MAX   50
#define Y_MAX   50
#define Z_MAX   30

#define J2_ZERO_ANGLE     (MAIN_STEER_INIT_ANGLE+90)
#define J3_ZERO_ANGLE     (SECOND_STEER_INIT_ANGLE)  //J3实际角度变化与旋转方向相反

typedef enum
{
    TONG_OPEN = 0,
    TONG_CLOSE,
}tong_state_t;

typedef enum
{
    CAM_NORMAL = 0,
    CAM_SIDE,
}cam_state_t;

typedef struct 
{
    float L0;  //定点高度(相对基准水平面)
    float L1;  //大臂长度
    float L2;  //小臂长度
    float L3;  //尾点补偿距离

    float J1;  //水平面旋转角度
    float J2;  //大臂转角
    float J3;  //小臂转角
    tong_state_t tong_state;
    // cam_state_t cam_state;

    float x, y, z; //三维坐标
}arm_t;

void ArmTask(void* param);

void SetArmState(float x, float y, float z, tong_state_t tong_sta);

uint8_t IsArmBusy(void);

#endif


