#include "route.h"

//函数指针数组, 方便串口命令函数调用
#define FUNCATION_NUM   20
typedef void (*fn_ptr) (void);
fn_ptr fn_arr[FUNCATION_NUM] = {NULL};
static uint16_t fun_arr_num =0;

static uint8_t testFlag =0;

static SemaphoreHandle_t route_sem = NULL;  //发车信号量


/// @brief 是否开启测试模式, 运行过程中不执行抓放动作, 主要进行定位测试
/// @param en 
void TestEnable(uint8_t en)
{
    MY_LOGI("route", "test falg:%d", en);
    testFlag = en;
}

static void WaitArmNotBusy(void)
{
    while (IsArmBusy())  //等待上一次机械臂动作执行完成
    {
        vTaskDelay(10);
    }
}

/// @brief 执行机械臂动作组
/// @param action_id 
void ExcuteArmActionArr(arm_action_enum_t action_id)
{
    if(arm_action_arr[action_id] == NULL){
        MY_LOGW("arm", "action id[%d] is invalid", action_id);
        return;
    }

    arm_action_t* temp_node = arm_action_arr[action_id];
    for(uint8_t j=0; temp_node!= NULL; j++)
    {
        if(temp_node->holder_wait){
            while(IsSteerBusy(TurnSteer)){  //等待载物转盘空闲
                vTaskDelay(10);
            }
        }

        WaitArmNotBusy();  //等待上一次机械臂动作执行完成

        SetArmState(temp_node->x, temp_node->y, temp_node->z, temp_node->tong_open);
        temp_node = temp_node->next_pos;        
    }
    free(temp_node);
}

/// @brief 车辆按照给定路径点移动
/// @param start_point 
/// @param end_point 
void ExcuteRoutePoint(uint8_t start_point, uint8_t end_point)
{
    uint8_t cnt =0;
    if(!IsFileReady()){
        MY_LOGE("route", "file is note ready");
        return;
    }
    IsEnableDistenceCtrl(1);
    IsEnableAngleCtrl(1);
    
    point_list_t node_list = route_list;
    while(node_list != NULL)
    {
        if(cnt++ < start_point){  //只从起始点开始执行
            continue;
        }
        else if(cnt > end_point+1){
            return;  //超过结束点部分不执行
        }

        // //等待车辆空闲
        // while(!IsCarReady()){
        //     vTaskDelay(30);  
        // }
        //直线行驶
        MY_LOGI("route", "[%d] coord:(%.1f, %.1f)", cnt-1, node_list->coord_x, node_list->coord_y);
        SetMoveCoord(node_list->coord_x, node_list->coord_y);
        vTaskDelay(30);   //释放控制权, 由car.c修改状态(必要)
        while(!IsCarReady()){
            vTaskDelay(30);  
        }
        //转向
        MY_LOGI("route", "car turn[%.1f]", node_list->angle);
        SetCarTurnAngle(node_list->angle);
        vTaskDelay(30);   //释放控制权, 由car.c修改状态(必要)
        while(!IsCarReady()){
            vTaskDelay(30);
        }
        //链表切换至下一个控制节点
        node_list = node_list->next_point;
    }
}


/// @brief 与openmv通信获取任务码
/// @param  
void GetTaskNumber(void)
{
    TriggerCodeGet();  //识别二维码
    MY_LOGI("OV", "send QR_CODE");

    ResetInfo();
    CommunicationTrans(QR_CODE, codeInfoStr, QR_CODE_INFO_LEN);   //发送二维码数字
    
    #if USE_GUI
    lv_label_set_text_fmt(ui_LabelTaskNum, "%s", codeInfoStr);  //屏幕显示
    #endif

    while(xSemaphoreTake(com_receive_sem, 1000) != pdTRUE)
    {
        MY_LOGW("com", "openmv no echo");
        MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
        MY_LOGI("OV", "re send QR_CODE");
        CommunicationTrans(QR_CODE, codeInfoStr, QR_CODE_INFO_LEN);   //发送二维码数字
    }

    if(com_infor.id != OV_STOP){
        MY_LOGW("ov", "id[%d] is wrong", com_infor.id);
        return;
    }
}

