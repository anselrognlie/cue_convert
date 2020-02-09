#include "cue_status_info.h"

#include <string.h>
#include <stdlib.h>

#include "mem_helpers.h"
#include "err_helpers.h"

static void* acquire(void const* instance);
static void release(void* instance);

struct object_vector_params cue_status_info_vector_ops = {
  acquire,
  release,
};

static void release(void* instance) {
  SAFE_FREE_HANDLER(instance, cue_status_info_free);
}

static void* acquire(void const* instance) {
  return (void *)instance;
}

IMPLEMENT_OBJECT_VECTOR(cue_status_info_vector, cue_status_info_t);

static errno_t set_detail(cue_status_info_t *self, char const *detail);

struct cue_status_info* cue_status_info_alloc_parse_error(
  size_t line_num,
  char const* line) {
  struct cue_status_info* self = malloc(sizeof(*self));
  if (!self) return NULL;

  errno_t err = cue_status_info_init_parse_error(self, line_num, line);
  if (!err) return self;

  SAFE_FREE(self);
  return NULL;
}

struct cue_status_info* cue_status_info_alloc_error(
  char const* msg) {
  struct cue_status_info* self = malloc(sizeof(*self));
  if (!self) return NULL;

  errno_t err = cue_status_info_init_error(self, msg);
  if (!err) return self;

  SAFE_FREE(self);
  return NULL;
}

struct cue_status_info* cue_status_info_alloc_status(
  char const* msg) {
  struct cue_status_info* self = malloc(sizeof(*self));
  if (!self) return NULL;

  errno_t err = cue_status_info_init_status(self, msg);
  if (!err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_status_info_init_parse_error(struct cue_status_info* self,
  size_t line_num,
  char const* line) {

  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    ERR_REGION_ERROR_CHECK(set_detail(self, line), err);

    self->line_num = line_num;
    self->type = EWC_CST_PARSE_ERROR;

  } ERR_REGION_END()

  return err;
}

errno_t cue_status_info_init_error(struct cue_status_info* self, char const* msg) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    ERR_REGION_ERROR_CHECK(set_detail(self, msg), err);

    self->type = EWC_CST_ERROR;

  } ERR_REGION_END()

  return err;
}

errno_t cue_status_info_init_status(struct cue_status_info* self, char const* msg) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    ERR_REGION_ERROR_CHECK(set_detail(self, msg), err);

    self->type = EWC_CST_STATUS;

  } ERR_REGION_END()

  return err;
}

void cue_status_info_uninit(struct cue_status_info* self) {
  SAFE_FREE(self->detail);
}

void cue_status_info_free(struct cue_status_info* self) {
  cue_status_info_uninit(self);
  SAFE_FREE(self);
}


struct cue_sheet_process_result* cue_sheet_process_result_alloc() {
  struct cue_sheet_process_result* self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t err = cue_sheet_process_result_init(self);
  if (! err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_sheet_process_result_init(struct cue_sheet_process_result* self) {
  memset(self, 0, sizeof(*self));

  cue_status_info_vector_t *vector = cue_status_info_vector_alloc();
  if (! vector) return -1;

  self->info_list = vector;

  return 0;
}

void cue_sheet_process_result_uninit(struct cue_sheet_process_result* self) {
  self->info_list->free(self->info_list);
}

void cue_sheet_process_result_free(struct cue_sheet_process_result* self) {
  cue_sheet_process_result_uninit(self);
  SAFE_FREE(self);
}

struct cue_status_info const* cue_sheet_process_result_add_parse_error(
  struct cue_sheet_process_result* self,
  size_t line_num,
  char const* line) {

  errno_t err = 0;
  cue_status_info_t* error = 0;
  cue_status_info_t const* added = 0;

  ERR_REGION_BEGIN() {

    error = cue_status_info_alloc_parse_error(line_num, line);
    ERR_REGION_NULL_CHECK(error, err);

    added = self->info_list->push(self->info_list, error);
    ERR_REGION_NULL_CHECK(added, err);

    self->has_errors = 1;

    return added;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(error, cue_status_info_free);

  return NULL;

}

struct cue_status_info const* cue_sheet_process_result_add_error(
  struct cue_sheet_process_result* self,
  char const* msg) {

  errno_t err = 0;
  cue_status_info_t* info = 0;
  cue_status_info_t const* added = 0;

  ERR_REGION_BEGIN() {

    info = cue_status_info_alloc_error(msg);
    ERR_REGION_NULL_CHECK(info, err);

    added = self->info_list->push(self->info_list, info);
    ERR_REGION_NULL_CHECK(added, err);

    self->has_errors = 1;

    return added;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(info, cue_status_info_free);

  return NULL;

}

struct cue_status_info const* cue_sheet_process_result_add_status(
  struct cue_sheet_process_result* self,
  char const* msg) {

  errno_t err = 0;
  cue_status_info_t* info = 0;
  cue_status_info_t const* added = 0;

  ERR_REGION_BEGIN() {

    info = cue_status_info_alloc_status(msg);
    ERR_REGION_NULL_CHECK(info, err);

    added = self->info_list->push(self->info_list, info);
    ERR_REGION_NULL_CHECK(added, err);

    self->has_status = 1;

    return added;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(info, cue_status_info_free);

  return NULL;

}

static errno_t set_detail(cue_status_info_t* self, char const* detail) {
  char const *buf = _strdup(detail);
  if (! buf) return -1;

  char const *old_buf = self->detail;
  SAFE_FREE(old_buf);

  self->detail = buf;
  return 0;
}
