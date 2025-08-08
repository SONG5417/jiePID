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
#include	"STC8G_H_EEPROM.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_NVIC.h"

/*************	本程序功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8G、STC8H系列芯片可通用参考.

通过串口对STC内部自带的EEPROM(FLASH)进行读写测试。

对FLASH做扇区擦除、写入、读出的操作，命令指定地址。

默认波特率:  115200,N,8,1. 

串口命令设置: (命令字母不区分大小写)
    E 0x0040             --> 对0x0040地址扇区内容进行擦除.
    W 0x0040 1234567890  --> 对0x0040地址写入字符1234567890.
    R 0x0040 10          --> 对0x0040地址读出10个字节数据. 

注意：下载时，下载界面"硬件选项"中设置用户EEPROM大小，

并确保串口命令中的地址在EEPROM设置的大小范围之内。

下载时, 选择时钟 22.1184MHz (可以在配置文件"config.h"中修改).

******************************************/


//========================================================================
//                               本地常量声明	
//========================================================================

#define     Max_Length       100    //读写EEPROM缓冲长度

//========================================================================
//                               本地变量声明
//========================================================================

u8  xdata   tmp[Max_Length];        //EEPROM操作缓冲

//========================================================================
//                               本地函数声明
//========================================================================


//========================================================================
//                            外部函数和变量声明
//========================================================================


//========================================================================
// 函数: EEPROM_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void EEPROM_init(void)
{
	COMx_InitDefine		COMx_InitStructure;					//结构定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//使用波特率, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

	PrintString1("STC8系列单片机EEPROM测试程序，串口命令设置如下:\r\n");    //UART1发送一个字符串
	PrintString1("E 0x0040             --> 对0x0040地址扇区内容进行擦除.\r\n");     //UART1发送一个字符串
	PrintString1("W 0x0040 1234567890  --> 对0x0040地址写入字符1234567890.\r\n");  //UART1发送一个字符串
	PrintString1("R 0x0040 10          --> 对0x0040地址读出10个字节内容.\r\n");    //UART1发送一个字符串
}

//========================================================================
// 函数: CheckData
// 描述: 数据校验函数.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
u8	CheckData(u8 dat)
{
	if((dat >= '0') && (dat <= '9'))		return (dat-'0');
	if((dat >= 'A') && (dat <= 'F'))		return (dat-'A'+10);
	if((dat >= 'a') && (dat <= 'f'))		return (dat-'a'+10);
	return 0xff;
}

//========================================================================
// 函数: GetAddress
// 描述: 计算各种输入方式的地址.
// 参数: 无.
// 返回: 16位EEPROM地址.
// 版本: V1.0, 2013-6-6
//========================================================================
u16 GetAddress(void)
{
	u16 address;
	u8  i,j;
	
	address = 0;
	if((RX1_Buffer[2] == '0') && (RX1_Buffer[3] == 'X'))
	{
		for(i=4; i<8; i++)
		{
			j = CheckData(RX1_Buffer[i]);
			if(j >= 0x10)   return 65535;   //error
			address = (address << 4) + j;
		}
		return (address);
	}
	return  65535;  //error
}

//========================================================================
// 函数: GetDataLength
// 描述: 获取要读出数据的字节数.
// 参数: 无.
// 返回: 1要读出数据的字节数.
// 版本: V1.0, 2013-6-6
//========================================================================
u8 GetDataLength(void)
{
	u8  i;
	u8  length;
	
	length = 0;
	for(i=9; i<COM1.RX_Cnt; i++)
	{
		if(CheckData(RX1_Buffer[i]) >= 10)  break;
		length = length * 10 + CheckData(RX1_Buffer[i]);
	}
	return (length);
}

//========================================================================
// 函数: Sample_EEPROM
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void Sample_EEPROM(void)
{
	u8  i,j;
	u16 addr;
	u8  status;

	if(COM1.RX_TimeOut > 0)		//超时计数
	{
		if(--COM1.RX_TimeOut == 0)
		{
			for(i=0; i<COM1.RX_Cnt; i++)    TX1_write2buff(RX1_Buffer[i]);    //把收到的数据原样返回,用于测试

			status = 0xff;  //状态给一个非0值
			if((COM1.RX_Cnt >= 8) && (RX1_Buffer[1] == ' ')) //最短命令为8个字节
			{
				for(i=0; i<8; i++)
				{
					if((RX1_Buffer[i] >= 'a') && (RX1_Buffer[i] <= 'z'))    RX1_Buffer[i] = RX1_Buffer[i] - 'a' + 'A';  //小写转大写
				}
				addr = GetAddress();
				if(addr < 63488)    //限制在0~123扇区
				{
					if(RX1_Buffer[0] == 'E')    //写入N个字节
					{
						EEPROM_SectorErase(addr);           //擦除扇区
						PrintString1("擦除成功！\r\n");
						status = 0; //命令正确
					}

					else if((RX1_Buffer[0] == 'W') && (RX1_Buffer[8] == ' '))    //写入N个字节
					{
						j = COM1.RX_Cnt - 9;
						if(j > Max_Length)  j = Max_Length; //越界检测
						//EEPROM_SectorErase(addr);           //擦除扇区
						EEPROM_write_n(addr,&RX1_Buffer[9],j);      //写N个字节
						PrintString1("已写入");
						if(j >= 100)    {TX1_write2buff(j/100+'0');   j = j % 100;}
						if(j >= 10)     {TX1_write2buff(j/10+'0');    j = j % 10;}
						TX1_write2buff(j%10+'0');
						PrintString1("字节！\r\n");
						status = 0; //命令正确
					}

					else if((RX1_Buffer[0] == 'R') && (RX1_Buffer[8] == ' '))   //PC请求返回N字节EEPROM数据
					{
						j = GetDataLength();
						if(j > Max_Length)  j = Max_Length; //越界检测
						if(j > 0)
						{
							PrintString1("读出");
							TX1_write2buff(j/10+'0');
							TX1_write2buff(j%10+'0');
							PrintString1("个字节内容如下：\r\n");
							EEPROM_read_n(addr,tmp,j);
							for(i=0; i<j; i++)  TX1_write2buff(tmp[i]);
							TX1_write2buff(0x0d);
							TX1_write2buff(0x0a);
							status = 0; //命令正确
						}
					}
				}
			}
			if(status != 0) PrintString1("命令错误！\r\n");
			COM1.RX_Cnt = 0;
		}
	}
}
