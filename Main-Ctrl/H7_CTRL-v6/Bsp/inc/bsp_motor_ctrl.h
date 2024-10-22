#ifndef _BSP_MOTOR_CTRL_H
#define _BSP_MOTOR_CTRL_H
#include "main.h"
#include "typedef_user.h"

#define task_nothing 0
#define task_move    1
#define task_angle   2
#define task_OK      3
uint8_t task_adjust(position_order task);//任务类型判断
uint8_t motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag);
void sync_ctrl_speed(int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4);                                
uint8_t acc_get(int16_t speed,uint16_t acc_time);                         
uint8_t move_ctrl_dietance(int16_t x,int16_t y,uint16_t speed);
uint8_t motor_stop();
void motor_clean();
typedef struct {
    float Kp;          // 比例增益
    float Ki;          // 积分增益
    float Kd;          // 微分增益
    float setpoint;    // 设定值
    float last_error;  // 上一次的误差
    float integral;    // 积分项
    float output;      // 控制器输出
    float output_max;
    float output_min;
} PIDController;

uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, uint8_t enable_state);
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag);
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id);
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1, int16_t speed2, int16_t speed3, int16_t speed4);
uint8_t can_motor_position_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,int16_t speed_rpm, uint8_t acceleration, int distance, uint8_t position_mode, uint8_t sync_flag);
int can_sync_position_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4,int distance1,int distance2,int distance3,int distance4);
void can_sync_quanxiang_position_control_four_motors(double target_x, double target_y, double current_x, double current_y,double w);
void can_read_four_motors(FDCAN_HandleTypeDef *hfdcan, uint8_t CONTROL,uint16_t motor_id);
uint32_t merge_8bit_to_32bit(uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3);
void PID_Init(PIDController *pid, float Kp, float Ki, float Kd, float setpoint);
void vPIDControlFunction();
float PID_Update(PIDController *pid, float measured_value, float dt);
#endif

