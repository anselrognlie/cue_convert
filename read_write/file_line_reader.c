#include "file_line_reader.h"
#include "file_helpers.h"

#include <string.h>

static size_t read_line(struct line_reader* self, char const** line_out, size_t* bytes_out);

void file_line_reader_init_fid(file_line_reader_t* self, FILE* fid) {
  memset(self, 0, sizeof(*self));
  self->fid = fid;
  self->line_reader.read_line = read_line;
  self->line_reader.self = self;
}

static size_t read_line(struct line_reader* self, char const** line_out, size_t* bytes_out) {
  file_line_reader_t *file_self = (file_line_reader_t*)self->self;
  return fh_getline(line_out, bytes_out, file_self->fid);
}