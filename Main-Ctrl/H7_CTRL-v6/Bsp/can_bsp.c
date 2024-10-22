#include "can_bsp.h"
#include "typedef_user.h"
#include "cmsis_os.h"
#include <string.h>
/**
************************************************************************
* @brief:       bsp_can_init(void)
* @param:       void
* @retval:      void
* @details:     CAN 使能
************************************************************************
**/

extern QueueHandle_t can_queue;
void bsp_can_init(void)
{
    can_filter_init();
    HAL_FDCAN_Start(&hfdcan1);                               //开启FDCAN
    HAL_FDCAN_Start(&hfdcan2);
    HAL_FDCAN_Start(&hfdcan3);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_ActivateNotification(&hfdcan3, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}
/**
************************************************************************
* @brief:       can_filter_init(void)
* @param:       void
* @retval:      void
* @details:     CAN滤波器初始化
************************************************************************
**/
void can_filter_init(void)
{
    FDCAN_FilterTypeDef fdcan_filter;

    // 配置canfd1
    fdcan_filter.IdType = FDCAN_EXTENDED_ID;                       // 扩展ID
    fdcan_filter.FilterIndex = 0;                                  // 滤波器索引
    fdcan_filter.FilterType = FDCAN_FILTER_MASK;
    fdcan_filter.FilterConfig =
        FDCAN_FILTER_TO_RXFIFO0;           // 过滤器0关联到FIFO0
    fdcan_filter.FilterID1 =
        0x100;                                // 设置要接收的ID起始值 0x100
    fdcan_filter.FilterID2 =
        0x700;                                // 掩码为0x700，匹配范围为0x100到0xA00

    HAL_FDCAN_ConfigFilter(&hfdcan1,
                           &fdcan_filter);               // 配置canfd1滤波器

    // 配置canfd2
    HAL_FDCAN_ConfigFilter(&hfdcan2,
                           &fdcan_filter);               // 配置canfd2滤波器

    // 配置canfd3
    HAL_FDCAN_ConfigFilter(&hfdcan3,
                           &fdcan_filter);               // 配置canfd3滤波器

    // 全局过滤设置为：拒绝标准ID，接受扩展ID并存入FIFO0，拒绝远程帧
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan3, FDCAN_REJECT, FDCAN_ACCEPT_IN_RX_FIFO0,
                                 FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

    // 配置FIFO 0的水位线，当FIFO 0中有1条消息时触发中断
    HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 1);
    HAL_FDCAN_ConfigFifoWatermark(&hfdcan2, FDCAN_CFG_RX_FIFO0, 1);
    HAL_FDCAN_ConfigFifoWatermark(&hfdcan3, FDCAN_CFG_RX_FIFO0, 1);

    // 可根据需要激活接收中断
    // HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    // HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    // HAL_FDCAN_ActivateNotification(&hfdcan3, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

/**
************************************************************************
* @brief:       fdcanx_send_data(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *data, uint32_t len)
* @param:       hfdcan：FDCAN句柄
* @param:       id：CAN设备ID
* @param:       data：发送的数据
* @param:       len：发送的数据长度
* @retval:      void
* @details:     发送数据
************************************************************************
**/
uint8_t fdcanx_send_data(hcan_t *hfdcan, uint32_t id, uint8_t *data,
                         uint32_t len)
{

    FDCAN_TxHeaderTypeDef fdcan_TxHeader;
    static uint8_t msgdata[8] = {0};
    memcpy(msgdata, data, len);
    fdcan_TxHeader.Identifier = id;                    //32位ID
    fdcan_TxHeader.IdType = FDCAN_EXTENDED_ID;                //标准ID
    fdcan_TxHeader.TxFrameType = FDCAN_DATA_FRAME;            //数据帧
    fdcan_TxHeader.DataLength = len<<16;            //数据长度
    fdcan_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    fdcan_TxHeader.BitRateSwitch = FDCAN_BRS_OFF;             //关闭速率切换
    fdcan_TxHeader.FDFormat = FDCAN_CLASSIC_CAN;              //传统的CAN模式
    fdcan_TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;   //无发送事件
    fdcan_TxHeader.MessageMarker = 0;
    if( HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &fdcan_TxHeader, msgdata) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/**
************************************************************************
* @brief:       fdcanx_receive(FDCAN_HandleTypeDef *hfdcan, uint8_t *buf)
* @param:       hfdcan：FDCAN句柄
* @param:       buf：接收数据缓存
* @retval:      接收的数据长度
* @details:     接收数据
************************************************************************
**/

uint8_t fdcanx_receive(hcan_t *hfdcan, uint32_t *rec_id, uint8_t *buf)
{
    FDCAN_RxHeaderTypeDef pRxHeader;
    uint8_t len;
    can_message_t message;

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &pRxHeader, buf) == HAL_OK)
    {
        *rec_id = pRxHeader.Identifier;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_8)
            len = pRxHeader.DataLength;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_12)
            len = 12;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_16)
            len = 16;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_20)
            len = 20;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_24)
            len = 24;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_32)
            len = 32;
        if (pRxHeader.DataLength <= FDCAN_DLC_BYTES_48)
            len = 48;

//        for (uint8_t i = 0; i < len; i++)
//        {
//            if(buf[0] > 0X30 && buf[0] < 0X40 || (buf[0] == 0XFF))
//            {
//                message.data[i] = buf[i];

//            }

//        }
//        if(message.data[0] > 0X30 && message.data[0] < 0X40 || (message.data[0] == 0XFF))
//        {
//            message.id = (*rec_id << 8);
//            message.id = (*rec_id);
//        }
//        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//        xQueueSendFromISR(can_queue, &message, &xHigherPriorityTaskWoken);
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return len;//接收数据
    }
    return 0;
}
#define speed_post      0XF6
#define RT_position     0X36
#define RT_speed        0X35
extern Motor_status motor[4];
void rx_date_deal(uint32_t *rec_id, uint8_t *buf)
{
    switch(buf[0])
    {
        case speed_post:
            motor[((*rec_id>>8)-1)].speed_ctrl_status=1;
            break;
        case RT_speed:
            motor[((*rec_id>>8)-1)].speed=(buf[2]<<8)+buf[3];
            
            break;
        case RT_position:
            motor[((*rec_id>>8)-1)].position=(buf[2]<<24)+(buf[3]<<16)+(buf[4]<<8)+buf[5];
            
            break;
    }
}

uint8_t rx_data1[8] = {0};
uint32_t rec_id1;
void fdcan1_rx_callback(void)
{
    fdcanx_receive(&hfdcan1, &rec_id1, rx_data1);
    rx_date_deal(&rec_id1, rx_data1);
}
uint8_t rx_data2[8] = {0};
uint32_t rec_id2;
void fdcan2_rx_callback(void)
{
    fdcanx_receive(&hfdcan2, &rec_id2, rx_data2);
}
uint8_t rx_data3[8] = {0};
uint32_t rec_id3;
void fdcan3_rx_callback(void)
{
    fdcanx_receive(&hfdcan3, &rec_id3, rx_data3);
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (hfdcan == &hfdcan1)
    {
        fdcan1_rx_callback();
    }
    if (hfdcan == &hfdcan2)
    {
        fdcan2_rx_callback();
    }
    if (hfdcan == &hfdcan3)
    {
        fdcan3_rx_callback();
    }
}











