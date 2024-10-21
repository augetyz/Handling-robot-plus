#include "bsp_motor_ctrl.h"
#include "can_bsp.h"
#include "cmsis_os.h"
#include "typedef_user.h"
#include <stdlib.h> // 包含 abs 函数

#define PI          3.1415926f

Motor_status motor[4]={0};

// 定义电机ID
extern PIDController pid_controller;
extern QueueHandle_t can_queue;
extern float imuAngle[3];

/**
 * @brief 发送使能或失能命令以控制指定电机的状态
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要控制的电机的 ID
 * @param enable_state 使能状态：0x01 = 使能，0x00 = 失能
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                         uint8_t enable_state) {
    uint8_t data[6];

    // 构建命令数据
    data[0] = motor_id;        // 电机ID
    data[1] = 0xF3;            // 固定命令字节
    data[2] = 0xAB;            // 固定命令字节
    data[3] = enable_state;    // 使能状态：0x01 = 使能，0x00 = 失能
    data[4] = 0x00;            // 多机同步标志，通常为 0x00
    data[5] = 0x6B;            // 固定校验字节 0x6B

    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
}
/**
 * @brief 发送速度控制命令以控制指定电机的运动
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要控制的电机的 ID
 * @param speed_rpm 电机的目标转速，单位为 RPM（每分钟转数），正值表示逆时针（CCW），负值表示顺时针（CW）
 * @param acceleration 加速度档位，取值根据电机的特性
 * @param sync_flag 多机同步标志：0x00 = 不启用，0x01 = 启用
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    uint8_t data[8];    // 数据缓冲区
    uint8_t checksum;    // 校验字节
    data[0] = 0xF6;     // 固定命令字节

    // 根据速度值判断方向，正值为逆时针（CCW），负值为顺时针（CW）
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // 将负值转换为正值
    } else {
        data[1] = 0x01; // CCW
    }
    // 分解速度为高字节和低字节
    data[2] = (speed_rpm << 8) & 0xFF; // 速度高字节
    data[3] = speed_rpm & 0xFF;        // 速度低字节
    data[4] = acceleration;             // 加速度档位
    data[5] = sync_flag;                // 多机同步标志：0x00 = 不启用，0x01 = 启用
    // 固定校验字节 0x6B
    checksum = 0x6B;
    data[6] = checksum;
    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(hfdcan, motor_id, data, 7);
}
/**
* @brief 发送位置控制命令以控制指定电机的运动
*
* @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
* @param motor_id 需要控制的电机的 ID
* @param speed_rpm 电机的目标转速，单位为 RPM（每分钟转数），正值表示逆时针（CCW），负值表示顺时针（CW）
* @param acceleration 加速度档位，取值根据电机的特性
* @param distance 电机目标位置，单位为毫米mm
* @param sync_flag 多机同步标志：0x00 = 不启用，0x01 = 启用
* @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
*/
uint8_t can_motor_position_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                   int16_t speed_rpm, uint8_t acceleration, int distance, uint8_t position_mode, uint8_t sync_flag) {
//		uint8_t tx_data[8] = {0XF6,0X01,0X00,0XA0,0X0A,0X00,0X6B};
    uint8_t data[8];    // 数据缓冲区
    uint8_t data2[8];   // 超过8字节分包发送
    uint8_t checksum;   // 校验字节
    // 定义每圈的脉冲数
    int pulses_per_revolution = 3200;
    double radius = 38;  // 38毫米转换为
    // 计算轮子的周长
    double circumference = 2 * PI * radius;
    double pulses_per_meter = (double)pulses_per_revolution / circumference;
    double error = (distance % 100) * 5;
    uint32_t pulses = (distance - error) * pulses_per_meter;
    // 构建命令数据
//    data[0] = motor_id; // 电机ID
    data[0] = 0xFD;     // 固定命令字节

    // 根据速度值判断方向，正值为逆时针（CCW），负值为顺时针（CW）
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // 将负值转换为正值
    } else {
        data[1] = 0x01; // CCW
    }

    // 分解速度为高字节和低字节
    data[2] = (speed_rpm >> 8) & 0xFF; // 速度高字节
    data[3] = speed_rpm & 0xFF;        // 速度低字节
    data[4] = acceleration;             // 加速度档位

    data[5] = (pulses >> 24) & 0xFF; // 脉冲位置高24-31
    data[6] = (pulses >> 16) & 0xFF; // 脉冲位置高24-31
    // 固定校验字节 0x6B
    data2[0] = 0xFD;
    data2[1] = (pulses >> 8) & 0xFF;
    data2[2] = (pulses) & 0xFF;
    data2[3] = position_mode;
    data2[4] = sync_flag;
    checksum = 0x6B;
    data2[5] = checksum;

    // 使用 fdcanx_send_data 发送数据
    fdcanx_send_data(hfdcan, motor_id, data, 7);
    return fdcanx_send_data(hfdcan, motor_id + 0X001, data2, 6);
}
/**
 * @brief 发送立即停止命令以停止指定电机的运动
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要停止的电机的 ID
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id) {
    uint8_t data[4];
    uint8_t data2[4];
    uint8_t checksum;

    // 构建命令数据
    data[0] = 0xFE;            // 固定命令字节
    data[1] = 0x98;            // 固定命令字节
    data[2] = 0x01;            // 多机同步标志固定为 0x00（不启用）
    data2[0] = 0xFF;           // 固定命令字节，表明是同步命令
    data2[1] = 0x66;           // 同步运动命令字段
    data2[2] = 0x6B;           // 校验字节，固定为 0x6B
    // 固定校验字节 0x6B
    checksum = 0x6B;
    data[3] = checksum;
    fdcanx_send_data(&hfdcan1, 0X100, data, 4);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X200, data, 4);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X300, data, 4);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X400, data, 4);
    osDelay(5);
    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(hfdcan, motor_id, data2, 3);
}



/**
 * @brief 同步控制四个电机的转速和方向
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param speed1 第一个电机的转速，int16_t 类型，正负值表示转向
 * @param speed2 第二个电机的转速，int16_t 类型，正负值表示转向
 * @param speed3 第三个电机的转速，int16_t 类型，正负值表示转向
 * @param speed4 第四个电机的转速，int16_t 类型，正负值表示转向
 */
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4)
{
    uint8_t acceleration = 0; // 假设所有电机使用相同的加速度档位
    uint8_t sync_flag = 1;     // 启用多机同步标志
    uint8_t data[4];               // 数据缓冲区，准备发送同步命令
    uint8_t sign = 0;
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    // 发送电机1速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_1, speed1, acceleration, sync_flag);
    osDelay(5);
//    // 发送电机2速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_2, speed2, acceleration, sync_flag);
    osDelay(5);
    // 发送电机3速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_3, speed3, acceleration, sync_flag);
    osDelay(5);
    // 发送电机4速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_4, speed4, acceleration, sync_flag);
    osDelay(5);
    // 构建多机同步命令数据
    data[0] = 0xFF;           // 固定命令字节，表明是同步命令
    data[1] = 0x66;           // 同步运动命令字段
    data[2] = 0x6B;           // 校验字节，固定为 0x6B

    fdcanx_send_data(hfdcan, 0x000, data, 3);// 使用 fdcanx_send_data 发送多机同步命令

}

