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
#include	"STC8H_PWM.h"
#include	"STC8G_H_GPIO.h"
#include	"STC8G_H_NVIC.h"
#include    "pid_storage.h"
#include    "APP_PWMB_Output.h"  // 添加头文件引用

#include    <stdio.h>
#include    <stdarg.h>

/*************	功能说明	**************

本例程基于STC8H8K64U为主控芯片的实验箱8进行编写测试，STC8H系列芯片可通用参考.

高级PWM定时器 PWM5,PWM6,PWM7,PWM8 每个通道都可独立实现PWM输出.

4个通道PWM根据需要设置对应输出口，可通过示波器观察输出的信号.

PWM周期和占空比可以自定义设置，最高可达65535.

下载时, 选择时钟 24MHZ (用户可在"config.h"修改频率).

******************************************/


//========================================================================
//                               本地常量声明	
//========================================================================


#define MAX_INTEGRAL 500// 积分限幅常量 - 增加积分限幅，让积分项能累积更多
//MAX_INTEGRAL用于限制PID积分项的累积，防止积分项过大导致系统超调、振荡或抖动。

//太小：积分作用不明显，系统稳态误差大。太大：积分项容易积累过多，导致超调、抖动甚至失控。

// PID参数精度优化常量
#define PID_SCALE 100.0f  // 参数缩放因子，将整数参数转换为浮点数精度

// PWM频率控制常量
#define PWM_MIN_FREQ 1000   // 最小PWM频率 1000Hz
#define PWM_MAX_FREQ 8000   // 最大PWM频率 8000Hz

//========================================================================
//                               本地变量声明
//========================================================================

PWMx_Duty PWMB_Duty;
bit PWM5_Flag;
bit PWM7_Flag;
u16 PWM_Flag=0;

// PWM频率控制变量
u16 pwm_freq = 2000;         // 当前PWM频率，默认2000Hz
u32 pwm_period = 12000;      // 当前PWM周期值
u8 pwm_last_used = 0;        // 最后使用的通道号（0=正转，1=反转）
u8 pwm_last_duty = 0;        // 最后使用的占空比（0-100）
u32 pwm_value = 0;           // 当前PWM值（全局变量）

char buf[16];
char *ptr; // 不要用p

//========================================================================
//                               全局变量
//========================================================================

float g_kP = 40.0f, g_kI = 4.0f, g_kD = 2.0f;  // PID参数（浮点数精度）
float deviation_SUM = 0.0f;  // 积分项累加（浮点数）
float err_now = 0.0f;        // 当前误差（浮点数）
float err_bef = 0.0f;        // 上次误差（浮点数）
u16 Target = 5;              // 目标位置（保持整数）

//========================================================================
//                               本地函数声明
//========================================================================
float PID_Choice(u16 User_Target, u16 current, float User_KP, float User_KI, float User_KD);
u16 Calculate_PWM_Duty(u8 duty_percent);  // 动态计算占空比




//========================================================================
// 函数: my_printf
// 描述: 格式化串口输出函数，类似STM32的my_printf
// 参数: format - 格式化字符串, ... - 可变参数
// 返回: 输出的字符数
//========================================================================
int my_printf(const char *format, ...)
{
    char buffer[256];
    va_list arg;
    int len;
    char *p;
    
    va_start(arg, format);
    len = vsprintf(buffer, format, arg);
    va_end(arg);
    
    // 逐字符发送
    p = buffer;
    while(*p) {
        TX1_write2buff(*p);
        p++;
    }
    
    return len;
}
//========================================================================
// 函数: Set_PWM_Frequency
// 描述: 动态设置PWM输出频率
// 参数: freq - 期望的PWM频率(Hz)
// 返回: 1-设置成功, 0-设置失败(频率超出范围)
//========================================================================
u8 Set_PWM_Frequency(u16 freq)//动态调整频率 后面所有PWM值都是根据当前频率动态计算的
{
    PWMx_InitDefine PWMx_InitStructure;  // 添加变量声明
    // 检查频率是否在允许范围内
    if(freq < PWM_MIN_FREQ || freq > PWM_MAX_FREQ) return 0;  // 频率超出范围
        
    pwm_freq = freq;
    // 计算PWM周期值：周期 = 主时钟频率/目标频率
    // MAIN_Fosc = 24MHz (在config.h中定义)
    pwm_period = MAIN_Fosc / freq;  
    
    // 配置PWM基本参数
    PWMx_InitStructure.PWM_Period = pwm_period;      // 设置PWM周期
    PWMx_InitStructure.PWM_DeadTime = 0;            // 设置死区时间为0
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;  // 使能PWM主输出
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE;     // 使能PWM计数器
    PWM_Configuration(PWMB, &PWMx_InitStructure);   // 应用PWM配置
    
    // 自动调整PID参数以适应新频率
    // Auto_Adjust_PID_By_Frequency();
    
    return 1;//返回0：表示频率设置失败（超出范围）
}

