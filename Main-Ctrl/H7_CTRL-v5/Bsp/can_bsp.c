#include "can_bsp.h"
#include <string.h>
/**
************************************************************************
* @brief:      	bsp_can_init(void)
* @param:       void
* @retval:     	void
* @details:    	CAN ʹ��
************************************************************************
**/
void bsp_can_init(void)
{
	can_filter_init();
	HAL_FDCAN_Start(&hfdcan1);                               //����FDCAN
	HAL_FDCAN_Start(&hfdcan2);
	HAL_FDCAN_Start(&hfdcan3);
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	HAL_FDCAN_ActivateNotification(&hfdcan3, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}
/**
************************************************************************
* @brief:      	can_filter_init(void)
* @param:       void
* @retval:     	void
* @details:    	CAN�˲�����ʼ��
************************************************************************
**/
void can_filter_init(void)
{
	FDCAN_FilterTypeDef fdcan_filter;
	
	fdcan_filter.IdType = FDCAN_STANDARD_ID;                       //��׼ID
	fdcan_filter.FilterIndex = 0;                                  //�˲�������                   
	fdcan_filter.FilterType = FDCAN_FILTER_MASK;                   
	fdcan_filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;           //������0������FIFO0  
	fdcan_filter.FilterID1 = 0x00;                               
	fdcan_filter.FilterID2 = 0x00;

	HAL_FDCAN_ConfigFilter(&hfdcan1,&fdcan_filter); 		 				  //����ID2
	//�ܾ�����ƥ�䲻�ɹ��ı�׼ID����չID,������Զ��֡
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,FDCAN_REJECT,FDCAN_REJECT,FDCAN_REJECT_REMOTE,FDCAN_REJECT_REMOTE);
	HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO0, 1);
//	HAL_FDCAN_ConfigFifoWatermark(&hfdcan1, FDCAN_CFG_RX_FIFO1, 1);
//	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0);
}
/**
************************************************************************
* @brief:      	fdcanx_send_data(FDCAN_HandleTypeDef *hfdcan, uint16_t id, uint8_t *data, uint32_t len)
* @param:       hfdcan��FDCAN���
* @param:       id��CAN�豸ID
* @param:       data�����͵�����
* @param:       len�����͵����ݳ���
* @retval:     	void
* @details:    	��������
************************************************************************
**/
uint8_t fdcanx_send_data(hcan_t *hfdcan, uint16_t id, uint8_t *data, uint32_t len)
{	
    FDCAN_TxHeaderTypeDef fdcan_TxHeader;
    static uint8_t msgdata[10]={0};
    memcpy(msgdata,data,len);
    fdcan_TxHeader.Identifier=id;                      //32λID
    fdcan_TxHeader.IdType=FDCAN_STANDARD_ID;                  //��׼ID
    fdcan_TxHeader.TxFrameType=FDCAN_DATA_FRAME;              //����֡
    fdcan_TxHeader.DataLength=FDCAN_DLC_BYTES_8;              //���ݳ���
    fdcan_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;            
    fdcan_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;               //�ر������л�
    fdcan_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;                //��ͳ��CANģʽ
    fdcan_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;     //�޷����¼�
    fdcan_TxHeader.MessageMarker=0;                           
    
    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan,&fdcan_TxHeader,msgdata);
    
    return 1;
}
/**
************************************************************************
* @brief:      	fdcanx_receive(FDCAN_HandleTypeDef *hfdcan, uint8_t *buf)
* @param:       hfdcan��FDCAN���
* @param:       buf���������ݻ���
* @retval:     	���յ����ݳ���
* @details:    	��������
************************************************************************
**/
uint8_t fdcanx_receive(hcan_t *hfdcan, uint16_t *rec_id, uint8_t *buf)
{	
	FDCAN_RxHeaderTypeDef pRxHeader;
	uint8_t len;
	
	if(HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0, &pRxHeader, buf)==HAL_OK)
	{
		*rec_id = pRxHeader.Identifier;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_8)
			len = pRxHeader.DataLength;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_12)
			len = 12;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_16)
			len = 16;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_20)
			len = 20;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_24)
			len = 24;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_32)
			len = 32;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_48)
			len = 48;
		if(pRxHeader.DataLength<=FDCAN_DLC_BYTES_64)
			len = 64;
		
		return len;//��������
	}
	return 0;	
}



uint8_t rx_data1[8] = {0};
uint16_t rec_id1;
void fdcan1_rx_callback(void)
{
	fdcanx_receive(&hfdcan1, &rec_id1, rx_data1);
}
uint8_t rx_data2[8] = {0};
uint16_t rec_id2;
void fdcan2_rx_callback(void)
{
	fdcanx_receive(&hfdcan2, &rec_id2, rx_data2);
}
uint8_t rx_data3[8] = {0};
uint16_t rec_id3;
void fdcan3_rx_callback(void)
{
	fdcanx_receive(&hfdcan3, &rec_id3, rx_data3);
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if(hfdcan == &hfdcan1)
	{
		fdcan1_rx_callback();
	}
	if(hfdcan == &hfdcan2)
	{
		fdcan2_rx_callback();
	}
	if(hfdcan == &hfdcan3)
	{
		fdcan3_rx_callback();
	}
}











