#pragma once

#include "line_reader.h"
#include <stdio.h>

typedef struct file_line_reader {
  line_reader_i line_reader;
  FILE *fid;
} file_line_reader_t;

void file_line_reader_init_fid(file_line_reader_t *self, FILE *fid);
