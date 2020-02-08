#pragma once

#include <stddef.h>

#include "object_vector.h"

typedef struct cue_status_info {
  size_t line_num;
  char const* line;
} cue_status_info_t;

extern struct object_vector_params cue_sheet_parse_error_vector_ops;

typedef struct cue_sheet_parse_error_vector {
  object_vector_t vector_t;
  INSERT_OBJECT_VECTOR_METHODS(cue_sheet_parse_error_vector, cue_status_info_t)
} cue_sheet_parse_error_vector_t;

DECLARE_OBJECT_VECTOR(cue_sheet_parse_error_vector, cue_status_info_t);

typedef struct cue_sheet_parse_result {
  short has_errors;
  cue_sheet_parse_error_vector_t* errors;
} cue_sheet_parse_result_t;

struct cue_status_info* cue_sheet_parse_error_alloc(
  size_t line_num,
  char const* line);
errno_t cue_sheet_parse_error_init(struct cue_status_info* self,
  size_t line_num,
  char const *line);
void cue_sheet_parse_error_uninit(struct cue_status_info* self);
void cue_sheet_parse_error_free(struct cue_status_info* self);

struct cue_sheet_parse_result* cue_sheet_parse_result_alloc();
errno_t cue_sheet_parse_result_init(struct cue_sheet_parse_result* self);
void cue_sheet_parse_result_uninit(struct cue_sheet_parse_result* self);
void cue_sheet_parse_result_free(struct cue_sheet_parse_result* self);

struct cue_status_info const* cue_sheet_parse_result_add_error(
  struct cue_sheet_parse_result* self,
  size_t line_num,
  char const* line);
