#include "imu_task.h"
#include "bmi088driver.h"
#include "MahonyAHRS.h"
#include "tim.h"
#include "vofa.h"
#include <math.h>
#include "typedef_user.h"

#define DES_TEMP    40.0f
#define KP          100.f
#define KI          50.f
#define KD          10.f
#define MAX_OUT     500

float gyro[3] = {0.0f};
float acc[3] = {0.0f};
static float temp = 0.0f;

float imuQuat[4] = {0.0f};
float imuAngle[3] = {0.0f};

float out = 0;
float err = 0;
float err_l = 0;
float err_ll = 0;

void AHRS_init(float quat[4])
{
    quat[0] = 1.0f;
    quat[1] = 0.0f;
    quat[2] = 0.0f;
    quat[3] = 0.0f;

}


void GetAngle(float q[4], float *yaw, float *pitch, float *roll)
{
    *yaw = atan2f(2.0f*(q[0]*q[3]+q[1]*q[2]), 2.0f*(q[0]*q[0]+q[1]*q[1])-1.0f)*180/3.1415f;
    *pitch = asinf(-2.0f*(q[1]*q[3]-q[0]*q[2]))*180/3.1415f;
    *roll = atan2f(2.0f*(q[0]*q[1]+q[2]*q[3]),2.0f*(q[0]*q[0]+q[3]*q[3])-1.0f)*180/3.1415f;
}
float gyroBias[3] = {0.0f};  // ���ڴ洢�����ǵ���ƫֵ

void CalibrateGyroBias()
{
    // ����ϵͳ������ʱ�Ǿ�ֹ�ģ�ͨ����ζ�ȡ��������������ƽ������Ϊ��ƫֵ
    float gyroSum[3] = {0.0f};
    int samples = 1000;  // ��������
    for (int i = 0; i < samples; i++)
    {
        BMI088_read(gyro, acc, &temp);
        gyroSum[0] += gyro[0];
        gyroSum[1] += gyro[1];
        gyroSum[2] += gyro[2];
        osDelay(1);
    }
    gyroBias[0] = gyroSum[0] / samples;
    gyroBias[1] = gyroSum[1] / samples;
    gyroBias[2] = gyroSum[2] / samples;
}

void AHRS_update(float quat[4], float gyro[3], float accel[3])
{
    // ʹ��У׼�������������
    float correctedGyro[3];
    correctedGyro[0] = gyro[0] - gyroBias[0];
    correctedGyro[1] = gyro[1] - gyroBias[1];
    correctedGyro[2] = gyro[2] - gyroBias[2];

    MahonyAHRSupdateIMU(quat, correctedGyro[0], correctedGyro[1], correctedGyro[2], accel[0], accel[1], accel[2]);
}

/**
  * @brief  IMU������ں��������ڴ���IMU������������
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
  * @retval None
  */
void ImuTask_Entry(void const * argument)
{
    osDelay(10);
    
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    while(BMI088_init())
    {
        osDelay(100);
    }
    // У׼�����ǵ���ƫ
    CalibrateGyroBias();
    
    AHRS_init(imuQuat);
    /* Infinite loop */
    for(;;)
    {
        BMI088_read(gyro, acc, &temp);
        
        AHRS_update(imuQuat, gyro, acc);
        GetAngle(imuQuat, imuAngle + INS_YAW_ADDRESS_OFFSET, imuAngle + INS_PITCH_ADDRESS_OFFSET, imuAngle + INS_ROLL_ADDRESS_OFFSET);
        
        err_ll = err_l;
        err_l = err;
        err = DES_TEMP - temp;
        out = KP*err + KI*(err + err_l + err_ll) + KD*(err - err_l);
        if (out > MAX_OUT) out = MAX_OUT;
        if (out < 0) out = 0.f;
        htim3.Instance->CCR4 = (uint16_t)out;
        
        vofa_send_data(0, imuAngle[1]);
        vofa_send_data(1, imuAngle[2]);
        vofa_send_data(2, imuAngle[0]);
        vofa_sendframetail();
        
        osDelay(1);
    }
}


