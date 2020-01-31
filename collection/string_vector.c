#include "string_vector.h"

#include <string.h>
#include <stdlib.h>

static void* string_acquire(void const *instance);
static void string_release(void* instance);

struct object_vector_params string_vector_ops = {
  string_acquire,
  string_release,
};

static void string_release(void* instance) {
  free(instance);
}

static void* string_acquire(void const* instance) {
  char* src_str = (char*)instance;
  size_t src_len = strlen(src_str);

  char *copied = malloc(src_len + 1);
  if (!copied) return NULL;

  strcpy_s(copied, src_len + 1, src_str);

  return copied;
}

IMPLEMENT_OBJECT_VECTOR(string_vector, char)