//========================================================================
// 函数: Calculate_PWM_Duty
// 描述: 根据目标占空比计算PWM值 将百分比形式的占空比转换为实际的PWM值
// 参数: duty_percent - 目标占空比(0-100)
// 返回: PWM值(0-pwm_period)
//========================================================================
u16 Calculate_PWM_Duty(u8 duty_percent)
{
    // 限制占空比在0-100%范围内
    if(duty_percent > 100) duty_percent = 100;
    // 计算PWM值：PWM值 = 周期 * 占空比/100
    return (pwm_period * duty_percent) / 100;
}
//当频率为2000Hz时，计算过程如下：
//主时钟频率MAIN_Fosc = 24MHz = 24,000,000Hz
//PWM周期 = 24,000,000 / 2000 = 12,000
//所以当占空比为100%时，PWM值 = 12,000 * 100% = 12,000

//========================================================================
// 函数: Get_Active_PWM_Infoch：
//当前有效PWM通道（0=正转，1=反转）
//duty：当前PWM输出占空比百分比（0~100）
//========================================================================
void Get_Active_PWM_Info(u8 *ch,u8 *duty){
    u16 d7=PWMB_Duty.PWM7_Duty, d6=PWMB_Duty.PWM6_Duty;
    u32 temp;
    
    // 判断当前激活的通道
    if(d7 > 0 && d6 == 0) {
        *ch = 0;  // 正转（PWM7激活）
        temp = ((u32)d7 * 100) / pwm_period;
    } else if(d6 > 0 && d7 == 0) {
        *ch = 1;  // 反转（PWM6激活）
        temp = ((u32)d6 * 100) / pwm_period;
    } else {
        *ch = 0;  // 默认状态
        temp = 0; // 占空比为0
    }
    
    // 限制占空比在0-100范围内
    if(temp > 100) temp = 100;
    *duty = (u8)temp;
}
float PID_Choice(
    u16 User_Target,         // 用户设置的目标值
    u16 current,             // 当前采样值 position是实际的当前采样值，通过ADC获取
    float User_KP,            // 用户设置的P系数（0=不调整）
    float User_KI,            // 用户设置的I系数（0=不调整）
    float User_KD)            // 用户设置的D系数（0=不调整）
{    
    float output = 0.0f;     // PID最终输出值（浮点数，保持精度）
    float pid_output_float = 0.0f;   // PID计算结果（浮点数，用于内部计算）
    float error_change = 0.0f; // 误差变化量
    
    // 更新PID参数（如果用户提供了新值且不为0）
    if(User_KP > 0.0f) g_kP = User_KP;
    if(User_KI > 0.0f) g_kI = User_KI;
    if(User_KD > 0.0f) g_kD = User_KD;
    

    
    // 计算偏差 - 先转为浮点再相减，避免无符号减法导致的下溢
    err_now = (float)User_Target - (float)current; // 误差 = 目标值 - 当前值
    
    // 更新积分项
    deviation_SUM += err_now;    // 误差累加
    
    // 积分限幅（防止积分饱和）
    if(deviation_SUM > MAX_INTEGRAL) deviation_SUM = MAX_INTEGRAL;
    if(deviation_SUM < -MAX_INTEGRAL) deviation_SUM = -MAX_INTEGRAL;
    // deviation_SUM *= 0.9f; // 注释掉积分衰减，让积分项更容易饱和
    
    // 计算误差变化量（在更新err_bef之前）
    error_change = err_now - err_bef;
    
//    // 添加误差变化量溢出保护
//    if(error_change > 1000.0f) error_change = 1000.0f;  // 限制最大误差变化
//    if(error_change < -1000.0f) error_change = -1000.0f; // 限制最小误差变化
    
    // 位置式PID公式计算： pid_result = kP * e(k) + kI * Σe(k) + kD * [e(k) - e(k-1)] 使用浮点运算提高精度，但最终输出为整数
    //内部计算
    pid_output_float = (g_kP * err_now + g_kI * deviation_SUM + g_kD * error_change) / 100.0f; // 除以100将输出缩放到占空比范围
    
    // 输出限幅 - 保持浮点数精度，与占空比范围匹配
    if(pid_output_float > 100.0f) output = 100.0f;
    else if(pid_output_float < -100.0f) output = -100.0f; // 负值限制在-100
    else {output = pid_output_float;} // 保持浮点数精度  PID最终输出值
    
    err_bef = err_now;  //当前误差保存为下次的上次误差 ，用于下次计算 
    
    return output;  // 返回控制量
    
}

