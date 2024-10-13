#include "my_lib/step_trape.h"



static trape_accel_cb_t stepCB_Arr[MAX_STEP_CTRL_NUM]; //内部数据控制块



/*---------------------------------------------------------------------------------------
 函数原型:  TrapeAccelID_t TrapeAccel_Init(pTRAPE_ACCEL_PARAM_t pParam)
 功    能:  加速模块初始化
 输入参数:  pParam:梯度加速输入参数
 输出参数:	NA
 返 回 值:	如果初始化成功，返回梯度加速度ID,其他函数的调用需要使用此ID，失败，返回-1
---------------------------------------------------------------------------------------*/ 
TRAPE_ID_t TrapeAccel_Init(trape_accel_param_t* pParam)
{
    static uint8_t firstInit=0; 
   
    if(!firstInit) //首次进入时，对所有控制块句柄赋值为-1
    {
        for(uint32_t i = 0; i < MAX_STEP_CTRL_NUM; i++)
        {
            stepCB_Arr[i].stepId = -1;
        }
        firstInit = 1;
    }

    

    for(int32_t i = 0; i < MAX_STEP_CTRL_NUM; i++)
    {
        if(stepCB_Arr[i].stepId == -1)  //寻找没有被使用过的控制块
        {
            stepCB_Arr[i].stepSize = pParam->stepSize;
            stepCB_Arr[i].ClkFrq = pParam->ClkFrq;
            stepCB_Arr[i].accelSpeed = (uint32_t)(pParam->accelSpeed*100); //加速度扩大100倍
            stepCB_Arr[i].decelSpeed = (uint32_t)(pParam->decelSpeed*100);//减速度扩大100倍
            stepCB_Arr[i].constSpeed = (uint32_t)(pParam->constSpeed*100);//速度扩大100倍
            stepCB_Arr[i].totalStep = pParam->totalStep;
            stepCB_Arr[i].stepId = i;
            return i;
        }
    }
    return -1;
}

/*---------------------------------------------------------------------------------------
 函数原型:  void TrapeAccel_Deinit(TrapeAccelID_t id)
 功    能: TrapeAccelID_t: 加速模块释放初始化
 输入参数:  id：梯度加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
void TrapeAccel_Deinit(TRAPE_ID_t id)
{
    USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    memset(&stepCB_Arr[id],0,sizeof(trape_accel_cb_t));

    stepCB_Arr[id].stepId = -1;
}





/*---------------------------------------------------------------------------------------
 函数原型:  static void TrapeAccel_Reset(TrapeAccelID_t id)
 功    能:  复位
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
static void TrapeAccel_Reset(trape_accel_cb_t* this)
{
    USER_ASSERT(this != NULL);

    this->status = ACCEL;
    this->rest = 0;
    this->stepCount = 0;
    this->prevCounterVal = 0;

}



/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetAccelSteps(TrapeAccelID_t id)
 功    能:  获取加速阶段步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
static inline void TrapeAccel_CalAccelStep(trape_accel_cb_t* this)
{

    USER_ASSERT(this != NULL);
  
    this->accelStep = (uint64_t)this->constSpeed * (uint64_t)this->constSpeed / (2 * this->accelSpeed * (uint64_t)(this->stepSize * 100));
}


/*-------------------------------------------------------------------------------------
函数原型:  static inline void TrapeAccel_CalAccelLimitStep(pTRAPE_ACCEL_CB_t this)
功    能:  计算加速极限步数
输入参数:  this: 梯形加速度控制块指针
输出参数:  NA
返 回 值:  NA
注意事项：
---------------------------------------------------------------------------------------*/
static inline void TrapeAccel_CalAccelLimitStep(trape_accel_cb_t* this)
{

    USER_ASSERT(this != NULL);

    
    this->accelLimStep = (uint64_t)this->decelSpeed * (uint64_t)this->totalStep / ( this->accelSpeed + this->decelSpeed);
}



