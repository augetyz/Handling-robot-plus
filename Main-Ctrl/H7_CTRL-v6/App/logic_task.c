#include "logic_task.h"
#include "cmsis_os.h"
#include "typedef_user.h"

extern QueueHandle_t Move_task_queue;
extern QueueHandle_t Move_task_queue;
/**
  * @brief  逻辑处理任务入口函数，用于管理系统的逻辑运算或决策流程
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void LogicTask_Entry(void const * argument)
{
    position_order Move_order={0};
    Move_order.x=000;
    Move_order.y=1200;
    Move_order.angle=0;
  /* Infinite loop */
//    xQueueSend(Move_task_queue,&Move_order,0);
  for(;;)
  {
		
    osDelay(1);
  }
}

