#include "usb_date_task.h"
#include "typedef_user.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include <string.h>  // Ϊ��ʹ�� memcpy
extern SemaphoreHandle_t usbGetDataSemaphore;
extern QueueHandle_t qrQueue;
extern uint16_t USB_RX_DATA_SIZE;
extern uint8_t UserRxBufferHS[2048];
QR_code_date USB_date_deal_QR(uint8_t *date);
color_ring_position USB_date_deal_CR(uint8_t *date);
color_block_position USB_date_deal_CB(uint8_t *date);

/**********************************************************************
ͨ��Э������֡��ʽ��
֡ͷ��    1 �ֽڣ����ڱ�ʶ֡�Ŀ�ʼ��0xAA��
֡���ͣ�  1 �ֽڣ���ʾ���ݵ����ͣ�QR �룺0xCC��ɫ��λ����Ϣ��0xDD��ɫ��λ����Ϣ��0xEE����
���ݳ��ȣ�1 �ֽڣ���ʾ���ݲ��ֵĳ��ȡ�
�������ݣ�QR �룺���ֽ�������ά�� ɫ��λ����Ϣ��ɫ��λ����Ϣ��X����2�ֽ� Y����2�ֽ� ��ɫ1�ֽ�
֡β��    1 �ֽڣ� ���ڱ�ʶ֡�Ľ�����0xBB��
***********************************************************************/

/**
  * @brief  USB���ݴ���������ں��������ڽ��պʹ���ͨ��USB���������
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
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
        // �ȴ��ź����������յ�����ʱ����ִ��
        if (xSemaphoreTake(usbGetDataSemaphore, portMAX_DELAY) == pdTRUE)
        {
            //  USB_RX_DATA_SIZE �� USB ���յ����ݴ�С��UserRxBufferHS �ǽ��յ������ݻ�����
            if (USB_RX_DATA_SIZE > 0)
            {
                // �ж�֡ͷ�Ƿ�Ϊ 0xAA ��֡β�Ƿ�Ϊ 0xBB
                if (UserRxBufferHS[0] == 0xAA && UserRxBufferHS[USB_RX_DATA_SIZE - 1] == 0xBB)
                {
                    // ��ȡ֡����
                    uint8_t frameType = UserRxBufferHS[1];

                    switch (frameType)
                    {
                        case 0xCC:  // QR ������
                            if (USB_RX_DATA_SIZE == 10) // ֡ͷ(1)+֡����(1)+����(1)+����(6)+֡β(1) = 10�ֽ�
                            {
                                qrCodeData = USB_date_deal_QR(UserRxBufferHS);
                                // ���� QR ������
                                xQueueSend(qrQueue, &qrCodeData, portMAX_DELAY);
                            }
                            break;

                        case 0xDD:  // ɫ��λ����Ϣ
                            if (USB_RX_DATA_SIZE == 9) // ֡ͷ(1)+֡����(1)+����(1)+����(5)+֡β(1) = 9�ֽ�
                            {
                                ringPos = USB_date_deal_CR(UserRxBufferHS);
                                // ����ɫ��λ������
                                
                            }
                            break;

                        case 0xEE:  // ɫ��λ����Ϣ
                            if (USB_RX_DATA_SIZE == 9) // ֡ͷ(1)+֡����(1)+����(1)+����(5)+֡β(1) = 9�ֽ�
                            {
                                blockPos = USB_date_deal_CB(UserRxBufferHS);
                                // ����ɫ��λ������
                                
                            }
                            break;

                        default:
                            // ����δ֪֡����
                            
                            break;
                    }
                }

                // ��� USB ���ջ����������ݴ�С
                memset(UserRxBufferHS, 0, sizeof(UserRxBufferHS));
                USB_RX_DATA_SIZE = 0;
            }
        }
    }
}

/**
 * @brief  ����USB���յ���QR������֡����ȡQR����Ϣ��
 * @param  date: ָ����յ���USB���ݵ�ָ�롣
 * @retval QR_code_date: ���ذ���QR����Ϣ�Ľṹ�塣
 */
QR_code_date USB_date_deal_QR(uint8_t *date)
{
    QR_code_date qrData;
    
    // ȷ��֡ͷ�Ƿ���ȷ
    if (date[0] == 0xAA && date[1] == 0xCC && date[2] == 6 && date[9] == 0xBB)
    {
        // ����6�ֽڵ�QR�����ݵ��ṹ����
        memcpy(qrData.QR_date, &date[3], 6);
    }
    else
    {
        // ���֡������Э�飬�������
        memset(qrData.QR_date, 0, 6);
    }

    return qrData;
}

/**
 * @brief  ����USB���յ���ɫ��λ����Ϣ֡����ȡɫ���������ɫ��Ϣ��
 * @param  date: ָ����յ���USB���ݵ�ָ�롣
 * @retval color_ring_position: ���ذ���ɫ��λ�ú���ɫ��Ϣ�Ľṹ�塣
 */
color_ring_position USB_date_deal_CR(uint8_t *date)
{
    color_ring_position ringPos;

    // ȷ��֡ͷ�Ƿ���ȷ
    if (date[0] == 0xAA && date[1] == 0xDD && date[2] == 5 && date[8] == 0xBB)
    {
        // ����x���꣨2�ֽڣ�
        ringPos.x = (date[3] << 8) | date[4];
        // ����y���꣨2�ֽڣ�
        ringPos.y = (date[5] << 8) | date[6];
        // ������ɫ��1�ֽڣ�
        ringPos.color = date[7];
    }
    else
    {
        // ���֡������Э�飬��ʼ������
        ringPos.x = 0;
        ringPos.y = 0;
        ringPos.color = 0;
    }

    return ringPos;
}
/**
 * @brief  ������յ��� USB ���ݲ�����Ϊ��ɫ��λ����Ϣ��
 * @param  date: ָ����յ��� USB ���ݵ�ָ�롣
 * @retval color_block_position: ���ؽ��������ɫ��λ����Ϣ�������������ɫ��
 */
color_block_position USB_date_deal_CB(uint8_t *date)
{
    color_block_position blockPos;

    // ȷ��֡ͷ�Ƿ���ȷ
    if (date[0] == 0xAA && date[1] == 0xEE && date[2] == 5 && date[8] == 0xBB)
    {
        // ����x���꣨2�ֽڣ�
        blockPos.x = (date[3] << 8) | date[4];
        // ����y���꣨2�ֽڣ�
        blockPos.y = (date[5] << 8) | date[6];
        // ������ɫ��1�ֽڣ�
        blockPos.color = date[7];
    }
    else
    {
        // ���֡������Э�飬��ʼ������
        blockPos.x = 0;
        blockPos.y = 0;
        blockPos.color = 0;
    }

    return blockPos;
}
//����opencvɨ���ά�빦������״̬
//1��ʹ�� 0��ʧ��
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
//����opencvʶ��ɫ������ʶ��ɫ��
//1��ɫ�� 0��ɫ��
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