/*-------------------------------------------------------------------------------------
函数原型:  static inline void TrapeAccel_CalDecelStep(pTRAPE_ACCEL_CB_t this)
功    能:  计算减速阶段的步数
输入参数:  accelParam：计算梯形加速相关参数
输出参数:  NA
返 回 值:  NA
注意事项： 此函数必须在TrapeAccel_CalAccelStep和TrapeAccel_CalAccelStep之后调用
---------------------------------------------------------------------------------------*/
static inline void TrapeAccel_CalDecelStep(trape_accel_cb_t* this)
{

    USER_ASSERT(this != NULL);

    if(this->accelLimStep > this->accelStep)
    {
        this->decelStep = -(int32_t)((uint64_t)this->accelStep * (uint64_t)this->accelSpeed / this->decelSpeed);     
    }
    else
    {
        this->decelStep =  (int32_t)this->accelLimStep - (int32_t)this->totalStep;
    }
}


/*-------------------------------------------------------------------------------------
函数原型:  static inline uint32_t TrapeAccel_CalFirstCounterVal(pTRAPE_ACCEL_CB_t this)
功    能:  计算电机加速运行时的初始计数器值
输入参数:  accelParam：计算梯形加速相关参数
输出参数:  NA
返 回 值:  计数器值
注意事项：
---------------------------------------------------------------------------------------*/
static inline uint32_t TrapeAccel_CalFirstCounterVal(trape_accel_cb_t* this)
{
    uint32_t T1_FQEQ;
    uint64_t  A_SQ;
    uint32_t counerVal;

    USER_ASSERT(this != NULL);

    T1_FQEQ = (uint32_t)(0.00676 * this->ClkFrq);

    A_SQ = (uint64_t)(20000000000LLU * (double)this->stepSize );

	
    counerVal = (uint32_t)(T1_FQEQ * sqrt(A_SQ /(uint64_t)this->accelSpeed)*0.01);
    this->prevCounterVal = counerVal;
    return counerVal;
}



/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_Start(TrapeAccelID_t id)
 功    能:  开始计算梯形加减速相关参数
 输入参数:  id：梯度加速度ID
 输出参数:	NA
 返 回 值:	第一个Counter值
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_Start(TRAPE_ID_t id, uint32_t totalStep)
{
    trape_accel_cb_t* this;
    uint32_t val;

    USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];

    TrapeAccel_Reset(this);  //复位状态机
    this->totalStep = totalStep;

    TrapeAccel_CalAccelStep(this);  //计算Accel_Step

    TrapeAccel_CalAccelLimitStep(this);//计算Accel_LimitStep

    TrapeAccel_CalDecelStep(this);//计算Decel_Step

    val = TrapeAccel_CalFirstCounterVal(this);//计算首个CounterVal
    return val;
}




/*-------------------------------------------------------------------------------------
函数原型:  static inline uint32_t TrapeAccel_CalCounterValAndRest(pTRAPE_ACCEL_CB_t this)
功    能:  计算电机下个周期内的计数器值以及余数
输入参数:  accelParam：计算梯形加速相关参数
输出参数:  NA
返 回 值:  计数器值
注意事项：
---------------------------------------------------------------------------------------*/
static inline uint32_t TrapeAccel_CalCounterValAndRest(trape_accel_cb_t* this)
{
    uint32_t temp;

    USER_ASSERT(this != NULL);

    
    temp = (uint32_t)((int32_t)this->prevCounterVal -
			(2 * (int32_t)this->prevCounterVal + this->rest) / (4*this->stepCount + 1));

    this->rest = (2 * (int32_t)this->prevCounterVal + this->rest) % (4*this->stepCount + 1);
    
    this->prevCounterVal = temp;

    return temp; 
}