/**
 * @brief 同步控制四个电机的位置
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param speed1 第一个电机的转速，int16_t 类型，正负值表示转向
 * @param speed2 第二个电机的转速，int16_t 类型，正负值表示转向
 * @param speed3 第三个电机的转速，int16_t 类型，正负值表示转向
 * @param speed4 第四个电机的转速，int16_t 类型，正负值表示转向
 */
int can_sync_position_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4,int distance1,int distance2,int distance3,int distance4) 
{
    uint8_t acceleration=0;
    uint8_t sync_flag = 1;     // 启用多机同步标志
    uint8_t data[4];               // 数据缓冲区，准备发送同步命令
    uint8_t sign = 0;
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    acceleration=acc_get(speed1>0?speed1:-speed1,1000);
    // 发送电机1速度控制命令
    can_motor_position_control(hfdcan, MOTOR_ID_1, speed1, acceleration, distance1, 0, sync_flag);
    osDelay(5);
//    // 发送电机2速度控制命令
    can_motor_position_control(hfdcan, MOTOR_ID_2, speed2, acceleration, distance2, 0, sync_flag);
    osDelay(5);
    acceleration=acc_get(speed3>0?speed3:-speed3,1000);
    // 发送电机3速度控制命令
    can_motor_position_control(hfdcan, MOTOR_ID_3, speed3, acceleration, distance3, 0, sync_flag);
    osDelay(5);
    // 发送电机4速度控制命令
    can_motor_position_control(hfdcan, MOTOR_ID_4, speed4, acceleration, distance4, 0, sync_flag);
    osDelay(5);
    // 构建多机同步命令数据
    data[0] = 0xFF;           // 固定命令字节，表明是同步命令
    data[1] = 0x66;           // 同步运动命令字段
    data[2] = 0x6B;           // 校验字节，固定为 0x6B

    // 使用 fdcanx_send_data 发送多机同步命令
    return fdcanx_send_data(hfdcan, 0x000, data, 3);
}

