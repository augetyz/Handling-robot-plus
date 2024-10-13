#include "my_lib/file_info.h"
/////////////////////////////variate//////////////////////////////////////
//fatfs
static FATFS file_sys;
static FIL file;
static FRESULT f_res;
static BYTE* pFileRead;
static UINT file_size =0;

static cJSON* child_obj;   //共用子对象
//file path 
static cJSON* path_obj;    //file_ctrl json 父对象
//rgb
static cJSON* rgb_obj;
static cJSON* red;
static cJSON* green;
static cJSON* blue;
rgb_value_t rgb_value = {255, 255, 255};
//route
static cJSON* route_obj;   //route json 父对象
static cJSON* points;
static cJSON* coord;
static cJSON* coord_x;
static cJSON* coord_y;
static cJSON* angle;
static cJSON* event;
point_list_t route_list = NULL;
//pid param 
static cJSON* param_obj;  //param json 父对象
static cJSON* kp_json;
static cJSON* ki_json;
static cJSON* kd_json;
static cJSON* out_max_json;
static cJSON* bias_allow_json;
car_param_t car_param = {0};
//arm action
static cJSON* arm_obj;  //arm json 父对象
static cJSON* pos_obj;
static cJSON* arm_x;
static cJSON* arm_y;
static cJSON* arm_z;
static cJSON* arm_tong;
static cJSON* arm_openmv;
arm_action_t* arm_action_arr[ARM_ACTION_NUM] = {NULL};
uint8_t actions_num = 0;

//
static uint8_t file_state=0;
/////////////////////////////variate//////////////////////////////////////

/// @brief 重写cubemx的函数, 使得sd检测是有效
/// @param  
/// @return 
uint8_t BSP_SD_IsDetected(void)
{
    uint8_t status = SD_PRESENT;

//  if (BSP_PlatformIsDetected() == 0x0)
//  {
//    status = SD_NOT_PRESENT;
//  }

    return status;
}

/// @brief 将fatfs返回的错误码转换成字符信息打印
/// @param error_num 
static void FATFS_ErrorPrint(FRESULT error_num)
{
    switch(error_num)
    {
        case 1:
            MY_LOGE("fatfs", "A hard error occurred in the low level disk I/O layer");
            break;
        case 2:
            MY_LOGE("fatfs", "Assertion failed");
            break;
        case 3:
            MY_LOGE("fatfs", "The physical drive cannot work");
            break;
        case 4:
            MY_LOGE("fatfs", "Could not find the file");
            break;
        case 5:
            MY_LOGE("fatfs", "Could not find the path");
            break;
        case 6:
            MY_LOGE("fatfs", "The path name format is invalid");
            break;
        case 7:
            MY_LOGE("fatfs", "Access denied due to prohibited access or directory full ");
            break;
        case 8:
            MY_LOGE("fatfs", "Access denied due to prohibited access");
            break;
        case 9:
            MY_LOGE("fatfs", "The file/directory object is invalid");
            break;
        case 10:
            MY_LOGE("fatfs", "The physical drive is write protected");
            break;
        case 11:
            MY_LOGE("fatfs", "The logical drive number is invalid");
            break;
        case 12:
            MY_LOGE("fatfs", "The volume has no work area");
            break;
        case 13:
            MY_LOGE("fatfs", "There is no valid FAT volume");
            break;
        case 14:
            MY_LOGE("fatfs", "The f_mkfs() aborted due to any problem");
            break;
        case 15:
            MY_LOGE("fatfs", "Could not get a grant to access the volume within defined period");
            break;
        case 16:
            MY_LOGE("fatfs", "The operation is rejected according to the file sharing policy");
            break;
        case 17:
            MY_LOGE("fatfs", "LFN working buffer could not be allocated");
            break;
        case 18:
            MY_LOGE("fatfs", "Number of open files > _FS_LOCK");
            break;
        case 19:
            MY_LOGE("fatfs", "Given parameter is invalid");
            break;
        default:
            MY_LOGE("fatfs", "not a invalid error number");
            break;
    }
}

/////////////////////////////////////route//////////////////////////////////////////////