/*-------------------------------------------------------------------------------------
函数原型: TRAPE_ACCEL_STAT_t  TrapeAccel_CalNextCounterVal(TrapeAccelID_t id, uint32_t * pCounterVal)
功    能:  计算下一步
输入参数:  id：梯形加速ID
输出参数:  pCounterVal：计数器的值指针
返 回 值:  电机当前状态，返回状态为STOP时，则停止计算
---------------------------------------------------------------------------------------*/
trape_sta_enum_t  TrapeAccel_CalNextCounterVal(TRAPE_ID_t id, uint32_t * pCounterVal)
{
    
    trape_accel_cb_t* this;

    USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);
    USER_ASSERT(pCounterVal != NULL);


    this = &stepCB_Arr[id];
    this->stepCount++;
    switch(this->status)
    {

        case ACCEL:
            if((uint32_t)this->stepCount > this->accelLimStep)
            {
                this->status = DECEL;
                this->stepCount = this->decelStep;
            }
            else if((uint32_t)this->stepCount > this->accelStep)
            {
                this->status = CONSTANT;
            }
            break;
        case CONSTANT:
            if((uint32_t)this->stepCount > (uint32_t)((int32_t)this->totalStep + this->decelStep))//因为this->decelStep本身带负号，所以用+
            {
                this->status = DECEL;
                this->stepCount = this->decelStep;
            }
            else
            {
                *pCounterVal = this->prevCounterVal;  //速度保持, 无须计算下一步arr数值
                return this->status;
            }
            break;
        case DECEL:
            if(this->stepCount == 0)
            {
                this->status = STOP;
            }
            break;
        case STOP:
            TrapeAccel_Reset(this); 
            break;
        
    }

    *pCounterVal = TrapeAccel_CalCounterValAndRest(this);
    return this->status;
}






/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetAccelSteps(TrapeAccelID_t id)
 功    能:  获取加速阶段步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetAccelSteps(TRAPE_ID_t id)
{
    
	trape_accel_cb_t* this;
	
	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];
    if(this->accelStep > this->accelLimStep)
    {
        return this->accelLimStep;
    }
    else
    {
        return this->accelStep;
    }
}

/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetdecelSteps(TrapeAccelID_t id)
 功    能:  获取减速阶段步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetdecelSteps(TRAPE_ID_t id)
{
    
	trape_accel_cb_t* this;
	
	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];

   return (uint32_t)(-this->decelStep);

}


/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetConstSteps(TrapeAccelID_t id)
 功    能:  获取匀速阶段的步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetConstSteps(TRAPE_ID_t id)
{
    
	trape_accel_cb_t* this;
	
	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];

    // MY_LOGI("trape", "total:%d acc:%d accLim:%d const:%d decl:%d", this->stepCount ,this->accelStep, this->accelLimStep, ((int32_t)this->totalStep-(int32_t)this->accelStep+this->decelStep), this->decelStep);

   return (uint32_t)((int32_t)this->totalStep-(int32_t)this->accelStep+this->decelStep);
}
/*---------------------------------------------------------------------------------------
 函数原型:  uint32_t TrapeAccel_GetAccelSteps(TrapeAccelID_t id)
 功    能:  获取已运行的步数
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
uint32_t TrapeAccel_GetStepCount(TRAPE_ID_t id)
{
    
	trape_accel_cb_t* this;
	
	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];
    if(this->status == DECEL)
    {
        return (uint32_t)((int32_t)this->totalStep + this->stepCount+1);
    }
    else
    {
        return (uint32_t)this->stepCount;
    }

    

}

/*---------------------------------------------------------------------------------------
 函数原型:  TRAPE_ACCEL_STAT_t TrapeAccel_GetStatus(TrapeAccelID_t id)
 功    能:  获取运行状态
 输入参数:  id: 梯形加速度ID
 输出参数:	NA
 返 回 值:	NA
 注意事项：	
---------------------------------------------------------------------------------------*/
trape_sta_enum_t TrapeAccel_GetStatus(TRAPE_ID_t id)
{
    
	trape_accel_cb_t* this;
	
	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

    this = &stepCB_Arr[id];
  
    return this->status;

}

// /// @brief 设置新的步数, 进行新一轮计算
// /// @param id 
// /// @param newStep 
// void TrapeSetNewStep(TRAPE_ID_t id, uint32_t newStep)
// {
//     trape_accel_cb_t* this;
	
// 	USER_ASSERT(id >=0 || id<MAX_STEP_CTRL_NUM);

//     this = &stepCB_Arr[id];
//     TrapeAccel_Reset(this);
//     this->totalStep = newStep;
// }


















