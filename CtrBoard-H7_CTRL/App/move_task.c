#include "move_task.h"
#include "cmsis_os.h"

/**
  * @brief  �ƶ�����������ں��������ڴ�������˵��ƶ�����
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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
