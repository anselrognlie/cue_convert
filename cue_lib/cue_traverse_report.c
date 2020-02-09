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
  cue_traverse_record_vector_t* skipped = 0;

  memset(self, 0, sizeof(*self));

  ERR_REGION_BEGIN() {

    transformed = cue_traverse_record_vector_alloc();
    ERR_REGION_NULL_CHECK(transformed, err);

    failed = cue_traverse_record_vector_alloc();
    ERR_REGION_NULL_CHECK(failed, err);

    skipped = cue_traverse_record_vector_alloc();
    ERR_REGION_NULL_CHECK(skipped, err);

    self->transformed_list = transformed;
    self->failed_list = failed;
    self->skipped_list = skipped;

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(skipped, cue_traverse_record_vector_free);
  SAFE_FREE_HANDLER(failed, cue_traverse_record_vector_free);
  SAFE_FREE_HANDLER(transformed, cue_traverse_record_vector_free);

  return err;
}

void cue_traverse_report_uninit(struct cue_traverse_report* self) {
  SAFE_FREE_HANDLER(self->skipped_list, cue_traverse_record_vector_free);
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
  cue_traverse_report_type_t report_type) {

  cue_traverse_record_t const *added = 0;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    if (report_type == EWC_CTR_TRANSFORMED) {
      added = self->transformed_list->push(self->transformed_list, record);
      ERR_REGION_NULL_CHECK(added, err);

      ++self->transformed_cue_count;
      ++self->found_cue_count;
    }
    else if (report_type == EWC_CTR_FAILED) {
      added = self->failed_list->push(self->failed_list, record);
      ERR_REGION_NULL_CHECK(added, err);

      ++self->failed_cue_count;
      ++self->found_cue_count;
    }
    else if (report_type == EWC_CTR_SKIPPED) {
      added = self->skipped_list->push(self->skipped_list, record);
      ERR_REGION_NULL_CHECK(added, err);

      ++self->skipped_cue_count;
      ++self->found_cue_count;
    }

    return added;
  } ERR_REGION_END()

  return NULL;
}
