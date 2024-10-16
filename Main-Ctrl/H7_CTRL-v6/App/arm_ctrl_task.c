#include "arm_ctrl_task.h"
#include "cmsis_os.h"

/**
  * @brief  机械臂控制任务入口函数，用于控制机械臂的位置和动作
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void ArmCtrlTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}

