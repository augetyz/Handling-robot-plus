#include "arm_ctrl_task.h"
#include "cmsis_os.h"

/**
  * @brief  ��е�ۿ���������ں��������ڿ��ƻ�е�۵�λ�úͶ���
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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

