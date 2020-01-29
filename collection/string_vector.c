#include "string_vector.h"

#include <string.h>
#include <stdlib.h>

static void* string_alloc(size_t size);
static void string_free(void* instance);
static void* string_copy_in(void* dst, void const* src);

struct cvector_params string_vector_ops = {
  sizeof(char),
  string_alloc,
  string_free,
  string_copy_in,
};

static void* string_alloc(size_t size) {
  char *self = malloc(size);
  if (! self) return NULL;

  memset(self, 0, size);
  return self;
}

static void string_free(void* instance) {
  free(instance);
}

static void* string_copy_in(void* dst, void const* src) {
  char* dst_str = (char*)dst;
  char* src_str = (char*)src;
  size_t dst_len = strlen(dst_str);
  size_t src_len = strlen(src_str);

  if (dst_len < src_len) {
    // reallocate
    char *new_dst = string_alloc(src_len + 1);
    if (! new_dst) return NULL;
    free(dst);
    dst = new_dst;
  }

  strcpy_s(dst, src_len + 1, src);

  return dst;
}

struct string_vector* string_vector_alloc() {
  string_vector_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t result = string_vector_init(self);
  if (result) return NULL;

  return self;
}

errno_t string_vector_init(struct string_vector* self) {
  return cvector_init(&self->self, &string_vector_ops);
}

errno_t string_vector_uninit(struct string_vector* self) {
  return cvector_uninit(&self->self);
}

errno_t string_vector_free(struct string_vector* self) {
  errno_t result = cvector_uninit(&self->self);
  if (result) return -1;
  free(self);
  return 0;
}

size_t string_vector_get_length(struct string_vector* self) {
  return cvector_get_length(&self->self);
}

char const* string_vector_get_buffer(struct string_vector* self) {
  return (char const *)cvector_get_length(&self->self);
}

char const* string_vector_get(struct string_vector* self, size_t i) {
  return (char const *)cvector_get(&self->self, i);
}

char const* string_vector_set(struct string_vector* self, size_t i, char const*instance) {
  return cvector_set(&self->self, i, (void const *)instance);
}

errno_t string_vector_delete_at(struct string_vector* self, size_t i) {
  return cvector_delete_at(&self->self, i);
}

errno_t string_vector_delete_at_keep(struct string_vector* self, size_t i, char **out) {
  return cvector_delete_at_keep(&self->self, i, (void **)out);
}

char const* string_vector_insert_at(struct string_vector* self, size_t i, char const*instance) {
  return (char const*)cvector_insert_at(&self->self, i, (void const *)instance);
}

char const* string_vector_push(struct string_vector* self, char const* instance) {
  return (char const*)cvector_push(&self->self, (void const*)instance);
}

errno_t string_vector_pop(struct string_vector* self) {
  return cvector_pop(&self->self);
}

errno_t string_vector_pop_keep(struct string_vector* self, char** out) {
  return cvector_pop_keep(&self->self, (void**)out);
}

char const* string_vector_unshift(struct string_vector* self, char const* instance) {
  return (char const*)cvector_unshift(&self->self, (void const *)instance);
}

errno_t string_vector_shift(struct string_vector* self) {
  return cvector_shift(&self->self);
}

errno_t string_vector_shift_keep(struct string_vector* self, char** out) {
  return cvector_shift_keep(&self->self, (void**)out);
}
