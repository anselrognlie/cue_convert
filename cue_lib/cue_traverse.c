#include "cue_traverse.h"

#include <stdio.h>

#include "err_helpers.h"
#include "filesystem.h"
#include "string_vector.h"
#include "char_vector.h"
#include "string_helpers.h"
#include "mem_helpers.h"
#include "cue_traverse_report.h"
#include "cue_traverse_record.h"
#include "cue_parser.h"
#include "cue_file.h"
#include "cue_sheet_parse_result.h"
#include "cue_transform.h"
#include "path.h"
#include "file_line_writer.h"
#include "format_helpers.h"

static char * make_path_with_history(char_vector_t const* root_path, string_vector_t const* history);
static char * make_simple_path(char const *directory, char const*filename);
static short is_cue_file(char const *filename);
static errno_t convert_record(cue_traverse_record_t *record, cue_traverse_visitor_t *visitor);
static errno_t write_transformed_cue(cue_traverse_record_t* record);
static errno_t process_track_files(cue_traverse_record_t* record);
static errno_t convert_file(char const *src_path, char const *trg_path);

//
// cue traversal visitor
//

static short ctv_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  cue_traverse_visitor_t* self = (cue_traverse_visitor_t*)self_i->self;
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;
  string_vector_t* history = state->history;

  short keep_traversing = 1;
  char * dst_path = 0;
  char *src_path = 0;
  cue_traverse_record_t* record = 0;
  cue_traverse_record_t const *added = 0;
  short transformed = 0;
  cue_traverse_report_t *report = self->report;

  ERR_REGION_BEGIN() {

    if (is_cue_file(entry->get_name(entry))) {
      // found a cue

      // create a traverse record for this
      src_path = make_simple_path(
        directory->get_path(directory),
        entry->get_name(entry)
      );
      ERR_REGION_NULL_CHECK_CODE(src_path, keep_traversing, 0);

      dst_path = make_path_with_history(self->target_path, history);
      ERR_REGION_NULL_CHECK_CODE(dst_path, keep_traversing, 0);

      record = cue_traverse_record_alloc_with_paths(dst_path, src_path);
      ERR_REGION_NULL_CHECK_CODE(record, keep_traversing, 0);

      // don't need the paths any longer
      SAFE_FREE(dst_path);
      SAFE_FREE(src_path);

      // try to convert
      transformed = convert_record(record, self) == 0;

      // add the appropriate report category
      added = cue_traverse_report_add_record(report, record, transformed);
      ERR_REGION_NULL_CHECK_CODE(added, keep_traversing, 0);
    }

    return keep_traversing;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(record, cue_traverse_record_free);
  SAFE_FREE(dst_path);
  SAFE_FREE(src_path);

  return keep_traversing;
}

errno_t cue_traverse_visitor_init(cue_traverse_visitor_t* self,
  char const* target_path,
  char const* source_path,
  short report_only) {

  errno_t err = 0;

  char_vector_t* target_path_str = 0;
  char_vector_t* source_path_str = 0;
  cue_traverse_report_t *report = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));
    self->handler_i.self = self;
    self->handler_i.visit = ctv_visit;
    self->report_only = report_only;

    target_path_str = char_vector_alloc();
    ERR_REGION_NULL_CHECK(target_path_str, err);
    ERR_REGION_NULL_CHECK(target_path_str->set_str(target_path_str, target_path), err);

    source_path_str = char_vector_alloc();
    ERR_REGION_NULL_CHECK(source_path_str, err);
    ERR_REGION_NULL_CHECK(source_path_str->set_str(source_path_str, source_path), err);

    report = cue_traverse_report_alloc();
    ERR_REGION_NULL_CHECK(report, err);

    self->target_path = target_path_str;
    self->source_path = source_path_str;
    self->report = report;

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(report, cue_traverse_report_free);
  SAFE_FREE_HANDLER(source_path_str, char_vector_free);
  SAFE_FREE_HANDLER(target_path_str, char_vector_free);

  return err;
}

void cue_traverse_visitor_uninit(cue_traverse_visitor_t* self) {
  SAFE_FREE_HANDLER(self->report, cue_traverse_report_free);
  SAFE_FREE_HANDLER(self->source_path, char_vector_free);
  SAFE_FREE_HANDLER(self->target_path, char_vector_free);
}

static char * make_simple_path(char const* directory, char const* filename) {
  char const* parts[] = { directory, filename };
  return join_cstrs(parts, 2, k_path_separator);
}

static char * make_path_with_history(char_vector_t const* root_path, string_vector_t const* history) {
  char * path = 0;
  char const* root = 0;
  char * parallel_path = 0;

  ERR_REGION_BEGIN() {

    path = join_cstrs(
      history->get_buffer(history),
      history->get_length(history),
      k_path_separator);
    ERR_REGION_NULL_CHECK_CODE(path, parallel_path, NULL);

    root = root_path->get_str(root_path);
    ERR_REGION_NULL_CHECK_CODE(root, parallel_path, NULL);

    parallel_path = make_simple_path(root, path);
    ERR_REGION_NULL_CHECK_CODE(parallel_path, parallel_path, NULL);

    SAFE_FREE(path);

    return parallel_path;

  } ERR_REGION_END()

  SAFE_FREE(parallel_path);
  SAFE_FREE(path);

  return NULL;
}

