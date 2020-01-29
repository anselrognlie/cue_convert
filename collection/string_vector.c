#include "string_vector.h"

#include <string.h>
#include <stdlib.h>

static void* string_alloc(size_t size);
static void string_free(void* instance);
static void* string_copy_in(void* dst, void const* src);

struct object_vector_params string_vector_ops = {
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

IMPLEMENT_OBJECT_VECTOR(string_vector, char)
