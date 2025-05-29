#pragma once

#ifdef PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <intrin.h> 

typedef signed char         i8;
typedef short               i16;
typedef int                 i32;
typedef __int64             i64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned __int64    u64;

typedef float               f32;
typedef double              f64;

typedef unsigned long       ul32;
typedef unsigned long long  ul64;
typedef unsigned __int32    dw32;
typedef unsigned __int64    dw64;

typedef wchar_t             wchar;

#define FORCE_INLINE        __forceinline
#define NO_INLINE           __declspec(noinline)
#define ALIGN(x)            __declspec(align(x))
#define API_EXPORT           __declspec(dllexport)
#define API_IMPORT           __declspec(dllimport)
#endif 
