#ifndef __REMOTE_CTRL_H__
#define __REMOTE_CTRL_H__

#include "main.h"
#include "my_sys.h"
#include "FreeRTOS.h"
#include "task.h"

#include "my_lib/my_log.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"

// #include "steer.h"
// #include "step_motor.h"
#include "car_ctrl.h"
#include "arm.h"

//手机蓝牙遥控

#define PACK_HEAD  0xA5
#define PACK_TAIL  0x5A

#define BOOL_NUM    0
#define BYTE_NUM    5
#define SHORT_NUM   3
#define INT_NUM     0
#define FLOAT_NUM   0
#define TOTAL_LENTH  (BOOL_NUM + BYTE_NUM + SHORT_NUM*2 + INT_NUM*4 + FLOAT_NUM*4 + 1)  //还有一位校验码


#define REMOTE_SPEED_DUTY_MAX    50
#define REMOTE_SPEED_RATIO  (REMOTE_SPEED_DUTY_MAX/128.0f)   //手机蓝牙端, 速度通道范围均为-128-128
#define REMOTE_MAIN_STEER_ANGLE_RATIO ((MAIN_STEER_MAX_ANGLE-MAIN_STEER_MIN_ANGLE)/100.0f)  //舵机角度通道值范围0-100
#define REMOTE_SEC_STEER_ANGLE_RATIO ((SECOND_STEER_MAX_ANGLE-SECOND_STEER_MIN_ANGLE)/100.0f)



typedef struct 
{
    // uint8_t mBool[BOOL_NUM];
    int8_t mInt8[BYTE_NUM];
    int16_t mInt16[SHORT_NUM];
    // int32_t mInt32[INT_NUM];
    // float mFloat[FLOAT_NUM];
}BluePack_t;

// typedef struct
// {
//     uint8_t remoteEnable;  //蓝牙遥控使能
//     int8_t forwardSpeed;   //前进速度
//     int8_t sideSpeed;      //侧向移动速度
//     int8_t turnSpeed;      //转向速度
//     int8_t tongSta;        //抓手状态
//     int16_t bigArmAngle;    //大臂舵机角度 单位mm
//     int16_t smallArmAngle;  //小臂舵机角度
//     int16_t stepAngle;     //步进电机角度
// }RemoteCtrl_t;


uint8_t BlueGetData(uint8_t info);
void BlueRemoteTask(void* param);

#endif

