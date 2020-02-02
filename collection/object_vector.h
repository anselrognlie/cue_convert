#pragma once

#include <stddef.h>

struct object_vector;
struct object_vector_params;

//extern const void *k_object_vector_shallow;

typedef struct object_vector_params {
  //size_t type_size;
  //void* (*alloc)(size_t size);  // if vector should not copy, return k_object_vector_shallow
  //void (*free)(void* instance);  // might receive k_object_vector_shallow
  //void* (*copy_in)(void* dst, void const* src);
  void* (*acquire)(void const* instance);
  void (*release)(void *instance);
} object_vector_params_t;

extern object_vector_params_t object_vector_weak_params;

typedef struct object_vector {
  size_t length;
  struct object_vector_params ops;
  void **array;
} object_vector_t;

struct object_vector* object_vector_alloc(struct object_vector_params const* ops);
struct object_vector* object_vector_alloc_copy(struct object_vector const* from);
errno_t object_vector_init(object_vector_t* self, struct object_vector_params const*ops);
void object_vector_uninit(struct object_vector* self);
void object_vector_free(struct object_vector* self);

size_t object_vector_get_length(struct object_vector const* self);
void const** object_vector_get_buffer(struct object_vector const* self);

void const* object_vector_get(struct object_vector const* self, size_t i);
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

void const** object_vector_copy_from(struct object_vector* self, struct object_vector const* from);

#define DECLARE_OBJECT_VECTOR(vtype, type) \
struct vtype* vtype##_alloc(); \
errno_t vtype##_init(struct vtype* self); \
void vtype##_uninit(struct vtype* self); \
void vtype##_free(struct vtype* self); \
\
size_t vtype##_get_length(struct vtype const* self); \
type const** vtype##_get_buffer(struct vtype const* self); \
\
type const* vtype##_get(struct vtype const* self, size_t i); \
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
\
type const** vtype##_copy_from(struct vtype* self, struct vtype const* from); \

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
struct vtype* vtype##_alloc_copy(struct vtype const* from) { \
  vtype##_t* self = vtype##_alloc(); \
  if (!self) return NULL; \
\
  type const **result = vtype##_copy_from(self, from); \
  if (! result) { \
    vtype##_free(self); \
    return NULL; \
  } \
\
  return self; \
} \
\
errno_t vtype##_init(struct vtype* self) { \
  return object_vector_init(&self->vector_t, &vtype##_ops); \
} \
\
void vtype##_uninit(struct vtype* self) { \
  object_vector_uninit(&self->vector_t); \
} \
\
void vtype##_free(struct vtype* self) { \
  object_vector_uninit(&self->vector_t); \
  free(self); \
} \
\
size_t vtype##_get_length(struct vtype const* self) { \
  return object_vector_get_length(&self->vector_t); \
} \
\
type const** vtype##_get_buffer(struct vtype const* self) { \
  return (type const**)object_vector_get_buffer(&self->vector_t); \
} \
\
type const* vtype##_get(struct vtype const* self, size_t i) { \
  return (type const*)object_vector_get(&self->vector_t, i); \
} \
\
type const* vtype##_set(struct vtype* self, size_t i, type const* instance) { \
  return object_vector_set(&self->vector_t, i, (void const*)instance); \
} \
\
errno_t vtype##_delete_at(struct vtype* self, size_t i) { \
  return object_vector_delete_at(&self->vector_t, i); \
} \
\
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type** out) { \
  return object_vector_delete_at_keep(&self->vector_t, i, (void**)out); \
} \
\
type const* vtype##_insert_at(struct vtype* self, size_t i, type const* instance) { \
  return (type const*)object_vector_insert_at(&self->vector_t, i, (void const*)instance); \
} \
\
type const* vtype##_push(struct vtype* self, type const* instance) { \
  return (type const*)object_vector_push(&self->vector_t, (void const*)instance); \
} \
\
errno_t vtype##_pop(struct vtype* self) { \
  return object_vector_pop(&self->vector_t); \
} \
\
errno_t vtype##_pop_keep(struct vtype* self, type** out) { \
  return object_vector_pop_keep(&self->vector_t, (void**)out); \
} \
\
type const* vtype##_unshift(struct vtype* self, type const* instance) { \
  return (type const*)object_vector_unshift(&self->vector_t, (void const*)instance); \
} \
\
errno_t vtype##_shift(struct vtype* self) { \
  return object_vector_shift(&self->vector_t); \
} \
\
errno_t vtype##_shift_keep(struct vtype* self, type** out) { \
  return object_vector_shift_keep(&self->vector_t, (void**)out); \
} \
\
type const** vtype##_copy_from(struct vtype* self, struct vtype const* from) { \
  return (type const **)object_vector_copy_from(&self->vector_t, &from->vector_t); \
} \
