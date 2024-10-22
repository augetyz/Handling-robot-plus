#include "logic_task.h"
#include "cmsis_os.h"
#include "typedef_user.h"
#include "usb_date_task.h"


extern QueueHandle_t Move_task_queue;
extern SemaphoreHandle_t taskOK_Semaphore;
/**
  * @brief  逻辑处理任务入口函数，用于管理系统的逻辑运算或决策流程
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void LogicTask_Entry(void const * argument)
{
    position_order Move_order={0};

  /* Infinite loop */
//    xQueueSend(Move_task_queue,&Move_order,0);
  for(;;)
  {
	
    osDelay(1);
  }
}
void move_ctrl_logic(int x,int y,float angle)
{
    position_order Move_order={0};
    Move_order.x=x;
    Move_order.y=y;
    Move_order.angle=angle;
    xQueueSend(Move_task_queue,&Move_order,0);
    xSemaphoreTake(taskOK_Semaphore,portMAX_DELAY);
}
