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
#include	"STC8G_H_GPIO.h"
#include	"STC8G_PWM15bit.h"

/*************   功能说明   ***************

本例程基于STC8G2K64进行编写测试，STC8G系列带15位增强型PWM芯片可通用参考.

输出4路PWM信号：
PWM00,PWM01  输出"呼吸灯"效果PWM.
PWM02  循环输出1秒钟PWM，1秒钟高电平，1秒钟低电平.
PWM03  固定PWM.

下载时, 选择时钟 24MHz (用户可在"config.h"修改频率).

******************************************/


//========================================================================
//                               本地常量声明	
//========================================================================


//========================================================================
//                               本地变量声明
//========================================================================

u16	pwm0,pwm1,pwm2;
bit	B_PWM0_Dir,B_PWM1_Dir;	//方向, 0为+, 1为-.

//========================================================================
//                               本地函数声明
//========================================================================


//========================================================================
//                            外部函数和变量声明
//========================================================================


//========================================================================
// 函数: PWM15bit_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void PWM15bit_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;				//结构定义
	PWM15_InitTypeDef		PWM15_InitStructure;

	GPIO_InitStructure.Pin  = GPIO_Pin_All;			//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7, 或操作
	GPIO_InitStructure.Mode = GPIO_PullUp;			//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P0,&GPIO_InitStructure);	//初始化

	PWM15_InitStructure.PWM_Enable    = ENABLE;		//PWM使能,  ENABLE, DISABLE
	PWM15_InitStructure.PWM_Period    = 0x1000;		//PWM周期,  0~0x7fff
	PWM15_InitStructure.PWM_Clock_Sel = PWMn_CLK_SYS;		//时钟源选择,  PWMn_CLK_SYS, PWMn_CLK_TM2
	PWM15_InitStructure.PWM_Clock_PS  = 0;				//系统时钟分频参数(PS+1分频),  0~15
	PWM15_InitStructure.PWM_CInt      = DISABLE;	//计数器归零中断使能,  ENABLE, DISABLE
	PWM15_InitStructure.PWM_Counter   = ENABLE;		//计数器使能,  ENABLE, DISABLE
	PWM15_Init(PWM0,&PWM15_InitStructure);
	
	PWM15Duty(PWM00,0x000,0x200);		//PWM_ID, 输出低电平位置, 输出高电平位置
	PWM15Duty(PWM01,0x000,0x400);		//PWM_ID, 输出低电平位置, 输出高电平位置
	PWM15Duty(PWM02,0x000,0x800);		//PWM_ID, 输出低电平位置, 输出高电平位置
	PWM15Duty(PWM03,0x200,0xc00);		//PWM_ID, 输出低电平位置, 输出高电平位置
	PWMChannelCtrl(PWM00,ENABLE,0,DISABLE,DISABLE,DISABLE);	//PWM_ID, 输出使能, 初始电平, PWM中断, 第二个触发点中断, 第一触发点中断
	PWMChannelCtrl(PWM01,ENABLE,0,DISABLE,DISABLE,DISABLE);	//PWM_ID, 输出使能, 初始电平, PWM中断, 第二个触发点中断, 第一触发点中断
	PWMChannelCtrl(PWM02,ENABLE,0,DISABLE,DISABLE,DISABLE);	//PWM_ID, 输出使能, 初始电平, PWM中断, 第二个触发点中断, 第一触发点中断
	PWMChannelCtrl(PWM03,ENABLE,0,DISABLE,DISABLE,DISABLE);	//PWM_ID, 输出使能, 初始电平, PWM中断, 第二个触发点中断, 第一触发点中断

	pwm0 = 0;
	pwm1 = 0;
	pwm2 = 0;
	B_PWM0_Dir = 0;
	B_PWM1_Dir = 0;
}

//========================================================================
// 函数: Sample_PWM15bit
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void Sample_PWM15bit(void)
{
	if(B_PWM0_Dir)
	{
		if(--pwm0 <= 0x100)	B_PWM0_Dir = 0;	//PWM00
	}
	else	if(++pwm0 >= 0xf00)	B_PWM0_Dir = 1;	//PWM00
	PWM15Duty(PWM00,0,pwm0);

	if(B_PWM1_Dir)
	{
		if(--pwm1 <= 0x100)		B_PWM1_Dir = 0;	//PWM01
	}
	else	if(++pwm1 >= 0xf00)	B_PWM1_Dir = 1;	//PWM01
	PWM15Duty(PWM01,0,pwm1);

	if(pwm2 >= 3000)
	{
		pwm2 = 0;
	}
	else	if(pwm2 >= 2000)
	{
		PWMLevelSet(PWM02,ENABLE,DISABLE);	//PWM_ID, 强制输出低电平, 强制输出高电平
	}
	else	if(pwm2 >= 1000)
	{
		PWMLevelSet(PWM02,DISABLE,ENABLE);	//PWM_ID, 强制输出低电平, 强制输出高电平
	}
	else
	{
		PWMLevelSet(PWM02,DISABLE,DISABLE);	//PWM_ID, 强制输出低电平, 强制输出高电平
	}
	pwm2++;
}



