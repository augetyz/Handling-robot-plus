#ifndef _BSP_MOTOR_CTRL_H
#define _BSP_MOTOR_CTRL_H
#include "main.h"



uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, uint8_t enable_state);
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag);
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id);
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1, int16_t speed2, int16_t speed3, int16_t speed4);


#endif

