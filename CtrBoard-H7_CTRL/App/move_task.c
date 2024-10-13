#include "move_task.h"
#include "cmsis_os.h"

void MoveTask_Entry(void const * argument)
{
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
}
