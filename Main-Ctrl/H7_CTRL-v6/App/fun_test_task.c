#include "fun_test_task.h"
#include "usart.h"
#include "main.h"
#include "tim.h"
#include "fdcan.h"
#include "ws2812.h"
#include "can_bsp.h"
#include <string.h>

#define U1_RX_LEN   5
#define U5_RX_LEN   5
#define U7_RX_LEN   5
#define U10_RX_LEN   5

uint8_t u1Rx[U1_RX_LEN];
uint8_t u1RxFlag = 0;

uint8_t u5Rx[U5_RX_LEN];
uint8_t u5RxFlag = 0;

uint8_t u7Rx[U7_RX_LEN];
uint8_t u7RxFlag = 0;

uint8_t u10Rx[U10_RX_LEN];
uint8_t u10RxFlag = 0;


uint8_t tx_data[8] = {0,1,2,3,4,5,6,7};


/* USER CODE BEGIN Header_FunTest_Entry */
/**
  * @brief  ���ܲ���������ں���������ϵͳ�����Բ���
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
  * @retval None
  */
void FunTest_Entry(void const * argument)
{
    /* USER CODE BEGIN FunTest_Entry */
    can_bsp_init();
    HAL_UART_Receive_IT(&huart1, u1Rx, U1_RX_LEN);
//    HAL_UART_Receive_IT(&huart5, u5Rx, U5_RX_LEN);
    HAL_UART_Receive_IT(&huart7, u7Rx, U7_RX_LEN);
    HAL_UART_Receive_IT(&huart10, u10Rx, U10_RX_LEN);
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    
    TIM1->CCR1 = 5000;                      // 50%ռ�ձ�
    TIM1->CCR3 = 5000;                      // 50%ռ�ձ�
    TIM1->CCR1 = 5000;                      // 50%ռ�ձ�
    TIM1->CCR3 = 5000;                      // 50%ռ�ձ�
    /* Infinite loop */
    for(;;)
    {      
//        fdcanx_send_data(&hfdcan1, 0x520, tx_data, 8);
//        osDelay(1);
//        fdcanx_send_data(&hfdcan2, 0x520, tx_data, 8);
//        osDelay(1);
//        fdcanx_send_data(&hfdcan3, 0x50, tx_data, 8);
        osDelay(10);
    }
    /* USER CODE END FunTest_Entry */
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (memcmp(u1Rx, "test\n", U1_RX_LEN) == 0)
        {
            u1RxFlag = 1;
        }
        HAL_UART_Receive_IT(&huart1, u1Rx, U1_RX_LEN);
    }
    else if (huart == &huart7)
    {
        if (memcmp(u7Rx, "test\n", U7_RX_LEN) == 0)
        {
            u7RxFlag = 1;
        }
        HAL_UART_Receive_IT(&huart7, u7Rx, U7_RX_LEN);
    }
    else if (huart == &huart10)
    {
        if (memcmp(u10Rx, "test\n", U10_RX_LEN) == 0)
        {
            u10RxFlag = 1;
        }
        HAL_UART_Receive_IT(&huart10, u10Rx, U10_RX_LEN);
    }
//    else if (huart == &huart5)
//    {
//        if (memcmp(u5Rx, "test\n", U5_RX_LEN) == 0)
//        {
//            u5RxFlag = 1;
//        }
//        HAL_UART_Receive_IT(&huart5, u5Rx, U5_RX_LEN);
//    }
}