/// @brief 与openmv协作完成位置校正
/// @param  adjustId, POS_ADJUST_CIRCLE/POS_ADJUST_RIGHT_ANGLE/POS_ADJUST_COLOR, 直角校正或者圆环校正或色块校正
void PosAdjust(infor_id_t adjustId)
{
    // uint8_t buf[10];
    int16_t x_bias, y_bias;
    MY_LOGI("OV", "send POS_ADJUST id[%d]", adjustId);

    ResetInfo();
    CommunicationTrans(adjustId, NULL, 0);
    IsPosAdjust(1); //开启位置校正
    while(1)
    {
        while(xSemaphoreTake(com_receive_sem, 60000) != pdTRUE)
        {
            MY_LOGW("com", "No information was obtained");
            MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
            MY_LOGI("OV", "send POS_ADJUST id[%d]", adjustId);
            CommunicationTrans(adjustId, NULL, 0);

            SetPosBias(0, 0);   //保护措施, 避免中断断开通信
        }

        if(com_infor.id == OV_STOP){ //openmv主动停止通信
            IsPosAdjust(0); //关闭位置校正
            MY_LOGI("ov", "pos adjust finish");
            ResetInfo();
            return;  
        }
        else if(com_infor.id != adjustId){
            IsPosAdjust(0); //关闭位置校正
            MY_LOGW("ov", "id[%d] is wrong", com_infor.id);
            return;
        }
        
        x_bias = com_infor.infor_buf[0]?  (-com_infor.infor_buf[1]):com_infor.infor_buf[1];
        y_bias = com_infor.infor_buf[2]?  (-com_infor.infor_buf[3]):com_infor.infor_buf[3];
        SetPosBias(x_bias, y_bias); //设置位置校正pid偏差
        MY_LOG_Print("bias: x[%d] y[%d]\r\n", x_bias, y_bias);
        vTaskDelay(20);
    }
}

/// @brief 与openmv协作完成航向角校正
/// @param  
static void YawAdjust(void)
{
    MY_LOGI("OV", "start yaw adjust");

    IsEnableDistenceCtrl(0); //关闭距离校正
    
    ResetInfo();
    CommunicationTrans(YAW_ADJUST, NULL, 0);  //持续通信校正
    while(1)
    {
        while(xSemaphoreTake(com_receive_sem, 1000) != pdTRUE)
        {
            MY_LOGW("com", "No information was obtained");
            MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
            MY_LOGI("OV", "send YAW_ADJUST id");
            CommunicationTrans(YAW_ADJUST, NULL, 0);
        }

        if(com_infor.id == OV_STOP){ //openmv主动停止通信
            MY_LOGI("ov", "yaw adjust finish");
            ResetInfo();
            IsEnableDistenceCtrl(1);    //恢复距离校正
            return;  
        }
        else if(com_infor.id != YAW_ADJUST){
            MY_LOGW("ov", "id[%d] is wrong", com_infor.id);
            IsEnableDistenceCtrl(1);    //恢复距离校正
            return;
        }
        
        // x_bias = com_infor.infor_buf[0]?  (-com_infor.infor_buf[1]):com_infor.infor_buf[1];
        // y_bias = com_infor.infor_buf[2]?  (-com_infor.infor_buf[3]):com_infor.infor_buf[3];
        float adjustAngle = (float)com_infor.infor_buf[1] + (float)com_infor.infor_buf[2]/10.0f;
        adjustAngle = com_infor.infor_buf[0]? (-adjustAngle):adjustAngle;

        SetCarTurnAngle(adjustAngle); //转向
        vTaskDelay(20);  //释放控制权
        MY_LOG_Print("\r\nadjust angle: %.1f", adjustAngle);
        if(fabs(adjustAngle) <= AnglePID.BiasAllow){
            MY_LOGW("route","adjust angle[%.1f] < bias[%.1f]", adjustAngle, AnglePID.BiasAllow);
        }
        while(!IsCarReady()){
            vTaskDelay(20);
        }
    }
}

////////////////////////////////////////////////////////task///////////////////////////////////////////
/// @brief 发车前准备
/// @param  
void Route_Ready(void)
{
    MY_LOGI("route","task ready");
    ExcuteArmActionArr(ARM_START_POS);
}

/// @brief 到达二维码区域, 获取任务码
/// @param  
void Route_ArriveCode(void)
{
    MY_LOGI("route","task get task num");
    // ExcuteArmActionArr(ARM_SCAN_CODE);
    GetTaskNumber();
    // ExcuteArmActionArr(ARM_ADJUST_POS_DISC);  //准备好校正姿态
}

