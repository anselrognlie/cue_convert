#include "format_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char* msnprintf(char const* fmt, ...) {
  char* buf = NULL;
  va_list args;

  va_start(args, fmt);
  buf = msnprintf_va(fmt, args);
  va_end(args);

  return buf;
}

char* msnprintf_va(char const* fmt, va_list args) {
  char* buf = NULL;
  int buf_req;

  buf_req = vsnprintf(NULL, 0, fmt, args);

  buf = malloc(buf_req + 1);
  if (!buf) return NULL;

  vsnprintf(buf, buf_req + 1, fmt, args);

  return buf;
}

int safe_fprintf(FILE* file, char const* fmt, ...) {
  va_list args;

  if (! file) return 0;

  va_start(args, fmt);
  int written = vfprintf(file, fmt, args);
  va_end(args);

  return written;
}