/// @brief 添加一个路径信息点至链表中
/// @param link_list 
/// @param car_head 
/// @param distence 
/// @param angle 
/// @param event 
static void AddPoint2List(point_list_t* link_list, double coord_x, double coord_y, double angle, uint8_t event)
{   
    point_list_t temp_node = *link_list;
    if(*link_list == NULL){          //先初始化头结点
        *link_list = (point_t*)malloc(sizeof(point_t));
        route_list->coord_x = coord_x;
        route_list->coord_y = coord_y;
        route_list->angle = angle;
        route_list->event = event;
        route_list->next_point = NULL;
    }
    else{
        while(temp_node->next_point != NULL)  //遍历至尾结点
        {
            temp_node = temp_node->next_point;
        }
        temp_node->next_point = (point_list_t)malloc(sizeof(point_t));
        temp_node->next_point->coord_x = coord_x;
        temp_node->next_point->coord_y = coord_y;
        temp_node->next_point->angle = angle;
        temp_node->next_point->event = event;
        temp_node->next_point->next_point = NULL;
    }
}

/// @brief 提取相应json文件中的路径信息
/// @param file_name_str 文件名称,比如"route.json"
/// @return 
uint8_t RouteExtract(char* file_name_str)
{
    char path_buffer[30];
    sprintf(path_buffer, "0:%s", file_name_str);
    f_res = f_open(&file, path_buffer, FA_OPEN_EXISTING|FA_READ);  //打开文件
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);         //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    route_obj = cJSON_Parse(pFileRead);
    free(pFileRead);                      //释放内存块 
    f_close(&file);         //关闭文件
    if(route_obj == NULL){
        MY_LOGW("cjson", "route cjson parse fail");
        return 0;
    }
    points = cJSON_GetObjectItem(route_obj, "points");
    uint8_t point_num = cJSON_GetArraySize(points);
    // MY_LOG_Print("\r\n****** save route point ******\r\n\r\n");
    for(uint8_t i=0; i<point_num; i++)  //获取所有结点信息
    {
        
        child_obj = cJSON_GetArrayItem(points, i);
        coord = cJSON_GetObjectItem(child_obj, "coord");
        coord_x = cJSON_GetArrayItem(coord, 0);
        coord_y = cJSON_GetArrayItem(coord, 1);
        angle = cJSON_GetObjectItem(child_obj, "angle");
        event = cJSON_GetObjectItem(child_obj, "event");
        //数据添加进链表
        AddPoint2List(&route_list, coord_x->valuedouble,coord_y->valuedouble,angle->valuedouble,event->valueint);
    }
    cJSON_Delete(route_obj);   //删除父对象
    return 1;
}


/// @brief 释放路径链表
/// @param  
static void FileCleanRoute(void)
{
    point_t* tempNode = route_list;
    point_t* tempNext = NULL;
    while(tempNode != NULL)
    {
        tempNext = tempNode->next_point;
        free(tempNode);
        tempNode = NULL;
        tempNode = tempNext;
    }

    route_list = NULL;  
}


/// @brief 读取方案2路径参数
/// @param  
/// @return 
uint8_t RouteExtract_Specil(void)
{
    FileCleanRoute();  //先清空默认读取的路径参数

    f_res = f_open(&file, "0:route_spl.json", FA_OPEN_EXISTING|FA_READ);  //打开文件
    if(f_res != FR_OK){
        MY_LOGW("fatfs", "open file_ctrl.json error");
        FATFS_ErrorPrint(f_res);
    }  
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);         //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    route_obj = cJSON_Parse(pFileRead);
    free(pFileRead);                      //释放内存块 
    f_close(&file);         //关闭文件
    if(route_obj == NULL){
        MY_LOGW("cjson", "route cjson parse fail");
        return 0;
    }
    points = cJSON_GetObjectItem(route_obj, "points");
    uint8_t point_num = cJSON_GetArraySize(points);
    // MY_LOG_Print("\r\n****** save route point ******\r\n\r\n");
    for(uint8_t i=0; i<point_num; i++)  //获取所有结点信息
    {
        
        child_obj = cJSON_GetArrayItem(points, i);
        coord = cJSON_GetObjectItem(child_obj, "coord");
        coord_x = cJSON_GetArrayItem(coord, 0);
        coord_y = cJSON_GetArrayItem(coord, 1);
        angle = cJSON_GetObjectItem(child_obj, "angle");
        event = cJSON_GetObjectItem(child_obj, "event");
        //数据添加进链表
        AddPoint2List(&route_list, coord_x->valuedouble,coord_y->valuedouble,angle->valuedouble,event->valueint);
    }
    cJSON_Delete(route_obj);   //删除父对象
    return 1;
}
/////////////////////////////////////route//////////////////////////////////////////////