/// @brief 到达圆盘区
/// @param  
void Route_ArriveStorage(void)
{
    MY_LOGI("route","pos adjust from disc");
    ExcuteArmActionArr(ARM_ADJUST_POS_DISC);
    WaitArmNotBusy();  //等待机械臂动作执行完成

    YawAdjust();
    PosAdjust(POS_ADJUST_RIGHT_ANGLE);

    ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
}

/// @brief 校正完成后后退一定距离才进行识别抓取
/// @param  
void Route_ArriveStorage_Add(void)
{
    ResetInfo();
    for(uint8_t i=0; i<3; i++)  //抓取三个物品
    {
        ExcuteArmActionArr(ARM_CRCLE_TAKE_READY);  //机械臂待抓取状态
        WaitArmNotBusy();  //等待机械臂动作执行完成
        
        MY_LOGI("OV", "send grab id, cnt[%d]", i+1);
        CommunicationTrans(READY_GRAB, NULL, 0);
        while(xSemaphoreTake(com_receive_sem, 60000) != pdTRUE)
        {
            MY_LOGW("com", "No information was obtained");
            MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
            MY_LOGI("OV", "re send grab id");
            CommunicationTrans(READY_GRAB, NULL, 0);
        }

        if(com_infor.id != READY_GRAB){ //信息错误
            MY_LOGW("ov", "id[%d] is wrong", com_infor.id);
            return;
        }

        MY_LOG_Print("cylinder color[%d]\r\n", com_infor.infor_buf[0]);
        switch (com_infor.infor_buf[0])
        {
        case RED_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        case GREEN_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        case BLUE_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        
        default:
            break;
        }
    }
    MY_LOGI("ov", "grab from disc finish");
    ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
}


/// @brief 方案2, 不矫正圆盘抓取
/// @param  
void Route_ArriveStorage_Speicial(void)
{
    MY_LOGI("route","disc spceicial");
    // ExcuteArmActionArr(ARM_ADJUST_POS_DISC);
    // WaitArmNotBusy();  //等待机械臂动作执行完成

    // YawAdjust();
    // PosAdjust(POS_ADJUST_RIGHT_ANGLE);

    ResetInfo();
    for(uint8_t i=0; i<3; i++)  //抓取三个物品
    {
        ExcuteArmActionArr(ARM_CRCLE_TAKE_READY);  //机械臂待抓取状态
        WaitArmNotBusy();  //等待机械臂动作执行完成
        
        MY_LOGI("OV", "send grab id, cnt[%d]", i+1);
        CommunicationTrans(READY_GRAB, NULL, 0);
        while(xSemaphoreTake(com_receive_sem, 60000) != pdTRUE)
        {
            MY_LOGW("com", "No information was obtained");
            MY_LOG_Print("crc-> [%0x]-[%0x]\r\n", com_infor.crc, CRC16_Check(com_infor.infor_buf, com_infor.infor_len));
            MY_LOGI("OV", "re send grab id");
            CommunicationTrans(READY_GRAB, NULL, 0);
        }

        if(com_infor.id != READY_GRAB){ //信息错误
            MY_LOGW("ov", "id[%d] is wrong", com_infor.id);
            return;
        }

        MY_LOG_Print("cylinder color[%d]\r\n", com_infor.infor_buf[0]);
        switch (com_infor.infor_buf[0])
        {
        case RED_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        case GREEN_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        case BLUE_CYLINDER:
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE);
            ExcuteArmActionArr(ARM_TAKE_FROM_STORAGE);
            break;
        
        default:
            break;
        }
    }
    MY_LOGI("ov", "grab from disc finish");

    ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
}


/// @brief 方案2中途校正
/// @param  
void Route_Adjust_Speicial(void)
{
    static uint8_t cnt=0;
    ExcuteArmActionArr(ARM_ADJUST_YAW);  //航向校正
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","yaw adjust");
    YawAdjust();

    MY_LOGI("route","pos adjust by sideway");
    ExcuteArmActionArr(ARM_ADJUST_POS_CLOSE_WAY);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    PosAdjust(POS_ADJUST_CIRCLE);

    ExcuteArmActionArr(ARM_TEMP);  //提前准备
}

