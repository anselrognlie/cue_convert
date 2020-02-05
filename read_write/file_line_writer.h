#pragma once

#include "line_writer.h"
#include <stdio.h>

typedef struct file_line_writer {
  line_writer_i line_writer;
  FILE* fid;
  short close_file_on_uninit;
} file_line_writer_t;

void file_line_writer_init_fid(file_line_writer_t* self, FILE* fid);
errno_t file_line_writer_init_path(file_line_writer_t* self, char const *path);
void file_line_writer_uninit(file_line_writer_t* self);
