/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCMCU.com --------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码，请在程序中注明使用了STC的资料及程序 */
/* 编码: GB2312 */
/*---------------------------------------------------------------------*/
#include	"APP.h"
#include	"APP_INT_UART1.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_Exti.h"
#include	"STC8G_H_UART.h"
#include	"STC8G_H_Delay.h"
#include	"STC8G_H_NVIC.h"
#include    "pid_storage.h"
#include    "APP_PWMB_Output.h"  // 添加头文件引用

// 全局变量转为局部变量
// 通过extern声明在其他文件中定义
// 只在一个文件中定义，避免重复定义影响PID计算
// 通过联合体可以在需要时转换为字节数组，传输时转换为数值
typedef union {
    float f_val;
    unsigned char c_val[4];
} float_byte_union;

// 全局变量声明为float类型
extern float g_kP, g_kI, g_kD;
extern float deviation_SUM, err_now, err_bef;
extern u16 Target;

/*************  功能说明  **************

函数名    功能    参数值    版本    备注

INTtoUART_init    用户初始化    None    V1.0    初始化程序
Sample_INTtoUART  串口收发回调  None    V1.0    串口收发处理

******************************************/

//========================================================================
//                               全局变量
//========================================================================

//u8 WakeUpCnt;
u8 start_flag=0;//状态机启动标志
u8 MOTOR_flag;//电机状态标志
u16 IP1S1,IP2S1,IP1S2,IP2S2,IP1S3,IP2S3;
//u16 IP1S2,IP2S2,IP1S3,IP2S3;//备用变量

//========================================================================
//                               全局函数
//========================================================================

//========================================================================
//                           局部变量结构体
//========================================================================
extern void Parameter_adjustment(void);
extern PWMx_Duty PWMB_Duty;
extern u16 Calculate_PWM_Duty(u8 duty_percent);  // 添加函数声明

//========================================================================
// 函数: INTtoUART_init
// 描述: 用户初始化
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void INTtoUART_init(void)
{
//	EXTI_InitTypeDef	Exti_InitStructure;							//外部中断结构体定义
	COMx_InitDefine		COMx_InitStructure;					//串口结构体定义

	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//使用定时器, BRT_Timer1, BRT_Timer2 (注: 定时器2使用时使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一帧 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收使能,   ENABLE/DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE/DISABLE
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1 UART1,UART2,UART3,UART4
	NVIC_UART1_Init(ENABLE,Priority_1);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3
	//------------------------------------------------
// 	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;//外部中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
// 	Ext_Inilize(EXT_INT0,&Exti_InitStructure);				//初始化
// 	NVIC_INT0_Init(ENABLE,Priority_0);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3

// 	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;//外部中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
// 	Ext_Inilize(EXT_INT1,&Exti_InitStructure);				//初始化
// 	NVIC_INT1_Init(ENABLE,Priority_0);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3

// 	NVIC_INT2_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3
// 	NVIC_INT3_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3
// 	NVIC_INT4_Init(ENABLE,NULL);		//中断使能, ENABLE/DISABLE; 中断优先级(越小优先级越高) Priority_0,Priority_1,Priority_2,Priority_3

	P3_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);		//P3.0~P3.1 端口为准双向口
}

// PIDľ̬����
void update_PID_params(u16 p, u8 i, u8 d) {  // P是16位，I和D是8位
    // 将整数参数转换为浮点数参数，提高精度
    // 例如：p=100 转换为 g_kP=1.0f，支持小数点精度 P参数是16位，可以支持更大范围
    g_kP = (float)p / PID_SCALE;  // 将整数参数转换为浮点数参数
    // I和D参数也进行缩放，提高精度
    g_kI = (float)i / 10; // I参数直接转换
    g_kD = (float)d / 10; // D参数缩放转换
    
    // 重置PID计算变量
    deviation_SUM = 0.0f; // 积分清零
    err_now = 0.0f;       // 当前误差清零
    err_bef = 0.0f;       // 上次误差清零
    
    PID_SaveParams();  // 保存到EEPROM
}

