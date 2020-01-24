#pragma once

#include "line_writer.h"
#include <stdio.h>

typedef struct array_line_writer {
  line_writer_i line_writer;
  int num_lines;
  char **lines;
} array_line_writer_t;

void array_line_writer_init(array_line_writer_t* self);
void array_line_writer_uninit(array_line_writer_t* self);
