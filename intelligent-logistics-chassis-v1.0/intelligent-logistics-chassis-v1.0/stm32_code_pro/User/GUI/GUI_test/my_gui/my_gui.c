#include "my_gui/my_gui.h"

#include "ui.h"

#include "my_log.h"
#include "m_shell.h"
// #include "display_3D.h"

#include "arm_math.h"//ARM???DSP?
#include "math.h"

#include "my_task/my_task.h"

static uint8_t isEnLvgl = 0;




/////////3D cube///////////////////////////////////////
//#define PI 3.1415926
#define XX 0.05
#define YY 0.05
#define ZZ 0.01

float box[8][3]={{-50,-50,-50},{-50,50,-50},{50,50,-50},{50,-50,-50},{-50,-50,50},{-50,50,50},{50,50,50},{50,-50,50}};
float box_dis[8][3]={{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};			
int lineid[]={1,2,2,3,3,4,4,1,5,6,6,7,7,8,8,5,8,4,7,3,6,2,5,1};//

uint8_t stopFlag =0;



void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //?????? 
	delta_y=y2-y1;
	uRow=x1;//??????
	uCol=y1;
	if(delta_x>0)incx=1; //?????? 
	else if (delta_x==0)incx=0;//??? 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//??? 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //????????? 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_Color_DrawPoint(uRow,uCol,color);//??
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


//arm_dot_prod_f32(pSrcA, pSrcB, 5, &result);??
void matconv1(float a[3],float b[3][3],float* out)//??DSP???????
{
	float res0,res1,res2,aa[3],bb0[3],bb1[3],bb2[3];
	float bbb[3];
	for(int i=0;i<3;i++)aa[i]=a[i];
	for(int i=0;i<3;i++)bb0[i]=b[0][i];
	for(int i=0;i<3;i++)bb1[i]=b[1][i];
	for(int i=0;i<3;i++)bb2[i]=b[2][i];
	arm_dot_prod_f32(aa, bb0, 3, &res0);
	arm_dot_prod_f32(aa, bb1, 3, &res1);
	arm_dot_prod_f32(aa, bb2, 3, &res2);
	out[0]=res0;out[1]=res1;out[2]=res2;
}

/*
arm_cos_f32(float32_t x)
arm_sin_f32(float32_t x)
*/
void rotation_matrix(float cube[8][3],float x,float y,float z,float cube_dis[8][3])//?????????
{
	x/=PI;y/=PI;z/=PI;
	float cube_m[8][3];
	float point [8][3];
	float p[3];
	//?????????
	float rz[3][3]={{arm_cos_f32(z),-arm_sin_f32(z),0},
									{arm_sin_f32(z),arm_cos_f32(z), 0},
									{0  ,     0  ,  1}};
	
	float rx[3][3]={{1    ,  0  , 0  },
									{0,arm_cos_f32(x),-arm_sin_f32(x)},
									{0,arm_sin_f32(x),arm_cos_f32(x)}};
	
	float ry[3][3]={{arm_cos_f32(y),0,arm_sin_f32(y)},
									{0   ,  1 ,    0},
									{-arm_sin_f32(y),0,arm_cos_f32(y)}};
	for(int i=0;i<8;i++)
	{
		
		matconv1(cube[i],rx,p);
		matconv1(p,ry,p);
		matconv1(p,rz,p);
		for(int j=0;j<3;j++)
		{
			point [i][j] = p[j];
		}
	}
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<3;j++)
		{
			cube_dis[i][j] = point [i][j];
		}
	}
}
void Draw_Cube(void)//???????
{
    float x=0, y=0, z=0;  //????????????
	x = 0;
	y = 0;
	z = 0;

	LCD_Clear(BLACK); 
	char temp[30];
	sprintf(temp, "bat voltage:%.1f", CheckBatteryVoltage());
	LCD_DisplayString_color(0,0, 24, temp, WHITE, BLACK);

	while(1)
	{
		if(isEnLvgl){
			return;
		}


		for(int i=0;i<24;i+=2)  //?????????
        {   
            // LCD_SetColors(LCD_COLOR_WHITE,LCD_COLOR_BLACK);//?????LCD??
            LCD_DrawLine(160+box_dis[lineid[i]-1][0],120+box_dis[lineid[i]-1][1],160+box_dis[lineid[i+1]-1][0],120+box_dis[lineid[i+1]-1][1], BLACK);
        }

		x = x + XX;
		y = y + YY;
		z = z + ZZ;//??????????
		rotation_matrix( box, x, y, z,box_dis);//?????????

		for(int i=0;i<24;i+=2)
        {   
            LCD_DrawLine(160+box_dis[lineid[i]-1][0],120+box_dis[lineid[i]-1][1],160+box_dis[lineid[i+1]-1][0],120+box_dis[lineid[i+1]-1][1], WHITE);
        }
        vTaskDelay(10);
	}
}

/////////3D cube///////////////////////////////////////

void IsEnableLvgl(uint8_t en)
{
    isEnLvgl = en;
}

void IsGuiTaskHighPriority(uint8_t en)
{
    uint8_t priority;
    if(en){
        priority = 4;
    }
    else{
        priority = 2;
    }
    
    TaskHandle_t task =  xTaskGetHandle("gui task");
    if(task == NULL){
        MY_LOGW("gui", "task name is wrong");
        return;
    }
    vTaskPrioritySet(task, priority);
    MY_LOGI("gui", "task prioroty set[%d]", priority);
}


static void SetGuiTaskPriority(int argc, char** argv)
{
    if(argc != 2){
        MY_LOGW("gui", "param num invalid");
        return;
    }

    uint8_t priority = atoi(argv[1]);
    TaskHandle_t task =  xTaskGetHandle("gui task");
    if(task == NULL){
        MY_LOGW("gui", "task name is wrong");
        return;
    }
    vTaskPrioritySet(task, priority);
    MY_LOGI("gui", "task prioroty set[%d]", priority);
}

static void GUI_Init(void)
{
    ShellCmdRegister("gui",
                    "set gui task priority, param:[priority]",
                    SetGuiTaskPriority);
    //lcd
    HAL_Delay(50);
    LCD_Init();        
    Set_Display_Mode(1);  //����ģʽ
    //touch
    XPT2046_Init();		
    //lvgl
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    ui_init();
}

void GUI_Task(void* param)
{
    GUI_Init();
    while(1)
    {
        if(isEnLvgl)
        {
            lv_timer_handler();
            vTaskDelay(5);
        }
        else
        {
            // vTaskDelay(20);
            Draw_Cube();   //???
        }
    }
}

