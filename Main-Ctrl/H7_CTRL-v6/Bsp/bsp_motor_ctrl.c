#include "bsp_motor_ctrl.h"
#include "can_bsp.h"
#include "cmsis_os.h"
#include "typedef_user.h"
#include <stdlib.h> // ���� abs ����

#define PI          3.1415926f

Motor_status motor[4]={0};

// ������ID
extern PIDController pid_controller;
extern QueueHandle_t can_queue;
extern float imuAngle[3];

/**
 * @brief ����ʹ�ܻ�ʧ�������Կ���ָ�������״̬
 *
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫ���Ƶĵ���� ID
 * @param enable_state ʹ��״̬��0x01 = ʹ�ܣ�0x00 = ʧ��
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t can_motor_enable(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                         uint8_t enable_state) {
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
uint8_t can_motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    uint8_t data[8];    // ���ݻ�����
    uint8_t checksum;    // У���ֽ�
    data[0] = 0xF6;     // �̶������ֽ�

    // �����ٶ�ֵ�жϷ�����ֵΪ��ʱ�루CCW������ֵΪ˳ʱ�루CW��
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // ����ֵת��Ϊ��ֵ
    } else {
        data[1] = 0x01; // CCW
    }
    // �ֽ��ٶ�Ϊ���ֽں͵��ֽ�
    data[2] = (speed_rpm << 8) & 0xFF; // �ٶȸ��ֽ�
    data[3] = speed_rpm & 0xFF;        // �ٶȵ��ֽ�
    data[4] = acceleration;             // ���ٶȵ�λ
    data[5] = sync_flag;                // ���ͬ����־��0x00 = �����ã�0x01 = ����
    // �̶�У���ֽ� 0x6B
    checksum = 0x6B;
    data[6] = checksum;
    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(hfdcan, motor_id, data, 7);
}
/**
* @brief ����λ�ÿ��������Կ���ָ��������˶�
*
* @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
* @param motor_id ��Ҫ���Ƶĵ���� ID
* @param speed_rpm �����Ŀ��ת�٣���λΪ RPM��ÿ����ת��������ֵ��ʾ��ʱ�루CCW������ֵ��ʾ˳ʱ�루CW��
* @param acceleration ���ٶȵ�λ��ȡֵ���ݵ��������
* @param distance ���Ŀ��λ�ã���λΪ����mm
* @param sync_flag ���ͬ����־��0x00 = �����ã�0x01 = ����
* @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
*/
uint8_t can_motor_position_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                   int16_t speed_rpm, uint8_t acceleration, int distance, uint8_t position_mode, uint8_t sync_flag) {
//		uint8_t tx_data[8] = {0XF6,0X01,0X00,0XA0,0X0A,0X00,0X6B};
    uint8_t data[8];    // ���ݻ�����
    uint8_t data2[8];   // ����8�ֽڷְ�����
    uint8_t checksum;   // У���ֽ�
    // ����ÿȦ��������
    int pulses_per_revolution = 3200;
    double radius = 38;  // 38����ת��Ϊ
    // �������ӵ��ܳ�
    double circumference = 2 * PI * radius;
    double pulses_per_meter = (double)pulses_per_revolution / circumference;
    double error = (distance % 100) * 5;
    uint32_t pulses = (distance - error) * pulses_per_meter;
    // ������������
//    data[0] = motor_id; // ���ID
    data[0] = 0xFD;     // �̶������ֽ�

    // �����ٶ�ֵ�жϷ�����ֵΪ��ʱ�루CCW������ֵΪ˳ʱ�루CW��
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // ����ֵת��Ϊ��ֵ
    } else {
        data[1] = 0x01; // CCW
    }

    // �ֽ��ٶ�Ϊ���ֽں͵��ֽ�
    data[2] = (speed_rpm >> 8) & 0xFF; // �ٶȸ��ֽ�
    data[3] = speed_rpm & 0xFF;        // �ٶȵ��ֽ�
    data[4] = acceleration;             // ���ٶȵ�λ

    data[5] = (pulses >> 24) & 0xFF; // ����λ�ø�24-31
    data[6] = (pulses >> 16) & 0xFF; // ����λ�ø�24-31
    // �̶�У���ֽ� 0x6B
    data2[0] = 0xFD;
    data2[1] = (pulses >> 8) & 0xFF;
    data2[2] = (pulses) & 0xFF;
    data2[3] = position_mode;
    data2[4] = sync_flag;
    checksum = 0x6B;
    data2[5] = checksum;

    // ʹ�� fdcanx_send_data ��������
    fdcanx_send_data(hfdcan, motor_id, data, 7);
    return fdcanx_send_data(hfdcan, motor_id + 0X001, data2, 6);
}
/**
 * @brief ��������ֹͣ������ָֹͣ��������˶�
 *
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫֹͣ�ĵ���� ID
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t can_motor_stop(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id) {
    uint8_t data[4];
    uint8_t data2[4];
    uint8_t checksum;

    // ������������
    data[0] = 0xFE;            // �̶������ֽ�
    data[1] = 0x98;            // �̶������ֽ�
    data[2] = 0x01;            // ���ͬ����־�̶�Ϊ 0x00�������ã�
    data2[0] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data2[1] = 0x66;           // ͬ���˶������ֶ�
    data2[2] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B
    // �̶�У���ֽ� 0x6B
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
    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(hfdcan, motor_id, data2, 3);
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
void can_sync_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4)
{
    uint8_t acceleration = 0; // �������е��ʹ����ͬ�ļ��ٶȵ�λ
    uint8_t sync_flag = 1;     // ���ö��ͬ����־
    uint8_t data[4];               // ���ݻ�������׼������ͬ������
    uint8_t sign = 0;
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    // ���͵��1�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_1, speed1, acceleration, sync_flag);
    osDelay(5);
//    // ���͵��2�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_2, speed2, acceleration, sync_flag);
    osDelay(5);
    // ���͵��3�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_3, speed3, acceleration, sync_flag);
    osDelay(5);
    // ���͵��4�ٶȿ�������
    can_motor_speed_control(hfdcan, MOTOR_ID_4, speed4, acceleration, sync_flag);
    osDelay(5);
    // �������ͬ����������
    data[0] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data[1] = 0x66;           // ͬ���˶������ֶ�
    data[2] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B

    fdcanx_send_data(hfdcan, 0x000, data, 3);// ʹ�� fdcanx_send_data ���Ͷ��ͬ������

}

/**
 * @brief ͬ�������ĸ������λ��
 *
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param speed1 ��һ�������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed2 �ڶ��������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed3 �����������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed4 ���ĸ������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 */
int can_sync_position_control_four_motors(FDCAN_HandleTypeDef *hfdcan, int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4,int distance1,int distance2,int distance3,int distance4) 
{
    uint8_t acceleration=0;
    uint8_t sync_flag = 1;     // ���ö��ͬ����־
    uint8_t data[4];               // ���ݻ�������׼������ͬ������
    uint8_t sign = 0;
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    acceleration=acc_get(speed1>0?speed1:-speed1,1000);
    // ���͵��1�ٶȿ�������
    can_motor_position_control(hfdcan, MOTOR_ID_1, speed1, acceleration, distance1, 0, sync_flag);
    osDelay(5);
//    // ���͵��2�ٶȿ�������
    can_motor_position_control(hfdcan, MOTOR_ID_2, speed2, acceleration, distance2, 0, sync_flag);
    osDelay(5);
    acceleration=acc_get(speed3>0?speed3:-speed3,1000);
    // ���͵��3�ٶȿ�������
    can_motor_position_control(hfdcan, MOTOR_ID_3, speed3, acceleration, distance3, 0, sync_flag);
    osDelay(5);
    // ���͵��4�ٶȿ�������
    can_motor_position_control(hfdcan, MOTOR_ID_4, speed4, acceleration, distance4, 0, sync_flag);
    osDelay(5);
    // �������ͬ����������
    data[0] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data[1] = 0x66;           // ͬ���˶������ֶ�
    data[2] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B

    // ʹ�� fdcanx_send_data ���Ͷ��ͬ������
    return fdcanx_send_data(hfdcan, 0x000, data, 3);
}

