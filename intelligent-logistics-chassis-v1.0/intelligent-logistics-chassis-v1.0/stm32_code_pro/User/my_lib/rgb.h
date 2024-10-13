#ifndef __RGB_H__
#define __RGB_H__

#include "stdint.h"




#define RESET_WORD      64  //reset信号对应时长所占字节数
#define RGB_NUM         8     //rgb灯数量
#define CODE_0          0xC0    //0码
#define CODE_1          0xFC    //1码



// void RGB_Task(void* param);
// void SetRGBTestColor(uint8_t red, uint8_t green, uint8_t blue);

// void RGB_Init(void);
void RGB_Task(void* param);
void IsRGB_BulbOn(uint8_t en);
#endif

