/*
 * @Author: dead-poem 2819731924@qq.com
 * @Date: 2022-10-08 14:29:09
 * @LastEditors: dead-poem 2819731924@qq.com
 * @LastEditTime: 2022-10-13 10:02:53
 * @FilePath: \stm32_foc_code\user\my_lib\vofa_communication.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%A
 */
#include "vofa_communication.h"

#include "main.h"
#include "usart.h"

/**
 * @description: 根据实际设备定义输出（串口）
 * @param {uint8_t*} addr
 * @param {uint8_t} byte_num
 * @return {*}
 */
static void _UartSend(uint8_t* addr, uint8_t byte_num)
{
    HAL_UART_Transmit(&huart1, addr, byte_num, 0xffff);
}

/// @brief 传输float类型到vofa上位机
/// @param ch_num 数据个数
/// @param addr 数据起始地址
void VofaFloatSend(uint8_t ch_num, float* addr)
{
    char tail[4] = {0x00, 0x00, 0x80, 0x7f};//协议帧尾

    _UartSend((uint8_t*)addr, sizeof(float) * ch_num);
    _UartSend(tail, 4);
}