/// @brief 到达暂存区
/// @param  
void Route_ArriveTemp(void)
{
    static uint8_t cnt=0;
    ExcuteArmActionArr(ARM_ADJUST_YAW);  //航向校正
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","yaw adjust");
    YawAdjust();

    MY_LOGI("route","pos adjust by sideway");
    ExcuteArmActionArr(ARM_ADJUST_POS_CLOSE_WAY);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    PosAdjust(POS_ADJUST_CIRCLE);

    if(testFlag){  //测试模式下不执行抓放
        ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
        return;
    }

    uint8_t indexOffset =0;
    if(++cnt == 1){  //第一轮抓放顺序
        indexOffset = 0;
    }
    else{           //第二轮抓放顺序
        indexOffset = 4;
    }

    for(uint8_t i=0; i<3; i++)  //放置
    {
        switch (codeInfoStr[i+indexOffset])
        {
        case '1':
            MY_LOGI("route","place r");
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_PLACE_RED_CLOSE_WAY); 
            break;
        case '2':
            MY_LOGI("route","place g");
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);  
            ExcuteArmActionArr(ARM_PLACE_GREEN_CLOSE_WAY);  
            break;     
        case '3':
            MY_LOGI("route","place b");
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE); 
            ExcuteArmActionArr(ARM_PLACE_BLUE_CLOSE_WAY); 
            break;           
        
        default:
            break;
        }

        vTaskDelay(100); //等待稳定再进行下一轮
    }
    for(uint8_t i=0; i<3; i++)  //抓取
    {
        switch (codeInfoStr[i+indexOffset])
        {
        case '1':
            MY_LOGI("route","grab r");
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_TAKE_RED_FROM_TEMP);
            break;
        case '2':
            MY_LOGI("route","grab g");
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);      
            ExcuteArmActionArr(ARM_TAKE_GREEN_FROM_TEMP);
            break;     
        case '3':
            MY_LOGI("route","grab b");
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE); 
            ExcuteArmActionArr(ARM_TAKE_BLUE_FROM_TEMP);  
            break;           
        
        default:
            break;
        }
    }
    
    MY_LOGI("route","temp storage task finish");

    ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
}

/// @brief 第一次到达加工区
/// @param  
void Route_ArriveProcessFirst(void)
{
    ExcuteArmActionArr(ARM_ADJUST_YAW);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","yaw adjust");
    YawAdjust();

    MY_LOGI("route","pos adjust by sideway");
    ExcuteArmActionArr(ARM_ADJUST_POS_CLOSE_WAY);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    PosAdjust(POS_ADJUST_CIRCLE);

    if(testFlag){  //测试模式下不执行抓放
        ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
        return;
    }


    for(uint8_t i=0; i<3; i++)  //放置
    {
        switch (codeInfoStr[i])
        {
        case '1':
            MY_LOGI("route","place r");
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_PLACE_RED_CLOSE_WAY); 
            break;
        case '2':
            MY_LOGI("route","place g");
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);  
            ExcuteArmActionArr(ARM_PLACE_GREEN_CLOSE_WAY);  
            break;     
        case '3':
            MY_LOGI("route","place b");
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE); 
            ExcuteArmActionArr(ARM_PLACE_BLUE_CLOSE_WAY); 
            break;           
        
        default:
            break;
        }
        vTaskDelay(100); //等待稳定再进行下一轮
    }

    MY_LOGI("route","process 1 task finish");

    ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
}

/// @brief 第二次到达加工区
/// @param  
void Route_ArriveProcessSecond(void)
{
    ExcuteArmActionArr(ARM_ADJUST_YAW);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","yaw adjust");
    YawAdjust();

    ExcuteArmActionArr(ARM_ADJUST_POS_STACK);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","pos adjust by stack");
    PosAdjust(POS_ADJUST_COLOR);

    if(testFlag){  //测试模式下不执行抓放
        ExcuteArmActionArr(ARM_TEMP);  //航向校正, 提前准备
        return;
    }

    for(uint8_t i=0; i<3; i++)  //放置
    {
        switch (codeInfoStr[i+4])
        {
        case '1':
            MY_LOGI("route","place r");
            SetSteerAngle(TurnSteer, TURN_STEER_RED);
            ExcuteArmActionArr(ARM_PLACE_RED_STACK);   
            break;
        case '2':
            MY_LOGI("route","place g");
            SetSteerAngle(TurnSteer, TURN_STEER_GREEN);  
            ExcuteArmActionArr(ARM_PLACE_GREEN_STACK);  
            break;     
        case '3':
            MY_LOGI("route","place b");
            SetSteerAngle(TurnSteer, TURN_STEER_BLUE); 
            ExcuteArmActionArr(ARM_PLACE_BLUE_STACK); 
            break;           
        
        default:
            break;
        }
        vTaskDelay(100); //等待稳定再进行下一轮
    } 

    // ExcuteArmActionArr(ARM_START_POS);  //任务完成收回机械臂
    MY_LOGI("route","process 2 task finish");

    ExcuteArmActionArr(ARM_TEMP);  //提前准备
}

