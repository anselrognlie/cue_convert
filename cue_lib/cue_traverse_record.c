#include "cue_traverse_record.h"

#include <stdlib.h>

#include "err_helpers.h"
#include "char_vector.h"
#include "cue_file.h"
#include "mem_helpers.h"

static void* ctr_alloc(size_t size);
static void ctr_free(void* instance);
static void* ctr_copy_in(void* dst, void const* src);

struct object_vector_params cue_traverse_record_vector_ops = {
  sizeof(cue_traverse_record_t),
  ctr_alloc,
  ctr_free,
  ctr_copy_in,
};

static void* ctr_alloc(size_t size) {
  return cue_traverse_record_alloc();
}

static void ctr_free(void* instance) {
  cue_traverse_record_free((cue_traverse_record_t*)instance);
}

static void* ctr_copy_in(void* dst, void const* src) {
  if (!cue_traverse_record_copy_from(dst, src)) return dst;

  return NULL;
}

IMPLEMENT_OBJECT_VECTOR(cue_traverse_record_vector, cue_traverse_record_t)

cue_traverse_record_t* cue_traverse_record_alloc(void) {
  cue_traverse_record_t* self = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    self = malloc(sizeof(*self));
    ERR_REGION_NULL_CHECK(self, err);

    ERR_REGION_ERROR_CHECK(cue_traverse_record_init(self), err);

    return self;
  } ERR_REGION_END()

  SAFE_FREE(self);

  return NULL;
}

cue_traverse_record_t* cue_traverse_record_alloc_copy(cue_traverse_record_t const* src) {
  cue_traverse_record_t* self = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    self = cue_traverse_record_alloc();
    ERR_REGION_NULL_CHECK(self, err);

    ERR_REGION_ERROR_CHECK(cue_traverse_record_copy_from(self, src), err);

    return self;
  } ERR_REGION_END()

  if (self) cue_traverse_record_free(self);

  return NULL;
}

errno_t cue_traverse_record_init(cue_traverse_record_t* self) {
  errno_t err = 0;
  errno_t op_err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    self->target_path = char_vector_alloc();
    ERR_REGION_NULL_CHECK(self->target_path, err);
    self->source_path = char_vector_alloc();
    ERR_REGION_NULL_CHECK(self->source_path, err);

    return err;
  } ERR_REGION_END()

  if (self) {
    if (self->source_path) op_err = char_vector_free(self->source_path);
    err = err ? err : op_err;
    if (self->target_path) op_err = char_vector_free(self->target_path);
    err = err ? err : op_err;
    SAFE_FREE(self);
  }

  return err;
}

errno_t cue_traverse_record_copy_from(cue_traverse_record_t* self, cue_traverse_record_t const* src) {
  cue_sheet_t* target_sheet = 0, * source_sheet = 0;
  char_vector_t* target_path = 0, * source_path = 0;
  cue_sheet_t* old_target_sheet = self->target_sheet;
  cue_sheet_t* old_source_sheet = self->source_sheet;
  char_vector_t* old_target_path = self->target_path;
  char_vector_t* old_source_path = self->source_path;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    target_path = char_vector_alloc_copy(src->target_path);
    ERR_REGION_NULL_CHECK(target_path, err);

    source_path = char_vector_alloc_copy(src->source_path);
    ERR_REGION_NULL_CHECK(source_path, err);

    if (src->target_sheet) {
      target_sheet = cue_sheet_alloc_copy(src->target_sheet);
      ERR_REGION_NULL_CHECK(target_sheet, err);
    }

    if (src->source_sheet) {
      source_sheet = cue_sheet_alloc_copy(src->source_sheet);
      ERR_REGION_NULL_CHECK(source_sheet, err);
    }

    self->target_path = target_path;
    self->source_path = source_path;
    self->target_sheet = target_sheet;
    self->source_sheet = source_sheet;

    if (old_target_sheet) cue_sheet_free(old_target_sheet);
    if (old_source_sheet) cue_sheet_free(old_source_sheet);
    char_vector_free(old_target_path);
    char_vector_free(old_source_path);

    return err;

  } ERR_REGION_END()

  if (source_sheet) cue_sheet_free(source_sheet);
  if (target_sheet) cue_sheet_free(target_sheet);
  if (source_path) char_vector_free(source_path);
  if (target_path) char_vector_free(target_path);

  return err;
}

errno_t cue_traverse_record_uninit(cue_traverse_record_t* self) {
  if (self->target_sheet) cue_sheet_free(self->target_sheet);
  if (self->source_sheet) cue_sheet_free(self->source_sheet);
  char_vector_free(self->source_path);
  char_vector_free(self->target_path);
  return 0;
}

errno_t cue_traverse_record_free(cue_traverse_record_t* self) {
  cue_traverse_record_uninit(self);
  SAFE_FREE(self);
  return 0;
}

