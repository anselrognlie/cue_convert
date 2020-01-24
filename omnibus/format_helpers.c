#include "format_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char* msnprintf(char const* fmt, ...) {
  char* buf = NULL;
  int buf_req;
  va_list args;

  va_start(args, fmt);
  buf_req = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  buf = malloc(buf_req + 1);
  if (!buf) return NULL;

  va_start(args, fmt);
  vsnprintf(buf, buf_req + 1, fmt, args);
  va_end(args);

  return buf;
}

