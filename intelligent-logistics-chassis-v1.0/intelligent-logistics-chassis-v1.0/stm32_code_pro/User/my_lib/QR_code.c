#include "QR_code.h"

static uint8_t  codeReceiveFlag =0;
static SemaphoreHandle_t Code_Sem_Handle =NULL; //QR code get sem
char codeInfoStr[QR_CODE_INFO_LEN+1];
static uint8_t byteCount =0;  //数据接收指针

void TriggerCodeGet(void)
{
    codeReceiveFlag = 1;
    byteCount = 0;

    uint8_t triggerCmd[3] = {0x16, 0x54, 0x0D};
    MY_LOGI("code", "trigger code scan");
    HAL_UART_Transmit(&huart4, triggerCmd, 3, 1000);

    while(xSemaphoreTake(Code_Sem_Handle, 2000) != pdTRUE){  //超时未接收, 重新发送
        MY_LOGI("code", "trigger code scan again");
        HAL_UART_Transmit(&huart4, triggerCmd, 3, 1000);
    }
    MY_LOG_Print("\r\ncode info:%s\r\n", codeInfoStr);
}

void ReceiveCodeInfo(char info)
{
    if(!codeReceiveFlag){  //未开启二维码获取功能, 无视数据
        return;
    }

    printf("%c");
    codeInfoStr[byteCount++] = info;
    if(byteCount >= QR_CODE_INFO_LEN)
    {
        codeReceiveFlag = 0;
        byteCount = 0;
        codeInfoStr[QR_CODE_INFO_LEN] = '\0'; //字符串结束符
        xSemaphoreGiveFromISR(Code_Sem_Handle, NULL);   //从中断中释放信号量
    }
}

static void CodeScanTest(int argc, char** argv)
{
    TriggerCodeGet();
}

void CodeScanInit(void)
{
    ShellCmdRegister("code",
                    "QR code scan test",
                    CodeScanTest);
    Code_Sem_Handle = xSemaphoreCreateBinary();  //cmd解析任务同步二值信号量
}



