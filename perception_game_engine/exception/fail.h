#pragma once
#include <Windows.h>
#include <iostream>
#include <cstdarg>
#include <cstdio>

template<typename... Args>
__forceinline void on_fail(Args&&... args)
{
#ifdef DEBUG
    (std::wcout << ... << std::forward<Args>(args));
    std::wcout << std::endl;
#else
#endif
}


