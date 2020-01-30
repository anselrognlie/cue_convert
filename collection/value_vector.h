#pragma once

#include <stddef.h>
#include "err_helpers.h"
#include "mem_helpers.h"

struct value_vector;
struct value_vector_params;

typedef struct value_vector_params {
  size_t type_size;
} value_vector_params_t;

typedef struct value_vector {
  size_t length;
  struct value_vector_params ops;
  char* array;
} value_vector_t;

struct value_vector* value_vector_alloc(struct value_vector_params const* ops);
struct value_vector* value_vector_alloc_copy(value_vector_t const* from);
errno_t value_vector_init(value_vector_t* self, struct value_vector_params const* ops);
errno_t value_vector_uninit(struct value_vector* self);
errno_t value_vector_free(struct value_vector* self);

// only to be used by extensions
char const* value_vector_resize(value_vector_t* self, size_t size);

size_t value_vector_get_length(struct value_vector const* self);
char const* value_vector_get_buffer(struct value_vector const* self);

char const* value_vector_get(struct value_vector const* self, size_t i);
void value_vector_set(struct value_vector* self, size_t i, char const* instance);
errno_t value_vector_delete_at(struct value_vector* self, size_t i);
errno_t value_vector_delete_at_keep(struct value_vector* self, size_t i, char* out);
char const* value_vector_insert_at(struct value_vector* self, size_t i, char const* instance);

char const* value_vector_push(struct value_vector* self, char const* instance);
errno_t value_vector_pop(struct value_vector* self);
errno_t value_vector_pop_keep(struct value_vector* self, char* out);
char const* value_vector_unshift(struct value_vector* self, char const* instance);
errno_t value_vector_shift(struct value_vector* self);
errno_t value_vector_shift_keep(struct value_vector* self, char* out);

char const* value_vector_copy_from(struct value_vector* self, struct value_vector const* from);

#define DECLARE_VALUE_VECTOR(vtype, type) \
struct vtype* vtype##_alloc(); \
struct vtype* vtype##_alloc_copy(struct vtype const* from); \
errno_t vtype##_init(struct vtype* self); \
errno_t vtype##_uninit(struct vtype* self); \
errno_t vtype##_free(struct vtype* self); \
\
size_t vtype##_get_length(struct vtype const* self); \
type const* vtype##_get_buffer(struct vtype const* self); \
\
type vtype##_get(struct vtype const* self, size_t i); \
void vtype##_set(struct vtype* self, size_t i, type instance); \
errno_t vtype##_delete_at(struct vtype* self, size_t i); \
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type *out); \
type const* vtype##_insert_at(struct vtype* self, size_t i, type instance); \
\
type const* vtype##_push(struct vtype* self, type instance); \
errno_t vtype##_pop(struct vtype* self); \
errno_t vtype##_pop_keep(struct vtype* self, type* out); \
type const* vtype##_unshift(struct vtype* self, type instance); \
errno_t vtype##_shift(struct vtype* self); \
errno_t vtype##_shift_keep(struct vtype* self, type* out); \
\
type const* vtype##_copy_from(struct vtype* dst, struct vtype const* src); \

#define IMPLEMENT_VALUE_VECTOR(vtype, type) \
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
  type const *result = vtype##_copy_from(self, from); \
  if (result) return NULL; \
\
  return self; \
} \
\
errno_t vtype##_init(struct vtype* self) { \
  return value_vector_init(&self->self, &vtype##_ops); \
} \
\
errno_t vtype##_uninit(struct vtype* self) { \
  return value_vector_uninit(&self->self); \
} \
\
errno_t vtype##_free(struct vtype* self) { \
  errno_t result = value_vector_uninit(&self->self); \
  if (result) return -1; \
  free(self); \
  return 0; \
} \
\
size_t vtype##_get_length(struct vtype const* self) { \
  return value_vector_get_length(&self->self); \
} \
\
type const* vtype##_get_buffer(struct vtype const* self) { \
  return (type const*)value_vector_get_buffer(&self->self); \
} \
\
type vtype##_get(struct vtype const* self, size_t i) { \
  return *(type const*)value_vector_get(&self->self, i); \
} \
\
void vtype##_set(struct vtype* self, size_t i, type instance) { \
  value_vector_set(&self->self, i, (char const *)&instance); \
} \
\
errno_t vtype##_delete_at(struct vtype* self, size_t i) { \
  return value_vector_delete_at(&self->self, i); \
} \
\
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type* out) { \
  return value_vector_delete_at_keep(&self->self, i, (char*)out); \
} \
\
type const* vtype##_insert_at(struct vtype* self, size_t i, type instance) { \
  return (type const*)value_vector_insert_at(&self->self, i, (char const *)&instance); \
} \
\
type const* vtype##_push(struct vtype* self, type instance) { \
  return (type const*)value_vector_push(&self->self, (char const*)&instance); \
} \
\
errno_t vtype##_pop(struct vtype* self) { \
  return value_vector_pop(&self->self); \
} \
\
errno_t vtype##_pop_keep(struct vtype* self, type* out) { \
  return value_vector_pop_keep(&self->self, (char*)out); \
} \
\
type const* vtype##_unshift(struct vtype* self, type instance) { \
  return (type const*)value_vector_unshift(&self->self, (char const*)&instance); \
} \
\
errno_t vtype##_shift(struct vtype* self) { \
  return value_vector_shift(&self->self); \
} \
\
errno_t vtype##_shift_keep(struct vtype* self, type* out) { \
  return value_vector_shift_keep(&self->self, (char*)out); \
} \
\
type const* vtype##_copy_from(struct vtype* dst, struct vtype const* src) { \
  return (type const *)value_vector_copy_from(&dst->self, &src->self); \
} \

#define DECLARE_POD_VALUE_VECTOR(type) \
extern struct value_vector_params type##_vector_ops; \
typedef struct type##_vector { \
  value_vector_t self; \
} type##_vector_t; \
DECLARE_VALUE_VECTOR(type##_vector, type)

#define IMPLEMENT_POD_VALUE_VECTOR(type) \
struct value_vector_params type##_vector_ops = { \
  sizeof(type), \
}; \
IMPLEMENT_VALUE_VECTOR(type##_vector, type)

DECLARE_POD_VALUE_VECTOR(int)
DECLARE_POD_VALUE_VECTOR(double)
