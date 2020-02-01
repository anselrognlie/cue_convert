#pragma once

#include <stdarg.h>

char* msnprintf(char const* fmt, ...);
char* msnprintf_va(char const* fmt, va_list args);
