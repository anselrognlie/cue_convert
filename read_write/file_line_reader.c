#include "file_line_reader.h"

#include <string.h>

#include "file_helpers.h"
#include "mem_helpers.h"

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

errno_t file_line_reader_init_path(file_line_reader_t* self, char const* path) {
  errno_t err = 0;
  FILE* file_in = 0;

  err = fopen_s(&file_in, path, "rb");
  if (!file_in) return -1;

  file_line_reader_init_fid(self, file_in);
  self->close_file_on_uninit = 1;

  return 0;
}

void file_line_reader_uninit(file_line_reader_t* self) {
  if (self->close_file_on_uninit) {
    SAFE_FREE_HANDLER(self->fid, fclose);
    self->close_file_on_uninit = 0;
  }
}

