#include "cue_traverse_report.h"

#include <stdlib.h>
#include <string.h>

#include "mem_helpers.h"
#include "err_helpers.h"
#include "cue_traverse_record.h"

struct cue_traverse_report* cue_traverse_report_alloc() {
  cue_traverse_report_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t err = cue_traverse_report_init(self);
  if (! err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_traverse_report_init(struct cue_traverse_report* self) {
  errno_t err = 0;
  cue_traverse_record_vector_t* transformed = 0;
  cue_traverse_record_vector_t* failed = 0;

  memset(self, 0, sizeof(*self));

  ERR_REGION_BEGIN() {

    transformed = cue_traverse_record_vector_alloc();
    ERR_REGION_NULL_CHECK(transformed, err);

    failed = cue_traverse_record_vector_alloc();
    ERR_REGION_NULL_CHECK(failed, err);

    self->transformed_list = transformed;
    self->failed_list = failed;

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(failed, cue_traverse_record_vector_free);
  SAFE_FREE_HANDLER(transformed, cue_traverse_record_vector_free);

  return err;
}

void cue_traverse_report_uninit(struct cue_traverse_report* self) {
  SAFE_FREE_HANDLER(self->failed_list, cue_traverse_record_vector_free);
  SAFE_FREE_HANDLER(self->transformed_list, cue_traverse_record_vector_free);
}

void cue_traverse_report_free(struct cue_traverse_report* self) {
  cue_traverse_report_uninit(self);
  SAFE_FREE(self);
}

struct cue_traverse_record const* cue_traverse_report_add_record(
  struct cue_traverse_report* self,
  struct cue_traverse_record* record,
  short transformed) {

  cue_traverse_record_t const *added = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    if (transformed) {
      added = self->transformed_list->push(self->transformed_list, record);
      ERR_REGION_NULL_CHECK(added, err);

      ++self->transformed_cue_count;
      ++self->found_cue_count;
    }
    else {
      added = self->failed_list->push(self->failed_list, record);
      ERR_REGION_NULL_CHECK(added, err);

      ++self->failed_cue_count;
      ++self->found_cue_count;
    }

    return added;
  } ERR_REGION_END()

  return NULL;
}
