#ifndef _CAR_CTRL_H
#define _CAR_CTRL_H

#include "my_sys.h"

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"
#include "vofa_communication.h"
// #include "cmd_analyse.h"
#include "m_shell.h"
#include "jy901.h"
#include "my_lib/file_info.h"

#include "math.h"

//移动距离转换比例(车辆行驶单位/cm)
#define STRAIGHT_MOVE_RATIO  0.05259836
#define SIDE_MOVE_RATIO      0.05344327

#define ACC_DISTENCE         (50*STRAIGHT_MOVE_RATIO)  //加减速距离, 50cm

//default distance pid define
#define DistanceKp   0.0f
#define DistanceKi   0.0f
#define DistanceKd   2.7f
#define DistanceOutMax  4.0f
#define DistanceBiasAllow 0.05f

//default angle pid define
#define AngleKp   0.1f
#define AngleKi   0.0f
#define AngleKd   0.0f
#define AngleOutMax  2.0f
#define AngleBiasAllow 0.2f

//default pos pid define
#define PosKp   2.5f
#define PosKi   0.1f
#define PosKd   0.0f
#define PosOutMax  0.3f
#define PosBiasAllow 0.0f


typedef struct 
{
    PID_Type DisPID;  //坐标矢量绝对值pid计算
    float AccDis;    //加/减速距离
    float TarX;
    float TarY;
    float PreX;
    float PreY;
    float OutX;
    float OutY;
}coord_ctrl_t;


typedef struct
{
    double x_bias;
    double y_bias;
}pos_adjust_t;

extern pos_adjust_t pos_adjust;
extern PID_Type AnglePID;     //角度PID


void CarTask(void* param);

void SetDistancePID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow);
void SetAnglePID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow);
void SetPosPID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow);

// void SetCarHead_Distance(CarHead_Enum head, float StraightDistance);
void SetMoveCoord(float x, float y);
void SetCarTurnAngle(float angle);

uint8_t IsCarReady(void);
uint8_t IsDisPID_OK(void);
uint8_t IsAglPID_OK(void);

void IsPosAdjust(uint8_t sta);
void SetPosBias(double x_bias, double y_bias);

// void CarEnable(double value);  //所有底盘pid控制使能/失能
void IsEnableDistenceCtrl(uint8_t value);  //距离控制使能/失能
void IsEnableAngleCtrl(uint8_t value);   //航向角控制使能/失能

#endif


