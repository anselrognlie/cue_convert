#include "line_writer.h"

#include <stdlib.h>
#include <stdarg.h>

#include "format_helpers.h"
#include "mem_helpers.h"

size_t line_writer_write_fmt(struct line_writer* self, char const* fmt, ...) {
  size_t written = 0;
  va_list args;

  va_start(args, fmt);
  char* buf = msnprintf_va(fmt, args);
  va_end(args);

  if (! buf) return 0;

  written = self->write_line(self, buf);

  SAFE_FREE(buf);

  return written;
}
