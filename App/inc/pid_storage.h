#ifndef __PID_STORAGE_H__
#define __PID_STORAGE_H__

#include "config.h"

// PID参数在EEPROM中的存储地址
#define PID_PARAMS_ADDR    0x0000

// PID参数结构体（支持浮点数存储）
typedef struct {
    u16 kP;  // 存储整数形式，内部转换为浮点数 (16位)
    u8 kI;   // 存储整数形式，内部转换为浮点数 (8位)
    u8 kD;   // 存储整数形式，内部转换为浮点数 (8位)
} PID_Params_t;

// 函数声明
void PID_LoadParams(void);    // 从EEPROM加载PID参数
void PID_SaveParams(void);    // 保存PID参数到EEPROM

#endif 