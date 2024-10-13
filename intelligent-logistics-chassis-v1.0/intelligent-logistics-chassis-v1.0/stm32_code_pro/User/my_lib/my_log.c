#include "my_log.h"

SemaphoreHandle_t print_mutex;


void MyLogInit(void)
{
    print_mutex = xSemaphoreCreateMutex();
}




