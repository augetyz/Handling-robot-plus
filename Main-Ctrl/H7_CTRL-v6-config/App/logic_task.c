#include "logic_task.h"
#include "cmsis_os.h"

/**
  * @brief  逻辑处理任务入口函数，用于管理系统的逻辑运算或决策流程
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void LogicTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}

