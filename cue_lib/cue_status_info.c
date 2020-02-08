#include "cue_status_info.h"

#include <string.h>
#include <stdlib.h>

#include "mem_helpers.h"
#include "err_helpers.h"

static void* acquire(void const* instance);
static void release(void* instance);

struct object_vector_params cue_sheet_parse_error_vector_ops = {
  acquire,
  release,
};

static void release(void* instance) {
  SAFE_FREE_HANDLER(instance, cue_sheet_parse_error_free);
}

static void* acquire(void const* instance) {
  return (void *)instance;
}

IMPLEMENT_OBJECT_VECTOR(cue_sheet_parse_error_vector, cue_status_info_t);

struct cue_status_info* cue_sheet_parse_error_alloc(
  size_t line_num,
  char const* line) {
  struct cue_status_info* self = malloc(sizeof(*self));
  if (!self) return NULL;

  errno_t err = cue_sheet_parse_error_init(self, line_num, line);
  if (!err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_sheet_parse_error_init(struct cue_status_info* self,
  size_t line_num,
  char const* line) {
  memset(self, 0, sizeof(*self));

  char const *buf = _strdup(line);
  if (!buf) return -1;

  self->line = buf;
  self->line_num = line_num;

  return 0;
}

void cue_sheet_parse_error_uninit(struct cue_status_info* self) {
  SAFE_FREE(self->line);
}

void cue_sheet_parse_error_free(struct cue_status_info* self) {
  cue_sheet_parse_error_uninit(self);
  SAFE_FREE(self);
}


struct cue_sheet_parse_result* cue_sheet_parse_result_alloc() {
  struct cue_sheet_parse_result* self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t err = cue_sheet_parse_result_init(self);
  if (! err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_sheet_parse_result_init(struct cue_sheet_parse_result* self) {
  memset(self, 0, sizeof(*self));

  cue_sheet_parse_error_vector_t *vector = cue_sheet_parse_error_vector_alloc();
  if (! vector) return -1;

  self->errors = vector;

  return 0;
}

void cue_sheet_parse_result_uninit(struct cue_sheet_parse_result* self) {
  self->errors->free(self->errors);
}

void cue_sheet_parse_result_free(struct cue_sheet_parse_result* self) {
  cue_sheet_parse_result_uninit(self);
  SAFE_FREE(self);
}

struct cue_status_info const* cue_sheet_parse_result_add_error(
  struct cue_sheet_parse_result* self,
  size_t line_num,
  char const* line) {

  errno_t err = 0;
  cue_status_info_t* error = 0;
  cue_status_info_t const* added = 0;

  ERR_REGION_BEGIN() {

    error = cue_sheet_parse_error_alloc(line_num, line);
    ERR_REGION_NULL_CHECK(error, err);

    added = self->errors->push(self->errors, error);
    ERR_REGION_NULL_CHECK(added, err);

    self->has_errors = 1;

    return added;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(error, cue_sheet_parse_error_free);

  return NULL;

}