void can_read_four_motors(FDCAN_HandleTypeDef *hfdcan, uint8_t CONTROL,uint16_t motor_id)
{
    uint8_t data[3];
    data[0] = CONTROL;         // �̶������ֽڣ�������ͬ������
    data[1] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B
    fdcanx_send_data(hfdcan, motor_id, data, 2);
}

//target_x,y  Ŀ��λ��
//current_x,y ��ǰλ��
void can_sync_quanxiang_position_control_four_motors(double target_x, double target_y, double current_x, double current_y,double w) {
// �������е��ʹ����ͬ�ļ��ٶȵ�λ
// ����Ŀ��λ���뵱ǰλ��֮��Ĳ�ֵ
    can_message_t message;
    uint32_t position = 0;
    uint8_t move_sign = 0;
    double dx = target_x - current_x;
    double dy = target_y - current_y;
    // ����Ŀ��λ�õĽǶ�
    double theta = atan2(dy, dx);
    // ����Ŀ��λ�õľ���
    double distance = sqrt(dx * dx + dy * dy);
    double r = 38.00;
    double L = 17.50;
    // ����ƽ̨���ٶ�Ϊ�����ٶ� v �ͽ��ٶ� w
    double w1,w2,w3,w4;  // ���ٶ� (rad/s)
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
        // ����ÿ�����ӵ��ٶ�
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
    // ��4��8λ���ݺϲ�Ϊһ��32λ����
    uint32_t result = 0;

    // ��data3�������8λ
    result |= (uint32_t)data3 << 24;
    // ��data2�����м�ĸ�8λ
    result |= (uint32_t)data2 << 16;
    // ��data1�����м�ĵ�8λ
    result |= (uint32_t)data1 << 8;
    // ��data0�������8λ
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
    // �������
    float error = pid->setpoint - measured_value;

    // ������
    float Pout = pid->Kp * error;

    // ������
    pid->integral += error * dt;
    if (pid->integral > 50) {
        pid->integral = 50;
    } else if (pid->integral < -50) {
        pid->integral = -50;
    }
    float Iout = pid->Ki * pid->integral;

    // ΢����
    float derivative = (error - pid->last_error) / dt;
    float Dout = pid->Kd * derivative;

    // �����ϴ����
    pid->last_error = error;

    // ���������
    pid->output = Pout + Iout + Dout;

    // �޷�
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

    // ��ȡ��ǰʱ��
    current_time = (float)xTaskGetTickCount() * portTICK_PERIOD_MS;

    // ����ʱ���
    dt = current_time - last_time;

    // ����PID������
//    current_position = get_car_position();
    control_signal = PID_Update(&pid_controller, current_position, dt);

    // ����С���ٶ�
    can_sync_control_four_motors(&hfdcan1, control_signal, -control_signal, -control_signal, control_signal);

    // �����ϴ�ʱ��
    last_time = current_time;
}


















