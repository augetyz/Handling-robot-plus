#include "logic_task.h"
#include "cmsis_os.h"


extern QueueHandle_t Move_task_queue;

/**
  * @brief  �߼�����������ں��������ڹ���ϵͳ���߼�������������
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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

