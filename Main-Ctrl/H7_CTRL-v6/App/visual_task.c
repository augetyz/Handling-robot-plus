#include "visual_task.h"
#include "cmsis_os.h"

/**
  * @brief  �Ӿ�����������ں���������ɫ����λУ׼��ɫ�鶨λУ׼
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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
