#pragma once

#include "cvector.h"

extern struct cvector_params string_vector_ops;

typedef struct string_vector {
  cvector_t self;
} string_vector_t;

struct string_vector* string_vector_alloc();
errno_t string_vector_init(struct string_vector* self);

DECLARE_VECTOR(string_vector, char)
