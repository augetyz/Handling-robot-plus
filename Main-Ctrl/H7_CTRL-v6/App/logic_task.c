#include "logic_task.h"
#include "cmsis_os.h"
#include "typedef_user.h"
#include "usb_date_task.h"
#include "lcd.h"


extern QueueHandle_t Move_task_queue;
extern SemaphoreHandle_t taskOK_Semaphore;
/**
  * @brief  �߼�����������ں��������ڹ���ϵͳ���߼�������������
  * @param  argument: ���ݸ�����Ĳ�����δʹ�ã�
  * @retval None
  */
void LogicTask_Entry(void const * argument)
{
    position_order Move_order= {0};

    /* Infinite loop */
//    xQueueSend(Move_task_queue,&Move_order,0);
    QRscan_enable(1);   //����ǰ������ά��ɨ��
    Colorscan_enable(1);//����ǰ����ɫ��ɨ��
    for(;;)
    {
        move_ctrl_logic(100,1000,0);
//        QR_scan_logic();
        move_ctrl_logic(0,0,0);//�������Ϊ0��
        Colorscan_enable(1);//����ǰ����ɫ��ɨ��
        move_ctrl_logic(0,950,0);
        move_ctrl_logic(0,0,0);//�������Ϊ0��
        for(;;)//��������
           osDelay(1000);
    }
}
void move_ctrl_logic(int x,int y,float angle)
{
    position_order Move_order= {0};
    Move_order.x=x; 
    Move_order.y=y;
    Move_order.angle=angle;
    xQueueSend(Move_task_queue, &Move_order,0);
    show_task("Car_moving");
    xSemaphoreTake(taskOK_Semaphore,portMAX_DELAY);
}
void QR_scan_logic(void)
{
    QRscan_enable(1);
    show_task("QR_scaning");
    xSemaphoreTake(taskOK_Semaphore,portMAX_DELAY);
    QRscan_enable(0);
}
void show_task(char *task)
{
    LCD_ShowString(10,180,(uint8_t*)task,0XF81F,WHITE,32,0);
}
