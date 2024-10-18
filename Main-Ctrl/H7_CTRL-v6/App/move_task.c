#include "move_task.h"
#include "cmsis_os.h"
#include "can_bsp.h"
#include "bsp_motor_ctrl.h"
/**
  * @brief  移动控制任务入口函数，用于处理机器人的移动控制
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void MoveTask_Entry(void const * argument)
{
  /* Infinite loop */
    uint8_t tx_data[8] = {0XF6,0X01,0X00,0XA0,0X0A,0X00,0X6B};
    bsp_can_init();
    
    for(;;)                                                                                                                                                                    
    {
//        can_motor_speed_control(&hfdcan1,1,0X100,0,0);
//        can_motor_speed_control(&hfdcan3,1,500,0,0);
        fdcanx_send_data(&hfdcan1,0x100,tx_data, 7);
        osDelay(100);
        fdcanx_send_data(&hfdcan1,0x200,tx_data, 7);
        osDelay(100);
        fdcanx_send_data(&hfdcan1,0x300,tx_data, 7);
        osDelay(100);
        fdcanx_send_data(&hfdcan1,0x400,tx_data, 7);
        osDelay(100);
        
        osDelay(100);
    }
}


