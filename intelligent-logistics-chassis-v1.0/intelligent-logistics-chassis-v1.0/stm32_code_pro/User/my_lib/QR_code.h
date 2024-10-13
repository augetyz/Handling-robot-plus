#ifndef __QR_CODE_H__
#define __QR_CODE_H__

#include "usart.h"

#include "my_sys.h"
#include "my_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "m_shell.h"

#define QR_CODE_INFO_LEN 7  //二维码有效信息字符长度(123+321)

extern char codeInfoStr[QR_CODE_INFO_LEN+1];


void TriggerCodeGet(void);
void ReceiveCodeInfo(char info);
void CodeScanInit(void);

#endif


