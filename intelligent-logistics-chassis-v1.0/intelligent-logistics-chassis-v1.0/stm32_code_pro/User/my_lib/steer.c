#include "my_lib/steer.h"


static steer_t steers[STEER_NUM] = {0};

static uint8_t steerEnFlag =0;



/// @brief 舵机控制使能/失能
/// @param en 
void SteerEnable(uint8_t en)
{
    steerEnFlag = en;
}

/// @brief 判断角度是否在对应舵机允许范围内
/// @param steer_id 
/// @param angle 
/// @return 
uint8_t IsAngleRight(steer_id_enum_t steer_id, float angle)
{
    if(angle<=steers[steer_id].max_angle && angle>=steers[steer_id].min_angle){
        return 1;
    }
    else{
        return 0;
    }
}

/// @brief 内部调用,将角度值转换为占空比输出
/// @param steer_id 
/// @param angle 
static void SteerAngle2PWM(steer_id_enum_t steer_id, double angle)
{
    uint16_t ccr = angle/STEER_ANGLE*(POSITIVE_ANGLE135-NEGATIVE_ANGLE135) + NEGATIVE_ANGLE135;
    switch (steer_id)
    {
    case MainSteer:
        __HAL_TIM_SET_COMPARE(&MAIN_STEER_TIM, MAIN_STEER_CHANNEL, ccr);
        break;
    case SecSteer:
        __HAL_TIM_SET_COMPARE(&SECOND_STEER_TIM, SECOND_STEER_CHANNEL, ccr);
        break;
    case TongSteer:
        __HAL_TIM_SET_COMPARE(&TONG_STEER_TIM, TONG_STEER_CHANNEL, ccr);
        break;
    case TurnSteer:
        __HAL_TIM_SET_COMPARE(&TURN_STEER_TIM, TURN_STEER_CHANNEL, ccr);
        break;
    
    default:
        break;
    }   
}

/// @brief 设置舵机目标角度
/// @param steer_id 舵机编号
/// @param angle 0-360
void SetSteerAngle(steer_id_enum_t steer_id, double angle)
{
    SteerEnable(1); //使能舵机

    if(steer_id>TurnSteer){
        MY_LOGE("steer", "id is invalid");
        return;
    }
    if(!IsAngleRight(steer_id, angle)){   //一定避免超幅度旋转
        MY_LOGE("steer", "steer[%d] angle[%.1f] is invalid", steer_id, angle);
        return;
    }
    steers[steer_id].tar_angle = angle;
    steers[steer_id].state = 1;
}


/// @brief 判断舵机是否处于运行状态
/// @param id 
/// @return 
uint8_t IsSteerBusy(steer_id_enum_t id)
{
    return steers[id].state;
}


/////////////////////////////cmd test/////////////////////////////////////

static void SteerAngle(int argc, char** argv)
{
    if(argc != 3){
        MY_LOGW("steer", "cmd param num invalid");
        return;
    }
    
    if(strcmp(argv[1], "-e") == 0){  //舵机使能/失能
        uint8_t en = atoi(argv[2]);
        SteerEnable(en);
        MY_LOGI("steer", "en sta:%d", en);
        return;
    }

    uint8_t id = atoi(argv[1]);
    float angle = atof(argv[2]);

    MY_LOGI("steer", "steer[%d]->angle[%.1f]", id, angle);
    SetSteerAngle(id, angle);
    // CtrlSteer();
}
/////////////////////////////cmd test/////////////////////////////////////

