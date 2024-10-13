#ifndef __M_SHELL_H__
#define __M_SHELL_H__

#include "my_sys.h"
#include "string.h"
#include "stdlib.h"
#include "my_lib/my_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define ECHO_ENABLE         1    //回显使能


#define SHELL_CMD_NUM_MAX   35   //最大shell cmd支持数量
#define LINE_CHAR_MAX_NUM   50   //1行数据包含的最大字符数量
#define PARAM_MAX_NUM       10   //一行命令中能够携带的最大参数数量

typedef void(*cmd_func_t)(int argc, char**argv);

typedef struct /* 定义命令结构体 */
{
	char* name; /* 命令的名字 */
	cmd_func_t func; /* 执行函数 */
	char* help; /* 帮助文字 */
}ShellCmd_t;


// double BufferToDouble(char* pBuf, uint8_t BufLen);
uint8_t ShellGetLine(uint8_t rcv_char);
void ShellCmdRegister(char* cmdName, char* helpStr, cmd_func_t cmd_fun);
void ShellTask(void* param);


#endif

