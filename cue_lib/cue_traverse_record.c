#include "cue_traverse_record.h"

#include <stdlib.h>

#include "err_helpers.h"
#include "char_vector.h"
#include "cue_file.h"
#include "mem_helpers.h"
#include "cue_sheet_parse_result.h"

static void* acquire(void const* instance);
static void release(void* instance);

struct object_vector_params cue_traverse_record_vector_ops = {
  acquire,
  release,
};

static void release(void* instance) {
  cue_traverse_record_free((cue_traverse_record_t*)instance);
}

static void* acquire(void const* instance) {
  return (void *)instance;
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

cue_traverse_record_t* cue_traverse_record_alloc_with_paths(char const* target_path, char const* source_path) {
  cue_traverse_record_t* self = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    self = malloc(sizeof(*self));
    ERR_REGION_NULL_CHECK(self, err);

    ERR_REGION_ERROR_CHECK(cue_traverse_record_init_with_paths(
      self,
      target_path,
      source_path), err);

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

  SAFE_FREE_HANDLER(self, cue_traverse_record_free);

  return NULL;
}

errno_t cue_traverse_record_init(cue_traverse_record_t* self) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    self->target_path = _strdup("");
    ERR_REGION_NULL_CHECK(self->target_path, err);
    self->source_path = _strdup("");
    ERR_REGION_NULL_CHECK(self->source_path, err);
    self->result = cue_sheet_parse_result_alloc();
    ERR_REGION_NULL_CHECK(self->result, err);

    return err;
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(self->result, cue_sheet_parse_result_free);
  SAFE_FREE(self->source_path);
  SAFE_FREE(self->target_path);

  return err;
}

errno_t cue_traverse_record_init_with_paths(cue_traverse_record_t* self, char const* target_path, char const* source_path) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    ERR_REGION_ERROR_CHECK(cue_traverse_record_init(self), err);

    const char* new_src = 0;
    const char* new_trg = 0;
    ERR_REGION_BEGIN() {
      const char* old_src = self->source_path;
      const char* old_trg = self->target_path;

      new_src = _strdup(source_path);
      ERR_REGION_NULL_CHECK(new_src, err);
      new_trg = _strdup(target_path);
      ERR_REGION_NULL_CHECK(new_trg, err);

      self->source_path = new_src;
      self->target_path = new_trg;
      SAFE_FREE(old_src);
      SAFE_FREE(old_trg);

      return err;
    } ERR_REGION_END()

    // init was fine, but internal sets failed, so make sure to uninit
    SAFE_FREE(new_trg);
    SAFE_FREE(new_src);
    cue_traverse_record_uninit(self);

  } ERR_REGION_END()

  return err;
}

errno_t cue_traverse_record_copy_from(cue_traverse_record_t* self, cue_traverse_record_t const* src) {
  cue_sheet_t* target_sheet = 0, * source_sheet = 0;
  char const* target_path = 0, * source_path = 0;
  cue_sheet_t* old_target_sheet = self->target_sheet;
  cue_sheet_t* old_source_sheet = self->source_sheet;
  char const * old_target_path = self->target_path;
  char const * old_source_path = self->source_path;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    target_path = _strdup(src->target_path);
    ERR_REGION_NULL_CHECK(target_path, err);

    source_path = _strdup(src->source_path);
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

    SAFE_FREE_HANDLER(old_target_sheet, cue_sheet_free);
    SAFE_FREE_HANDLER(old_source_sheet, cue_sheet_free);
    SAFE_FREE(old_target_path);
    SAFE_FREE(old_source_path);

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(source_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(target_sheet, cue_sheet_free);
  SAFE_FREE(source_path);
  SAFE_FREE(target_path);

  return err;
}

errno_t cue_traverse_record_uninit(cue_traverse_record_t* self) {
  SAFE_FREE_HANDLER(self->result, cue_sheet_parse_result_free);
  SAFE_FREE_HANDLER(self->target_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(self->source_sheet, cue_sheet_free);
  SAFE_FREE(self->source_path);
  SAFE_FREE(self->target_path);
  return 0;
}

errno_t cue_traverse_record_free(cue_traverse_record_t* self) {
  cue_traverse_record_uninit(self);
  SAFE_FREE(self);
  return 0;
}

