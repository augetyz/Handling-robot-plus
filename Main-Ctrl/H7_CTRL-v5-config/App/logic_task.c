#include "logic_task.h"
#include "cmsis_os.h"

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

