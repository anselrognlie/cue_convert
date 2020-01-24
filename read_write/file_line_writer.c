#include "file_line_writer.h"

#include <string.h>

static size_t write_line_range(line_writer_i* self, char const* start, char const* end);
static size_t write_line(line_writer_i* self, char const* line);

void file_line_writer_init_fid(file_line_writer_t* self, FILE* fid) {
  memset(self, 0, sizeof(*self));
  self->fid = fid;
  self->line_writer.write_line = write_line;
  self->line_writer.write_line_range = write_line_range;
  self->line_writer.self = self;
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
