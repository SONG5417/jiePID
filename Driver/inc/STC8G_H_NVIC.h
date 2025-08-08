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

#ifndef	__NVIC_H
#define	__NVIC_H

#include	"config.h"
#include	"STC8H_PWM.h"
#include	"STC8G_PCA.h"
#include	"STC8G_H_ADC.h"
#include	"STC8G_H_I2C.h"
#include	"STC8G_H_SPI.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_Exti.h"
#include	"STC8G_H_Timer.h"
#include	"STC8G_H_Compare.h"

//========================================================================
//                              定义声明
//========================================================================

#define	FALLING_EDGE		1		//产生下降沿中断
#define	RISING_EDGE			2		//产生上升沿中断

//========================================================================
//                           外部函数和变量声明
//========================================================================

u8 NVIC_Timer0_Init(u8 State, u8 Priority);
u8 NVIC_Timer1_Init(u8 State, u8 Priority);
u8 NVIC_Timer2_Init(u8 State, u8 Priority);
u8 NVIC_Timer3_Init(u8 State, u8 Priority);
u8 NVIC_Timer4_Init(u8 State, u8 Priority);
u8 NVIC_INT0_Init(u8 State, u8 Priority);
u8 NVIC_INT1_Init(u8 State, u8 Priority);
u8 NVIC_INT2_Init(u8 State, u8 Priority);
u8 NVIC_INT3_Init(u8 State, u8 Priority);
u8 NVIC_INT4_Init(u8 State, u8 Priority);
u8 NVIC_ADC_Init(u8 State, u8 Priority);
u8 NVIC_SPI_Init(u8 State, u8 Priority);
u8 NVIC_CMP_Init(u8 State, u8 Priority);
u8 NVIC_I2C_Init(u8 Mode, u8 State, u8 Priority);
u8 NVIC_UART1_Init(u8 State, u8 Priority);
u8 NVIC_UART2_Init(u8 State, u8 Priority);
u8 NVIC_UART3_Init(u8 State, u8 Priority);
u8 NVIC_UART4_Init(u8 State, u8 Priority);
u8 NVIC_PWM_Init(u8 Channel, u8 State, u8 Priority);
u8 NVIC_PCA_Init(u8 Channel, u8 State, u8 Priority);

#endif

