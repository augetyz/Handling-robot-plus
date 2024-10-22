#ifndef TYPEDEF_USER_H
#define TYPEDEF_USER_H

#include "stdint.h"
#include "main.h"

typedef struct {
    int32_t x;         // X���꣬��ʾλ�õ�ˮƽ����
    int32_t y;         // Y���꣬��ʾλ�õĴ�ֱ����
    float angle;      // �Ƕȣ���ʾ��ǰλ�õĳ����Ի��Ȼ������ʾ��
} position_order;

typedef struct {
    uint8_t speed_ctrl_status;
    uint8_t position_ctrl_status;
    int16_t speed;
    int32_t position;
}Motor_status;

typedef struct {
    uint8_t QR_date[6]; // QR�����ݣ��洢6���ֽڵ�QR����Ϣ
} QR_code_date;

typedef struct {
    float Kp;          // ��������
    float Ki;          // ��������
    float Kd;          // ΢������
    float setpoint;    // �趨ֵ
    float last_error;  // ��һ�ε����
    float integral;    // ������
    float output;      // ���������
    float output_max;
    float output_min;
} PIDController;


typedef struct {
    int32_t x;         // X���꣬��ʾɫ����λ�õ�ˮƽ����
    int32_t y;         // Y���꣬��ʾɫ����λ�õĴ�ֱ����
    uint8_t color;     // ��ʾɫ������ɫ 1����ɫ 2����ɫ 3����ɫ
} color_ring_position;
typedef struct {
    int32_t x;         // X���꣬��ʾɫ���λ�õ�ˮƽ����
    int32_t y;         // Y���꣬��ʾɫ���λ�õĴ�ֱ����
    uint8_t color;     // ��ʾɫ�����ɫ 1����ɫ 2����ɫ 3����ɫ
} color_block_position;

typedef struct {
    float angle;       // �Ƕȣ���ʾ��е�۵���̨�˶��Ƕ�
    int32_t R;         // �뾶����ʾ��е�۵�ĩ�˾���ο����ˮƽ����
    int32_t H;         // �߶ȣ���ʾ��е�۵�ĩ�˾���ο������ֱ�߶�
    uint8_t jaw_status; // ��צ״̬����ʾ��е�ۼ�צ�ĵ�ǰ״̬����򿪡��رա�ץȡ�ȣ�
    uint8_t turntable_status;//����̨״̬��1����ɫ 2����ɫ 3����ɫ
} Arm_ctrl_position;

typedef struct {
    uint16_t id;  // ��ϢID
    uint8_t data[8];  // ���ݻ�����
    uint8_t len;  // ���ݳ���
} can_message_t;


#endif
