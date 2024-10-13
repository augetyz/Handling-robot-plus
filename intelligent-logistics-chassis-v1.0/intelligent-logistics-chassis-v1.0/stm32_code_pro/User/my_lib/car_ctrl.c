#include "car_ctrl.h"

#define PI  3.1415926f
// #define PI2AG(x)     (x/PI*180)  //弧度转为角度

static uint8_t gDistenceCtrl = 0, gAngleCtrl = 0, gPosAdjustCtrl =0;  //flag for ctrl
static uint8_t DtcVofaTransFlag = 0, AngleVofaTransFlag =0, PosVofaTransFlag =0;   //pid波形输出控制标志位

// static PID_Type DisPID = {0};  //距离PID
coord_ctrl_t coord_ctrl = {0};
PID_Type AnglePID = {0};     //角度PID
static PID_Type Pos_PID = {0};     //位置校正PID
pos_adjust_t pos_adjust = {0};    //openmv位置校正信息结构体

static float CarMotorSpeed[4] = {0};  //按顺序存放4电机速度


void SetDistancePID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow)
{
    coord_ctrl.DisPID.Kp = PID_P;
    coord_ctrl.DisPID.Ki = PID_I;
    coord_ctrl.DisPID.Kd = PID_D;
    coord_ctrl.DisPID.OutLimit = Limit;
    coord_ctrl.DisPID.BiasAllow = BiasAllow;
}

void SetAnglePID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow)
{
    AnglePID.Kp = PID_P;
    AnglePID.Ki = PID_I;
    AnglePID.Kd = PID_D;
    AnglePID.OutLimit = Limit;
    AnglePID.BiasAllow = BiasAllow;
}

///// @brief position pid param
///// @param PID_P 
///// @param PID_I 
///// @param PID_D 
///// @param Limit 
///// @param BiasAllow 
void SetPosPID(float PID_P, float PID_I, float PID_D, float Limit, float BiasAllow)
{
   Pos_PID.Kp = PID_P;
   Pos_PID.Ki = PID_I;
   Pos_PID.Kd = PID_D;
   Pos_PID.OutLimit = Limit;
   Pos_PID.BiasAllow = BiasAllow;
}


/// @brief 写入默认dis,angle pid参数
/// @param  
void CarCtrl_Init(void)
{
    if(!IsFileReady()){  //等待sd卡文件提取
        vTaskDelay(500);
        if(!IsFileReady()){  //sd卡文件提取失败, 加载默认参数
            SetDistancePID(DistanceKp, DistanceKi, DistanceKd, DistanceOutMax, DistanceBiasAllow);
            SetAnglePID(AngleKp, AngleKi, AngleKd, AngleOutMax, AngleBiasAllow);  
            //position
            SetPosPID(PosKp, PosKi, PosKd, PosOutMax, PosBiasAllow);
            MY_LOGI("car", "load default pid param");
            return;
        }
    }
    //加载sd卡文件中参数
    SetDistancePID(car_param.distence_pid.kp, car_param.distence_pid.ki, car_param.distence_pid.kd, car_param.distence_pid.out_max, car_param.distence_pid.bias_allow);
    SetAnglePID(car_param.angle_pid.kp, car_param.angle_pid.ki, car_param.angle_pid.kd, car_param.angle_pid.out_max, car_param.angle_pid.bias_allow);
    SetPosPID(car_param.pos_pid.kp, car_param.pos_pid.ki, car_param.pos_pid.kd, car_param.pos_pid.out_max, car_param.pos_pid.bias_allow);
    MY_LOGI("car", "load param from file");
}

// /// @brief 设置车头方向,目标行驶距离,并清空电机里程
// /// @param head 车头方向
// /// @param StraightDistance 目标行驶距离(厘米)
// void SetCarHead_Distance(CarHead_Enum head, float Distance)
// {
//     gCarHead = head;
//     if(head==FrontHead || head==BackHead){
//         DisPID.Target = Distance*STRAIGHT_MOVE_RATIO;
//     }
//     else{
//         DisPID.Target = Distance*SIDE_MOVE_RATIO;
//     }
    
//     for(uint8_t i=0; i<4; i++)
//     {
//         ClearMotorMileage(i+1);
//     }
// }

