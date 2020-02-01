#include "cue_traverse_report_writer.h"

#include <stdio.h>

#include "cue_traverse_report.h"
#include "cue_traverse_record.h"
#include "char_vector.h"
#include "line_writer.h"
#include "err_helpers.h"
#include "mem_helpers.h"
#include "format_helpers.h"

struct cue_traverse_report_writer* cue_traverse_report_writer_alloc_params(struct line_writer* writer) {
  cue_traverse_report_writer_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t err = cue_traverse_report_writer_init_params(self, writer);
  if (! err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_traverse_report_writer_init_params(struct cue_traverse_report_writer* self, struct line_writer* writer) {
  self->writer = writer;  // non-owned
  return 0;
}

void cue_traverse_report_writer_uninit(struct cue_traverse_report_writer* self) {
  self->writer = NULL;
}

void cue_traverse_report_writer_free(struct cue_traverse_report_writer* self) {
  cue_traverse_report_writer_uninit(self);
  SAFE_FREE(self);
}

errno_t cue_traverse_report_writer_write(
  struct cue_traverse_report_writer* self, 
  struct cue_traverse_report* report) {

  line_writer_i *writer = self->writer;
  errno_t err = 0;
  size_t written = 0;

  ERR_REGION_BEGIN() {

    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s", "CONVERSION REPORT"), err);
    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s%d", "Found cue files: ", report->found_cue_count), err);
    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s", "Transformed files:"), err);

    for (int i = 0; i < report->transformed_cue_count; ++i) {
      cue_traverse_record_t const* record = cue_traverse_record_vector_get(report->transformed_list, i);
      char const* src_path = char_vector_get_str(record->source_path);
      char const* dst_path = char_vector_get_str(record->target_path);
      written = line_writer_write_fmt(writer, "%s%s%s%s", "  ", src_path, " -> ", dst_path);
      if (! written) break;
    }
    ERR_REGION_CMP_CHECK(!written, err);

    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s%d", "Transformed total: ", report->transformed_cue_count), err);
    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s", "Failed files:"), err);

    for (int i = 0; i < report->failed_cue_count; ++i) {
      cue_traverse_record_t const* record = cue_traverse_record_vector_get(report->failed_list, i);
      char const* src_path = char_vector_get_str(record->source_path);
      char const* dst_path = char_vector_get_str(record->target_path);
      written = line_writer_write_fmt(writer, "%s%s%s%s", "  ", src_path, " -> ", dst_path);
      if (!written) break;
    }
    ERR_REGION_CMP_CHECK(!written, err);

    ERR_REGION_CMP_CHECK(!line_writer_write_fmt(writer, "%s%d", "Failed total: ", report->failed_cue_count), err);

  } ERR_REGION_END()

  return err;
}
