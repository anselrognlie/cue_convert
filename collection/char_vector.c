#include "char_vector.h"

#include <string.h>
#include <stdlib.h>

static char* alloc(struct value_vector* self, size_t item_count);

struct value_vector_params char_vector_ops = {
  sizeof(char),
  alloc,
};

#undef VALUE_VECTOR_INIT_CUSTOM
#define VALUE_VECTOR_INIT_CUSTOM \
  self->set_str = char_vector_set_str; \
  self->get_str = char_vector_get_str; \
  self->get_str_buf = char_vector_get_str_buf; \

IMPLEMENT_VALUE_VECTOR(char_vector, char)

char const* char_vector_set_str(struct char_vector* self, char const* instance) {
  int new_len = strlen(instance);
  char *buf = (char* )value_vector_resize(&self->vector_t, new_len);
  if (! buf) return NULL;

  memmove_s(buf, new_len, instance, new_len);
  return buf;
}

char const* char_vector_get_str(struct char_vector const* self) {
  return (char const* )self->vector_t.array;
}

void char_vector_get_str_buf(struct char_vector const* self, char* buf, size_t buf_len) {
  // ensure the buffer is large enough (including the null byte)
  size_t str_len = value_vector_get_length(&self->vector_t);
  if (buf_len < str_len + 1) return;

  memmove_s(buf, buf_len, value_vector_get_buffer(&self->vector_t), str_len);
  buf[str_len] = 0;
}

static char* alloc(struct value_vector* self, size_t item_count) {
  char *buf = malloc(item_count + 1);
  if (! buf) return NULL;

  buf[item_count] = 0;
  return buf;
}