/////////////////////////////////////pid param//////////////////////////////////////////////
/// @brief 为pid结构体赋值
/// @param pid_info 
/// @param kp 
/// @param ki 
/// @param kd 
void AddPID_Param(pid_param_t* pid_info, double kp, double ki, double kd, double out_max, double bias_allow)
{
    pid_info->kp = kp;
    pid_info->ki = ki;
    pid_info->kd = kd;
    pid_info->out_max = out_max;
    pid_info->bias_allow = bias_allow;
}


/// @brief 从文件中提取pid参数信息
/// @param file_name_str 
/// @return 
uint8_t ParamExtract(char* file_name_str)
{
    char path_buffer[30];
    sprintf(path_buffer, "0:%s", file_name_str);
    f_res = f_open(&file, path_buffer, FA_OPEN_EXISTING|FA_READ);  //打开文件
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);         //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    param_obj = cJSON_Parse(pFileRead);
    if(param_obj == NULL){
        MY_LOGW("cjson", "param cjson parse fail");
        return 0;
    }
    free(pFileRead);                      //释放内存块 
    f_close(&file);        //关闭文件
    if(param_obj == NULL){
        MY_LOGW("cjson", "param cjson parse fail");
        return 0;
    }
    // MY_LOG_Print("\r\n****** save param info ******\r\n\r\n");

    //motor pid param
    child_obj = cJSON_GetObjectItem(param_obj, "motor_pid");
    kp_json = cJSON_GetObjectItem(child_obj, "kp");
    ki_json = cJSON_GetObjectItem(child_obj, "ki");
    kd_json = cJSON_GetObjectItem(child_obj, "kd");
    out_max_json = cJSON_GetObjectItem(child_obj, "out_max");
    bias_allow_json = cJSON_GetObjectItem(child_obj, "bias_allow");
    AddPID_Param(&car_param.motor_pid, kp_json->valuedouble, ki_json->valuedouble, kd_json->valuedouble, out_max_json->valuedouble, bias_allow_json->valuedouble);
    //distence pid param
    child_obj = cJSON_GetObjectItem(param_obj, "dtc_pid");
    kp_json = cJSON_GetObjectItem(child_obj, "kp");
    ki_json = cJSON_GetObjectItem(child_obj, "ki");
    kd_json = cJSON_GetObjectItem(child_obj, "kd");
    out_max_json = cJSON_GetObjectItem(child_obj, "out_max");
    bias_allow_json = cJSON_GetObjectItem(child_obj, "bias_allow");
    AddPID_Param(&car_param.distence_pid, kp_json->valuedouble, ki_json->valuedouble, kd_json->valuedouble, out_max_json->valuedouble, bias_allow_json->valuedouble);
    //angle pid param
    child_obj = cJSON_GetObjectItem(param_obj, "agl_pid");
    kp_json = cJSON_GetObjectItem(child_obj, "kp");
    ki_json = cJSON_GetObjectItem(child_obj, "ki");
    kd_json = cJSON_GetObjectItem(child_obj, "kd");
    out_max_json = cJSON_GetObjectItem(child_obj, "out_max");
    bias_allow_json = cJSON_GetObjectItem(child_obj, "bias_allow");
    AddPID_Param(&car_param.angle_pid, kp_json->valuedouble, ki_json->valuedouble, kd_json->valuedouble, out_max_json->valuedouble, bias_allow_json->valuedouble);
    //pos pid param
    child_obj = cJSON_GetObjectItem(param_obj, "pos_pid");
    kp_json = cJSON_GetObjectItem(child_obj, "kp");
    ki_json = cJSON_GetObjectItem(child_obj, "ki");
    kd_json = cJSON_GetObjectItem(child_obj, "kd");
    out_max_json = cJSON_GetObjectItem(child_obj, "out_max");
    bias_allow_json = cJSON_GetObjectItem(child_obj, "bias_allow");
    AddPID_Param(&car_param.pos_pid, kp_json->valuedouble, ki_json->valuedouble, kd_json->valuedouble, out_max_json->valuedouble, bias_allow_json->valuedouble);
    cJSON_Delete(param_obj);   //删除父对象
    return 1;
}
/////////////////////////////////////pid param//////////////////////////////////////////////