//========================================================================
// : Sample_INTtoUART 串口收发回调
// 描述: 用户应调用此函数.串口收发处理
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-24
//========================================================================
void Sample_INTtoUART(void)
{
    u8 ch, duty;
    u16 freq;
    u16 target_adc;

    u8 pwm_direction;  // 方向
    u8 pwm_duty;       // 占空比
    
    if(COM1.RX_Cnt >= 7)  // 确认接收到的数据帧长度
    {
        if(COM1.RX_TimeOut > 0)		//超时处理
        {
            if(--COM1.RX_TimeOut == 0)
            {
                if(COM1.RX_Cnt >=7)
                {
                    if(RX1_Buffer[0]==0xA5)
                    {
                        switch(RX1_Buffer[2]) {        
                            case 0xF0:  // 停止
                                {
                                    start_flag = 0;
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF0);
                                }
                            break;
                                
                            case 0xF1:  // 停止
                                {
                                    start_flag = 1;
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF1);
                                }
                            break;

                            case 0xF2:  // 发送指令
                                // 从接收缓冲区读取参数
                                pwm_direction = RX1_Buffer[3];  // 0x00转0x01转
                                pwm_duty = RX1_Buffer[4];       // 占空比，0-100
                                
                                // 使用当前PWM频率，pwm_freq，默认2000Hz
                                // 频率通过0xFA指令指定，存储在APP_PWMB_Output.c中的pwm_freq
                                // 频率对应的PWM值通过Calculate_PWM_Duty动态计算
                                pwm_value = Calculate_PWM_Duty(pwm_duty);
                                
                                // 设置当前状态
                                if(pwm_direction == 0x00) {
                                    start_flag = 2;  // 正转
                                } else if(pwm_direction == 0x01) {
                                    start_flag = 3;  // 反转
                                }
                                
                                // 发送确认信息
                                TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF2);
                                break;

//                            case 0xF3:  // 设置PID死区
//                                {
//                                    // 设置16位死区值，高8位+低8位
//                                    u16 deadzone = (RX1_Buffer[3] << 8) | RX1_Buffer[4]; // 为16位死区值
//                                    g_pid_deadzone = deadzone; // 设置PID死区值
//                                    // 发送确认信息
//                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF3);
//                                }
//                            break;

//                            case 0xF4:  // 查询PID死区
//                                {
//                                    // 发送十六进制帧头 + 指令 + 死区高8位 + 死区低8位
//                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF4);
//                                    TX1_write2buff((g_pid_deadzone >> 8) & 0xFF);  // 死区高8位
//                                    TX1_write2buff(g_pid_deadzone & 0xFF);         // 死区低8位
//                                }
//                                break;  

                            case 0xF5:  // 发送PID参数
                                {  // 支持高精度PID，P为16位，I和D为8位
                                    // 数据长度：帧头(2) + 指令(1) + P(8位) + P(8位) + I(1) + D(1) + 校验(1) = 8字节
                                    // P参数为16位
                                    u16 new_kP = (RX1_Buffer[3] << 8) | RX1_Buffer[4];  // 高8位 + 低8位
                                    // I和D参数为8位
                                    u8 new_kI = RX1_Buffer[5];  // 8位
                                    u8 new_kD = RX1_Buffer[6];  // 8位
                                    
                                    update_PID_params(new_kP, new_kI, new_kD);
                                    
                                    // 发送确认信息
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF5);
                                }
                                break;

                            case 0xF6:  // 查看PID参数
                                {
                                    // 将浮点数参数转换为十六进制格式
                                    u16 kp_int = (u16)(g_kP * PID_SCALE);  // P为16位
                                    u8 ki_int = (u8)(g_kI * 10);    // I为8位，直接转换
                                    u8 kd_int = (u8)(g_kD * 10);    // D为8位，缩放转换
                                    
                                    // 发送十六进制帧头 + 指令 + P(8位) + P(8位) + I + D
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF6);
                                    TX1_write2buff((kp_int >> 8) & 0xFF);  // P高8位
                                    TX1_write2buff(kp_int & 0xFF);         // P低8位
                                    TX1_write2buff(ki_int);                // I
                                    TX1_write2buff(kd_int);                // D
                                }
                                break;

                            case 0xF7:  // 查看目标ADC值
                                {
                                    //------------------电压-----------------------
                                    // 读取16位ADC值，高8位+低8位
                                    target_adc = (RX1_Buffer[3] << 8) | RX1_Buffer[4]; // 为16位ADC值
                                    Target = target_adc; // 直接设置为目标ADC值
                                    //---------------------------------------------
                                    start_flag=20; // 启动PID计算
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF7);
                                }
                            break;  
                            case 0xF8:  //查看当前电压
                                {
                                    u16 fb=get_adc1; // 直接使用原始值，假设100
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF8);
                                    TX1_write2buff(fb>>8); // 高8位
                                    TX1_write2buff(fb);     // 低8位
                                }
                            break; 

                            case 0xF9:  // 查看设定目标 电压值Target
                                {
                                //------------------电压-------------------// 直接使用目标ADC值，高8位和低8位
                                TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xF9);
                                TX1_write2buff(Target>>8); // 高8位
                                TX1_write2buff(Target);     // 低8位
                                }   
                            break;

                            case 0xFA: //设置PWM频率 范围1000-8000Hz
                                {
                                    freq = (RX1_Buffer[3] << 8) | RX1_Buffer[4];  // 合并两个字节得到频率值
                                    if(Set_PWM_Frequency(freq))// 调用函数设置频率
                                    { TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xFA); } // 成功 返回成功
                                }
                            break;        

                            case 0xFB: //查询当前PWM频率 
                                {
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xFB);
                                    TX1_write2buff((pwm_freq >> 8) & 0xFF);  // 频率高8位
                                    TX1_write2buff(pwm_freq & 0xFF);         // 频率低8位
                                }
                            break;
                              
                            case 0xFC: //查看PWM状态和占空比
                                // 对应关系：00转 01转
                                {
                                    Get_Active_PWM_Info(&ch,&duty);
                                    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xFC);
                                    TX1_write2buff(ch);TX1_write2buff(duty);
                                }
                            break;
                              
                            default:  // 超出规定范围时
                                start_flag = 0;
                                break;

                        }
                    }
                }
                COM1.RX_Cnt = 0;  // 清空接收缓冲区
            }
        }
    }
}