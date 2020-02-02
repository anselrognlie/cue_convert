#include "cue_traverse_record.h"

#include <stdlib.h>

#include "err_helpers.h"
#include "char_vector.h"
#include "cue_file.h"
#include "mem_helpers.h"

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

    self->target_path = char_vector_alloc();
    ERR_REGION_NULL_CHECK(self->target_path, err);
    self->source_path = char_vector_alloc();
    ERR_REGION_NULL_CHECK(self->source_path, err);

    return err;
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(self->source_path, char_vector_free);
  SAFE_FREE_HANDLER(self->target_path, char_vector_free);

  return err;
}

errno_t cue_traverse_record_init_with_paths(cue_traverse_record_t* self, char const* target_path, char const* source_path) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    ERR_REGION_ERROR_CHECK(cue_traverse_record_init(self), err);

    ERR_REGION_BEGIN() {
      ERR_REGION_NULL_CHECK(self->source_path->set_str(self->source_path, source_path), err);
      ERR_REGION_NULL_CHECK(self->target_path->set_str(self->target_path, target_path), err);
      return err;
    } ERR_REGION_END()

    // init was fine, but internal sets failed, so make sure to uninit
    cue_traverse_record_uninit(self);
  } ERR_REGION_END()

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

    SAFE_FREE_HANDLER(old_target_sheet, cue_sheet_free);
    SAFE_FREE_HANDLER(old_source_sheet, cue_sheet_free);
    SAFE_FREE_HANDLER(old_target_path, char_vector_free);
    SAFE_FREE_HANDLER(old_source_path, char_vector_free);

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(source_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(target_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(source_path, char_vector_free);
  SAFE_FREE_HANDLER(target_path, char_vector_free);

  return err;
}

errno_t cue_traverse_record_uninit(cue_traverse_record_t* self) {
  SAFE_FREE_HANDLER(self->target_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(self->source_sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(self->source_path, char_vector_free);
  SAFE_FREE_HANDLER(self->target_path, char_vector_free);
  return 0;
}

errno_t cue_traverse_record_free(cue_traverse_record_t* self) {
  cue_traverse_record_uninit(self);
  SAFE_FREE(self);
  return 0;
}