void SetMoveCoord(float x, float y)
{
    coord_ctrl.TarX = x*STRAIGHT_MOVE_RATIO;
    coord_ctrl.TarY = y*SIDE_MOVE_RATIO;
    coord_ctrl.DisPID.Target = sqrt((coord_ctrl.TarX*coord_ctrl.TarX)+(coord_ctrl.TarY*coord_ctrl.TarY));
    coord_ctrl.AccDis = coord_ctrl.DisPID.OutLimit*coord_ctrl.DisPID.OutLimit/2.0f/coord_ctrl.DisPID.Kd;
    for(uint8_t i=0; i<4; i++)
    {
        ClearMotorMileage(i+1);
    }
}

/// @brief 设定车头旋转(相对)角度-179~179
/// @param angle -179~179
void SetCarTurnAngle(float angle)
{
    float angle_set = AnglePID.Target+angle;
    while(angle_set > 179)
    {
        angle_set -= 360;
    }
    while (angle_set < -179)
    {
        angle_set += 360;
    }
    
    AnglePID.Target = angle_set;
}

/// @brief 将计算出来的电机速度进行控制
/// @param enable 
static void CarMotorCtrl(uint8_t enable)
{
    SetMotorParam(1, CarMotorSpeed[0], enable);
    SetMotorParam(2, CarMotorSpeed[1], enable);
    SetMotorParam(3, CarMotorSpeed[2], enable);
    SetMotorParam(4, CarMotorSpeed[3], enable);
}

/// @brief 根据距离pid和角度环pid设定每个电机的速度
/// @param base 基础速度(距离环)
/// @param diff 差速(角度环)
/// @param enable 使能电机
static void CarSpeed2Motor(float x, float y, float diff)
{
    CarMotorSpeed[0] = x-y-diff;
    CarMotorSpeed[1] = x+y-diff;
    CarMotorSpeed[2] = x-y+diff;
    CarMotorSpeed[3] = x+y+diff;
}

/// @brief 根据车头方向以及每个电机的里程得到车辆行驶里程
/// @return 
static void MotorDistance2Car()
{
    coord_ctrl.PreX = (GetMotorMileage(1)+GetMotorMileage(2)+GetMotorMileage(3)+GetMotorMileage(4)) / 4;
    coord_ctrl.PreY = (GetMotorMileage(2)+GetMotorMileage(4)-GetMotorMileage(1)-GetMotorMileage(3)) / 4;
}