/////////////////////////////////////arm//////////////////////////////////////////////
/// @brief 填充动作组链表信息
/// @param action_id 
/// @param x 坐标
/// @param y 
/// @param z 
/// @param tong_sta 手抓状态
/// @param holder_wait  是否等待载物转盘空闲
static void AddArmAction2List(arm_action_enum_t action_id, float x, float y, float z, uint8_t tong_sta, uint8_t holder_wait)
{   
    arm_action_t* temp_link = arm_action_arr[action_id];
    if(temp_link == NULL){          //先初始化头结点
        arm_action_arr[action_id] = (arm_action_t*)malloc(sizeof(arm_action_t));
        arm_action_arr[action_id]->x = x;
        arm_action_arr[action_id]->y = y;
        arm_action_arr[action_id]->z = z;
        arm_action_arr[action_id]->tong_open = tong_sta;
        arm_action_arr[action_id]->holder_wait = holder_wait;
        arm_action_arr[action_id]->next_pos = NULL;
        // //temp test
        // MY_LOG_Print("save action[%d]: x[%.1f] y[%.1f] z[%.1f] tong[%d] ov[%d]\r\n", action_id, arm_action_arr[action_id]->x, arm_action_arr[action_id]->y, arm_action_arr[action_id]->z, arm_action_arr[action_id]->tong_open, arm_action_arr[action_id]->holder_wait);
    }
    else{
        while(temp_link->next_pos != NULL)  //遍历至尾结点
        {
            temp_link = temp_link->next_pos;
        }
        temp_link->next_pos = (arm_action_t*)malloc(sizeof(arm_action_t));
        temp_link->next_pos->x = x;
        temp_link->next_pos->y = y;
        temp_link->next_pos->z = z;
        temp_link->next_pos->tong_open = tong_sta;
        temp_link->next_pos->holder_wait = holder_wait;
        temp_link->next_pos->next_pos = NULL;
        // //temp test
        // MY_LOG_Print("save action[%d]: x[%.1f] y[%.1f] z[%.1f] tong[%d] ov[%d]\r\n", action_id, arm_action_arr[action_id]->x, arm_action_arr[action_id]->y, arm_action_arr[action_id]->z, arm_action_arr[action_id]->tong_open, arm_action_arr[action_id]->holder_wait);
    }
}

/// @brief 提取机械臂动作组
/// @param file_name_str 
/// @return 
uint8_t ArmExtract(char* file_name_str)
{
    char path_buffer[30];
    sprintf(path_buffer, "0:%s", file_name_str);
    f_res = f_open(&file, path_buffer, FA_OPEN_EXISTING|FA_READ);  //打开文件
    if(f_res != FR_OK){
        MY_LOGW("fatfs", "open %s file error", file_name_str);
        FATFS_ErrorPrint(f_res);
        return 0;
    }
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);         //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    arm_obj = cJSON_Parse(pFileRead);  //从文件中提取json内容
    free(pFileRead);                      //释放内存块 
    f_close(&file);        //关闭文件
    if(arm_obj == NULL){
        MY_LOGW("cjson", "%s parse fail",file_name_str);
        return 0;
    }
    // MY_LOG_Print("\r\n****** save arm info ******\r\n\r\n");
    uint8_t pre_action_num = actions_num;
    actions_num += cJSON_GetArraySize(arm_obj);  //获取动作组数量
    MY_LOGI("arm file", "action num[%d] -- predefined num[%d]", actions_num, ARM_ACTION_NUM);
    for(uint8_t i=pre_action_num; i<actions_num; i++)  //获取所有动作组信息
    {
       char str_name[5];
        if(i >= 10){
            sprintf(str_name, "%c%c", '0'+i/10, '0'+i%10);
        }
        else{
            sprintf(str_name, "%c", '0'+i);
        }
        child_obj = cJSON_GetObjectItem(arm_obj, str_name);
        uint8_t pos_num = cJSON_GetArraySize(child_obj);  //获取单个动作运动位置数量
        for(uint8_t j=0; j<pos_num; j++)
        {
            pos_obj = cJSON_GetArrayItem(child_obj, j);
            arm_x = cJSON_GetArrayItem(pos_obj, 0);
            arm_y = cJSON_GetArrayItem(pos_obj, 1);
            arm_z = cJSON_GetArrayItem(pos_obj, 2);
            arm_tong = cJSON_GetArrayItem(pos_obj, 3);
            arm_openmv = cJSON_GetArrayItem(pos_obj, 4);
            //添加单个动作中的单个执行信息进入对应动作组的链表中
            AddArmAction2List(i, arm_x->valuedouble, arm_y->valuedouble, arm_z->valuedouble, arm_tong->valueint, arm_openmv->valueint);
        }
    }

    cJSON_Delete(arm_obj);   //删除父对象
    return 1;
}
/////////////////////////////////////arm//////////////////////////////////////////////


