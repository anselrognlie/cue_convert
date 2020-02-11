#pragma once

#include "line_reader.h"
#include <stdio.h>

typedef struct file_line_reader {
  line_reader_i line_reader;
  FILE *fid;
  short close_file_on_uninit;
} file_line_reader_t;

void file_line_reader_init_fid(file_line_reader_t *self, FILE *fid);
errno_t file_line_reader_init_path(file_line_reader_t* self, char const* path);
void file_line_reader_uninit(file_line_reader_t* self);
