#include "array_line_writer.h"

#include <string.h>
#include <stdlib.h>

#include "mem_helpers.h"

static size_t write_line_range(line_writer_i* self, char const* start, char const* end);
static size_t write_line(line_writer_i* self, char const* line);

void array_line_writer_init(array_line_writer_t* self) {
  memset(self, 0, sizeof(*self));
  self->line_writer.write_line = write_line;
  self->line_writer.write_line_range = write_line_range;
  self->line_writer.self = self;
}

void array_line_writer_uninit(array_line_writer_t* self) {
  if (self->lines) {
    for (int i = 0; i < self->num_lines; ++i) {
      free(self->lines[i]);
    }

    SAFE_FREE(self->lines);
  }

  self->num_lines = 0;
}

static size_t write_line_range(line_writer_i* self, char const* start, char const* end) {
  array_line_writer_t* array_self = (array_line_writer_t*)self->self;

  if (start > end) return 0;

  // increase size of array
  int num_lines = array_self->num_lines + 1;
  char** lines = realloc(array_self->lines, num_lines * sizeof(char*));
  if (!lines) {
    return 0;
  }

  size_t len = end - start;
  char* line_dup = malloc(len + 1);
  if (!line_dup) {
    free(lines);
    return 0;
  }

  char *dup_i = line_dup;
  while (start < end) {
    *dup_i = *start;
    ++dup_i;
    ++start;
  }

  *dup_i = 0;

  array_self->lines = lines;
  array_self->lines[num_lines] = line_dup;
  array_self->num_lines = num_lines;

  return len;
}

static size_t write_line(line_writer_i* self, char const* line) {
  array_line_writer_t* array_self = (array_line_writer_t*)self->self;

  // increase size of array
  int num_lines = array_self->num_lines + 1;
  char **lines = realloc(array_self->lines, num_lines * sizeof(char *));
  if (!lines) {
    return 0;
  }

  char *line_dup = _strdup(line);
  if (!line_dup) {
    free(lines);
    return 0;
  }

  array_self->lines = lines;
  array_self->lines[num_lines - 1] = line_dup;
  array_self->num_lines = num_lines;

  return strlen(line);
}
