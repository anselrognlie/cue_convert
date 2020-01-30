#pragma once

#include "value_vector.h"

extern struct value_vector_params char_vector_ops;

typedef struct char_vector {
  value_vector_t self;
} char_vector_t;

DECLARE_VALUE_VECTOR(char_vector, char)

typedef struct char_vector string_t;

char const* char_vector_set_str(struct char_vector* self, char const* instance);
char const* char_vector_get_str(struct char_vector const* self);
void char_vector_get_str_buf(struct char_vector const* self, char *buf, size_t buf_len);
char const* char_vector_copy_from(struct char_vector* dst, struct char_vector const* src);
