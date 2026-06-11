#pragma once

// 跨平台类型定义，替代 Windows 类型
#include <cstdint>
#include <cstring>
#include <cassert>

using BYTE  = uint8_t;
using WORD  = uint16_t;
using DWORD = uint32_t;
using UINT  = uint32_t;
using INT   = int32_t;
using BOOL  = int;

// MFC 兼容常量
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// 断言宏统一
#define ASSERT(expr) assert(expr)

// 内存操作宏统一
inline void ZeroMemory(void* dest, size_t len) { std::memset(dest, 0, len); }
inline void CopyMemory(void* dest, const void* src, size_t len) { std::memcpy(dest, src, len); }
