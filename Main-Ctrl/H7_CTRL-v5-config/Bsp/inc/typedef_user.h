#ifndef TYPEDEF_USER_H
#define TYPEDEF_USER_H

#include "stdint.h"
#include "main.h"

typedef struct {
    int32_t x;         // X坐标，表示位置的水平坐标
    int32_t y;         // Y坐标，表示位置的垂直坐标
    float angle;      // 角度，表示当前位置的朝向（以弧度或度数表示）
} position_order;

typedef struct {
    uint8_t QR_date[6]; // QR码数据，存储6个字节的QR码信息
} QR_code_date;

typedef struct {
    int32_t x;         // X坐标，表示色环的位置的水平坐标
    int32_t y;         // Y坐标，表示色环的位置的垂直坐标
    uint8_t color;     // 表示色环的颜色 1：红色 2：绿色 3：蓝色
} color_ring_position;

typedef struct {
    int32_t x;         // X坐标，表示色块的位置的水平坐标
    int32_t y;         // Y坐标，表示色块的位置的垂直坐标
    uint8_t color;     // 表示色块的颜色 1：红色 2：绿色 3：蓝色
} color_block_position;

typedef struct {
    float angle;       // 角度，表示机械臂的云台运动角度
    int32_t R;         // 半径，表示机械臂的末端距离参考点的水平距离
    int32_t H;         // 高度，表示机械臂的末端距离参考点的竖直高度
    uint8_t jaw_status; // 夹爪状态，表示机械臂夹爪的当前状态（如打开、关闭、抓取等）
    uint8_t turntable_status;//物料台状态，1：红色 2：绿色 3：蓝色
} Arm_ctrl_position;



#endif
