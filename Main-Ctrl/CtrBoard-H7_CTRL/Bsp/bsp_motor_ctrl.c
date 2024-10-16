#include "bsp_motor_ctrl.h"
#include "can_bsp.h"
#include <stdlib.h> // ���� abs ����
// ������ID
#define MOTOR_ID_1  1
#define MOTOR_ID_2  2
#define MOTOR_ID_3  3
#define MOTOR_ID_4  4

/**
 * @brief ����ʹ�ܻ�ʧ�������Կ���ָ�������״̬
 * 
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫ���Ƶĵ���� ID
 * @param enable_state ʹ��״̬��0x01 = ʹ�ܣ�0x00 = ʧ��
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, uint8_t enable_state) {
    uint8_t data[6];

    // ������������
    data[0] = motor_id;        // ���ID
    data[1] = 0xF3;            // �̶������ֽ�
    data[2] = 0xAB;            // �̶������ֽ�
    data[3] = enable_state;    // ʹ��״̬��0x01 = ʹ�ܣ�0x00 = ʧ��
    data[4] = 0x00;            // ���ͬ����־��ͨ��Ϊ 0x00
    data[5] = 0x6B;            // �̶�У���ֽ� 0x6B

    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
}
/**
 * @brief �����ٶȿ��������Կ���ָ��������˶�
 * 
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫ���Ƶĵ���� ID
 * @param speed_rpm �����Ŀ��ת�٣���λΪ RPM��ÿ����ת��������ֵ��ʾ��ʱ�루CCW������ֵ��ʾ˳ʱ�루CW��
 * @param acceleration ���ٶȵ�λ��ȡֵ���ݵ��������
 * @param sync_flag ���ͬ����־��0x00 = �����ã�0x01 = ����
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id, int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    
    uint8_t data[8];    // ���ݻ�����
    uint8_t checksum;    // У���ֽ�

    // ������������
    data[0] = motor_id; // ���ID
    data[1] = 0xF6;     // �̶������ֽ�
    
    // �����ٶ�ֵ�жϷ�����ֵΪ��ʱ�루CCW������ֵΪ˳ʱ�루CW��
    if (speed_rpm < 0) {
        data[2] = 0x00; // CW
        speed_rpm = -speed_rpm; // ����ֵת��Ϊ��ֵ
    } else {
        data[2] = 0x01; // CCW
    }

    // �ֽ��ٶ�Ϊ���ֽں͵��ֽ�
    data[3] = (speed_rpm >> 8) & 0xFF; // �ٶȸ��ֽ�
    data[4] = speed_rpm & 0xFF;        // �ٶȵ��ֽ�
    data[5] = acceleration;             // ���ٶȵ�λ
    data[6] = sync_flag;                // ���ͬ����־��0x00 = �����ã�0x01 = ����

    // �̶�У���ֽ� 0x6B
    checksum = 0x6B;                    
    data[7] = checksum;

    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
}
/**
 * @brief ��������ֹͣ������ָֹͣ��������˶�
 * 
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫֹͣ�ĵ���� ID
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id) {
    uint8_t data[5];
    uint8_t checksum;

    // ������������
    data[0] = motor_id;        // ���ID
    data[1] = 0xFE;            // �̶������ֽ�
    data[2] = 0x98;            // �̶������ֽ�
    data[3] = 0x00;            // ���ͬ����־�̶�Ϊ 0x00�������ã�

    // �̶�У���ֽ� 0x6B
    checksum = 0x6B;
    data[4] = checksum;

    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(hfdcan, motor_id, data, sizeof(data));
}


/**
 * @brief ͬ�������ĸ������ת�ٺͷ���
 * 
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param speed1 ��һ�������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed2 �ڶ��������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed3 �����������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed4 ���ĸ������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 */
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1, int16_t speed2, int16_t speed3, int16_t speed4) {
    uint8_t acceleration = 0x08; // �������е��ʹ����ͬ�ļ��ٶȵ�λ
    uint8_t sync_flag = 0x01;     // ���ö��ͬ����־
    uint8_t data[4];               // ���ݻ�������׼������ͬ������

    // ���͵��1�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_1, speed1, acceleration, sync_flag);

    // ���͵��2�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_2, speed2, acceleration, sync_flag);

    // ���͵��3�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_3, speed3, acceleration, sync_flag);

    // ���͵��4�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_4, speed4, acceleration, sync_flag);

    // �������ͬ����������
    data[0] = 0x00;           // ��ַ�ֶ�Ϊ0���㲥�����е����
    data[1] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data[2] = 0x66;           // ͬ���˶������ֶ�
    data[3] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B

    // ʹ�� fdcanx_send_data ���Ͷ��ͬ������
    fdcanx_send_data(hfdcan, 0x00, data, sizeof(data)); // ���Ͷ��ͬ������
}