/////////////////////////////////////rgb//////////////////////////////////////////////

/// @brief 提取rgb参数
/// @param file_name_str 
/// @return 
uint8_t RGB_Extract(char* file_name_str)
{
    char path_buffer[30];
    sprintf(path_buffer, "0:%s", file_name_str);
    f_res = f_open(&file, path_buffer, FA_OPEN_EXISTING|FA_READ);  //打开文件
    if(f_res != FR_OK){
        MY_LOGW("fatfs", "open %s file error", file_name_str);
        FATFS_ErrorPrint(f_res);
        return 0;
    }
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);         //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    rgb_obj = cJSON_Parse(pFileRead);  //从文件中提取json内容
    free(pFileRead);                      //释放内存块 
    f_close(&file);        //关闭文件
    if(rgb_obj == NULL){
        MY_LOGW("cjson", "%s parse fail",file_name_str);
        return 0;
    }

    child_obj = cJSON_GetObjectItem(rgb_obj, "rgb");
    red = cJSON_GetArrayItem(child_obj, 0);
    green = cJSON_GetArrayItem(child_obj, 1);
    blue = cJSON_GetArrayItem(child_obj, 2);

    rgb_value.red = red->valueint;
    rgb_value.green = green->valueint;
    rgb_value.blue = blue->valueint;
    cJSON_Delete(rgb_obj);   //删除父对象
    return 1;
}

void RGB_Save(void)
{
    rgb_obj = cJSON_CreateObject();
    child_obj = cJSON_CreateArray();

    cJSON_AddItemToArray(child_obj, cJSON_CreateNumber(rgb_value.red));
    cJSON_AddItemToArray(child_obj, cJSON_CreateNumber(rgb_value.green));
    cJSON_AddItemToArray(child_obj, cJSON_CreateNumber(rgb_value.blue));

    cJSON_AddItemToObject(rgb_obj, "rgb", child_obj);

    char* str_print = cJSON_Print(rgb_obj);

    MY_LOG_Print("\r\n%s",str_print);

    f_res = f_open(&file, "0:rgb.json", FA_OPEN_ALWAYS|FA_WRITE);  //打开文件
    int str_num = strlen(str_print);
    int cnt=0;
    taskENTER_CRITICAL();
    f_res = f_write(&file, str_print, str_num+1, &cnt);
    taskEXIT_CRITICAL();
    f_close(&file);

    MY_LOGI("fl", "write[%d] infact[%d]", str_num+1, cnt);

    cJSON_Delete(rgb_obj);
}

/////////////////////////////////////arm//////////////////////////////////////////////