void can_read_four_motors(FDCAN_HandleTypeDef *hfdcan, uint8_t CONTROL,uint16_t motor_id)
{
    uint8_t data[3];
    data[0] = CONTROL;         // 固定命令字节，表明是同步命令
    data[1] = 0x6B;           // 校验字节，固定为 0x6B
    fdcanx_send_data(hfdcan, motor_id, data, 2);
}

//target_x,y  目标位置
//current_x,y 当前位置
void can_sync_quanxiang_position_control_four_motors(double target_x, double target_y, double current_x, double current_y,double w) {
// 假设所有电机使用相同的加速度档位
// 计算目标位置与当前位置之间的差值
    can_message_t message;
    uint32_t position = 0;
    uint8_t move_sign = 0;
    double dx = target_x - current_x;
    double dy = target_y - current_y;
    // 计算目标位置的角度
    double theta = atan2(dy, dx);
    // 计算目标位置的距离
    double distance = sqrt(dx * dx + dy * dy);
    double r = 38.00;
    double L = 17.50;
    // 假设平台的速度为线性速度 v 和角速度 w
    double w1,w2,w3,w4;  // 角速度 (rad/s)
    double v1 = w1 * r;
    double v2 = w2 * r;
    double v3 = w3 * r;
    double v4 = w4 * r;
    double vx = w * cos(theta);
    double vy = w * sin(theta);
//		position = 20000;

    if(dx == 1 || dy == 1)
    {
        move_sign = 0;
        if(dx == 0)
        {
            if(dy < 0 && move_sign == 0)
            {
                distance = -dy;
                can_sync_position_control_four_motors(&hfdcan1, -w, -w, -w, -w,distance,distance,distance,distance);
                move_sign = 1;
            }
            if(move_sign == 0)
            {
                distance = dy;
                can_sync_position_control_four_motors(&hfdcan1,w,w,w,w,distance,distance,distance,distance);
                osDelay(5);

            }
            if(move_sign == 1)
            {
                while (xQueueReceive(can_queue, &message, portMAX_DELAY) == pdPASS)
                {
                    osDelay(5);
                    can_read_four_motors(&hfdcan1, 0X37, 0X100);
                    if (message.data[0] == 0X37)
                    {
                        if(message.data[1] == 0X01 || message.data[1] == 0X00)
                        {
                            position = merge_8bit_to_32bit(message.data[5], message.data[4],message. data[3], message.data[2]);
                            if( position <= 20 &&  position >= -20 )
                            {
                                osDelay(5);
                                break;
                            }

                        }
                    }
                    osDelay(1);
                }
            }

        }
        if(dy == 0)
        {
            if(dx < 0 && move_sign == 0)
            {
                distance = -dy;
                can_sync_position_control_four_motors(&hfdcan1, -w,-w,w,w,distance,distance,distance,distance);
                move_sign = 1;
            }
            distance = dx;
            if(move_sign == 0)
            {
                osDelay(5);
                move_sign = 1;
                can_sync_position_control_four_motors(&hfdcan1,w,w, -w, -w,distance,distance,distance,distance);
            }
            if(move_sign == 1)
            {
                while (xQueueReceive(can_queue, &message, portMAX_DELAY) == pdPASS)
                {
                    osDelay(5);
                    can_read_four_motors(&hfdcan1, 0X37, 0X300);
                    if (message.data[0] == 0X37)
                    {
                        if(message.data[1] == 0X01 || message.data[1] == 0X00)
                        {
                            position = merge_8bit_to_32bit(message.data[5], message.data[4],message. data[3], message.data[2]);
                            if( position <= 20 &&  position >= -20 )
                            {
                                osDelay(5);
                                break;
                            }

                        }
                    }
                    osDelay(1);
                }
            }
        }
    }
    else
    {
        // 计算每个轮子的速度
        w1 = vx + vy;
        w2 = vx + vy;
        w3 = vx - vy;
        w4 = vx - vy;
        if(move_sign == 0)
        {
            can_sync_position_control_four_motors(&hfdcan1,w1,w2, -w3, -w4,distance,distance,distance,distance);
            osDelay(5);
            move_sign = 1;
        }
        while (xQueueReceive(can_queue, &message, portMAX_DELAY) == pdPASS)
        {
            if(((dx > 0) && (dy > 0))|| ((dx < 0) && (dy < 0)))
            {
                can_read_four_motors(&hfdcan1, 0X37, 0X100);
            }
            if(((dx < 0) && (dy > 0)) ||((dx > 0) && (dy > 0)))
            {
                can_read_four_motors(&hfdcan1, 0X37, 0X300);
            }
            if (message.data[0] == 0X37)
            {
                if(message.data[1] == 0X01 || message.data[1] == 0X00)
                {

                    position = merge_8bit_to_32bit(message.data[5], message.data[4],message. data[3], message.data[2]);
                    if( position <= 20)
                    {
                        can_motor_stop(&hfdcan1, 0x000);
                        osDelay(1);
                        break;
                    }

                }
            }
            osDelay(1);
        }

    }
}
uint32_t merge_8bit_to_32bit(uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3) {
    // 将4个8位数据合并为一个32位整数
    uint32_t result = 0;

    // 将data3放在最高8位
    result |= (uint32_t)data3 << 24;
    // 将data2放在中间的高8位
    result |= (uint32_t)data2 << 16;
    // 将data1放在中间的低8位
    result |= (uint32_t)data1 << 8;
    // 将data0放在最低8位
    result |= (uint32_t)data0;

    return result;
}

