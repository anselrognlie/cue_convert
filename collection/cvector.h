#pragma once

#include <stddef.h>

struct cvector;
struct cvector_params;

typedef struct cvector_params {
  size_t type_size;
  void* (*alloc)(size_t size);
  void (*free)(void* instance);
  void* (*copy_in)(void* dst, void const* src);
} cvector_params_t;

typedef struct cvector {
  size_t length;
  struct cvector_params ops;
  void **array;
} cvector_t;

struct cvector* cvector_alloc(struct cvector_params* ops);
errno_t cvector_init(cvector_t* self, struct cvector_params *ops);

#define DECLARE_VECTOR(vtype, type) \
errno_t vtype##_uninit(struct vtype* self); \
errno_t vtype##_free(struct vtype* self); \
\
size_t vtype##_get_length(struct vtype* self); \
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

DECLARE_VECTOR(cvector, void)