/// @brief 文件管理初始化
/// @param  
/// @return 
uint8_t FileInit(void)
{
    uint8_t re = 1;  //函数状态返回
    // MY_LOG_Print("\r\n****** sd card read ******\r\n\r\n");
    f_res = f_mount(&file_sys, "0:", 1);       //注册文件系统
    if(f_res != FR_OK){
        MY_LOGW("fatfs", "f_mount error");
        FATFS_ErrorPrint(f_res);\
        re = 0;
    }
    f_res = f_open(&file, "0:file_ctrl.json", FA_OPEN_EXISTING|FA_READ);  //打开文件
    if(f_res != FR_OK){
        MY_LOGW("fatfs", "open file_ctrl.json error");
        FATFS_ErrorPrint(f_res);
        re = 0;
    }
    file_size = f_size(&file);                                 
    pFileRead = malloc(file_size);        //申请内存存储文件信息
    f_res = f_read(&file, pFileRead, file_size, &file_size);
    //cjson 解析file_ctrl,获得其它文件路径
    path_obj = cJSON_Parse(pFileRead);
    free(pFileRead);                      //释放内存块
    f_close(&file);                       //关闭文件

    //提取路径信息
    child_obj = cJSON_GetObjectItem(path_obj, "route");  //对象为文件路径字符串
    if(child_obj == NULL){
        MY_LOGW("cjson", "get route obj failed");
        re = 0;
    }
    else{
        RouteExtract(child_obj->valuestring);   //保存路径信息到链表中
    }
    
    //提取参数信息
    child_obj = cJSON_GetObjectItem(path_obj, "param");
    if(child_obj == NULL){
        MY_LOGW("cjson", "get param obj failed");
        re = 0;
    }
    else{
        ParamExtract(child_obj->valuestring);  //提取参数信息
    }

    //提取机械臂动作信息
    child_obj = cJSON_GetObjectItem(path_obj, "arm");
    if(child_obj == NULL){
        MY_LOGW("cjson", "get arm obj failed");
        re = 0;
    }
    else{
        ArmExtract(child_obj->valuestring);  //提取参数信息
    }
    child_obj = cJSON_GetObjectItem(path_obj, "arm_add");
    if(child_obj == NULL){
        MY_LOGW("cjson", "get arm_add obj failed");
    }
    else{
        MY_LOGI("cjson", "get arm_add cjson");
        ArmExtract(child_obj->valuestring);  //提取参数信息
    }

    //提取rgb参数
    child_obj = cJSON_GetObjectItem(path_obj, "rgb");
    if(child_obj == NULL){
        MY_LOGW("cjson", "get param obj failed");
        re = 0;
    }
    else{
        RGB_Extract(child_obj->valuestring);  //提取参数信息
    }

    cJSON_Delete(path_obj);  //删除cjson父对象
    return re;
}
//*****************************cmd test*****************************************************************************//
/// @brief 展示存储在链表中的路径信息
/// @param head_node 
static void ShowPointList(int argc, char** argv)
{
    point_list_t temp_node = route_list;
    MY_LOG_Print("\r\n****** show point list ******\r\n\r\n");
    if(temp_node == NULL)
    {
        MY_LOGW("list", "point list is empty");
        return;
    }
        MY_LOG_Print("        head   dis   agl   event\r\n");
    for(uint8_t i=0; temp_node!= NULL; i++)
    {
        MY_LOG_Print("point[%d]: (%.1f, %.1f)   A[%.1f]    E[%d]\r\n", i, temp_node->coord_x, temp_node->coord_y, temp_node->angle, temp_node->event);
        temp_node = temp_node->next_point;
    }
}

static void ShellLoadRoute_Spl(int argc, char** argv)
{
    MY_LOGI("file", "load specil route");
    RouteExtract_Specil();
    ShowPointList(0,NULL);
}

/// @brief 打印参数信息
/// @param  
void ShowPIDParam(int argc, char** argv)
{
    MY_LOG_Print("\r\n****** show param info ******\r\n\r\n");
    MY_LOG_Print("           KP         KI         KD         MAX         BiasAllow\r\n");
    MY_LOG_Print("motor:   %8.4f,  %8.4f,  %8.4f,  %8.4f,  %8.4f\r\n", car_param.motor_pid.kp, car_param.motor_pid.ki, car_param.motor_pid.kd, car_param.motor_pid.out_max, car_param.motor_pid.bias_allow);
    MY_LOG_Print("distence:%8.4f,  %8.4f,  %8.4f,  %8.4f,  %8.4f\r\n", car_param.distence_pid.kp, car_param.distence_pid.ki, car_param.distence_pid.kd, car_param.distence_pid.out_max, car_param.distence_pid.bias_allow);
    MY_LOG_Print("angle:   %8.4f,  %8.4f,  %8.4f,  %8.4f,  %8.4f\r\n", car_param.angle_pid.kp, car_param.angle_pid.ki, car_param.angle_pid.kd, car_param.angle_pid.out_max, car_param.angle_pid.bias_allow);
    MY_LOG_Print("pos adjt:%8.4f,  %8.4f,  %8.4f,  %8.4f,  %8.4f\r\n", car_param.pos_pid.kp, car_param.pos_pid.ki, car_param.pos_pid.kd, car_param.pos_pid.out_max, car_param.pos_pid.bias_allow);
}


