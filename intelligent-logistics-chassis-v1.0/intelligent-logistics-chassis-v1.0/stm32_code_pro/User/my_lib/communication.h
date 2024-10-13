#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__


#include "string.h"
#include "usart.h"

#include "my_lib/my_log.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"


//格式 帧头1+帧头2+数据字节长度+信息id码+字节1+字节2...+crc校验码高8位+校验码低八位

#define FRAME_HEAD1       0xA5
#define FRAME_HEAD2       0x5A
#define INFOR_MAX_LENGHT  10


typedef enum
{
    OV_STOP = 0,
    QR_CODE,
    POS_ADJUST_CIRCLE,    //圆环位置校正
    POS_ADJUST_RIGHT_ANGLE,  //直角位置校正
    READY_GRAB,
    YAW_ADJUST,              //航向校正
    UI_BTN1_EVENT,
    UI_BTN2_EVENT,
    POS_ADJUST_COLOR,   //色块位置校正
    UI_BTN_SWITCH_EVENT,    //openmv屏幕开关指令
    POS_ADJUST_RIGHT_ANGLE2,
    UI_BTN3_EVENT,
    UI_BTN4_EVENT,
    UI_BTN_WRITE_EVENT,
    COM_NONE,
}infor_id_t;

typedef struct 
{
    infor_id_t id;
    uint16_t infor_len;
    uint8_t infor_buf[INFOR_MAX_LENGHT];
    uint16_t crc;
}infor_t;


extern infor_t com_infor;
extern SemaphoreHandle_t com_receive_sem;

void CommunicationReceive(uint8_t info_byte);
void CommunicationTrans(infor_id_t id, uint8_t* pData, uint16_t len);
void ResetInfo(void);

void CommunicationInit(void);
uint16_t CRC16_Check(uint8_t* pData, uint16_t len);


#endif

