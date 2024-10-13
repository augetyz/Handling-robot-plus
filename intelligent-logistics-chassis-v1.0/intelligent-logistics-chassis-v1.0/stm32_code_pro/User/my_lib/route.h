#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "stdint.h"

#include "FreeRTOS.h"
#include "task.h"

#include "my_lib/my_log.h"
#include "my_lib/motor.h"
#include "my_lib/car_ctrl.h"
#include "my_lib/file_info.h"
// #include "my_lib/cmd_analyse.h"
#include "m_shell.h"
#include "my_lib/communication.h"
#include "my_lib/arm.h"
#include "QR_code.h"
#include "my_sys.h"
#include "beep.h"

#include "lvgl.h"
#include "ui.h"

typedef enum
{
    RED_CYLINDER = 0,
    GREEN_CYLINDER,
    BLUE_CYLINDER,
}cylinder_color_enum_t;

void RouteTask(void* param);
void GiveRouteExecuteSem(double param);

void ExcuteArmActionArr(arm_action_enum_t action_id);
void GetTaskNumber(void);
void PosAdjust(infor_id_t adjustId);
void Route_ArriveStorage(void);


void TestEnable(uint8_t en);

#endif


