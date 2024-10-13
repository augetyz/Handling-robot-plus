#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

#include "fatfs.h"
#include "cJson/cJSON.h"
#include "stdlib.h"

#include "bsp_driver_sd.h"

// #include "my_lib/car_ctrl.h"
// #include "my_lib/motor.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"

/////////////////////////////////////////////行走////////////////////////////////////////////
//小车路径事件
typedef enum
{
    CAR_EVENT_NONE =      0,
    ARRIVE_QR_CODE =      1,     //扫描二维码区域
    ARRIVE_STORAGE =      2,     //圆盘区
    ARRIVE_TEMP_STORAGE = 3,     //暂存区
    ARRIVE_PROCESS_AREA = 4,     //加工区
    ARRIVE_SEC_PRO_AREA = 5,     //第二次到达加工区
    ARRIVR_BACK_ADJUST  = 6,         //返航校正
    ARRIVR_SPECIAL_ADJUST  = 7,         //方案2中途校正
    ARRIVR_SPECIAL_STORAGE  = 8,         //方案2中途校正
    ARRIVR_STORAGE_ADD  = 9,         //圆盘区校正补充距离
}car_event_enum_t;

//路径执行动作表
typedef struct point
{
    double           coord_x;
    double           coord_y;
    double           angle;
    car_event_enum_t event;
    struct point*    next_point; //单向链表
}point_t;
typedef point_t* point_list_t;
/////////////////////////////////////////////行走////////////////////////////////////////////

/////////////////////////////////////////////参数////////////////////////////////////////////
//pid 参数
typedef struct
{
    double kp;
    double ki;
    double kd;
    double out_max;
    double bias_allow;
}pid_param_t;

//小车所有pid参数
typedef struct
{
    pid_param_t motor_pid;
    pid_param_t distence_pid;
    pid_param_t angle_pid;
    pid_param_t pos_pid;
}car_param_t;
/////////////////////////////////////////////参数////////////////////////////////////////////

/////////////////////////////////////////////机械臂动作////////////////////////////////////////////
#define ARM_ACTION_NUM  18   //对应枚举最大数值+1, 用作动作数组空间大小
typedef enum
{
    ARM_START_POS = 0,           //机械臂发车状态
    ARM_ADJUST_POS_DISC,         //车辆校准姿态(直角)
    ARM_ADJUST_POS_CLOSE_WAY,    //车辆校准姿态(靠近路边)
    ARM_ADJUST_POS_STACK,      //车辆校准姿态(码垛)
    ARM_CRCLE_TAKE_READY,        //圆盘待抓取
    ARM_TAKE_FROM_STORAGE,      //从原料区(圆盘)抓取
    ARM_TAKE_RED_FROM_TEMP,      //从暂存区抓取红色
    ARM_TAKE_GREEN_FROM_TEMP,    //从暂存区抓取绿色
    ARM_TAKE_BLUE_FROM_TEMP,     //从暂存区抓取蓝色
    ARM_PLACE_RED_CLOSE_WAY,      //安置红色
    ARM_PLACE_GREEN_CLOSE_WAY,    //安置绿色
    ARM_PLACE_BLUE_CLOSE_WAY,     //安置蓝色
    ARM_PLACE_RED_STACK,        //安置红色(码垛)
    ARM_PLACE_GREEN_STACK,      //安置绿色(码垛)
    ARM_PLACE_BLUE_STACK,       //安置蓝色(码垛)
    ARM_ADJUST_YAW,               //航向校正姿态
    ARM_ADJUST_BACK,              //返程校正
    ARM_TEMP,
}arm_action_enum_t;

//单个动作链表, 存储每一个动连续执行的位置坐标
typedef struct arm_action
{
    double x;
    double y;
    double z;
    uint8_t tong_open;
    uint8_t holder_wait;  //等待载物台旋转完成空闲状态
    struct arm_action* next_pos;
}arm_action_t;


/////////////////////////////////////////////机械臂动作////////////////////////////////////////////


/////////////////////////////////////////////rgb参数////////////////////////////////////////////
//rgb灯颜色参数

typedef struct 
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}rgb_value_t;

/////////////////////////////////////////////rgb参数////////////////////////////////////////////




extern car_param_t car_param;
extern point_list_t route_list;
extern arm_action_t* arm_action_arr[ARM_ACTION_NUM];
extern rgb_value_t rgb_value;


void FileInfoTask(void* param);
uint8_t IsFileReady(void);

uint8_t RouteExtract_Specil(void);

#endif


