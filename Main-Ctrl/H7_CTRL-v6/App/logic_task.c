#include "logic_task.h"
#include "cmsis_os.h"
#include "typedef_user.h"
#include "usb_date_task.h"


extern QueueHandle_t Move_task_queue;
extern SemaphoreHandle_t taskOK_Semaphore;
/**
  * @brief  �߼�����������ں��������ڹ���ϵͳ���߼�������������
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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
