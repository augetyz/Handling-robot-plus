#include "move_task.h"
#include "cmsis_os.h"
#include "can_bsp.h"
#include "bsp_motor_ctrl.h"
/**
  * @brief  �ƶ�����������ں��������ڴ�������˵��ƶ�����
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
  * @retval None
  */
void MoveTask_Entry(void const * argument)
{
  /* Infinite loop */
    can_motor_enable(&hfdcan1,1,1);
    can_motor_speed_control(&hfdcan1,1,500,0,0);
    for(;;)
    {
        can_motor_speed_control(&hfdcan1,1,500,0,0);
        can_motor_speed_control(&hfdcan3,1,500,0,0);
        osDelay(1000);
    }
}


