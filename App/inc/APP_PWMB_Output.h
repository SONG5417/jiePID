#ifndef __APP_PWMB_OUTPUT_H__
#define __APP_PWMB_OUTPUT_H__

#include "APP.h"

// PID参数精度优化常量
#define PID_SCALE 100.0f  // 参数缩放因子，将整数参数转换为浮点数精度

// PID参数精度优化后的函数声明
float PID_Choice(u16 User_Target, u16 current, float User_KP, float User_KI, float User_KD);
void Parameter_adjustment(void);
void PWMB_Output_init(void);
void Sample_PWMB_Output(void);

// PWM频率控制函数声明
u8 Set_PWM_Frequency(u16 freq);
u16 Calculate_PWM_Duty(u8 duty_percent);
void Get_Active_PWM_Info(u8 *ch, u8 *duty);

// 通用PWM值计算函数声明
u16 Calculate_PID_PWM_Value(u8 duty);
u16 Calculate_OpenLoop_PWM_Value(u8 duty_percent);


// 外部变量声明
extern float g_kP, g_kI, g_kD;  // PID参数（浮点数精度）
extern u16 Target;              // 目标位置
extern float deviation_SUM;     // PID积分项累加
extern float err_now;           // PID当前误差
extern float err_bef;           // PID上次误差

// PWM频率控制变量
extern u16 pwm_freq;            // 当前PWM频率 默认2000Hz
extern u32 pwm_period;          // 当前PWM周期值
extern u32 pwm_value;           // 当前PWM值（全局变量）

extern u8 pwm_last_used ;        // 最后使用的通道号（0=正转，1=反转）
extern u8 pwm_last_duty;        // 最后使用的占空比（0-100）


#endif 