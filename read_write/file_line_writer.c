#include "file_line_writer.h"

#include <string.h>

#include "mem_helpers.h"

static size_t write_line_range(line_writer_i* self, char const* start, char const* end);
static size_t write_line(line_writer_i* self, char const* line);

void file_line_writer_init_fid(file_line_writer_t* self, FILE* fid) {
  memset(self, 0, sizeof(*self));
  self->fid = fid;
  self->line_writer.write_line = write_line;
  self->line_writer.write_line_range = write_line_range;
  self->line_writer.self = self;
}

errno_t file_line_writer_init_path(file_line_writer_t* self, char const* path) {
  errno_t err = 0;
  FILE *file_out = 0;

  err = fopen_s(&file_out, path, "wb");
  if (! file_out) return -1;

  file_line_writer_init_fid(self, file_out);
  self->close_file_on_uninit = 1;

  return 0;
}

void file_line_writer_uninit(file_line_writer_t* self) {
  if (self->close_file_on_uninit) {
    SAFE_FREE_HANDLER(self->fid, fclose);
    self->close_file_on_uninit = 0;
  }
}

static size_t write_line_range(line_writer_i* self, char const* start, char const* end) {
  file_line_writer_t* file_self = (file_line_writer_t*)self->self;

  size_t bytes = 0;
  while (start < end) {
    if (fputc(*start, file_self->fid) == EOF) break;

    ++start;
    ++bytes;
  }

  if (fputc('\n', file_self->fid) != EOF) {
    ++bytes;
  }
  
  return bytes;
}

static size_t write_line(line_writer_i* self, char const* line) {
  file_line_writer_t* file_self = (file_line_writer_t*)self->self;

  return fprintf(file_self->fid, "%s\n", line);
}
