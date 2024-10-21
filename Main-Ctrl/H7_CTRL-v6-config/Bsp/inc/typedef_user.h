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
    uint8_t QR_date[6]; // QR�����ݣ��洢6���ֽڵ�QR����Ϣ
} QR_code_date;

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



#endif
