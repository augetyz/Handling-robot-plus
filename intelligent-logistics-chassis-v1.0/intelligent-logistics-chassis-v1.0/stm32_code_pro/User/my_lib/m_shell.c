#include "m_shell.h"


static ShellCmd_t shellCmds[SHELL_CMD_NUM_MAX]={0};
static uint16_t shellCmdNum =0;
static uint8_t lineReBuf[LINE_CHAR_MAX_NUM]; //数据接收缓存
static SemaphoreHandle_t Shell_Sem_Handle =NULL; //cmd任务同步


/// @brief 接收单个字符信息
/// @param rcv_char 
/// @return 
uint8_t ShellGetLine(uint8_t rcv_char)
{
	static uint8_t count = 0; /*用于记录除特殊字符外的其他有效字符的数量*/
    if (count >= LINE_CHAR_MAX_NUM) /*长度超限*/
    {
        count = 0; /*清零计数器以便后续使用*/
        return 0;  /*返回无效标志*/
    }
    lineReBuf[count] = rcv_char; /*记录数据*/
    switch (rcv_char)
    {
        case 0x08:
        case 0x7F: /*退格键或者删除键*/
        {
            if (count > 0)
            {
                count--; /*删除上一个接收到的字符*/
            }
        }
        break;

        case '\r':  
        case '\n': /*接收到回车换行，证明已经收到一个完整的命令*/
        {
            lineReBuf[count] = '\0'; /*添加字符串结束符，刚好可以去掉'\r'或者'\n'*/
            count = 0;			/*清零计数器以便后续使用*/
            xSemaphoreGiveFromISR(Shell_Sem_Handle, NULL);   //从中断中释放信号量
            return 1;			/*返回有效标志*/
        }
        //break;

        default:
            count++;
    }

    #if ECHO_ENABLE
    printf("%c",rcv_char); /*把收到的字符输出到串口*/
    #endif

	return 0;
}


/// @brief 解析每行命令中的参数
/// @param msg 
/// @param delim 
/// @param get 
/// @param max_num 
/// @return 
static int ShellGetParam(char* msg, char* delim, char* get[], int max_num)
{
	int i,ret;
	char *ptr = NULL;
	ptr = strtok(msg, delim);
	for(i=0; ptr!=NULL &&i<max_num; i++)
	{
		get[i] = ptr;
		ptr = strtok(NULL, delim);
	}
	ret = i;
	return ret;
}

/// @brief 注册命令
/// @param _CmdId 字符串长度取决于 宏定义CMD_ID_LENGTH
/// @param Description 命令描述字符串
/// @param Funcation 命令执行函数
void ShellCmdRegister(char* cmdName, char* helpStr, cmd_func_t cmd_fun)
{
    if(shellCmdNum >= SHELL_CMD_NUM_MAX )
    {
        MY_LOGW("shell", "cmd buffer full, can not add");
        return;
    }

    shellCmds[shellCmdNum].name = cmdName;
    shellCmds[shellCmdNum].help = helpStr;
    shellCmds[shellCmdNum].func = cmd_fun;

    shellCmdNum++;
}

 /// @brief shell测试
 /// @param argc 
 /// @param argv 
 /// @return 
static void ShellTest(int argc, char**argv)
 {
 	int i = 0;
 	MY_LOG_Print("\r\n  argc=%d ", argc);
 	MY_LOG_Print("argv=[");
 	for(i=0; i<(argc-1); i++)
 	{
 		MY_LOG_Print("%s,", argv[i]);
 	}
 	MY_LOG_Print("%s]\r\n", argv[i]);
 }

static void ShellCmdHelp(int argc, char** argv)
{
    MY_LOG_Print("\r\n");
	for(int i=0; i<shellCmdNum; i++)
	{
        MY_LOG_Print("cmd[%d]:  %s\r\n", i, shellCmds[i].name);
        MY_LOG_Print("   %s\r\n", shellCmds[i].help );
	}
    // MY_LOG_Print("\r\n");
}

/// @brief 初始化
/// @param  
static void ShellCmdInit(void)
{
    Shell_Sem_Handle = xSemaphoreCreateBinary();  //cmd解析任务同步二值信号量
    ShellCmdRegister("help",      //注册帮助命令
                "show all cmds",
                ShellCmdHelp);

    ShellCmdRegister("test",      //注册帮助命令
                "shell cmd test",
                ShellTest);
}

void ShellTask(void* param)
{
    ShellCmdInit();
    while (1)
    {
        char* argv[PARAM_MAX_NUM];
        uint8_t argc=0;
        xSemaphoreTake(Shell_Sem_Handle, portMAX_DELAY);

        if(strlen(lineReBuf)) /* 判断接受到的指令字符串是否为0 */
        {
            argc = ShellGetParam(lineReBuf, " ", argv, PARAM_MAX_NUM);
            uint16_t i=0;
            for(i=0; i<shellCmdNum; i++)
            {
                if(strcmp(argv[0], shellCmds[i].name) == 0)
                {
                    if(argc == 2){
                        if(strcmp(argv[1], "-h") == 0){   //帮助信息
                            MY_LOG_Print("\r\n  %s", shellCmds[i].help);
                            break;
                        }
                    }

                    shellCmds[i].func(argc, argv);
                    break;
                }
            }
            if(i == shellCmdNum)
            {
                MY_LOGW("shell","cmd \"%s\" does not exist!!",argv[0]);
            }
        }
        MY_LOG_Print("\r\n->");
    }
}


