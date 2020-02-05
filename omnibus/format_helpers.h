#pragma once

#include <stdarg.h>
#include <stdio.h>

char* msnprintf(char const* fmt, ...);
char* msnprintf_va(char const* fmt, va_list args);

int safe_fprintf(FILE *file, char const *fmt, ...);
