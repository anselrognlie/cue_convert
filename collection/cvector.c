#include "cvector.h"

#include <stdlib.h>
#include <string.h>

struct object_vector* object_vector_alloc(struct object_vector_params* ops) {
  object_vector_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t result = object_vector_init(self, ops);
  if (result) {
    free(self);
    self = NULL;
  }

  return self;
}

errno_t object_vector_init(struct object_vector* self, struct object_vector_params* ops) {
  memset(self, 0, sizeof(*self));
  self->ops = *ops;

  self->array = malloc(0);
  if (! self->array) return -1;

  return 0;
}

errno_t object_vector_uninit(struct object_vector* self) {
  // release each thing we own
  for (size_t i = 0; i < self->length; ++i) {
    self->ops.free(self->array[i]);
  }

  free(self->array);

  return 0;
}

errno_t object_vector_free(struct object_vector* self) {
  errno_t result = object_vector_uninit(self);
  if (result) return result;

  free(self);
  return 0;
}

size_t object_vector_get_length(struct object_vector* self) {
  return self->length;
}

void const** object_vector_get_buffer(struct object_vector* self) {
  return self->array;
}

void const* object_vector_get(struct object_vector* self, size_t i) {
  return self->array[i];
}

static void * copy_in(struct object_vector* self, size_t i, void const* instance) {
  void* owned = self->ops.alloc(self->ops.type_size);
  if (!owned) return NULL;

  void* copied = self->ops.copy_in(owned, instance);
  if (!copied) {
    self->ops.free(owned);
    return NULL;
  }

  return copied;
}

void const* object_vector_set(struct object_vector* self, size_t i, void const* instance) {
  void **current_i = self->array + i;
  if (*current_i == instance) return instance;

  void *copied = copy_in(self, i, instance);
  if (!copied) {
    return NULL;
  }

  void *old = self->array[i];
  self->array[i] = copied;
  self->ops.free(old);

  return copied;
}

errno_t object_vector_delete_at(struct object_vector* self, size_t i) {
  return object_vector_delete_at_keep(self, i, NULL);
}

errno_t object_vector_delete_at_keep(struct object_vector* self, size_t i, void** out) {
  size_t new_len = self->length - 1;

  void** new_array = malloc(new_len * sizeof(void*));
  if (!new_array) return -1;

  // copy everything into the new array except the one we're deleting
  size_t dst = 0;
  for (size_t src = 0; src < self->length; ++src) {
    if (src == i) continue;

    new_array[dst] = self->array[src];
    ++dst;
  }

  if (out) {
    *out = self->array[i];
  }
  else {
    self->ops.free(self->array[i]);
  }

  free(self->array);
  self->array = new_array;
  self->length = new_len;

  return 0;
}

void const* object_vector_insert_at(struct object_vector* self, size_t i, void const* instance) {
  size_t new_len = self->length + 1;

  void** new_array = malloc(new_len * sizeof(void*));
  if (!new_array) return NULL;

  // copy everything into the new array except the one we're deleting
  size_t dst = 0;
  for (size_t src = 0; src < self->length; ++src) {
    if (src == i) ++dst;

    new_array[dst] = self->array[src];
    ++dst;
  }

  void* copied = copy_in(self, i, instance);
  if (!copied) {
    free(new_array);
    return NULL;
  }

  free(self->array);
  new_array[i] = copied;
  self->array = new_array;
  self->length = new_len;

  return copied;
}

void const* object_vector_push(struct object_vector* self, void const* instance) {
  return object_vector_insert_at(self, self->length, instance);
}

errno_t object_vector_pop(struct object_vector* self) {
  return object_vector_pop_keep(self, NULL);
}

errno_t object_vector_pop_keep(struct object_vector* self, void** out) {
  return object_vector_delete_at_keep(self, self->length - 1, out);
}

void const* object_vector_unshift(struct object_vector* self, void const* instance) {
  return object_vector_insert_at(self, 0, instance);
}

errno_t object_vector_shift(struct object_vector* self) {
  return object_vector_shift_keep(self, NULL);
}

errno_t object_vector_shift_keep(struct object_vector* self, void** out) {
  return object_vector_delete_at_keep(self, 0, out);
}