static const char s_cue_suffix[] = ".cue";

static short is_cue_file(char const* filename) {
  return cstr_ends_with(filename, s_cue_suffix);
}

static errno_t convert_record(cue_traverse_record_t* record, cue_traverse_visitor_t* visitor) {
  errno_t err = 0;
  cue_sheet_t* src = 0;
  cue_sheet_t* converted = 0;
  char const* src_path = record->source_path;
  char const* trg_path = record->target_path;
  char *buf = 0;

  ERR_REGION_BEGIN() {
    // try to load the source cue
    src = cue_sheet_parse_filename(src_path, record->result);
    ERR_REGION_NULL_CHECK(src, err);

    cue_sheet_t* local_src = src;
    src = NULL;
    record->source_sheet = local_src;

    // try to convert it
    cue_transform_audio_options_t options;
    options.target_type = EWC_CAT_OGG;
    converted = cue_sheet_transform_audio(local_src, &options);
    ERR_REGION_NULL_CHECK(converted, err);

    cue_sheet_t* local_converted = converted;
    converted = NULL;
    record->target_sheet = local_converted;

    // if we are actually running, try to write the converted cue
    if (!visitor->report_only) {
      errno_t write_err;
      write_err = write_transformed_cue(record);

      // if there was an error creating the cue, log it here as a line 0 error
      if (write_err) {
        buf = msnprintf("Failed to create cue file: %s", trg_path);
        ERR_REGION_NULL_CHECK(buf, err);

        ERR_REGION_NULL_CHECK(cue_sheet_parse_result_add_error(record->result, 0, buf), err);
        SAFE_FREE(buf);
      }

      ERR_REGION_ERROR_CHECK(write_err, err);

      ERR_REGION_ERROR_CHECK(process_track_files(record), err);
    }

  } ERR_REGION_END()

  SAFE_FREE(buf);

  return err;
}

static errno_t write_transformed_cue(cue_traverse_record_t *record) {
  errno_t err = 0;
  char const *dir = 0;
  cue_sheet_t const *cue = record->target_sheet;
  char const *path = record->target_path;
  cue_sheet_parse_result_t *result = record->result;

  ERR_REGION_BEGIN() {

    dir = path_dir_part(path);
    ERR_REGION_NULL_CHECK(dir, err);

    // ensure that the target directory exists
    ERR_REGION_ERROR_CHECK(ensure_dir(dir), err);

    // create a writer for the desired file
    ERR_REGION_ERROR_CHECK(cue_sheet_write_filename(cue, path), err);
    
  } ERR_REGION_END()

  SAFE_FREE(dir);

  return err;
}

static errno_t process_track_files(cue_traverse_record_t* record) {

  // we can assume that the number of files in the source and target cues
  // are the same, since the target was derived from the source.
  // we iterate over both cues, comparing the types of corresponding
  // files.  If they are the same, just copy them.  If they differ
  // (target is OGG) then we convert during the copy.

  errno_t err = 0;
  cue_sheet_t const* src = record->source_sheet;
  cue_sheet_t const* trg = record->target_sheet;
  short num_files = src->num_files;
  char const* src_dir = 0;
  char const* trg_dir = 0;
  char const* src_path = 0;
  char const* trg_path = 0;

  ERR_REGION_BEGIN() {
    src_dir = path_dir_part(record->source_path);
    ERR_REGION_NULL_CHECK(src_dir, err);
    trg_dir = path_dir_part(record->target_path);
    ERR_REGION_NULL_CHECK(trg_dir, err);

    for (short i = 0; i < num_files; ++i) {
      cue_file_t const *src_file = src->file[i];
      cue_file_t const *trg_file = trg->file[i];

      src_path = join_dir_file_path(src_dir, src_file->filename);
      ERR_REGION_NULL_CHECK(src_path, err);

      trg_path = join_dir_file_path(trg_dir, trg_file->filename);
      ERR_REGION_NULL_CHECK(trg_path, err);

      if (src_file->type == trg_file->type) {
        ERR_REGION_ERROR_CHECK(copy_file(src_path, trg_path), err);
      }
      else {
        ERR_REGION_ERROR_CHECK(convert_file(src_path, trg_path), err);
      }

      SAFE_FREE(trg_path);
      SAFE_FREE(src_path);

    } ERR_REGION_ERROR_BUBBLE(err);

  } ERR_REGION_END()

  SAFE_FREE(src_path);
  SAFE_FREE(trg_path);
  SAFE_FREE(src_dir);
  SAFE_FREE(trg_dir);

  return err;
}

static errno_t convert_file(char const* src_path, char const* trg_path) {
  errno_t err = 0;

  return err;
}
