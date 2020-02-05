#pragma once

#include <stddef.h>

#include "line_writer.h"

typedef struct null_line_writer {
  line_writer_i line_writer;
} null_line_writer_t;

errno_t null_line_writer_init(null_line_writer_t* self);
void null_line_writer_uninit(null_line_writer_t* self);