/// @brief 回程校正
/// @param  
void Route_BackAdjust(void)
{
    // static uint8_t cnt=0;

    ExcuteArmActionArr(ARM_ADJUST_BACK);
    WaitArmNotBusy();  //等待机械臂动作执行完成
    MY_LOGI("route","back adjust");
    // YawAdjust();
    PosAdjust(POS_ADJUST_RIGHT_ANGLE2);

    SetSteerAngle(TurnSteer, TURN_STEER_INIT_ANGLE);
    ExcuteArmActionArr(ARM_START_POS);//回到车库收回机械臂
}
////////////////////////////////////////////////////////task///////////////////////////////////////////


/// @brief 车辆任务执行核心函数
/// @param  
static void ExecuteRoute(void)
{
    if(!IsFileReady()){
        MY_LOGE("route", "file is note ready");
        return;
    }
    point_list_t node_list = route_list;
    while(node_list != NULL)
    {
        // //等待车辆空闲
        // while(!IsCarReady()){
        //     vTaskDelay(30);  
        // }
        //直线行驶
        MY_LOGI("route", "coord:(%.1f, %.1f)", node_list->coord_x, node_list->coord_y);
        SetMoveCoord(node_list->coord_x, node_list->coord_y);
        vTaskDelay(30);   //释放控制权, 由car.c修改状态(必要)
        while(!IsDisPID_OK()){
            vTaskDelay(30);  
        }
        //转向
        MY_LOGI("route", "car turn[%.1f]", node_list->angle);
        SetCarTurnAngle(node_list->angle);
        vTaskDelay(30);   //释放控制权, 由car.c修改状态(必要)
        while(!IsAglPID_OK()){
            vTaskDelay(30);
        }
        //特殊点任务处理
        MY_LOGI("route", "car event[%d]", node_list->event);
        switch(node_list->event)
        {
            case ARRIVE_QR_CODE:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveCode();
                break;
            case ARRIVE_STORAGE:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveStorage();
                break;
            case ARRIVE_TEMP_STORAGE:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveTemp();
                break;
            case ARRIVE_PROCESS_AREA:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveProcessFirst();
                break;
            case ARRIVE_SEC_PRO_AREA:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveProcessSecond();
                break;
            case ARRIVR_BACK_ADJUST:
                SetBeepFun(0, BeepOnMement, 100);
                Route_BackAdjust();
                break;
            case ARRIVR_SPECIAL_ADJUST:
                SetBeepFun(0, BeepOnMement, 100);
                Route_Adjust_Speicial();
                break;
            case ARRIVR_SPECIAL_STORAGE:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveStorage_Speicial();
                break;
            case ARRIVR_STORAGE_ADD:
                SetBeepFun(0, BeepOnMement, 100);
                Route_ArriveStorage_Add();
                break;
        }
        //链表切换至下一个控制节点
        node_list = node_list->next_point;
    }
}

void GiveRouteExecuteSem(double param)
{
    MY_LOGI("route", "start execute route");
    xSemaphoreGive(route_sem);
}


//////////////////////////////////cmd test///////////////////////////////////////////////////

static void StartRoute(int argc, char** argv)
{
    if(argc == 2){
        if(strcmp("-t", argv[1]) == 0){
            TestEnable(1);    //测试模式
        }
    }
    GiveRouteExecuteSem(0);
}

/// @brief 填充测试函数进入函数指针数组
/// @param  
void OvComTestInit(void)
{
    // uint8_t cnt =0;
    fn_arr[fun_arr_num++] = GetTaskNumber;              //0
    fn_arr[fun_arr_num++] = Route_ArriveStorage;        //1
    fn_arr[fun_arr_num++] = Route_ArriveTemp;           //2
    fn_arr[fun_arr_num++] = Route_ArriveProcessFirst;   //3
    fn_arr[fun_arr_num++] = Route_ArriveProcessSecond;  //4
    fn_arr[fun_arr_num++] = YawAdjust;                  //5
}

