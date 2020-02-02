#pragma once

#include "object_vector.h"

extern struct object_vector_params string_vector_ops;

typedef struct string_vector {
  object_vector_t vector_t;
  INSERT_OBJECT_VECTOR_METHODS(string_vector, char)
} string_vector_t;

DECLARE_OBJECT_VECTOR(string_vector, char)