void PID_Init(PIDController *pid, float Kp, float Ki, float Kd, float setpoint) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->setpoint = setpoint;
    pid->last_error = 0.0;
    pid->integral = 0.0;
    pid->output = 0.0;
    pid->output_max = 120;
    pid->output_min = -120;
}
float PID_Update(PIDController *pid, float measured_value, float dt) {
    // 计算误差
    float error = pid->setpoint - measured_value;

    // 比例项
    float Pout = pid->Kp * error;

    // 积分项
    pid->integral += error * dt;
    if (pid->integral > 50) {
        pid->integral = 50;
    } else if (pid->integral < -50) {
        pid->integral = -50;
    }
    float Iout = pid->Ki * pid->integral;

    // 微分项
    float derivative = (error - pid->last_error) / dt;
    float Dout = pid->Kd * derivative;

    // 更新上次误差
    pid->last_error = error;

    // 计算总输出
    pid->output = Pout + Iout + Dout;

    // 限幅
    if (pid->output > pid->output_max) {
        pid->output = pid->output_max;
    } else if (pid->output < pid->output_min) {
        pid->output = pid->output_min;
    }

    return pid->output;
}
//R = 38

void vPIDControlFunction() {
    static float last_time = 0.0f;
    static float current_time = 0.0f;
    float dt;
    float current_position = imuAngle[0];
    float control_signal;

    // 获取当前时间
    current_time = (float)xTaskGetTickCount() * portTICK_PERIOD_MS;

    // 计算时间差
    dt = current_time - last_time;

    // 更新PID控制器
//    current_position = get_car_position();
    control_signal = PID_Update(&pid_controller, current_position, dt);

    // 设置小车速度
    can_sync_control_four_motors(&hfdcan1, control_signal, -control_signal, -control_signal, control_signal);

    // 更新上次时间
    last_time = current_time;
}


