// PWM参数调整（PID控制）进行PID计算并===调整电机PWM输出===

void Parameter_adjustment(void) {
    float output = PID_Choice(Target, get_adc1, g_kP, g_kI, g_kD);  // 计算PID输出（浮点数）
    u8 duty_percent;  // PWM占空比百分比（0-100）
    u16 pwm_duty_value;  // PWM占空比数值
    
    // 处理有符号PID输出，转换为占空比--------------------------------------------------
    if(output > 0.0f) 
    {
        duty_percent = (u8)(output + 0.5f);  // 正值，正向控制，四舍五入
        if(duty_percent > 100) duty_percent = 100;  // 限制最大输出为100%
    } 
    else if(output < 0.0f) 
    {
        duty_percent = (u8)(-output + 0.5f);  // 负值，反向控制，四舍五入
        if(duty_percent > 100) duty_percent = 100;  // 限制最大输出为100%
    } 
    else 
    {
        duty_percent = 0;  // 零值，停止控制
    }
    //----------------------------------------------------------------------------------
    pwm_duty_value = Calculate_PWM_Duty(duty_percent);

    // 根据PID输出正负值控制电机方向--------------------------------------------------
    if(output > 0.0f) {
        // 正值，正向控制（PWM7）
        PWMB_Duty.PWM7_Duty = pwm_duty_value;
        PWMB_Duty.PWM6_Duty = 0;
        P32 = 1;
        P55 = 0;
    } else if(output < 0.0f) {
        // 负值，反向控制（PWM6）
        PWMB_Duty.PWM7_Duty = 0;
        PWMB_Duty.PWM6_Duty = pwm_duty_value;
        P32 = 0;
        P55 = 1;
    } else {
        // 零值，停止控制
        PWMB_Duty.PWM7_Duty = 0;
        PWMB_Duty.PWM6_Duty = 0;
        P32 = 0;
        P55 = 0;
    }
    UpdatePwm(PWMB, &PWMB_Duty);
    

    // 位置信息
    //my_printf("{Target}%.0f\r\n", (float)Target);
    //my_printf("{Actual}%.0f\r\n", (float)get_adc1);
    

}
//========================================================================
// 函数: PWMB_Output_init
// 描述: 用户初始化程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================
void PWMB_Output_init(void)
{
	PWMx_InitDefine		PWMx_InitStructure;
	
	PWMB_Duty.PWM5_Duty = 0;
	PWMB_Duty.PWM6_Duty = 0;
	PWMB_Duty.PWM7_Duty = 0;
	PWMB_Duty.PWM8_Duty = 1024;

	PWMx_InitStructure.PWM_Mode    =	CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty    = PWMB_Duty.PWM6_Duty;	//PWM占空比时间, 0~Period
	PWMx_InitStructure.PWM_EnoSelect   = ENO6P;					//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWM_Configuration(PWM6, &PWMx_InitStructure);				//初始化PWM,  PWMA,PWMB

	PWMx_InitStructure.PWM_Mode    =	CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty    = PWMB_Duty.PWM7_Duty;	//PWM占空比时间, 0~Period
	PWMx_InitStructure.PWM_EnoSelect   = ENO7P;					//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWM_Configuration(PWM7, &PWMx_InitStructure);				//初始化PWM,  PWMA,PWMB

	// 使用动态频率初始化PWM
    //PWMx_InitStructure.PWM_Period   = 12000;                            //周期时间,   0~65535
	PWMx_InitStructure.PWM_Period   = pwm_period;							//使用动态周期时间,   0~65535
	PWMx_InitStructure.PWM_DeadTime = 0;								//死区发生器设置, 0~255
	PWMx_InitStructure.PWM_MainOutEnable= ENABLE;				//主输出使能, ENABLE,DISABLE
	PWMx_InitStructure.PWM_CEN_Enable   = ENABLE;				//使能计数器, ENABLE,DISABLE
	PWM_Configuration(PWMB, &PWMx_InitStructure);				//初始化PWM通用寄存器,  PWMA,PWMB

	NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
	P1_MODE_IO_PU(GPIO_Pin_1|GPIO_Pin_2);
	P3_MODE_IO_PU(GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_7);P33=0;
	P5_MODE_IO_PU(GPIO_Pin_4|GPIO_Pin_5);P54=0;		//P1.5,P1.6,P1.7,P5.4 设置为准双向口
    P3_MODE_IN_HIZ(GPIO_Pin_4|GPIO_Pin_6);//P3.3,P3.4,P3.6 设置为高阻输入
    
}

