#include "bsp_motor_ctrl.h"
#include "can_bsp.h"
#include <stdlib.h> // 包含 abs 函数
// 定义电机ID
#define MOTOR_ID_1  1
#define MOTOR_ID_2  2
#define MOTOR_ID_3  3
#define MOTOR_ID_4  4

/**
 * @brief 发送使能或失能命令以控制指定电机的状态
 * 
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要控制的电机的 ID
 * @param enable_state 使能状态：0x01 = 使能，0x00 = 失能
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, uint8_t enable_state) {
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
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    
    uint8_t data[8];    // 数据缓冲区
    uint8_t checksum;    // 校验字节

    // 构建命令数据
    data[0] = motor_id; // 电机ID
    data[1] = 0xF6;     // 固定命令字节
    
    // 根据速度值判断方向，正值为逆时针（CCW），负值为顺时针（CW）
    if (speed_rpm < 0) {
        data[2] = 0x00; // CW
        speed_rpm = -speed_rpm; // 将负值转换为正值
    } else {
        data[2] = 0x01; // CCW
    }

    // 分解速度为高字节和低字节
    data[3] = (speed_rpm >> 8) & 0xFF; // 速度高字节
    data[4] = speed_rpm & 0xFF;        // 速度低字节
    data[5] = acceleration;             // 加速度档位
    data[6] = sync_flag;                // 多机同步标志：0x00 = 不启用，0x01 = 启用

    // 固定校验字节 0x6B
    checksum = 0x6B;                    
    data[7] = checksum;

    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
}
/**
 * @brief 发送立即停止命令以停止指定电机的运动
 * 
 * @param hfdcan 指向 FDCAN_HandleTypeDef 结构体的指针，用于 CAN 总线通信
 * @param motor_id 需要停止的电机的 ID
 * @return uint8_t 返回发送状态，0 表示成功，其他值表示错误
 */
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id) {
    uint8_t data[5];
    uint8_t checksum;

    // 构建命令数据
    data[0] = motor_id;        // 电机ID
    data[1] = 0xFE;            // 固定命令字节
    data[2] = 0x98;            // 固定命令字节
    data[3] = 0x00;            // 多机同步标志固定为 0x00（不启用）

    // 固定校验字节 0x6B
    checksum = 0x6B;
    data[4] = checksum;

    // 使用 fdcanx_send_data 发送数据
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
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
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1, int16_t speed2, int16_t speed3, int16_t speed4) {
    uint8_t acceleration = 0x08; // 假设所有电机使用相同的加速度档位
    uint8_t sync_flag = 0x01;     // 启用多机同步标志
    uint8_t data[4];               // 数据缓冲区，准备发送同步命令

    // 发送电机1速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_1, speed1, acceleration, sync_flag);

    // 发送电机2速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_2, speed2, acceleration, sync_flag);

    // 发送电机3速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_3, speed3, acceleration, sync_flag);

    // 发送电机4速度控制命令
    can_motor_speed_control(hfdcan, MOTOR_ID_4, speed4, acceleration, sync_flag);

    // 构建多机同步命令数据
    data[0] = 0x00;           // 地址字段为0（广播给所有电机）
    data[1] = 0xFF;           // 固定命令字节，表明是同步命令
    data[2] = 0x66;           // 同步运动命令字段
    data[3] = 0x6B;           // 校验字节，固定为 0x6B

    // 使用 fdcanx_send_data 发送多机同步命令
    fdcanx_send_data(hfdcan, 0x00, data, sizeof(data)); // 发送多机同步命令
}





