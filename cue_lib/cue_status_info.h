#pragma once

#include <stddef.h>

#include "object_vector.h"

typedef enum cue_status_type {
  EWC_CST_STATUS = 0,
  EWC_CST_ERROR,
  EWC_CST_PARSE_ERROR,
  EWC_CST_LAST,
} cue_status_type_t;

typedef struct cue_status_info {
  cue_status_type_t type;
  size_t line_num;
  char const* detail;
} cue_status_info_t;

extern struct object_vector_params cue_status_info_vector_ops;

typedef struct cue_status_info_vector {
  object_vector_t vector_t;
  INSERT_OBJECT_VECTOR_METHODS(cue_status_info_vector, cue_status_info_t)
} cue_status_info_vector_t;

DECLARE_OBJECT_VECTOR(cue_status_info_vector, cue_status_info_t);

typedef struct cue_sheet_parse_result {
  short has_errors;
  cue_status_info_vector_t* errors;
} cue_sheet_parse_result_t;

struct cue_status_info* cue_status_info_alloc(
  size_t line_num,
  char const* line);
errno_t cue_status_info_init_parse_error(struct cue_status_info* self,
  size_t line_num,
  char const* line);
errno_t cue_status_info_init_error(struct cue_status_info* self,
  char const* msg);
errno_t cue_status_info_init_status(struct cue_status_info* self,
  char const* msg);
void cue_status_info_uninit(struct cue_status_info* self);
void cue_status_info_free(struct cue_status_info* self);

struct cue_sheet_parse_result* cue_sheet_parse_result_alloc();
errno_t cue_sheet_parse_result_init(struct cue_sheet_parse_result* self);
void cue_sheet_parse_result_uninit(struct cue_sheet_parse_result* self);
void cue_sheet_parse_result_free(struct cue_sheet_parse_result* self);

struct cue_status_info const* cue_sheet_parse_result_add_error(
  struct cue_sheet_parse_result* self,
  size_t line_num,
  char const* line);
