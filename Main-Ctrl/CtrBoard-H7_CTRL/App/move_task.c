#include "move_task.h"
#include "cmsis_os.h"

/**
  * @brief  移动控制任务入口函数，用于处理机器人的移动控制
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void MoveTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}

void motor_ctrl(uint8_t id,int16_t speed)
{
    
}