uint8_t task_adjust(position_order task)
{
    uint8_t task_type=0;
    if(task.x==0 && task.y==0)
    {
        task_type=task_angle;//方向调整任务类型
    }
    if(task.x!=0 || task.y!=0)
    {
        task_type=task_move;//方向调整任务类型
    }
    return task_type;
}

/**
 * @brief 同步控制四个电机的转速和方向
 *
 * @param speed1 第一个电机的转速，int16_t 类型，正负值表示转向
 * @param speed2 第二个电机的转速，int16_t 类型，正负值表示转向
 * @param speed3 第三个电机的转速，int16_t 类型，正负值表示转向
 * @param speed4 第四个电机的转速，int16_t 类型，正负值表示转向
 */
void sync_ctrl_speed(int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4)
{
    uint8_t acceleration = 0XD0; // 假设所有电机使用相同的加速度档位
    uint8_t sync_flag = 1;     // 启用多机同步标志
    uint8_t data[4];               // 数据缓冲区，准备发送同步命令
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    acceleration=acc_get(speed1,300);
    // 发送电机1速度控制命令

    motor_speed_control(&hfdcan1, MOTOR_ID_1, speed1, acceleration, sync_flag);
    osDelay(3);
//    // 发送电机2速度控制命令
    motor_speed_control(&hfdcan1, MOTOR_ID_2, speed2, acceleration, sync_flag);
    osDelay(3);
    acceleration=acc_get(speed3,300);
    // 发送电机3速度控制命令
    motor_speed_control(&hfdcan1, MOTOR_ID_3, speed3, acceleration, sync_flag);
    osDelay(3);
    // 发送电机4速度控制命令
    motor_speed_control(&hfdcan1, MOTOR_ID_4, speed4, acceleration, sync_flag);
    osDelay(3);
    // 构建多机同步命令数据
    data[0] = 0xFF;           // 固定命令字节，表明是同步命令
    data[1] = 0x66;           // 同步运动命令字段
    data[2] = 0x6B;           // 校验字节，固定为 0x6B
    fdcanx_send_data(&hfdcan1, 0x000, data, 3);// 使用 fdcanx_send_data 发送多机同步命令
}
/**
 * @brief 发送速度控制命令以控制指定电机的运动
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要控制的电机的 ID
 * @param speed_rpm 电机的目标转速，单位为 RPM（每分钟转数），正值表示逆时针（CCW），负值表示顺时针（CW）
 * @param acceleration 加速度档位，取值根据电机的特性
 * @param sync_flag 多机同步标志：0x00 = 不启用，0x01 = 启用
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    uint8_t data[8];    // 数据缓冲区
    uint8_t checksum;    // 校验字节
    data[0] = 0xF6;     // 固定命令字节
    uint32_t num=0;
    // 根据速度值判断方向，正值为逆时针（CCW），负值为顺时针（CW）
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // 将负值转换为正值
    } else {
        data[1] = 0x01; // CCW
    }
    // 分解速度为高字节和低字节
    data[2] = (speed_rpm << 8) & 0xFF; // 速度高字节
    data[3] = speed_rpm & 0xFF;        // 速度低字节
    data[4] = acceleration;             // 加速度档位
    data[5] = sync_flag;                // 多机同步标志：0x00 = 不启用，0x01 = 启用
    // 固定校验字节 0x6B
    checksum = 0x6B;
    data[6] = checksum;
    // 使用 fdcanx_send_data 发送数据
    fdcanx_send_data(hfdcan, motor_id, data, 7);
    motor[(motor_id>>8)-1].speed_ctrl_status=0;
    while(motor[(motor_id>>8)-1].speed_ctrl_status==0)
    {
        
        if(num>0X2)
        {
            return 1;//超时
        }
        osDelay(1);
        num++;
    }
    motor[(motor_id>>8)-1].speed_ctrl_status=0;
    return 0;
}
uint8_t acc_get(int16_t speed,uint16_t acc_time)
{
    speed=speed>0?speed:-speed;
    float dert_t=(float)acc_time/(float)speed;
    uint8_t acc=0;
    acc=256-dert_t*20;
    if(speed==0)
        return 0XE0;
    return acc;
}
int32_t distance_dert=0;
uint8_t move_ctrl_dietance(int16_t x,int16_t y,uint16_t speed)
{
    double distance=(uint16_t)sqrt((double)(x*x+y*y));
    double vx = speed * x/distance;
    double vy = speed * y/distance;
    float scale=0;
    static uint8_t sign=0;
    static int32_t w1,w2,w3,w4;
    if(sign==0)
    {
        w1 = vx + vy;
        w2 = vx + vy;
        w3 = vx - vy;
        w4 = vx - vy; 
        sync_ctrl_speed(w1,w2,-w3,-w4);
        sign=1;
    }
    if(abs(w1)>=abs(w3))
    {
        if(abs(x)>=abs(y))
            distance_dert=x-(-1)*motor[0].position/65535*PI*76*x/distance;
        else
            distance_dert=y-(-1)*motor[0].position/65535*PI*76*y/distance;
    }
    else if(abs(w3)>abs(w1))
    {
        if(abs(x)>=abs(y))
            distance_dert=x-motor[2].position/65535*PI*76*x/distance;
        else
            distance_dert=y-motor[2].position/65535*PI*76*y/distance;
    }  
    if(abs(distance_dert)<100)
    {
        sign=0;
        sync_ctrl_speed(0,0,0,0);
        return 0;
    }
    else
        return 1;
}
/**
 * @brief 发送立即停止命令以停止指定电机的运动
 *
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要停止的电机的 ID
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t motor_stop() {
    uint8_t data[7];
    uint8_t data2[4];
    uint8_t checksum;

    // 构建命令数据
    data[0] = 0xF6;            
    data[1] = 0x01;            
    data[2] = 0x01;    
    data[3] = 0x00;            
    data[4] = 0x00;            
    data[5] = 0xD0;
    data[6] = 0x01;
    data2[0] = 0xFF;           // 固定命令字节，表明是同步命令
    data2[1] = 0x66;           // 同步运动命令字段
    data2[2] = 0x6B;           // 校验字节，固定为 0x6B
    // 固定校验字节 0x6B
    checksum = 0x6B;
    data[3] = checksum;
    fdcanx_send_data(&hfdcan1, 0X100, data, 7);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X200, data, 7);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X300, data, 7);
    osDelay(5);
    fdcanx_send_data(&hfdcan1, 0X400, data, 7);
    osDelay(5);
    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(&hfdcan1, 0X00, data2, 3);
}
int16_t read_motor_speed(FDCAN_HandleTypeDef *hfdcan,uint16_t motor_id)
{
    uint8_t data[3];
    data[0] = 0X35;         // 固定命令字节，表明是同步命令
    data[1] = 0x6B;           // 校验字节，固定为 0x6B
    fdcanx_send_data(hfdcan, motor_id, data, 2);
    osDelay(2);
    return motor[((motor_id>>8)-1)].speed;
}


