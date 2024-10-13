#ifndef __MY_LOG_H__
#define __MY_LOG_H__

#include "stdio.h"
#include "string.h"

#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t print_mutex;


//log日志输出总开关
#define USE_MY_LOG  1

typedef enum
{
    MY_LOG_LEVEL_NONE =0,
    MY_LOG_LEVEL_ERROR,
    MY_LOG_LEVEL_WARNING,
    MY_LOG_LEVEL_INFOR,
    MY_LOG_LEVEL_DEBUG,
    MY_LOG_LEVEL_VERBOSE,
}my_log_level_t;

//全局通用log_level
#define MY_LOG_GLOBAL_LEVEL  MY_LOG_LEVEL_DEBUG


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"





#if USE_MY_LOG
#define MY_LOG_Print(format,...)                 \
do                                               \
{                                                \
    xSemaphoreTake(print_mutex, portMAX_DELAY);  \
    printf(format, ##__VA_ARGS__);               \
    xSemaphoreGive(print_mutex);                 \
} while (0)   
// #define MY_LOG_Print(format,...)         printf(format, ##__VA_ARGS__);                     
#else
#define MY_LOG_Print(format, ...) 
#endif


//error log
#define MY_LOGE(tag,format,...)                                                       \
do                                                                                      \
{                                                                                       \
    if(MY_LOG_GLOBAL_LEVEL >= MY_LOG_LEVEL_ERROR)                                       \
    {                                                                                   \
        MY_LOG_Print(ANSI_COLOR_RED "\r\nE ["tag"] "format"" ANSI_COLOR_RESET, ##__VA_ARGS__); \
    }                                                                                   \
} while (0)

//warning log
#define MY_LOGW(tag,format,...)                                                       \
do                                                                                      \
{                                                                                       \
    if(MY_LOG_GLOBAL_LEVEL >= MY_LOG_LEVEL_WARNING)                                     \
    {                                                                                   \
       MY_LOG_Print(ANSI_COLOR_YELLOW "\r\nW ["tag"] "format"" ANSI_COLOR_RESET, ##__VA_ARGS__); \
    }                                                                                   \
} while (0)

//information log
#define MY_LOGI(tag,format,...)                                                       \
do                                                                                      \
{                                                                                       \
    if(MY_LOG_GLOBAL_LEVEL >= MY_LOG_LEVEL_INFOR)                                       \
    {                                                                                   \
        MY_LOG_Print(ANSI_COLOR_GREEN "\r\nI ["tag"] "format"" ANSI_COLOR_RESET, ##__VA_ARGS__); \
    }                                                                                   \
} while (0)

//debug log
#define MY_LOGD(tag,format,...)                                                       \
do                                                                                      \
{                                                                                       \
    if(MY_LOG_GLOBAL_LEVEL >= MY_LOG_LEVEL_DEBUG)                                        \
    {                                                                                   \
        MY_LOG_Print(ANSI_COLOR_CYAN "\r\nD ["tag"] "format"" ANSI_COLOR_RESET, ##__VA_ARGS__); \
    }                                                                                   \
} while (0)




void MyLogInit(void);

#endif

