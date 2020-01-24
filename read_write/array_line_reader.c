#include "array_line_reader.h"

#include <string.h>
#include <stdio.h>

static size_t read_line(struct line_reader* self, char const** line_out, size_t* bytes_out);

void array_line_reader_init_lines(array_line_reader_t* self, char const ** lines, int num_lines) {
  memset(self, 0, sizeof(*self));
  self->array = lines;
  self->num_lines = num_lines;
  self->line_reader.read_line = read_line;
  self->line_reader.self = self;
}

static size_t read_line(struct line_reader* self, char const** line_out, size_t* bytes_out) {
  array_line_reader_t* array_self = (array_line_reader_t*)self->self;

  if (array_self->current_line == array_self->num_lines) return EOF;

  char const *line = array_self->array[array_self->current_line];
  ++(array_self->current_line);
  size_t len = strlen(line);

  char *buf = malloc(len + 1);
  if (! buf) return EOF;

  strcpy_s(buf, len + 1, line);

  *line_out = buf;
  *bytes_out = len;

  return len;
}