//========================================================================
// 函数: Sample_PWMB_Output
// 描述: 用户应用程序.
// 参数: None.
// 返回: None.
// 版本: V1.0, 2020-09-28
//========================================================================

void Sample_PWMB_Output(void) //STC8H的库中，通常会有这样的函数来设置各个PWM通道的占空比。
{
//    u16 temp_value; // 临时变量声明
    
    // 调试：函数开始时的pwm_value
//    TX1_write2buff(0x5A);TX1_write2buff(0xA5);TX1_write2buff(0xFA);
//    TX1_write2buff((pwm_value >> 8) & 0xFF);
//    TX1_write2buff(pwm_value & 0xFF);
    
    switch(start_flag) {
        case 0: // 初始化所有输出和标志位 不励磁
            PWMB_Duty.PWM7_Duty = 0;    
            PWMB_Duty.PWM6_Duty = 0;    
            P32 = 1;P55 = 1;                   
            break;

        case 1: // 初始化所有输出和标志位 励磁停
            PWMB_Duty.PWM7_Duty = 0;    
            PWMB_Duty.PWM6_Duty = 0;    
            P32 = 0; P55 = 0;                          
            break;
        case 2: // 开环正转 00
            PWMB_Duty.PWM7_Duty = pwm_value; // 直接用原始值，不再除以pwm_divider
            PWMB_Duty.PWM6_Duty = 0;
            P32 = 1; P55 = 0;
            break;
        case 3: // 开环反转 01
            PWMB_Duty.PWM7_Duty = 0;
            PWMB_Duty.PWM6_Duty = pwm_value; // 直接用原始值，不再除以pwm_divider
            P32 = 0; P55 = 1;
            break;

            
        default:
            break;
    }

    
    UpdatePwm(PWMB, &PWMB_Duty); // 更新PWM输出
}

//========================================================================
// 函数: Auto_Adjust_PID_By_Frequency
// 描述: 根据PWM频率自动调整PID参数，确保高频稳定性
// 参数: None
// 返回: None
//========================================================================
/*
void Auto_Adjust_PID_By_Frequency(void)
{
    // 基于2000Hz时的最佳参数：P=30, I=0.5, D=0.5
    if(pwm_freq <= 2000) {
        // 1000-2000Hz：使用基准参数
        g_kP = 30.0f;
        g_kI = 0.5f;
        g_kD = 0.5f;
    }
    else if(pwm_freq <= 3000) {
        // 2000-3000Hz：轻微调整
        g_kP = 20.0f;
        g_kI = 0.33f;
        g_kD = 0.33f;
    }
    else if(pwm_freq <= 4000) {
        // 3000-4000Hz：中等调整
        g_kP = 15.0f;
        g_kI = 0.25f;
        g_kD = 0.25f;
    }
    else if(pwm_freq <= 5000) {
        // 4000-5000Hz：较大调整
        g_kP = 10.0f;
        g_kI = 0.15f;
        g_kD = 0.15f;
    }
    else if(pwm_freq <= 6000) {
        // 5000-6000Hz：大幅调整
        g_kP = 6.0f;
        g_kI = 0.08f;
        g_kD = 0.08f;
    }
    else {
        // 6000-8000Hz：最保守参数
        g_kP = 4.0f;
        g_kI = 0.05f;
        g_kD = 0.05f;
    }
    
    // 重置PID计算变量
    deviation_SUM = 0.0f;
    err_now = 0.0f;
    err_bef = 0.0f;
}
*/


