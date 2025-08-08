/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCMCU.com --------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/

#include	"APP.h"
#include	"STC8G_PCA.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_NVIC.h"

/*************   功能说明   ***************

本例程基于STC8G1K08-20PIN进行编写测试，STC8G系列芯片可通用参考.

输出3路变化的PWM信号, 类似"呼吸灯"的驱动.
PWM0  为8位PWM.
PWM1  为7位PWM.
PWM2  为10位PWM.

下载时, 选择时钟 24MHz (用户可在"config.h"修改频率).

******************************************/


//========================================================================
//                               本地常量声明	
//========================================================================


//========================================================================
//                               本地变量声明
//========================================================================

static u16	pwm0,pwm1,pwm2;
static bit	B_PWM0_Dir,B_PWM1_Dir,B_PWM2_Dir;	//方向, 0为+, 1为-.

//========================================================================
//                               本地函数声明
//========================================================================


//========================================================================
//                            外部函数和变量声明
//========================================================================


//========================================================================
// 函数: PCA_PWM_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void PCA_PWM_init(void)
{
	PCA_InitTypeDef		PCA_InitStructure;

	P1_MODE_IO_PU(GPIO_Pin_All);		//P1 设置为准双向口
	P3_MODE_IO_PU(GPIO_Pin_7);			//P3.7 设置为准双向口
	//--------------------------------------------
	PCA_InitStructure.PCA_Clock    = PCA_Clock_1T;		//PCA_Clock_1T, PCA_Clock_2T, PCA_Clock_4T, PCA_Clock_6T, PCA_Clock_8T, PCA_Clock_12T, PCA_Clock_Timer0_OF, PCA_Clock_ECI
	PCA_InitStructure.PCA_RUN      = DISABLE;				//ENABLE, DISABLE
	PCA_Init(PCA_Counter,&PCA_InitStructure);			//设置公用Counter

	PCA_InitStructure.PCA_PWM_Wide = PCA_PWM_8bit;		//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 128 << 8;				//对于PWM,高8位为PWM占空比
	PCA_Init(PCA0,&PCA_InitStructure);

	PCA_InitStructure.PCA_PWM_Wide = PCA_PWM_7bit;		//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 64 << 8;					//对于PWM,高8位为PWM占空比
	PCA_Init(PCA1,&PCA_InitStructure);

	PCA_InitStructure.PCA_PWM_Wide = PCA_PWM_10bit;		//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit, PCA_PWM_10bit
	PCA_InitStructure.PCA_Value    = 32 << 8;					//对于PWM,高8位为PWM占空比
	PCA_Init(PCA2,&PCA_InitStructure);

	NVIC_PCA_Init(PCA_Counter,PCA_Mode_PWM,Priority_0);
	NVIC_PCA_Init(PCA0,PCA_Mode_PWM,Priority_0);
	NVIC_PCA_Init(PCA1,PCA_Mode_PWM,Priority_0);
	NVIC_PCA_Init(PCA2,PCA_Mode_PWM,Priority_0);
	CR = 1;							//启动PCA
	//--------------------------------------------
	pwm0 = 128;
	pwm1 = 64;
	pwm2 = 512;
	B_PWM0_Dir = 0;
	B_PWM1_Dir = 0;
	B_PWM2_Dir = 0;

	UpdatePcaPwm(PCA0,pwm0);
	UpdatePcaPwm(PCA1,pwm1);
	UpdatePcaPwm(PCA2,pwm2);
}


//========================================================================
// 函数: Sample_PCA_PWM
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void Sample_PCA_PWM(void)
{
	if(B_PWM0_Dir)
	{
		if(--pwm0 <= 16)	B_PWM0_Dir = 0;	//8位PWM
	}
	else	if(++pwm0 >= 240)	B_PWM0_Dir = 1;	//8位PWM
	UpdatePcaPwm(PCA0,pwm0);

	if(B_PWM1_Dir)
	{
		if(--pwm1 <= 8)		B_PWM1_Dir = 0;	//7位PWM
	}
	else	if(++pwm1 >= 120)	B_PWM1_Dir = 1;	//7位PWM
	UpdatePcaPwm(PCA1,pwm1);

	if(B_PWM2_Dir)
	{
		if(--pwm2 <= 24)		B_PWM2_Dir = 0;	//10位PWM
	}
	else	if(++pwm2 >= 1000)	B_PWM2_Dir = 1;	//10位PWM
	UpdatePcaPwm(PCA2,pwm2);
}



