#pragma once

#include <stdlib.h>

struct line_writer;

typedef struct line_writer {
  void* self;
  size_t(*write_line_range)(struct line_writer* self, char const* start, char const* end);
  size_t(*write_line)(struct line_writer* self, char const* line);
} line_writer_i;