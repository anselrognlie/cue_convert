#pragma once

#include "line_reader.h"

typedef struct array_line_reader {
  line_reader_i line_reader;
  char const **array;
  int num_lines;
  int current_line;
} array_line_reader_t;

void array_line_reader_init_lines(array_line_reader_t* self, char const **lines, int num_lines);
