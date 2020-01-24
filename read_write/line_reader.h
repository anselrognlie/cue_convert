#pragma once

#include <stdlib.h>

struct line_reader;

typedef struct line_reader {
  void *self;
  size_t (*read_line)(struct line_reader *self, char const **line_out, size_t *bytes_out);
} line_reader_i;