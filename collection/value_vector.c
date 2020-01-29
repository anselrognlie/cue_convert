#include "value_vector.h"

#include <stdlib.h>
#include <string.h>

struct value_vector* value_vector_alloc(struct value_vector_params* ops) {
  value_vector_t* self = malloc(sizeof(*self));
  if (!self) return NULL;

  errno_t result = value_vector_init(self, ops);
  if (result) {
    free(self);
    self = NULL;
  }

  return self;
}

errno_t value_vector_init(struct value_vector* self, struct value_vector_params* ops) {
  memset(self, 0, sizeof(*self));
  self->ops = *ops;

  self->array = malloc(0);
  if (!self->array) return -1;

  return 0;
}

errno_t value_vector_uninit(struct value_vector* self) {
  free(self->array);

  return 0;
}

errno_t value_vector_free(struct value_vector* self) {
  errno_t result = value_vector_uninit(self);
  if (result) return result;

  free(self);
  return 0;
}

char const *value_vector_resize(value_vector_t* self, size_t size) {
  size_t new_len = size;
  size_t type_size = self->ops.type_size;

  char* new_array = malloc(new_len * type_size);
  if (!new_array) return NULL;

  memset(new_array, 0, new_len * type_size);

  free(self->array);
  self->array = new_array;
  self->length = new_len;

  return new_array;
}

size_t value_vector_get_length(struct value_vector* self) {
  return self->length;
}

char const* value_vector_get_buffer(struct value_vector* self) {
  return self->array;
}

char const* value_vector_get(struct value_vector* self, size_t i) {
  return self->array + (i * self->ops.type_size);
}

static char *copy_at(struct value_vector* self, char *dst, char const* instance) {
  memmove_s(dst, self->ops.type_size, instance, self->ops.type_size);
  return dst;
}

static char *copy_in(struct value_vector* self, size_t i, char const* instance) {
  char *dst = self->array + (i * self->ops.type_size);
  return copy_at(self, dst, instance);
}

void value_vector_set(struct value_vector* self, size_t i, char const* instance) {
  char* current_i = self->array + (i * self->ops.type_size);
  if (current_i == instance) return;

  copy_in(self, i, instance);
}

errno_t value_vector_delete_at(struct value_vector* self, size_t i) {
  return value_vector_delete_at_keep(self, i, NULL);
}

errno_t value_vector_delete_at_keep(struct value_vector* self, size_t i, char* out) {
  size_t new_len = self->length - 1;

  char* new_array = malloc(new_len * self->ops.type_size);
  if (!new_array) return -1;

  // copy everything into the new array except the one we're deleting
  size_t dst = 0;
  size_t type_size = self->ops.type_size;
  for (size_t src = 0; src < self->length; ++src) {
    if (src == i) continue;

    copy_at(self, new_array + (dst * type_size), self->array + (src * type_size));
    ++dst;
  }

  if (out) {
    copy_at(self, out, self->array + (i * type_size));
  }

  free(self->array);
  self->array = new_array;
  self->length = new_len;

  return 0;
}

char const* value_vector_insert_at(struct value_vector* self, size_t i, char const* instance) {
  size_t new_len = self->length + 1;
  size_t type_size = self->ops.type_size;

  char* new_array = malloc(new_len * type_size);
  if (!new_array) return NULL;

  // copy everything into the new array except the one we're deleting
  size_t dst = 0;
  for (size_t src = 0; src < self->length; ++src) {
    if (src == i) ++dst;

    copy_at(self, new_array + (dst * type_size), self->array + (src * type_size));
    ++dst;
  }

  char* copied = copy_at(self, new_array + (i * type_size), instance);

  free(self->array);
  self->array = new_array;
  self->length = new_len;

  return copied;
}

char const* value_vector_push(struct value_vector* self, char const* instance) {
  return value_vector_insert_at(self, self->length, instance);
}

errno_t value_vector_pop(struct value_vector* self) {
  return value_vector_pop_keep(self, NULL);
}

errno_t value_vector_pop_keep(struct value_vector* self, char* out) {
  return value_vector_delete_at_keep(self, self->length - 1, out);
}

char const* value_vector_unshift(struct value_vector* self, char const* instance) {
  return value_vector_insert_at(self, 0, instance);
}

errno_t value_vector_shift(struct value_vector* self) {
  return value_vector_shift_keep(self, NULL);
}

errno_t value_vector_shift_keep(struct value_vector* self, char* out) {
  return value_vector_delete_at_keep(self, 0, out);
}

IMPLEMENT_POD_VALUE_VECTOR(int)
IMPLEMENT_POD_VALUE_VECTOR(double)