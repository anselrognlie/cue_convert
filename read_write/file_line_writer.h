#pragma once

#include "line_writer.h"
#include <stdio.h>

typedef struct file_line_writer {
  line_writer_i line_writer;
  FILE* fid;
} file_line_writer_t;

void file_line_writer_init_fid(file_line_writer_t* self, FILE* fid);
