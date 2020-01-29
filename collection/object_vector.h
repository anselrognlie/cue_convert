#pragma once

#include <stddef.h>

struct object_vector;
struct object_vector_params;

typedef struct object_vector_params {
  size_t type_size;
  void* (*alloc)(size_t size);
  void (*free)(void* instance);
  void* (*copy_in)(void* dst, void const* src);
} object_vector_params_t;

typedef struct object_vector {
  size_t length;
  struct object_vector_params ops;
  void **array;
} object_vector_t;

struct object_vector* object_vector_alloc(struct object_vector_params* ops);
errno_t object_vector_init(object_vector_t* self, struct object_vector_params *ops);
errno_t object_vector_uninit(struct object_vector* self);
errno_t object_vector_free(struct object_vector* self);

size_t object_vector_get_length(struct object_vector* self);
void const** object_vector_get_buffer(struct object_vector* self);

void const* object_vector_get(struct object_vector* self, size_t i);
void const * object_vector_set(struct object_vector* self, size_t i, void const*instance);
errno_t object_vector_delete_at(struct object_vector* self, size_t i);
errno_t object_vector_delete_at_keep(struct object_vector* self, size_t i, void **out);
void const* object_vector_insert_at(struct object_vector* self, size_t i, void const*instance);

void const* object_vector_push(struct object_vector* self, void const* instance);
errno_t object_vector_pop(struct object_vector* self);
errno_t object_vector_pop_keep(struct object_vector* self, void** out);
void const* object_vector_unshift(struct object_vector* self, void const* instance);
errno_t object_vector_shift(struct object_vector* self);
errno_t object_vector_shift_keep(struct object_vector* self, void** out);

#define DECLARE_OBJECT_VECTOR(vtype, type) \
struct vtype* vtype##_alloc(); \
errno_t vtype##_init(struct vtype* self); \
errno_t vtype##_uninit(struct vtype* self); \
errno_t vtype##_free(struct vtype* self); \
\
size_t vtype##_get_length(struct vtype* self); \
type const** vtype##_get_buffer(struct vtype* self); \
\
type const* vtype##_get(struct vtype* self, size_t i); \
type const * vtype##_set(struct vtype* self, size_t i, type const*instance); \
errno_t vtype##_delete_at(struct vtype* self, size_t i); \
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type **out); \
type const* vtype##_insert_at(struct vtype* self, size_t i, type const*instance); \
\
type const* vtype##_push(struct vtype* self, type const* instance); \
errno_t vtype##_pop(struct vtype* self); \
errno_t vtype##_pop_keep(struct vtype* self, type** out); \
type const* vtype##_unshift(struct vtype* self, type const* instance); \
errno_t vtype##_shift(struct vtype* self); \
errno_t vtype##_shift_keep(struct vtype* self, type** out); \

#define IMPLEMENT_OBJECT_VECTOR(vtype, type) \
struct vtype* vtype##_alloc() { \
  vtype##_t* self = malloc(sizeof(*self)); \
  if (!self) return NULL; \
\
  errno_t result = vtype##_init(self); \
  if (result) return NULL; \
\
  return self; \
} \
\
errno_t vtype##_init(struct vtype* self) { \
  return object_vector_init(&self->self, &vtype##_ops); \
} \
\
errno_t vtype##_uninit(struct vtype* self) { \
  return object_vector_uninit(&self->self); \
} \
\
errno_t vtype##_free(struct vtype* self) { \
  errno_t result = object_vector_uninit(&self->self); \
  if (result) return -1; \
  free(self); \
  return 0; \
} \
\
size_t vtype##_get_length(struct vtype* self) { \
  return object_vector_get_length(&self->self); \
} \
\
type const** vtype##_get_buffer(struct vtype* self) { \
  return (type const**)object_vector_get_buffer(&self->self); \
} \
\
type const* vtype##_get(struct vtype* self, size_t i) { \
  return (type const*)object_vector_get(&self->self, i); \
} \
\
type const* vtype##_set(struct vtype* self, size_t i, type const* instance) { \
  return object_vector_set(&self->self, i, (void const*)instance); \
} \
\
errno_t vtype##_delete_at(struct vtype* self, size_t i) { \
  return object_vector_delete_at(&self->self, i); \
} \
\
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type** out) { \
  return object_vector_delete_at_keep(&self->self, i, (void**)out); \
} \
\
type const* vtype##_insert_at(struct vtype* self, size_t i, type const* instance) { \
  return (type const*)object_vector_insert_at(&self->self, i, (void const*)instance); \
} \
\
type const* vtype##_push(struct vtype* self, type const* instance) { \
  return (type const*)object_vector_push(&self->self, (void const*)instance); \
} \
\
errno_t vtype##_pop(struct vtype* self) { \
  return object_vector_pop(&self->self); \
} \
\
errno_t vtype##_pop_keep(struct vtype* self, type** out) { \
  return object_vector_pop_keep(&self->self, (void**)out); \
} \
\
type const* vtype##_unshift(struct vtype* self, type const* instance) { \
  return (type const*)object_vector_unshift(&self->self, (void const*)instance); \
} \
\
errno_t vtype##_shift(struct vtype* self) { \
  return object_vector_shift(&self->self); \
} \
\
errno_t vtype##_shift_keep(struct vtype* self, type** out) { \
  return object_vector_shift_keep(&self->self, (void**)out); \
} \
