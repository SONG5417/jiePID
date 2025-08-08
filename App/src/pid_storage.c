#include "pid_storage.h"
#include "STC8G_H_EEPROM.h"
#include "APP_PWMB_Output.h"  // 包含PID_SCALE定义

// 声明外部变量
extern float g_kP, g_kI, g_kD;  // 修改为float类型

// 从EEPROM加载PID参数
void PID_LoadParams(void) {
    PID_Params_t params;
    
    // 读取EEPROM中的参数
    EEPROM_read_n(PID_PARAMS_ADDR, (u8 *)&params, sizeof(PID_Params_t));
    
    // 检查是否是首次使用（全0xFFFF）
    if(params.kP == 0xFFFF && params.kI == 0xFF && params.kD == 0xFF) {
        // 使用默认值（整数形式）
        u16 default_kP = 1500;  // 默认P值 15.0 * 100 (16位) - 降低P值减少振荡
        u8 default_kI = 15;     // 默认I值 15 (8位) - 直接使用整数值
        u8 default_kD = 100;    // 默认D值 1.0 * 100 (8位) - 增加D值提高稳定性
        
        // 转换为浮点数并赋值
        g_kP = (float)default_kP / PID_SCALE;
        g_kI = (float)default_kI / 10; // I参数直接转换
        g_kD = (float)default_kD / 10; // D参数缩放转换
        
        // 保存默认值到EEPROM
        PID_SaveParams();
    } else {
        // 使用EEPROM中的值，转换为浮点数
        g_kP = (float)params.kP / PID_SCALE;
        g_kI = (float)params.kI / 10; // I参数直接转换
        g_kD = (float)params.kD / 10; // D参数缩放转换
    }
}

// 保存PID参数到EEPROM
void PID_SaveParams(void) {
    PID_Params_t params;
    
    // 将浮点数参数转换为整数形式存储
    params.kP = (u16)(g_kP * PID_SCALE);  // 16位
    params.kI = (u8)(g_kI * 10);   // 8位，I参数直接转换
    params.kD = (u8)(g_kD * 10);   // 8位，D参数缩放转换
    
    // 先擦除扇区
    EEPROM_SectorErase(PID_PARAMS_ADDR);
    
    // 写入新参数
    EEPROM_write_n(PID_PARAMS_ADDR, (u8 *)&params, sizeof(PID_Params_t));
} 