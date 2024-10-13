#ifndef __STEP_TRAPE_H__
#define __STEP_TRAPE_H__

#include "my_sys.h"
#include "math.h"
#include "string.h"

#include "my_lib/my_log.h"




#define MAX_STEP_CTRL_NUM  5  //目前设置的同时支持10个电机的加减速

#define TRAPE_ID_t     int16_t

// typedef enum
// {
//     BaseSpin = 0,
// }TRAPE_ID_t;

typedef enum
{
    ACCEL = 0,
    CONSTANT,
    DECEL,
    STOP,
}trape_sta_enum_t;


typedef struct   
{
    float               stepSize;         //电机步长或步距角
    uint32_t            ClkFrq;           //定时器频率
    float               accelSpeed;            //加速段加速度
    float               decelSpeed;            //减速段减速度
    float               constSpeed;       //最大速度，即匀速运行段速度        
    uint32_t            totalStep;        //电机走过的总步长或总角度，需要与stepLengh单位相同 
} trape_accel_param_t;


typedef struct 
{
    TRAPE_ID_t                       stepId;
    float                           stepSize;          //电机步长或步距角
    uint32_t                        ClkFrq;            //定时器频率
    uint32_t                        accelSpeed;             //加速段加速度
    uint32_t                        decelSpeed;        //减速段减速度
    uint32_t                        constSpeed;        //匀速运行段速度
    uint32_t                        totalStep;        //电机走过的总步长或总角度，需要与stepLengh单位相同 
    uint32_t                        accelLimStep;      //理论最大步数
    uint32_t                        accelStep;         //达到匀速段速度要运行的步数 
    int32_t                         decelStep;         //减速段步数
    int32_t                         stepCount;         //用来记录各状态下的步数，共过程计算使用
    uint32_t                        prevCounterVal;    //上一个定时器寄存器的值
    int32_t                         rest;              //上次计算所得余数
    trape_sta_enum_t                status;            //状态
}trape_accel_cb_t;



/*---------------------------------------------------------------------------------------
 函数原型:  TRAPE_ID_t TrapeAccel_Init(trape_accel_cb_t* pParam)
 功    能:  加速模块初始化
 输入参数:  pParam:梯度加速输入参数
 输出参数:	NA
 返 回 值:	如果初始化成功，返回梯度加速度ID,其他函数的调用需要使用此ID，失败，返回-1
---------------------------------------------------------------------------------------*/ 
TRAPE_ID_t TrapeAccel_Init(trape_accel_param_t* pParam);

/*---------------------------------------------------------------------------------------
 函数原型:  void TrapeAccel_Deinit(TRAPE_ID_t id)
 功    能: TRAPE_ID_t: 加速模块释放初始化
 输入参数:  id：梯度加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
void TrapeAccel_Deinit(TRAPE_ID_t id);

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_Start(TRAPE_ID_t id)
 功    能:  开始计算梯形加减速相关参数
 输入参数:  id：梯度加速度ID
 输出参数:	NA
 返 回 值:	第一个Counter值
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_Start(TRAPE_ID_t id, uint32_t totalStep);

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetAccelSteps(TRAPE_ID_t id)
 功    能:  获取加速阶段步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetAccelSteps(TRAPE_ID_t id);

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetdecelSteps(TRAPE_ID_t id)
 功    能:  获取减速阶段步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetdecelSteps(TRAPE_ID_t id);

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetConstSteps(TRAPE_ID_t id)
 功    能:  获取匀速阶段的步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetConstSteps(TRAPE_ID_t id);

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetAccelSteps(TRAPE_ID_t id)
 功    能:  获取已运行的步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetStepCount(TRAPE_ID_t id);


/*---------------------------------------------------------------------------------------
 函数原型:  TRAPE_ACCEL_STAT_t TrapeAccel_GetStatus(TRAPE_ID_t id)
 功    能:  获取运行状态
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
trape_sta_enum_t TrapeAccel_GetStatus(TRAPE_ID_t id);

/*-------------------------------------------------------------------------------------
函数原型: TRAPE_ACCEL_STAT_t  TrapeAccel_CalNextCounterVal(TRAPE_ID_t id, uint32_t * pCounterVal)
功    能:  计算下一步
输入参数:  id：梯形加速ID
输出参数:  pCounterVal：计数器的值指针
返 回 值:  电机当前状态，返回状态为STOP时，则停止计算
---------------------------------------------------------------------------------------*/
trape_sta_enum_t  TrapeAccel_CalNextCounterVal(TRAPE_ID_t id, uint32_t * pCounterVal);


// /// @brief 设置新的步数, 进行新一轮计算
// /// @param id 
// /// @param newStep 
// void TrapeSetNewStep(TRAPE_ID_t id, uint32_t newStep);







#endif




