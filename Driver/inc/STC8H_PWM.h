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

#ifndef __PWM_H
#define __PWM_H	 

#include	"config.h"

#define ENO1P       0x01
#define ENO1N       0x02
#define ENO2P       0x04
#define ENO2N       0x08
#define ENO3P       0x10
#define ENO3N       0x20
#define ENO4P       0x40
#define ENO4N       0x80

#define ENO5P       0x01
#define ENO6P       0x04
#define ENO7P       0x10
#define ENO8P       0x40

#define	PWM1	1
#define	PWM2	2
#define	PWM3	3
#define	PWM4	4
#define	PWM5	5
#define	PWM6	6
#define	PWM7	7
#define	PWM8	8

#define	PWMA	128
#define	PWMB	129

#define PWM_BIE     0x80
#define PWM_TIE     0x40
#define PWM_COMIE   0x20
#define PWM_CC4IE   0x10
#define PWM_CC3IE   0x08
#define PWM_CC2IE   0x04
#define PWM_CC1IE   0x02
#define PWM_CC8IE   0x10
#define PWM_CC7IE   0x08
#define PWM_CC6IE   0x04
#define PWM_CC5IE   0x02
#define PWM_UIE     0x01

typedef struct
{ 
	u8	PWM_Mode;			//模式,   CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	u16	PWM_Period;		//周期时间,   0~65535
	u16	PWM_Duty;			//占空比时间, 0~Period
	u8	PWM_DeadTime;	//死区发生器设置, 0~255
//	u8	PWM_Reload;				//输出比较的预装载使能,   ENABLE,DISABLE
//	u8	PWM_Fast;					//输出比较快速功能使能,   ENABLE,DISABLE
//	u8	PWM_PreLoad;			//预装载,     ENABLE,DISABLE
//	u8	PWM_BrakeEnable;	//刹车输入使能,  ENABLE,DISABLE
	u8	PWM_EnoSelect;		//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	u8	PWM_CEN_Enable;		//使能计数器, ENABLE,DISABLE
	u8	PWM_MainOutEnable;//主输出使能,  ENABLE,DISABLE
} PWMx_InitDefine; 

typedef struct
{ 
	u16	PWM1_Duty;			//PWM1占空比时间, 0~Period
	u16	PWM2_Duty;			//PWM2占空比时间, 0~Period
	u16	PWM3_Duty;			//PWM3占空比时间, 0~Period
	u16	PWM4_Duty;			//PWM4占空比时间, 0~Period
	u16	PWM5_Duty;			//PWM5占空比时间, 0~Period
	u16	PWM6_Duty;			//PWM6占空比时间, 0~Period
	u16	PWM7_Duty;			//PWM7占空比时间, 0~Period
	u16	PWM8_Duty;			//PWM8占空比时间, 0~Period
} PWMx_Duty; 

u8	PWM_Configuration(u8 PWM, PWMx_InitDefine *PWMx);
void UpdatePwm(u8 PWM, PWMx_Duty *PWMx);

#endif

