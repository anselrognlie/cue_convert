#include "null_line_writer.h"

#include <string.h>

static size_t write_line_range(line_writer_i* self, char const* start, char const* end);
static size_t write_line(line_writer_i* self, char const* line);

errno_t null_line_writer_init(null_line_writer_t* self) {
  errno_t err = 0;
  memset(self, 0, sizeof(*self));
  self->line_writer.write_line = write_line;
  self->line_writer.write_line_range = write_line_range;
  self->line_writer.self = self;
  return err;
}

void null_line_writer_uninit(null_line_writer_t* self) {
}

static size_t write_line_range(line_writer_i* self, char const* start, char const* end) {
  return end - start;
}

static size_t write_line(line_writer_i* self, char const* line) {
  return strlen(line);
}
