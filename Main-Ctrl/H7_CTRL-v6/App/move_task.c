#include "move_task.h"
#include "cmsis_os.h"
#include "can_bsp.h"
#include "bsp_motor_ctrl.h"
#include "typedef_user.h"
#include "ws2812.h"
/**
  * @brief  �ƶ�����������ں��������ڴ�������˵��ƶ�����
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
  * @retval None
  */
extern float imuAngle[3];
extern QueueHandle_t can_queue;
extern QueueHandle_t Move_task_queue;
extern SemaphoreHandle_t taskOK_Semaphore;

enum {
    CAR_START,
    CAR_START1,
    CAR_START2,
    CAR_START3,
    CAR_STOP,
    CAR_BACK,
    CAR_STABLIZE
// ���ڱ��ö�ٵ����ֵ������ѭ���������߼�
} ;
extern uint8_t key_mid;
uint8_t CAR = CAR_START;
PIDController pid_controller;
void MoveTask_Entry(void const * argument)
{
    uint32_t position;
    uint8_t move_sign;
    uint8_t move_later;
    uint8_t motor_sign = 0;
    uint8_t tx_data[4] = {0X36,0X6B};
    
    can_message_t message;
    /**��ʼ������**/
    position_order Move_order={0};
    uint8_t task_status=0;
    
    /**��ʼ������**/
    bsp_can_init();
    PID_Init(&pid_controller, 5, 0.01, 0, 0);
    /* Infinite loop */
    osDelay(2000);

//    
//    can_sync_position_control_four_motors(&hfdcan1,200,200, 100, 100,1200,1200,600,600);
    for(;;)
    {
        
        if(task_status==task_nothing)
        {
            if (xQueueReceive(Move_task_queue,&Move_order,0) == pdTRUE)
            {
                motor_clean();                
                task_status=task_adjust(Move_order);                               
            }
        }
        if(task_status == task_OK)
        {
            //����Move_task��ɱ�־
            xSemaphoreGive(taskOK_Semaphore);

            task_status=task_nothing;
        }
        if(task_status==task_move)
        {
            if(move_ctrl_dietance(Move_order.x ,Move_order.y,160)==0)
            {
                task_status = task_OK;
            }
        }
        if(task_status==task_angle)
        {
            
        }
        can_read_four_motors(&hfdcan1,0X36,0X100);
        osDelay(3);
        can_read_four_motors(&hfdcan1,0X36,0X300);
        osDelay(3);
        can_read_four_motors(&hfdcan1,0X35,0X100);
        osDelay(3);
        can_read_four_motors(&hfdcan1,0X35,0X300);
        osDelay(3);
    }
}