/// @brief 非标准pid, 坐标位移控制
/// @param  
static void CoordPID_Ctrl(uint16_t PassTick)
{
    MotorDistance2Car();
    coord_ctrl.DisPID.Present = sqrt((coord_ctrl.PreX*coord_ctrl.PreX)+(coord_ctrl.PreY*coord_ctrl.PreY));
    float biasCoordX = coord_ctrl.TarX - coord_ctrl.PreX;
    float biasCoordY = coord_ctrl.TarY - coord_ctrl.PreY;
    coord_ctrl.DisPID.Bias = sqrt((biasCoordX*biasCoordX)+(biasCoordY*biasCoordY));
    coord_ctrl.DisPID.LastBias += coord_ctrl.DisPID.Bias;  //last_bias用作bias_sum
    if(fabs(coord_ctrl.DisPID.Bias) < fabs(coord_ctrl.DisPID.BiasAllow))
    {  //避免原地振荡
        coord_ctrl.DisPID.Output = 0;
        coord_ctrl.DisPID.LastBias = 0;
    }
    else
    {
        if(fabs(coord_ctrl.DisPID.Target) < 2*coord_ctrl.AccDis)  //两个阶段, 加速, 减速
        {
            if(fabs(coord_ctrl.DisPID.Present) < coord_ctrl.DisPID.Target/2)  //加速
            {
                coord_ctrl.DisPID.Output += coord_ctrl.DisPID.Bias>0? coord_ctrl.DisPID.Kd*PassTick/1000 : (-coord_ctrl.DisPID.Kd*PassTick/1000);
            }
            else   //减速
            {
                coord_ctrl.DisPID.Output -= coord_ctrl.DisPID.Bias>0? coord_ctrl.DisPID.Kd*PassTick/1000 : (-coord_ctrl.DisPID.Kd*PassTick/1000);
                if(coord_ctrl.DisPID.Bias>0 && coord_ctrl.DisPID.Output<coord_ctrl.DisPID.Ki){  //ki作为最小基础速度
                    coord_ctrl.DisPID.Output = coord_ctrl.DisPID.Bias<coord_ctrl.DisPID.Ki;
                }
                else if(coord_ctrl.DisPID.Bias<0 && coord_ctrl.DisPID.Output>-coord_ctrl.DisPID.Ki){
                    coord_ctrl.DisPID.Output = -coord_ctrl.DisPID.Bias<coord_ctrl.DisPID.Ki;
                }
            }
        }
        else    //三个阶段, 加速, 保持匀速, 减速 
        {
            if(fabs(coord_ctrl.DisPID.Present) < coord_ctrl.AccDis)  //50cm 的距离作为起步加速阶段, Kd用作加速度
            {
                coord_ctrl.DisPID.Output += coord_ctrl.DisPID.Bias>0? coord_ctrl.DisPID.Kd*PassTick/1000 : (-coord_ctrl.DisPID.Kd*PassTick/1000);
            }
            else if(fabs(coord_ctrl.DisPID.Bias) < coord_ctrl.AccDis)  //距离小于一定范围开始减速, Kp作为减加速度
            {
                coord_ctrl.DisPID.Output -= coord_ctrl.DisPID.Bias>0? coord_ctrl.DisPID.Kd*PassTick/1000 : (-coord_ctrl.DisPID.Kd*PassTick/1000);
                if(coord_ctrl.DisPID.Bias>0 && coord_ctrl.DisPID.Output<coord_ctrl.DisPID.Ki){  //ki作为最小基础速度
                    coord_ctrl.DisPID.Output = coord_ctrl.DisPID.Bias<coord_ctrl.DisPID.Ki;
                }
                else if(coord_ctrl.DisPID.Bias<0 && coord_ctrl.DisPID.Output>-coord_ctrl.DisPID.Ki){
                    coord_ctrl.DisPID.Output = -coord_ctrl.DisPID.Bias<coord_ctrl.DisPID.Ki;
                }
            }
        }
    }
    //输出限幅
    if(coord_ctrl.DisPID.Output > coord_ctrl.DisPID.OutLimit)
    {
        coord_ctrl.DisPID.Output = coord_ctrl.DisPID.OutLimit;
    }else if(coord_ctrl.DisPID.Output < -coord_ctrl.DisPID.OutLimit)
    {
        coord_ctrl.DisPID.Output = -coord_ctrl.DisPID.OutLimit;
    }

    double rad;
    if(biasCoordX == 0){
        if (biasCoordY > 0) rad = PI/2;
        else  rad = -PI/2;
    }
    else{
        rad = atan(biasCoordY/biasCoordX);
        if(biasCoordX<0 && biasCoordY>=0)
            rad += PI;
        else if(biasCoordX<0 && biasCoordY<0)
            rad -= PI;
    }
    coord_ctrl.OutX = coord_ctrl.DisPID.Output*cos(rad);
    coord_ctrl.OutY = coord_ctrl.DisPID.Output*sin(rad);
}

/// @brief 位置式, 方向pid控制
/// @param PassTick 
static void AnglePID_Ctrl(uint16_t PassTick)
{
    static uint8_t flag =0;
    if(IsIMU_Start() && !flag)  //以IMU初始化角度作为小车初始角度
    {
        flag =1;
        AnglePID.Target = GetYaw();
        // MY_LOGD("car", "angle pid tar->%.1f", GetYaw());
    }
    else if(!IsIMU_Start())
    {
        return;
    }

    AnglePID.Present = GetYaw();
    AnglePID.LastBias += AnglePID.Bias;   //last_bias用作bias累加项
    AnglePID.Bias = AnglePID.Target - AnglePID.Present;
    if(AnglePID.Bias >= 180) AnglePID.Bias -= 360;
    else if(AnglePID.Bias <= -180) AnglePID.Bias += 360;

    //小车静态下避免角度调整造成的原地振荡
    if(fabs(AnglePID.Bias)<fabs(AnglePID.BiasAllow) && fabs(coord_ctrl.DisPID.Bias) < fabs(coord_ctrl.DisPID.BiasAllow)){  
        AnglePID.Output = 0;
         AnglePID.LastBias  = 0;
         AnglePID.BiasSum = 0;
    }
    else{
        AnglePID.BiasSum += AnglePID.Bias;
        AnglePID.Output = AnglePID.Kp*AnglePID.Bias + AnglePID.Ki*AnglePID.BiasSum;
    }
    //限幅
    if(AnglePID.Output > AnglePID.OutLimit) AnglePID.Output = AnglePID.OutLimit;
    else if(AnglePID.Output < -AnglePID.OutLimit) AnglePID.Output = -AnglePID.OutLimit;
}

