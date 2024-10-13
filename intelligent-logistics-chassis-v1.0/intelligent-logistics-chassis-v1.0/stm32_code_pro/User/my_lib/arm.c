#include "my_lib/arm.h"

arm_t arm = {0};

void ArmInit(void)
{
    arm.L0 = BASE_HIGH;
    arm.L1 = MAIN_ARM_LEN;
    arm.L2 = SECOND_ARM_LEN;
    arm.L3 = ADD_DISTENCE;

    // SteerInit();
    StepMotorInit();
}

void Inverse(arm_t* _arm)
{
	float L, LL, LV;
	float gama, alpha, beta;
    if (_arm->x == 0 && _arm->y >0) {    //避免被除数为零
        _arm->J1 = 90;
        L = _arm->y;
    }
    else if (_arm->x == 0 && _arm->y < 0) {
        _arm->J1 = -90;
        L = -_arm->y;
    }
    else {
        if(_arm->x > 0){
            _arm->J1 = atan(_arm->y / _arm->x);
            L = _arm->x / cos(_arm->J1);
            _arm->J1 = PI2AG(_arm->J1);
        }
        else{
            _arm->J1 = atan(_arm->y / _arm->x);
            L = fabs(_arm->x / cos(_arm->J1));
            _arm->J1 = PI2AG(_arm->J1);
            if(_arm->y > 0){
                _arm->J1 += 180;
            }
            else{
                _arm->J1 -= 180;
            }
        }

    }

    LL = sqrt((L - _arm->L3) * (L - _arm->L3) + (_arm->z - _arm->L0) * (_arm->z - _arm->L0));
    gama = acos((_arm->L1 * _arm->L1 + _arm->L2 * _arm->L2 - LL * LL) / (2 * _arm->L1 * _arm->L2));
    gama = PI2AG(gama);

    alpha = acos((_arm->L1*_arm->L1 + LL*LL - _arm->L2*_arm->L2) / (2*LL*_arm->L1));
    alpha = PI2AG(alpha);

    LV = sqrt(_arm->z*_arm->z + (L-_arm->L3)*(L-_arm->L3));
    beta = acos((LL*LL + _arm->L0*_arm->L0 - LV*LV) / (2*LL*_arm->L0));
    beta = PI2AG(beta);
    _arm->J2 = 360 - beta - alpha - 90;
    _arm->J3 = _arm->J2 - gama;
}

/// @brief 非阻塞式控制
/// @param _arm 
void Arm2Ctrl(arm_t* _arm)
{
    //步进电机
    StepMotorAngle(_arm->J1); 
    //三舵机
    SetSteerAngle(MainSteer, J2_ZERO_ANGLE-_arm->J2);
    SetSteerAngle(SecSteer, J3_ZERO_ANGLE+_arm->J3);

    if(_arm->tong_state == TONG_OPEN){
        SetSteerAngle(TongSteer, TONG_STEER_MIN_ANGLE);
    }
    else{
        SetSteerAngle(TongSteer, TONG_STEER_MAX_ANGLE);
    }
}


/// @brief 判断机械臂动作是否完成
/// @param  
/// @return 
uint8_t IsArmBusy(void)
{
    //temp test
    // MY_LOG_Print("\r\nbusy:%d %d %d %d", IsStepBusy(), IsSteerBusy(MainSteer), IsSteerBusy(SecSteer), IsSteerBusy(TongSteer));
    return IsStepBusy()||IsSteerBusy(MainSteer)||IsSteerBusy(SecSteer)||IsSteerBusy(TongSteer);
}

/// @brief 阻塞运行模式
/// @param x 
/// @param y 
/// @param z 
/// @param tong_sta 
/// @param cam_sta 
void SetArmState(float x, float y, float z, tong_state_t tong_sta)
{
    // if(x > X_MAX){
    //     MY_LOGW("arm", "x[%.1f] > X_MAX[%d]", x, X_MAX);
    //     return;
    // }
    // if(y > Y_MAX){
    //     MY_LOGW("arm", "y[%.1f] > Y_MAX[%d]", y, Y_MAX);
    //     return;
    // }
    if(z > Z_MAX){
        MY_LOGW("arm", "z[%.1f] > Z_MAX[%d]", z, Z_MAX);
        return;
    }
    arm.x = x;
    arm.y = y;
    arm.z = z;
    arm.tong_state = tong_sta;

    Inverse(&arm);
    Arm2Ctrl(&arm);
}

//////////cmd test///////////////////////

static void ArmCtrlForCmd(int argc, char** argv)
{
    if(argc != 5){
        MY_LOGW("arm", "cmd param num invalid");
        return;
    }
    double x = atof(argv[1]);
    double y = atof(argv[2]);
    double z = atof(argv[3]);
    uint8_t tongSta = (uint8_t)atoi(argv[4]);

    MY_LOGI("arm", "x[%.1f] y[%.1f] z[%.1f] tong[%d]", x,y,z,tongSta);
    SetArmState(x, y, z, tongSta);
    MY_LOGI("arm", "J1[%.1f] J2[%.1f] J3[%.1f]", arm.J1, arm.J2, arm.J3);
}
//////////cmd test///////////////////////

void ArmTask(void* param)
{
    ArmInit();
    ShellCmdRegister("arm",
                "ctrl arm, param:[x] [y] [z] [tong]",
                ArmCtrlForCmd);
    while(1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}
