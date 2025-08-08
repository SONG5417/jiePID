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

#include	"APP_AD_UART2.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_ADC.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"
#include	"APP_PWM.h"
#include    "APP_PWMB_Output.h"  // 添加头文件引用


/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8G、STC8H系列芯片可通用参考.

本程序演示多路ADC查询采样，通过串口2发送给上位机，波特率115200,N,8,1。

用定时器做波特率发生器，建议使用1T模式(除非低波特率用12T)，并选择可被波特率整除的时钟频率，以提高精度。

下载时, 选择时钟 22.1184MHz (可以在配置文件"config.h"中修改).

******************************************/


//========================================================================
//                               本地常量声明	
//========================================================================


//========================================================================
//                               本地变量声明
//========================================================================
bit ad_flag;
//u8 index = 0;
u16 get_adc0;
u16 get_adc1;
//========================================================================
//                               本地函数声明
//========================================================================


//========================================================================
//                            外部函数和变量声明
//========================================================================
extern u8 start_flag;//状态机主流程标志

//========================================================================
// 函数: ADtoUART_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void ADtoUART_init(void)
{
	ADC_InitTypeDef		ADC_InitStructure;		//结构定义
//	COMx_InitDefine		COMx_InitStructure;					//结构定义
  
	P1_MODE_IN_HIZ(GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_0);		//P1.0~P1.3 设置为高阻输入
//	P1_MODE_IO_PU(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
//	P4_MODE_IO_PU(GPIO_Pin_6 | GPIO_Pin_7);	//P4.6,P4.7 设置为准双向口
	
//	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
////	COMx_InitStructure.UART_BRT_Use   = BRT_Timer2;			//选择波特率发生器, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2, 所以不用选择)
//	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率,     110 ~ 115200
//	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
//	UART_Configuration(UART2, &COMx_InitStructure);		//初始化串口2 USART1,USART2,USART3,USART4
//	NVIC_UART2_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	ADC_InitStructure.ADC_SMPduty   = 15;		//ADC 模拟信号采样时间控制, 0~31（注意： SMPDUTY 一定不能设置小于 10）
	ADC_InitStructure.ADC_CsSetup   = 0;		//ADC 通道选择时间控制 0(默认),1
	ADC_InitStructure.ADC_CsHold    = 0;		//ADC 通道选择保持时间控制 0,1(默认),2,3
	ADC_InitStructure.ADC_Speed     = ADC_SPEED_2X1T;		//设置 ADC 工作时钟频率	ADC_SPEED_2X1T~ADC_SPEED_2X16T
	ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;	//ADC结果调整,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
	ADC_Inilize(&ADC_InitStructure);		//初始化
	ADC_PowerControl(ENABLE);						//ADC电源开关, ENABLE或DISABLE
	NVIC_ADC_Init(DISABLE,Priority_0);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

//========================================================================
// 函数: Sample_ADtoUART
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-24
//========================================================================
void Sample_ADtoUART(void)
{
	// ADC采样 - 必须执行，否则PID控制无法获取当前位置
   get_adc1= Get_ADCResult(6);
	if(start_flag == 20) {  // 只在PID控制模式下执行
        Parameter_adjustment();     // PWM参数调整
//		            if(abs(get_adc1 - Target) < 50) //用于"到位判定"，只影响是否发送到位信号
//            {  
//            // 发送PID控制目标位置确认信息
//                TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0x20);
//            }
    }

	//函数作用：这是一个PID控制器的实现，用于控制电机的位置
//它通过比较目标位置(Target)和当前位置(get_adc1)来计算误差
//根据误差使用PID算法计算输出值，然后调整PWM占空比来控制电机运动
//当位置误差小于死区阈值时，会停止电机运动
//为什么每20ms执行一次：
//从代码中可以看到，这个函数是在Sample_ADtoUART()中被调用的
//根据Task.c中的配置，这个任务每500ms执行一次
//20ms的执行频率是PID控制的一个常见采样周期，这个频率：
//足够快以保持控制的精确性
//又不会太快导致系统资源浪费
//能够及时响应位置变化
//符合大多数电机控制系统的需求
//	u16	j;
//  if(ad_flag)
//	{get_adc1= Get_ADCResult(6);}//IP1S
//	else
//	{get_adc0= Get_ADCResult(0);}//IP2S
//  ad_flag=!ad_flag;	
//	P12=!P12;

//	get_adc=j;
// 	TX2_write2buff('A');
// 	TX2_write2buff('D');
// 	TX2_write2buff(index+'0');
// 	TX2_write2buff('=');
// 	TX2_write2buff(j/1000 + '0');
// 	TX2_write2buff(j%1000/100 + '0');
// 	TX2_write2buff(j%100/10 + '0');
// 	TX2_write2buff(j%10 + '0');
// 	TX2_write2buff(' ');
// 	TX2_write2buff(' ');

// 	index++;
// 	if(index > 4)
// 	{
// 		index = 0;
// 		PrintString2("\r\n");
// 	}
}