/// @brief 位置校正pid控制
/// @param PassTick 
static void PosAdjustPID_Ctrl(uint16_t PassTick)
{
    if(gDistenceCtrl){   //进行位置校正时,应停止距离pid控制
        MY_LOGE("car", "distance pid is run when pos adjust");
        return;
    }
    Pos_PID.Bias =sqrt(pos_adjust.x_bias*pos_adjust.x_bias + pos_adjust.y_bias*pos_adjust.y_bias); //由坐标计算偏差距离
    if(fabs(Pos_PID.Bias) <= fabs(Pos_PID.BiasAllow)){  //避免原地振荡
        Pos_PID.Output = 0;
    }
    else{
        Pos_PID.Output = Pos_PID.Kp*Pos_PID.Bias;
    }
    Pos_PID.LastBias = Pos_PID.Bias;
    //输出限幅
    if(Pos_PID.Output > Pos_PID.OutLimit)
    {
        Pos_PID.Output = Pos_PID.OutLimit;
    }else if(Pos_PID.Output < -Pos_PID.OutLimit)
    {
        Pos_PID.Output = -Pos_PID.OutLimit;
    }

    double rad;
    if(pos_adjust.x_bias == 0){
        if (pos_adjust.y_bias > 0) rad = PI/2;
        else  rad = -PI/2;
    }
    else{
        rad = atan(pos_adjust.y_bias/pos_adjust.x_bias);
        if(pos_adjust.x_bias<0 && pos_adjust.y_bias>=0)
            rad += PI;
        else if(pos_adjust.x_bias<0 && pos_adjust.y_bias<0)
            rad -= PI;
    }
    float x_speed, y_speed;
    x_speed = Pos_PID.Output*cos(rad);
    y_speed = Pos_PID.Output*sin(rad);
    //设置速度(角度环控制任然存在, 采用并联输出)
    CarMotorSpeed[0] += (-x_speed+y_speed);
    CarMotorSpeed[1] += (-x_speed-y_speed);
    CarMotorSpeed[2] += (-x_speed+y_speed);
    CarMotorSpeed[3] += (-x_speed-y_speed);
}

/// @brief 是否启用位置校正功能
/// @param sta 
void IsPosAdjust(uint8_t sta)
{
    SetMoveCoord(0, 0);  //清空电机里程, 否则位置调整结束时, 距离pid控制将产生不必要的位移
    pos_adjust.x_bias = 0;
    pos_adjust.y_bias = 0;
    
    gPosAdjustCtrl = sta;
    gDistenceCtrl = !sta;  //启用pos adjust则需要关闭距离pid控制
}

/// @brief 根据openmv回传数据设置位置偏差, 用于pid处理
/// @param x_bias 和机械臂坐标系相同
/// @param y_bias 
void SetPosBias(double x_bias, double y_bias)
{
    pos_adjust.x_bias = x_bias;
    pos_adjust.y_bias = y_bias;
}


uint8_t IsDisPID_OK(void)
{
    if(fabs(coord_ctrl.DisPID.Output)<0.01)
        return 1;
    else
        return 0;
}

uint8_t IsAglPID_OK(void)
{
    if(fabs(AnglePID.Output)<0.01)
        return 1;
    else
        return 0;
}

uint8_t IsCarReady(void)
{
    if(fabs(coord_ctrl.DisPID.Output)<0.001 && fabs(AnglePID.Output)<0.001)
    {
        return 1;
    }
    else{
        return 0;
    }
}





/////////////////////////////////temp test///////////////////////////////////////////////////////
void IsEnableDistenceCtrl(uint8_t value)
{
    gDistenceCtrl = (uint8_t)value;
    // SetCarHead_Distance(FrontHead, 0);
    SetMoveCoord(0,0);
    MY_LOGI("car","distence ctrl state:%d", gDistenceCtrl);
}

void IsEnableAngleCtrl(uint8_t value)
{
    // MY_LOG_Print("value:%.1f,  %d\r\n", value, (uint8_t)value);
    gAngleCtrl = (uint8_t)value;
    // SetCarAngle(GetYaw());
    AnglePID.Target = GetYaw();
    MY_LOGI("car","angle ctrl state:%d", gAngleCtrl);
}

static void IsCarPidCtrl(int argc, char** argv)
{
    if(argc != 3){
        MY_LOGW("car", "shell cmd num invalid");
        return;
    }
    char* cmd_id = argv[1];  //提取字符串中的命令字符
    int8_t value = atoi(argv[2]);
    switch (*cmd_id)
    {
    case 'D':
        IsEnableDistenceCtrl(value);
        break;
    case 'A':
        IsEnableAngleCtrl(value);
        break;        
    
    default:
        MY_LOGW("car", "cmd id invalid");
        break;
    }
}


