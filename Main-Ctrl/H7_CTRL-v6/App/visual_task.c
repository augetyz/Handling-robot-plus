#include "visual_task.h"
#include "cmsis_os.h"

/**
  * @brief  视觉处理任务入口函数，用于色环定位校准和色块定位校准
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void VisualTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