/// @brief 启动定时器pwm, 设置初始化角度
/// @param  
static void SteerInit(void)
{
    HAL_TIM_PWM_Start(&MAIN_STEER_TIM, MAIN_STEER_CHANNEL);
    HAL_TIM_PWM_Start(&SECOND_STEER_TIM, SECOND_STEER_CHANNEL);
    HAL_TIM_PWM_Start(&TONG_STEER_TIM, TONG_STEER_CHANNEL);
    HAL_TIM_PWM_Start(&TURN_STEER_TIM, TURN_STEER_CHANNEL);

    steers[MainSteer].max_angle = MAIN_STEER_MAX_ANGLE;
    steers[MainSteer].min_angle = MAIN_STEER_MIN_ANGLE;
    steers[MainSteer].pre_angle = MAIN_STEER_INIT_ANGLE-1;  //必要, 防止启动时角度超幅度
    steers[MainSteer].step_angle = MAIN_STEER_STEP_ANGLE;  
    steers[MainSteer].step_tick = MAIN_STEER_STEP_TICK;



    steers[SecSteer].max_angle = SECOND_STEER_MAX_ANGLE;
    steers[SecSteer].min_angle = SECOND_STEER_MIN_ANGLE;
    steers[SecSteer].pre_angle = SECOND_STEER_INIT_ANGLE-1;
    steers[SecSteer].step_angle = SECOND_STEER_STEP_ANGLE;
    steers[SecSteer].step_tick = SECOND_STEER_STEP_TICK;
 

    steers[TongSteer].max_angle = TONG_STEER_MAX_ANGLE;
    steers[TongSteer].min_angle = TONG_STEER_MIN_ANGLE;
    steers[TongSteer].pre_angle = TONG_STEER_INIT_ANGLE-1;
    steers[TongSteer].step_angle = TONG_STEER_STEP_ANGLE;
    steers[TongSteer].step_tick = TONG_STEER_STEP_TICK;

    steers[TurnSteer].max_angle = TURN_STEER_MAX_ANGLE;
    steers[TurnSteer].min_angle = TURN_STEER_MIN_ANGLE;
    steers[TurnSteer].pre_angle = TURN_STEER_INIT_ANGLE-1;
    steers[TurnSteer].step_angle = TURN_STEER_STEP_ANGLE;
    steers[TurnSteer].step_tick = TURN_STEER_STEP_TICK;

    ShellCmdRegister("steer",
                "set steer angle, param:[id(0-3)] [angle] or [-e][en(0/1)]",
                SteerAngle);
    //舵机旋转至默认初始位置, 角度-1是为了避免初始角度赋值相同不被调度
    // SetSteerAngle(TongSteer, TONG_STEER_INIT_ANGLE-1);
    // SetSteerAngle(MainSteer, MAIN_STEER_INIT_ANGLE-1);
    // SetSteerAngle(SecSteer, SECOND_STEER_INIT_ANGLE-1);
    // SetSteerAngle(TurnSteer, TURN_STEER_INIT_ANGLE-1);
}

void SteerTask(void* param)
{
    SteerInit();
    while(1)
    {
        if(!steerEnFlag){  //默认上电不自动使能舵机
        __HAL_TIM_SET_COMPARE(&MAIN_STEER_TIM, MAIN_STEER_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&SECOND_STEER_TIM, SECOND_STEER_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&TONG_STEER_TIM, TONG_STEER_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&TURN_STEER_TIM, TURN_STEER_CHANNEL, 0);

        vTaskDelay(10);
        continue;
        }


        if(steers[0].state || steers[1].state || steers[2].state || steers[3].state)
        {
            for(uint16_t i=0; i<STEER_NUM; i++)
            {
                if(!steers[i].state){  //未置位工作状态, 退出
                    continue;
                }

                if((GetSysTick()-steers[i].last_tick) >= steers[i].step_tick){
                    steers[i].last_tick = GetSysTick();
                }
                else{
                    continue; //未到时间, 退出
                }

                // MY_LOG_Print("\r\nsta:%d tick:%d", sta, steers[i].step_tick);

                if(steers[i].pre_angle == steers[i].tar_angle){  //已达到目标角度, 退出
                    steers[i].state = 0;
                    continue;
                }
                else if(fabs(steers[i].pre_angle-steers[i].tar_angle) < steers[i].step_angle){
                    steers[i].pre_angle = steers[i].tar_angle;
                    steers[i].state = 0; 
                }
                else if(steers[i].tar_angle > steers[i].pre_angle){
                    steers[i].pre_angle += steers[i].step_angle;
                }
                else
                {
                    steers[i].pre_angle -= steers[i].step_angle;         
                }
                SteerAngle2PWM(i, steers[i].pre_angle);
            }
            vTaskDelay(2);
        }
        else{
            vTaskDelay(20);
        }
    }
}






