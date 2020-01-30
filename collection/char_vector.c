#include "char_vector.h"

#include <string.h>
#include <stdlib.h>

struct value_vector_params char_vector_ops = {
  sizeof(char),
};

IMPLEMENT_VALUE_VECTOR(char_vector, char)

char const* char_vector_set_str(struct char_vector* self, char const* instance) {
  int new_len = strlen(instance);
  char *buf = (char* )value_vector_resize(&self->self, new_len);
  if (! buf) return NULL;

  memmove_s(buf, new_len, instance, new_len);
  return buf;
}

char const* char_vector_get_str(struct char_vector const* self) {
  size_t str_len = value_vector_get_length(&self->self);
  char *buf = malloc(str_len + 1);
  if (! buf) return NULL;

  memmove_s(buf, str_len + 1, value_vector_get_buffer(&self->self), str_len);
  buf[str_len] = 0;
  return buf;
}

void char_vector_get_str_buf(struct char_vector const* self, char* buf, size_t buf_len) {
  // ensure the buffer is large enough (including the null byte)
  size_t str_len = value_vector_get_length(&self->self);
  if (buf_len < str_len + 1) return;

  memmove_s(buf, buf_len, value_vector_get_buffer(&self->self), str_len);
  buf[str_len] = 0;
}

