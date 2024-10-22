/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "typedef_user.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
SemaphoreHandle_t usbGetDataSemaphore;
SemaphoreHandle_t taskOK_Semaphore;
QueueHandle_t qrQueue;
QueueHandle_t can_queue;
QueueHandle_t Move_task_queue;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId KeyTaskHandle;
osThreadId LcdTaskHandle;
osThreadId ImuTaskHandle;
osThreadId FunTestHandle;
osThreadId MoveTaskHandle;
osThreadId ArmCtrlTaskHandle;
osThreadId UsbDataTaskHandle;
osThreadId OrderTaskHandle;
osThreadId VisualTaskHandle;
osThreadId LogicTaskHandle;

uint32_t KeyTaskBuffer[128];
uint32_t LcdTaskBuffer[256];
uint32_t ImuTaskBuffer[1024];
uint32_t FunTestBuffer[128];
uint32_t MoveTaskBuffer[512];
uint32_t ArmCtrlTaskBuffer[512];
uint32_t UsbDataTaskBuffer[1024];
uint32_t OrderTaskBuffer[512];
uint32_t VisualTaskBuffer[512];
uint32_t LogicTaskBuffer[512];

osStaticThreadDef_t KeyTaskControlBlock;
osStaticThreadDef_t LcdTaskControlBlock;
osStaticThreadDef_t ImuTaskControlBlock;
osStaticThreadDef_t FunTestControlBlock;
osStaticThreadDef_t MoveTaskControlBlock;
osStaticThreadDef_t ArmCtrlTaskControlBlock;
osStaticThreadDef_t UsbDataTaskControlBlock;
osStaticThreadDef_t OrderTaskControlBlock;
osStaticThreadDef_t VisualTaskControlBlock;
osStaticThreadDef_t LogicTaskControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void KeyTask_Entry(void const * argument);
void LcdTask_Entry(void const * argument);
void ImuTask_Entry(void const * argument);
void FunTest_Entry(void const * argument);
void MoveTask_Entry(void const * argument);
void ArmCtrlTask_Entry(void const * argument);
void UsbDataTask_Entry(void const * argument);
void OrderTask_Entry(void const * argument);
void VisualTask_Entry(void const * argument);
void LogicTask_Entry(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
    *ppxTimerTaskStackBuffer = &xTimerStack[0];
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
    /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    // 创建USB数据接受中断二值信号量
    usbGetDataSemaphore = xSemaphoreCreateBinary();
    taskOK_Semaphore    = xSemaphoreCreateBinary();
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    //创建消息队列，最多存储 1 个 QR_code_date 数据
    qrQueue = xQueueCreate(1, sizeof(QR_code_date));
	can_queue = xQueueCreate(16, sizeof(can_message_t));
    Move_task_queue = xQueueCreate(1, sizeof(position_order));  
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* definition and creation of KeyTask */
    osThreadStaticDef(KeyTask, KeyTask_Entry, osPriorityRealtime, 0, 128, KeyTaskBuffer, &KeyTaskControlBlock);
    KeyTaskHandle = osThreadCreate(osThread(KeyTask), NULL);

    /* definition and creation of LcdTask */
    osThreadStaticDef(LcdTask, LcdTask_Entry, osPriorityNormal, 0, 256, LcdTaskBuffer, &LcdTaskControlBlock);
    LcdTaskHandle = osThreadCreate(osThread(LcdTask), NULL);

    /* definition and creation of ImuTask */
    osThreadStaticDef(ImuTask, ImuTask_Entry, osPriorityHigh, 0, 1024, ImuTaskBuffer, &ImuTaskControlBlock);
    ImuTaskHandle = osThreadCreate(osThread(ImuTask), NULL);

    /* definition and creation of FunTest */
    osThreadStaticDef(FunTest, FunTest_Entry, osPriorityRealtime, 0, 128, FunTestBuffer, &FunTestControlBlock);
    FunTestHandle = osThreadCreate(osThread(FunTest), NULL);

    /* definition and creation of MoveTask */
    osThreadStaticDef(MoveTask, MoveTask_Entry, osPriorityHigh, 0, 512, MoveTaskBuffer, &MoveTaskControlBlock);
    MoveTaskHandle = osThreadCreate(osThread(MoveTask), NULL);

    /* definition and creation of ArmCtrlTask */
    osThreadStaticDef(ArmCtrlTask, ArmCtrlTask_Entry, osPriorityHigh, 0, 512, ArmCtrlTaskBuffer, &ArmCtrlTaskControlBlock);
    ArmCtrlTaskHandle = osThreadCreate(osThread(ArmCtrlTask), NULL);

    /* definition and creation of UsbDataTask */
    osThreadStaticDef(UsbDataTask, UsbDataTask_Entry, osPriorityNormal, 0, 1024, UsbDataTaskBuffer, &UsbDataTaskControlBlock);
    UsbDataTaskHandle = osThreadCreate(osThread(UsbDataTask), NULL);

    /* definition and creation of OrderTask */
    osThreadStaticDef(OrderTask, OrderTask_Entry, osPriorityNormal, 0, 512, OrderTaskBuffer, &OrderTaskControlBlock);
    OrderTaskHandle = osThreadCreate(osThread(OrderTask), NULL);

    /* definition and creation of VisualTask */
    osThreadStaticDef(VisualTask, VisualTask_Entry, osPriorityNormal, 0, 512, VisualTaskBuffer, &VisualTaskControlBlock);
    VisualTaskHandle = osThreadCreate(osThread(VisualTask), NULL);

    /* definition and creation of LogicTask */
    osThreadStaticDef(LogicTask, LogicTask_Entry, osPriorityAboveNormal, 0, 512, LogicTaskBuffer, &LogicTaskControlBlock);
    LogicTaskHandle = osThreadCreate(osThread(LogicTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
    /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_MoveTask_Entry */
/**
* @brief Function implementing the MoveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MoveTask_Entry */
__weak void MoveTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* USER CODE BEGIN Header_ArmCtrlTask_Entry */
/**
* @brief Function implementing the ArmCtrlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ArmCtrlTask_Entry */
__weak void ArmCtrlTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* USER CODE BEGIN Header_UsbDataTask_Entry */
/**
* @brief Function implementing the UsbDataTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UsbDataTask_Entry */
__weak void UsbDataTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* USER CODE BEGIN Header_OrderTask_Entry */
/**
* @brief Function implementing the OrderTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_OrderTask_Entry */
__weak void OrderTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* USER CODE BEGIN Header_VisualTask_Entry */
/**
* @brief Function implementing the VisualTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_VisualTask_Entry */
__weak void VisualTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* USER CODE BEGIN Header_LogicTask_Entry */
/**
* @brief Function implementing the LogicTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LogicTask_Entry */
__weak void LogicTask_Entry(void const * argument)
{
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
