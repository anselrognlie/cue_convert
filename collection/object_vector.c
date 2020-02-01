#include "object_vector.h"

#include <stdlib.h>
#include <string.h>

#include "err_helpers.h"
#include "mem_helpers.h"

//void const* k_object_vector_shallow = (void const*)-1;

static void weak_release(void* instance);
static void* weak_acquire(void const* instance);

object_vector_params_t object_vector_weak_params = {
  weak_acquire,
  weak_release,
};

static void weak_release(void* instance) {
  return;
}

static void* weak_acquire(void const* instance) {
  return (void* )instance;
}


struct object_vector* object_vector_alloc(struct object_vector_params const* ops) {
  object_vector_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t result = object_vector_init(self, ops);
  if (result) {
    free(self);
    self = NULL;
  }

  return self;
}

struct object_vector* object_vector_alloc_copy(object_vector_t const* from) {
  object_vector_t* self = 0;
  errno_t err = 0;
  void const** copy_result = 0;

  ERR_REGION_BEGIN() {
    self = object_vector_alloc(&from->ops);
    ERR_REGION_NULL_CHECK(self, err);

    copy_result = object_vector_copy_from(self, from);
    ERR_REGION_NULL_CHECK(copy_result, err);

    return self;

  } ERR_REGION_END()

  object_vector_free(self);

  return NULL;
}

errno_t object_vector_init(struct object_vector* self, struct object_vector_params const* ops) {
  memset(self, 0, sizeof(*self));
  self->ops = *ops;

  self->array = malloc(0);
  if (! self->array) return -1;

  return 0;
}

void object_vector_uninit(struct object_vector* self) {
  // release each thing we own
  for (size_t i = 0; i < self->length; ++i) {
    self->ops.release(self->array[i]);
  }

  free(self->array);
}

void object_vector_free(struct object_vector* self) {
  object_vector_uninit(self);
  free(self);
}

size_t object_vector_get_length(struct object_vector const* self) {
  return self->length;
}

void const** object_vector_get_buffer(struct object_vector const* self) {
  return self->array;
}

void const* object_vector_get(struct object_vector const* self, size_t i) {
  return self->array[i];
}

void const* object_vector_set(struct object_vector* self, size_t i, void const* instance) {
  void **current_i = self->array + i;
  if (*current_i == instance) return instance;

  void *copied = self->ops.acquire(instance);
  if (!copied) {
    return NULL;
  }

  void *old = self->array[i];
  self->array[i] = copied;
  self->ops.release(old);

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

    ERR_IGNORE_WARNING(6386, new_array[dst] = self->array[src];)
    ++dst;
  }

  if (out) {
    *out = self->array[i];
  }
  else {
    self->ops.release(self->array[i]);
  }

  free(self->array);
  self->array = new_array;
  self->length = new_len;

  return 0;
}

void const* object_vector_insert_at(struct object_vector* self, size_t i, void const* instance) {
  size_t new_len = self->length + 1;

  if (i >= new_len) return NULL;

  void** new_array = malloc(new_len * sizeof(void*));
  if (!new_array) return NULL;

  // copy everything into the new array except the one we're deleting
  size_t dst = 0;
  for (size_t src = 0; src < self->length; ++src) {
    if (src == i) ++dst;

    new_array[dst] = self->array[src];
    ++dst;
  }

  void* copied = self->ops.acquire(instance);
  if (!copied) {
    free(new_array);
    return NULL;
  }

  free(self->array);
  ERR_IGNORE_WARNING(6386, new_array[i] = copied;)
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

void const** object_vector_copy_from(struct object_vector* self, struct object_vector const* from) {
  size_t len = from->length;
  void** array = from->array;
  void** old_array = self->array;
  size_t old_len = self->length;
  void** buf = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    size_t bytes = len * sizeof(void*);
    buf = malloc(bytes);
    ERR_REGION_NULL_CHECK(buf, err);

    void* copied = 0;
    for (size_t i = 0; i < len; ++i) {
      copied = self->ops.acquire(*(array + i));
      if (!copied) break;
      ERR_IGNORE_WARNING(6386, buf[i] = copied;)
    }
    ERR_REGION_NULL_CHECK(copied, err);

    self->array = buf;
    self->length = len;

    for (size_t i = 0; i < old_len; ++i) {
      self->ops.acquire(old_array[i]);
    }
    SAFE_FREE(old_array);

    return self->array;

  } ERR_REGION_END()

  return NULL;
}
