#include "communication.h"

infor_t com_infor = {0};
// uint8_t communication_receive_flag = 0;
SemaphoreHandle_t com_receive_sem = NULL;

/// @brief 传输一字节(硬件平台相关)
/// @param pData 
/// @param len 
void TransBytes(uint8_t* pData, uint16_t len)
{
    HAL_UART_Transmit(&huart4, pData, len, 1000);
}

/// @brief crc16 校验
/// @param pData 数据指针
/// @param len 数据长度
/// @return 
uint16_t CRC16_Check(uint8_t* pData, uint16_t len)
{
    uint16_t CRC16 = 0xFFFF;
    uint8_t state, i, j;
    for (i = 0; i < len; i++)
    {
        CRC16 ^= pData[i];
        for (j = 0; j < 8; j++)
        {
            state = CRC16 & 0x01;
            CRC16 >>= 1;
            if (state)
            {
                CRC16 ^= 0xA001;
            }
        }
    }
    return CRC16;
}

/// @brief 信息打包发送
/// @param id 
/// @param pData 
/// @param len 信息字节长度
void CommunicationTrans(infor_id_t id, uint8_t* pData, uint16_t len)
{
    uint8_t infor_frame[INFOR_MAX_LENGHT+6] = {0}; //帧头两位, 数据长度一位, id 1位, crc16校验码两位
    uint8_t cnt =0;
    uint16_t crc = 0;
    infor_frame[cnt++] = FRAME_HEAD1;
    infor_frame[cnt++] = FRAME_HEAD2;
    infor_frame[cnt++] = len;
    infor_frame[cnt++] = id;
    for(uint16_t i=0; i<len; i++)
    {
        infor_frame[cnt++] = pData[i];
    }
    crc = CRC16_Check(pData, len);
    infor_frame[cnt++] = crc>>8;
    infor_frame[cnt++] = crc&0xff;
    TransBytes(infor_frame, cnt);
}


/// @brief 接收数据字节(放接收中断中读取)
/// @param info_byte 
void CommunicationReceive(uint8_t info_byte)
{
    static uint8_t step = 0, cnt = 0;
    if(info_byte == FRAME_HEAD1){//帧头1, 新接收可以打断旧接收
        step = 1;
        cnt = 0;
        return;
    }
    switch (step)
    {
    case 1: //帧头2
        if(info_byte == FRAME_HEAD2){
            step++;
        }
        else{
            step = 0;  
        }
        break;
    case 2: //信息字节长度
        com_infor.infor_len = info_byte;
        step++;
        break;
    case 3: //信息id码
        com_infor.id = info_byte;
        step++;
        if(com_infor.infor_len == 0){  //没有数据内容, 跳过下一步
            step++;
        }
        break;
    case 4: //数据内容
        if(cnt < com_infor.infor_len){
            com_infor.infor_buf[cnt++] = info_byte;
            if(cnt >= com_infor.infor_len){
                step++;
            }
        }
        break;
    case 5: //校验码高八位
        com_infor.crc = info_byte;
        step++;
        break;
    case 6: //校验码低八位
        com_infor.crc  = com_infor.crc <<8;
        com_infor.crc  |= info_byte;
        if(com_infor.crc  == CRC16_Check(com_infor.infor_buf, com_infor.infor_len)){
            // communication_receive_flag = 1;   //置位解析成功标志位
            xSemaphoreGiveFromISR(com_receive_sem, NULL);
        }
        else{
            step = 0;  
        }
        break;
    
    default:
        break;
    }
}

// /// @brief 判读是否有接收到数据并复位标志位
// /// @param  
// /// @return 
// uint8_t IsComReceive(void)
// {
//     uint8_t re = communication_receive_flag;
//     return re;
// }

/// @brief 重置通信信息结构体
/// @param  
void ResetInfo(void)
{
    xSemaphoreTake(com_receive_sem, 0);
    memset(com_infor.infor_buf, 0, INFOR_MAX_LENGHT);
    com_infor.crc = 0;
    com_infor.id = COM_NONE;
}


////////////////////////////////////cmd test////////////////////////////////////////////
/// @brief 通信收发测试
/// @param param 三位整数, 第一位id, 第二位数据1, 第三位数据2
static void COM_Test(int argc, char** argv)
{
    uint8_t data_buf[2] = {8, 9};
    uint8_t id = atoi(argv[1]);
    data_buf[0] = atoi(argv[2]);
    data_buf[1] = atoi(argv[3]);
    
    CommunicationTrans(id, data_buf, 2);
    MY_LOGI("com", "trans: id[%d] %d %d", id, data_buf[0], data_buf[1]);
    //等待接收
    while(xSemaphoreTake(com_receive_sem, 1000) != pdTRUE)
    {
        MY_LOGW("com", "No information was obtained");
        MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
    }
    MY_LOGI("com", "reveive info->id[%d] len[%d]:", com_infor.id, com_infor.infor_len);
    for(uint8_t i=0; i<com_infor.infor_len; i++)
    {
        MY_LOG_Print(" %0x", com_infor.infor_buf[i]);
    }
    MY_LOG_Print("\r\n");
}

/// @brief openmv指令测试
/// @param argc 
/// @param argv 
static void OV_COM_Test(int argc, char** argv)
{
    infor_id_t id = atoi(argv[1]);
    CommunicationTrans(id, NULL, 0);
    MY_LOGI("com", "send ov id[%d]", id);
}

void CommunicationInit(void)
{
    com_receive_sem = xSemaphoreCreateBinary();
    ShellCmdRegister("com_tt",
                "communication test: param:[id] [data1] [data2]",
                COM_Test);
    ShellCmdRegister("com_ov",
                "send ov com id: param:[id]",
                OV_COM_Test);
}




