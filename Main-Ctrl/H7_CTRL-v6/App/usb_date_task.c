#include "usb_date_task.h"
#include "typedef_user.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include <string.h>  // 为了使用 memcpy
extern SemaphoreHandle_t usbGetDataSemaphore;
extern QueueHandle_t qrQueue;
extern uint16_t USB_RX_DATA_SIZE;
extern uint8_t UserRxBufferHS[2048];
QR_code_date USB_date_deal_QR(uint8_t *date);
color_ring_position USB_date_deal_CR(uint8_t *date);
color_block_position USB_date_deal_CB(uint8_t *date);

/**********************************************************************
通信协议数据帧格式：
帧头：    1 字节，用于标识帧的开始。0xAA。
帧类型：  1 字节，表示数据的类型（QR 码：0xCC、色环位置信息：0xDD、色块位置信息：0xEE）。
数据长度：1 字节，表示数据部分的长度。
数据内容：QR 码：六字节描述二维码 色环位置信息、色块位置信息：X坐标2字节 Y坐标2字节 颜色1字节
帧尾：    1 字节， 用于标识帧的结束。0xBB。
***********************************************************************/

/**
  * @brief  USB数据处理任务入口函数，用于接收和处理通过USB传输的数据
  * @param  argument: 传递给任务的参数（未使用）
  * @retval None
  */
void UsbDataTask_Entry(void const * argument)
{

    QR_code_date qrCodeData;
    color_ring_position ringPos;
    color_block_position blockPos;

    /* Infinite loop */
    for(;;)
    {
        // 等待信号量，当接收到数据时继续执行
        if (xSemaphoreTake(usbGetDataSemaphore, portMAX_DELAY) == pdTRUE)
        {
            //  USB_RX_DATA_SIZE 是 USB 接收的数据大小，UserRxBufferHS 是接收到的数据缓冲区
            if (USB_RX_DATA_SIZE > 0)
            {
                // 判断帧头是否为 0xAA 和帧尾是否为 0xBB
                if (UserRxBufferHS[0] == 0xAA && UserRxBufferHS[USB_RX_DATA_SIZE - 1] == 0xBB)
                {
                    // 获取帧类型
                    uint8_t frameType = UserRxBufferHS[1];

                    switch (frameType)
                    {
                        case 0xCC:  // QR 码数据
                            if (USB_RX_DATA_SIZE == 10) // 帧头(1)+帧类型(1)+长度(1)+数据(6)+帧尾(1) = 10字节
                            {
                                qrCodeData = USB_date_deal_QR(UserRxBufferHS);
                                // 发送 QR 码数据
                                xQueueSend(qrQueue, &qrCodeData, portMAX_DELAY);
                            }
                            break;

                        case 0xDD:  // 色环位置信息
                            if (USB_RX_DATA_SIZE == 9) // 帧头(1)+帧类型(1)+长度(1)+数据(5)+帧尾(1) = 9字节
                            {
                                ringPos = USB_date_deal_CR(UserRxBufferHS);
                                // 处理色环位置数据
                                
                            }
                            break;

                        case 0xEE:  // 色块位置信息
                            if (USB_RX_DATA_SIZE == 9) // 帧头(1)+帧类型(1)+长度(1)+数据(5)+帧尾(1) = 9字节
                            {
                                blockPos = USB_date_deal_CB(UserRxBufferHS);
                                // 处理色块位置数据
                                
                            }
                            break;

                        default:
                            // 处理未知帧类型
                            
                            break;
                    }
                }

                // 清空 USB 接收缓冲区和数据大小
                memset(UserRxBufferHS, 0, sizeof(UserRxBufferHS));
                USB_RX_DATA_SIZE = 0;
            }
        }
    }
}

/**
 * @brief  解析USB接收到的QR码数据帧，提取QR码信息。
 * @param  date: 指向接收到的USB数据的指针。
 * @retval QR_code_date: 返回包含QR码信息的结构体。
 */
QR_code_date USB_date_deal_QR(uint8_t *date)
{
    QR_code_date qrData;
    
    // 确认帧头是否正确
    if (date[0] == 0xAA && date[1] == 0xCC && date[2] == 6 && date[9] == 0xBB)
    {
        // 复制6字节的QR码数据到结构体中
        memcpy(qrData.QR_date, &date[3], 6);
    }
    else
    {
        // 如果帧不符合协议，清空数据
        memset(qrData.QR_date, 0, 6);
    }

    return qrData;
}

/**
 * @brief  解析USB接收到的色环位置信息帧，提取色环坐标和颜色信息。
 * @param  date: 指向接收到的USB数据的指针。
 * @retval color_ring_position: 返回包含色环位置和颜色信息的结构体。
 */
color_ring_position USB_date_deal_CR(uint8_t *date)
{
    color_ring_position ringPos;

    // 确认帧头是否正确
    if (date[0] == 0xAA && date[1] == 0xDD && date[2] == 5 && date[8] == 0xBB)
    {
        // 解析x坐标（2字节）
        ringPos.x = (date[3] << 8) | date[4];
        // 解析y坐标（2字节）
        ringPos.y = (date[5] << 8) | date[6];
        // 解析颜色（1字节）
        ringPos.color = date[7];
    }
    else
    {
        // 如果帧不符合协议，初始化数据
        ringPos.x = 0;
        ringPos.y = 0;
        ringPos.color = 0;
    }

    return ringPos;
}
/**
 * @brief  处理接收到的 USB 数据并解析为颜色块位置信息。
 * @param  date: 指向接收到的 USB 数据的指针。
 * @retval color_block_position: 返回解析后的颜色块位置信息，包含坐标和颜色。
 */
color_block_position USB_date_deal_CB(uint8_t *date)
{
    color_block_position blockPos;

    // 确认帧头是否正确
    if (date[0] == 0xAA && date[1] == 0xEE && date[2] == 5 && date[8] == 0xBB)
    {
        // 解析x坐标（2字节）
        blockPos.x = (date[3] << 8) | date[4];
        // 解析y坐标（2字节）
        blockPos.y = (date[5] << 8) | date[6];
        // 解析颜色（1字节）
        blockPos.color = date[7];
    }
    else
    {
        // 如果帧不符合协议，初始化数据
        blockPos.x = 0;
        blockPos.y = 0;
        blockPos.color = 0;
    }

    return blockPos;
}
//控制opencv扫描二维码功能运行状态
//1：使能 0：失能
void QRscan_enable(uint8_t status)
{
    uint8_t buf='3';
    if(status==0)
    {
        buf='4';
        CDC_Transmit_HS(&buf,1);
    } 
    else
    {
        buf='3';
        CDC_Transmit_HS(&buf,1);
    }
    
}
//控制opencv识别色环或者识别色块
//1：色块 0：色环
void Colorscan_enable(uint8_t status)
{
    uint8_t buf='0';
    if(status==0)
    {
        buf='2';
        CDC_Transmit_HS(&buf,1);
    } 
    else
    {
        buf='1';
        CDC_Transmit_HS(&buf,1);
    }
    
}

