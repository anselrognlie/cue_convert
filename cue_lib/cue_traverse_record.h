#pragma once

#include "object_vector.h"

#include <stddef.h>

typedef struct cue_traverse_record {
  struct char_vector* target_path;
  struct cue_sheet* target_sheet;
  struct char_vector* source_path;
  struct cue_sheet* source_sheet;
} cue_traverse_record_t;

cue_traverse_record_t* cue_traverse_record_alloc(void);
cue_traverse_record_t* cue_traverse_record_alloc_with_paths(char const *target_path, char const *source_path);
cue_traverse_record_t* cue_traverse_record_alloc_copy(cue_traverse_record_t const* src);
errno_t cue_traverse_record_init(cue_traverse_record_t* self);
errno_t cue_traverse_record_init_with_paths(cue_traverse_record_t* self, char const* target_path, char const* source_path);
errno_t cue_traverse_record_copy_from(cue_traverse_record_t* dest, cue_traverse_record_t const* src);
errno_t cue_traverse_record_uninit(cue_traverse_record_t* self);
errno_t cue_traverse_record_free(cue_traverse_record_t* self);

extern struct object_vector_params cue_traverse_record_vector_ops;

typedef struct cue_traverse_record_vector {
  object_vector_t vector_t;
  INSERT_OBJECT_VECTOR_METHODS(cue_traverse_record_vector, cue_traverse_record_t)
} cue_traverse_record_vector_t;

DECLARE_OBJECT_VECTOR(cue_traverse_record_vector, cue_traverse_record_t)