static void CarCtrl(int argc, char** argv)
{
    if(strcmp("-s", argv[1]) == 0)
    {
        MY_LOGI("car", "car ready?[%d] / dis_pid ok?[%d] / agl_pid ok?[%d]", IsCarReady(), (fabs(coord_ctrl.DisPID.Output)<0.001), (fabs(AnglePID.Output)<0.001));
    }
    else if(strcmp("-a", argv[1]) == 0)
    {
        float value = atof(argv[2]);
        IsEnableAngleCtrl(1);
        MY_LOGI("car","car angle->%.1f", AnglePID.Target + value);
        SetCarTurnAngle(value);
    }
    else if(strcmp("-m", argv[1]) == 0)
    {
        float x = atof(argv[2]);
        float y = atof(argv[3]);
        IsEnableDistenceCtrl(1);
        MY_LOGI("car","car move->%.1f, %.1f", x, y);
        SetMoveCoord(x,y);
    }
    else
    {
        MY_LOGW("car", "cmd id invalid");
    }
}


static void IsCarTrans(int argc, char** argv)
{
    if(argc != 3){
        MY_LOGW("car", "shell cmd num invalid");
        return;
    }

    char* cmd_id = argv[1];  //提取字符串中的命令字符
    uint8_t en = atof(argv[2]);
    switch (*cmd_id)
    {
    case 'D':
        DtcVofaTransFlag = en;
        MY_LOGI("car", "dtc transfer state:%d", DtcVofaTransFlag);
        break;
    case 'A':
        AngleVofaTransFlag = en;
        MY_LOGI("car", "angle transfer state:%d", AngleVofaTransFlag);
        break;
    case 'P':
        PosVofaTransFlag = en;
        MY_LOGI("car", "pos transfer state:%d", PosVofaTransFlag);
        break;
    
    default:
        MY_LOGW("car", "cmd id invalid");
        break;
    }
}


void CarTestInit(void)
{  
    ShellCmdRegister("car",
                    "car ctrl, param:[\"-s\"] or [\"-a\"][angle] or [\"-m\"][x][y]",
                    CarCtrl);

    ShellCmdRegister("car_pid",
                "enbale car pid, param:[D/A] [en]",
                IsCarPidCtrl);   

    ShellCmdRegister("car_tsf",
                    "enable car pid trans, param:[D/A/P] [en]",
                IsCarTrans);  
}

/////////////////////////////////temp test///////////////////////////////////////////////////////


void CarTask(void* param)
{
    static uint32_t TimeTick;
    CarCtrl_Init();
    //temp test
    CarTestInit();
    while(1)
    {
        uint32_t TimePass = GetSysTick()-TimeTick;
        TimeTick = GetSysTick();
        if(gDistenceCtrl){      //距离pid控制
            CoordPID_Ctrl(TimePass);
        }else{
            coord_ctrl.DisPID.Output = 0;
        }
        if(gAngleCtrl){        //角度环pid控制
            AnglePID_Ctrl(TimePass);
        }else{
            AnglePID.Output = 0;
        }
        CarSpeed2Motor(coord_ctrl.OutX, coord_ctrl.OutY, AnglePID.Output);  //距离pid+角度pid速度计算

        if(gPosAdjustCtrl){
            PosAdjustPID_Ctrl(TimePass);
        }

        if(gDistenceCtrl || gAngleCtrl || gPosAdjustCtrl){   //pid都失能,才可以单独控制电机速度
            CarMotorCtrl(1);
        }

        //temp test
        if(DtcVofaTransFlag)
        {
            float Info[6];
            Info[0] = coord_ctrl.TarX;
            Info[1] = coord_ctrl.PreX;
            Info[2] = coord_ctrl.OutX;
            Info[3] = coord_ctrl.TarY;
            Info[4] = coord_ctrl.PreY;
            Info[5] = coord_ctrl.OutY;
            // Info[3] = coord_ctrl.DisPID.Bias;
            // Info[4] = coord_ctrl.DisPID.Output;
            // Info[5] = coord_ctrl.OutY;
            VofaFloatSend(6,Info);
        }
        else if(AngleVofaTransFlag)
        {
            float Info[4];
            Info[0] = AnglePID.Present;
            Info[1] = AnglePID.Target;
            Info[2] = AnglePID.Bias;
            Info[3] = AnglePID.Output;
            VofaFloatSend(4,Info);
        }
        else if(PosVofaTransFlag)
        {
            float Info[4];
            Info[0] = AnglePID.Bias;
            Info[1] = AnglePID.Output;
            VofaFloatSend(2,Info);
        }
        vTaskDelay(20);
    }
}