/// @brief 从函数指针数组中调用测试函数
/// @param param 函数指针数组下标(整数)
static void OvComTest(int argc, char** argv)
{
    uint8_t index = atoi(argv[1]);
    if(index >= fun_arr_num){
        MY_LOGE("OV", "index[%d] is invalid", index);
        return;
    }

    fn_arr[index]();
}


void PosAdjustTest(int argc, char** argv)
{
    uint8_t id = atoi(argv[1]);
    if(id == 0){
        PosAdjust(POS_ADJUST_CIRCLE);
    }
    else if(id == 1)
    {
        PosAdjust(POS_ADJUST_RIGHT_ANGLE);
    }
    else if(id == 2)
    {
        PosAdjust(POS_ADJUST_COLOR);
    }
    else if(id == 3)
    {
        PosAdjust(POS_ADJUST_RIGHT_ANGLE2);
    }
    else{
        MY_LOGW("pos adj", "id[%d] is invalid", id);
    }
}


static void ArmActionArrTest(int argc, char** argv)
{
    uint8_t index = atoi(argv[1]);
    if(arm_action_arr[index] == NULL){
        MY_LOGW("arm", "action arr index is invalid");
        return;
    }

    MY_LOG_Print("action arr[%d]:\r\n", index);
    arm_action_t* temp_node = arm_action_arr[index];
    for(uint8_t j=0; temp_node!= NULL; j++)
    {
        MY_LOG_Print("  pos[%d]: x[%.1f] y[%.1f] z[%.1f] tong[%d] holder[%d]\r\n", j, temp_node->x, temp_node->y, temp_node->z, temp_node->tong_open, temp_node->holder_wait);
        SetArmState(temp_node->x, temp_node->y, temp_node->z, temp_node->tong_open);
        temp_node = temp_node->next_pos;
        while(IsArmBusy()){
            vTaskDelay(10);
        }
    }
    free(temp_node);
}

static void RoutePointTest(int argc, char** argv)
{
    if(argc != 3){
        MY_LOGW("route", "action arr index is invalid");
        return;
    }

    
    uint8_t start = atoi(argv[1]);
    uint8_t end = atoi(argv[2]);
    MY_LOGI("route", "start[%d], end[%d]", start, end);
    ExcuteRoutePoint(start, end);
}

//////////////////////////////////cmd test///////////////////////////////////////////////////

static void RouteTaskInit(void)
{
    route_sem = xSemaphoreCreateBinary();
    ShellCmdRegister("start",
                "start compitition task, param:[null] or [\"-t\"](test mode)",
                StartRoute);

    OvComTestInit();
    ShellCmdRegister("ov_tt",
                "openmv communication test, param:[fn_index]",
                OvComTest);     

    ShellCmdRegister("arm_grp",
                "arm action group test, param:[arr_index]",
                ArmActionArrTest);  

    ShellCmdRegister("route",
                "route point test, param:[start][end]",
                RoutePointTest);  

    ShellCmdRegister("pos_adj",
                "pos adjust test, param:[0/1/2/3], 0,circle adj / 1,right adj / 2,color adj / 3,back adj",
                PosAdjustTest);                              
}


/// @brief 比赛路径任务
/// @param param 
void RouteTask(void* param)
{
    static uint32_t timeCount, runTime;  //记录运行时间
    RouteTaskInit();
    xSemaphoreTake(route_sem, portMAX_DELAY);  //死等

    timeCount = GetSysTick(); 

    IsEnableDistenceCtrl(1);  //距离控制使能/失能
    IsEnableAngleCtrl(1);   //航向角控制使能/失能
    MotorCtrlEnable(1);
    
    MY_LOGI("route", "execute route plan");
    SetBeepFun(0, BeepOnMement, 500);

    ExecuteRoute();   //执行任务

    runTime = GetSysTick()-timeCount;

    uint8_t min = runTime/1000/60;
    uint8_t sec = runTime/1000 - min*60;
    #if USE_GUI
    lv_label_set_text_fmt(ui_LabelFinishTime, "time: %d:%d", min, sec);  //屏幕显示
    lv_obj_clear_flag(ui_LabelFinishTime, LV_OBJ_FLAG_HIDDEN);
    #endif

    MY_LOGI("route", "finish route plan, time: %d:%d", min, sec);
    SetBeepFun(0, BeepOnMement, 500);

    vTaskDelete(NULL); //删除任务
}

