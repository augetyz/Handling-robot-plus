#include "order_task.h"
#include "cmsis_os.h"

void OrderTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