void ShowArmAction(int argc, char** argv)
{
    MY_LOG_Print("\r\n****** show action list ******\r\n\r\n");
    for(uint8_t i=0; i<actions_num; i++)
    {
        MY_LOG_Print("action[%d]:\r\n", i);
        arm_action_t* temp_node = arm_action_arr[i];
        if(temp_node == NULL){
            MY_LOGW("arm arr", "arr is empty");
            return;
        }
        for(uint8_t j=0; temp_node!= NULL; j++)
        {
            MY_LOG_Print("  pos[%d]: x[%.1f] y[%.1f] z[%.1f] tong[%d] ov[%d]\r\n", j, temp_node->x, temp_node->y, temp_node->z, temp_node->tong_open, temp_node->holder_wait);
            temp_node = temp_node->next_pos;
        }
        free(temp_node);
    }
}

void ShowRGB_Value(int argc, char** argv)
{
    if(argc == 2){
        if(strcmp("-s", argv[1]) == 0){
            RGB_Save();
            MY_LOGI("file", "save rgb value");
        }
        else{
            MY_LOGW("file", "param invalid");
        }
    }
    else{
        MY_LOG_Print("\r\nrgb value:R[%d], G[%d], B[%d]", rgb_value.red, rgb_value.green, rgb_value.blue);
    }
}


/// @brief 将json文件中获取的pid参数更新至控制器
/// @param param 
void UpdataParamFromFile(int argc, char** argv)
{
    if(FileInit()){
        file_state = 1;
        MY_LOGI("file", "file load");
        // IsMotorUseDefaultParam(0);
        // IsCarUseDefaultParam(0);
        // MY_LOGI("file", "update param from file");
        // SetSpeedPID(car_param.motor_pid.kp, car_param.motor_pid.ki, car_param.motor_pid.kd, car_param.motor_pid.out_max);
        // SetDistancePID(car_param.distence_pid.kp, car_param.distence_pid.ki, car_param.distence_pid.kd, car_param.distence_pid.out_max, car_param.distence_pid.bias_allow);
        // SetAnglePID(car_param.angle_pid.kp, car_param.angle_pid.ki, car_param.angle_pid.kd, car_param.angle_pid.out_max, car_param.angle_pid.bias_allow);
        // SetPosPID(car_param.pos_pid.kp, car_param.pos_pid.ki, car_param.pos_pid.kd, car_param.pos_pid.out_max, car_param.pos_pid.bias_allow);
    }
    else{
        MY_LOGW("file", "load failed");
    }

}

uint8_t IsFileReady(void)
{
    return file_state;
}
//*****************************cmd test*******************************************************************************//

void FileInfoTask(void* param)
{
    UpdataParamFromFile(0, NULL);
    ShellCmdRegister("fl_point",
                "show file route point list",
                ShowPointList);
    ShellCmdRegister("fl_param",
                "show file pid params",
                ShowPIDParam);
    ShellCmdRegister("fl_arm",
                "show file arm actions",
                ShowArmAction);
    ShellCmdRegister("fl_rgb",
                "show/save file rgb value, param:null(just show) or [\"-s\"](save)",
                ShowRGB_Value);
    ShellCmdRegister("update",
                "update params from file to controller",
                UpdataParamFromFile);
    ShellCmdRegister("fl_spl",
                "load specil route point",
                ShellLoadRoute_Spl);
    while(1)
    {
        vTaskDelay(portMAX_DELAY);
    }
    // vTaskDelete(NULL);
}




