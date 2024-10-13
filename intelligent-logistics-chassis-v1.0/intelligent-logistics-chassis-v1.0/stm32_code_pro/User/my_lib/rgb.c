#include "my_lib/rgb.h"

#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"

#include "m_shell.h"
#include "my_lib/my_log.h"
#include "file_info.h"

#define RESET_WORD 64  //reset信号对应时长所占字节数
#define RGB_NUM  8     //rgb灯数量
#define CODE_0 0xC0    //0码
#define CODE_1 0xFC    //1码

//rgb数据缓存
static uint8_t rgb_buffer[RGB_NUM*24+RESET_WORD] = {0};

static  uint32_t rgb_bulb_color = 0x112233;

/// @brief 位寻址rgb控制
/// @param addr rgb单位地址,0-(RGB_NUM-1)
/// @param RGB  rgb码 
void SetRGB_One(uint8_t addr, uint32_t RGB)
{
    if(addr >= RGB_NUM){      //检测地址是否超出实际定义范围
        MY_LOGW("rgb", "addr is invalid");
        return;
    }
    for(uint8_t i=0; i<8; i++)   //设置G值
    {
        rgb_buffer[addr*24+i] = (RGB<<i)&0x008000? CODE_1:CODE_0;
    }
    for(uint8_t i=0; i<8; i++)  //设置R值
    {
        rgb_buffer[addr*24+8+i] = (RGB<<i)&0x800000? CODE_1:CODE_0;
    }
    for(uint8_t i=0; i<8; i++)  //设置B值
    {
        rgb_buffer[addr*24+16+i] = (RGB<<i)&0x000080? CODE_1:CODE_0;
    }
}



/// @brief rgb显存清零(不立即刷新)
/// @param  
void RGB_CleanBuffer(void)
{
    for(uint8_t i=0; i<RGB_NUM; i++)
    {
        SetRGB_One(i, 0x000000);
    }
}

/// @brief 更新rgb数据缓存到设备
/// @param  
void RGB_Refresh(void)
{
    taskENTER_CRITICAL();
    HAL_SPI_Transmit_DMA(&hspi2, rgb_buffer, sizeof(rgb_buffer));
    taskEXIT_CRITICAL();

    while(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_BSY))  //等待数据发送完毕
    {
        vTaskDelay(1);
    }
}

///////////////////////////rgb bulb////////////////////////

/// @brief 开关rgb照明灯
/// @param en 
void IsRGB_BulbOn(uint8_t en)
{
    if(en)
    {
        for(uint8_t i=0; i<RGB_NUM; i++)
        {
            SetRGB_One(i, rgb_bulb_color);
            RGB_Refresh();
        }
    }
    else
    {
        RGB_CleanBuffer();
        RGB_Refresh();
    }
}

void SetRGB_BulbColor(uint8_t red, uint8_t green, uint8_t blue)
{
    rgb_bulb_color= 0;
    rgb_bulb_color |= red<<16;
    rgb_bulb_color |= green<<8;
    rgb_bulb_color |= blue;

    IsRGB_BulbOn(1);
}


///////////////////////////rgb bulb////////////////////////

void Shell_SetRGB_Value(int argc, char** argv)
{
    if(argc < 2)
    {
        MY_LOGW("rgb", "param invalid");
        return;
    }

    if(strcmp("-c", argv[1]) == 0){
        rgb_value.red = atoi(argv[2]);
        rgb_value.green = atoi(argv[3]);
        rgb_value.blue = atoi(argv[4]);

        SetRGB_BulbColor(rgb_value.red, rgb_value.green, rgb_value.blue);

        MY_LOGI("rgb", "set rgb value:R[%d] G[%d] B[%d]", rgb_value.red, rgb_value.green, rgb_value.blue);
    }
    else if(strcmp("-e", argv[1]) == 0){
        uint8_t en = atoi(argv[2]);
        IsRGB_BulbOn(en);
        MY_LOGI("rgb", "rgb buld on?[%d]", en);
    }
}

/// @brief 初始化重置rgb
/// @param  
void RGB_Init(void)
{
    ShellCmdRegister("rgb",
                    "rgb cmd, param:[\"-e\"][0/1](switch bulb) or [\"-c\"][R][G][B](set rgb value)",
                    Shell_SetRGB_Value);
    if(!IsFileReady()){
        vTaskDelay(500);
        if(!IsFileReady()){
            MY_LOGI("rgb", "use default rgb value");
            RGB_CleanBuffer();
            RGB_Refresh();
            return;
        }
    }

    SetRGB_BulbColor(rgb_value.red, rgb_value.green, rgb_value.blue);
    MY_LOGI("rgb", "load rgb value from file");

    RGB_CleanBuffer();
    RGB_Refresh();
}

void RGB_Task(void* param)
{
    RGB_Init();
    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}



