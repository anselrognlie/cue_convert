#pragma once

#include <stddef.h>
#include "err_helpers.h"
#include "mem_helpers.h"

struct value_vector;
struct value_vector_params;

typedef struct value_vector_params {
  size_t type_size;
  char *(*array_alloc)(struct value_vector *self, size_t item_count);
} value_vector_params_t;

typedef struct value_vector {
  size_t length;
  struct value_vector_params ops;
  char* array;
} value_vector_t;

struct value_vector* value_vector_alloc(struct value_vector_params const* ops);
struct value_vector* value_vector_alloc_copy(value_vector_t const* from);
errno_t value_vector_init(value_vector_t* self, struct value_vector_params const* ops);
void value_vector_uninit(struct value_vector* self);
void value_vector_free(struct value_vector* self);

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
void vtype##_uninit(struct vtype* self); \
void vtype##_free(struct vtype* self); \
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
type const* vtype##_copy_from(struct vtype* self, struct vtype const* from); \

#define INSERT_VALUE_VECTOR_METHODS(vtype, type) \
  void (*uninit)(struct vtype* self); \
  void (*free)(struct vtype* self); \
  size_t (*get_length)(struct vtype const* self); \
  type const* (*get_buffer)(struct vtype const* self); \
  type (*get)(struct vtype const* self, size_t i); \
  void (*set)(struct vtype* self, size_t i, type instance); \
  errno_t (*delete_at)(struct vtype* self, size_t i); \
  errno_t (*delete_at_keep)(struct vtype* self, size_t i, type *out); \
  type const* (*insert_at)(struct vtype* self, size_t i, type instance); \
  type const* (*push)(struct vtype* self, type instance); \
  errno_t (*pop)(struct vtype* self); \
  errno_t (*pop_keep)(struct vtype* self, type* out); \
  type const* (*unshift)(struct vtype* self, type instance); \
  errno_t (*shift)(struct vtype* self); \
  errno_t (*shift_keep)(struct vtype* self, type* out); \
  type const* (*copy_from)(struct vtype* self, struct vtype const* from); \

#define ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, method) \
  self->method = vtype##_##method \

#define VALUE_VECTOR_INIT_CUSTOM ;

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
  if (! result) return NULL; \
\
  return self; \
} \
\
errno_t vtype##_init(struct vtype* self) { \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, uninit); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, free); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, get_length); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, get_buffer); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, get); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, set); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, delete_at); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, delete_at_keep); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, insert_at); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, push); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, pop); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, pop_keep); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, unshift); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, shift); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, shift_keep); \
  ZZZ_INTERNAL_IMPLEMENT_VALUE_VECTOR_METHODS(vtype, copy_from); \
\
  VALUE_VECTOR_INIT_CUSTOM \
  return value_vector_init(&self->vector_t, &vtype##_ops); \
} \
\
void vtype##_uninit(struct vtype* self) { \
  value_vector_uninit(&self->vector_t); \
} \
\
void vtype##_free(struct vtype* self) { \
  value_vector_uninit(&self->vector_t); \
  free(self); \
} \
\
size_t vtype##_get_length(struct vtype const* self) { \
  return value_vector_get_length(&self->vector_t); \
} \
\
type const* vtype##_get_buffer(struct vtype const* self) { \
  return (type const*)value_vector_get_buffer(&self->vector_t); \
} \
\
type vtype##_get(struct vtype const* self, size_t i) { \
  return *(type const*)value_vector_get(&self->vector_t, i); \
} \
\
void vtype##_set(struct vtype* self, size_t i, type instance) { \
  value_vector_set(&self->vector_t, i, (char const *)&instance); \
} \
\
errno_t vtype##_delete_at(struct vtype* self, size_t i) { \
  return value_vector_delete_at(&self->vector_t, i); \
} \
\
errno_t vtype##_delete_at_keep(struct vtype* self, size_t i, type* out) { \
  return value_vector_delete_at_keep(&self->vector_t, i, (char*)out); \
} \
\
type const* vtype##_insert_at(struct vtype* self, size_t i, type instance) { \
  return (type const*)value_vector_insert_at(&self->vector_t, i, (char const *)&instance); \
} \
\
type const* vtype##_push(struct vtype* self, type instance) { \
  return (type const*)value_vector_push(&self->vector_t, (char const*)&instance); \
} \
\
errno_t vtype##_pop(struct vtype* self) { \
  return value_vector_pop(&self->vector_t); \
} \
\
errno_t vtype##_pop_keep(struct vtype* self, type* out) { \
  return value_vector_pop_keep(&self->vector_t, (char*)out); \
} \
\
type const* vtype##_unshift(struct vtype* self, type instance) { \
  return (type const*)value_vector_unshift(&self->vector_t, (char const*)&instance); \
} \
\
errno_t vtype##_shift(struct vtype* self) { \
  return value_vector_shift(&self->vector_t); \
} \
\
errno_t vtype##_shift_keep(struct vtype* self, type* out) { \
  return value_vector_shift_keep(&self->vector_t, (char*)out); \
} \
\
type const* vtype##_copy_from(struct vtype* self, struct vtype const* from) { \
  return (type const *)value_vector_copy_from(&self->vector_t, &from->vector_t); \
} \

#define DECLARE_POD_VALUE_VECTOR(type) \
extern struct value_vector_params type##_vector_ops; \
typedef struct type##_vector { \
  value_vector_t vector_t; \
  INSERT_VALUE_VECTOR_METHODS(type##_vector, type) \
} type##_vector_t; \
DECLARE_VALUE_VECTOR(type##_vector, type)

#define IMPLEMENT_POD_VALUE_VECTOR(type) \
struct value_vector_params type##_vector_ops = { \
  sizeof(type), \
  0, \
}; \
IMPLEMENT_VALUE_VECTOR(type##_vector, type)

DECLARE_POD_VALUE_VECTOR(int)
DECLARE_POD_VALUE_VECTOR(double)