uint8_t task_adjust(position_order task)
{
    uint8_t task_type=0;
    if(task.x==0 && task.y==0)
    {
        task_type=task_angle;//���������������
    }
    if(task.x!=0 || task.y!=0)
    {
        task_type=task_move;//���������������
    }
    return task_type;
}

/**
 * @brief ͬ�������ĸ������ת�ٺͷ���
 *
 * @param speed1 ��һ�������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed2 �ڶ��������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed3 �����������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 * @param speed4 ���ĸ������ת�٣�int16_t ���ͣ�����ֵ��ʾת��
 */
void sync_ctrl_speed(int16_t speed1,int16_t speed2, int16_t speed3, int16_t speed4)
{
    uint8_t acceleration = 0XD0; // �������е��ʹ����ͬ�ļ��ٶȵ�λ
    uint8_t sync_flag = 1;     // ���ö��ͬ����־
    uint8_t data[4];               // ���ݻ�������׼������ͬ������
    speed1 = speed1;
    speed2 = -speed2;
    speed3 = -speed3;
    speed4 = speed4;
    acceleration=acc_get(speed1,300);
    // ���͵��1�ٶȿ�������

    motor_speed_control(&hfdcan1, MOTOR_ID_1, speed1, acceleration, sync_flag);
    osDelay(3);
//    // ���͵��2�ٶȿ�������
    motor_speed_control(&hfdcan1, MOTOR_ID_2, speed2, acceleration, sync_flag);
    osDelay(3);
    acceleration=acc_get(speed3,300);
    // ���͵��3�ٶȿ�������
    motor_speed_control(&hfdcan1, MOTOR_ID_3, speed3, acceleration, sync_flag);
    osDelay(3);
    // ���͵��4�ٶȿ�������
    motor_speed_control(&hfdcan1, MOTOR_ID_4, speed4, acceleration, sync_flag);
    osDelay(3);
    // �������ͬ����������
    data[0] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data[1] = 0x66;           // ͬ���˶������ֶ�
    data[2] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B
    fdcanx_send_data(&hfdcan1, 0x000, data, 3);// ʹ�� fdcanx_send_data ���Ͷ��ͬ������
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
uint8_t motor_speed_control(FDCAN_HandleTypeDef *hfdcan, uint16_t motor_id,
                                int16_t speed_rpm, uint8_t acceleration, uint8_t sync_flag) {
    uint8_t data[8];    // ���ݻ�����
    uint8_t checksum;    // У���ֽ�
    data[0] = 0xF6;     // �̶������ֽ�
    uint32_t num=0;
    // �����ٶ�ֵ�жϷ�����ֵΪ��ʱ�루CCW������ֵΪ˳ʱ�루CW��
    if (speed_rpm < 0) {
        data[1] = 0x00; // CW
        speed_rpm = -speed_rpm; // ����ֵת��Ϊ��ֵ
    } else {
        data[1] = 0x01; // CCW
    }
    // �ֽ��ٶ�Ϊ���ֽں͵��ֽ�
    data[2] = (speed_rpm << 8) & 0xFF; // �ٶȸ��ֽ�
    data[3] = speed_rpm & 0xFF;        // �ٶȵ��ֽ�
    data[4] = acceleration;             // ���ٶȵ�λ
    data[5] = sync_flag;                // ���ͬ����־��0x00 = �����ã�0x01 = ����
    // �̶�У���ֽ� 0x6B
    checksum = 0x6B;
    data[6] = checksum;
    // ʹ�� fdcanx_send_data ��������
    fdcanx_send_data(hfdcan, motor_id, data, 7);
    motor[(motor_id>>8)-1].speed_ctrl_status=0;
    while(motor[(motor_id>>8)-1].speed_ctrl_status==0)
    {
        
        if(num>0X2)
        {
            return 1;//��ʱ
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
 * @brief ��������ֹͣ������ָֹͣ��������˶�
 *
 * @param hfdcan ָ�� FDCAN_HandleTypeDef �ṹ���ָ�룬���� CAN ����ͨ��
 * @param motor_id ��Ҫֹͣ�ĵ���� ID
 * @return uint8_t ���ط���״̬��0 ��ʾ�ɹ�������ֵ��ʾ����
 */
uint8_t motor_stop() {
    uint8_t data[7];
    uint8_t data2[4];
    uint8_t checksum;

    // ������������
    data[0] = 0xF6;            
    data[1] = 0x01;            
    data[2] = 0x01;    
    data[3] = 0x00;            
    data[4] = 0x00;            
    data[5] = 0xD0;
    data[6] = 0x01;
    data2[0] = 0xFF;           // �̶������ֽڣ�������ͬ������
    data2[1] = 0x66;           // ͬ���˶������ֶ�
    data2[2] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B
    // �̶�У���ֽ� 0x6B
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
    // ʹ�� fdcanx_send_data ��������
    return fdcanx_send_data(&hfdcan1, 0X00, data2, 3);
}
int16_t read_motor_speed(FDCAN_HandleTypeDef *hfdcan,uint16_t motor_id)
{
    uint8_t data[3];
    data[0] = 0X35;         // �̶������ֽڣ�������ͬ������
    data[1] = 0x6B;           // У���ֽڣ��̶�Ϊ 0x6B
    fdcanx_send_data(hfdcan, motor_id, data, 2);
    osDelay(2);
    return motor[((motor_id>>8)-1)].speed;
}


