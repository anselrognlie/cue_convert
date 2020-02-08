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
#include "cue_status_info.h"
#include "cue_transform.h"
#include "path.h"
#include "file_line_writer.h"
#include "format_helpers.h"

#include "oggenc.h"

static short is_cue_file(char const *filename);
static errno_t convert_record(cue_traverse_visitor_t* self, cue_traverse_record_t *record, short reort_only);
static errno_t write_transformed_cue(cue_traverse_record_t const* record);
static errno_t process_track_files(cue_traverse_visitor_t* self, cue_traverse_record_t const* record);
static errno_t convert_file(
  cue_traverse_visitor_t* self,
  char const* src_path, cue_file_type_t src_type,
  char const* trg_path, cue_file_type_t trg_type);
static errno_t convert_to_ogg(
  cue_traverse_visitor_t* self,
  char const* src_path, cue_file_type_t src_type,
  char const* trg_path);

//
// cue traversal visitor
//

static short ctv_visit(parallel_visitor_t *self_t, parallel_visitor_state_t const* state) {

  cue_traverse_visitor_t* self = (cue_traverse_visitor_t*)self_t->self;
  file_handle_i const* directory = state->base_state->directory;
  directory_entry_i const* entry = state->base_state->entry;

  short keep_traversing = 1;
  char const *dst_path = 0;
  char const *src_path = 0;
  cue_traverse_record_t* record = 0;
  cue_traverse_record_t const *added = 0;
  short transformed = 0;
  cue_traverse_report_t *report = self->report;
  line_writer_i *writer = 0;

  ERR_REGION_BEGIN() {

    if (is_cue_file(entry->get_name(entry))) {
      // found a cue

      writer = self->writer;

      // create a traverse record for this
      src_path = join_dir_file_path(
        directory->get_path(directory),
        entry->get_name(entry)
      );
      ERR_REGION_NULL_CHECK_CODE(src_path, keep_traversing, 0);

      ERR_REGION_CMP_CHECK_CODE(
        ! line_writer_write_fmt(writer, "%s%s", "Processing ", src_path), 
        keep_traversing, 0);

      dst_path = state->parallel_path;

      // if the destination already exists, and we are not in overwrite mode,
      // just terminate this visit

      if (file_exists(dst_path)) {
        if (!self->overwrite) {
          ERR_REGION_CMP_CHECK_CODE(
            !line_writer_write_fmt(writer, "%s%s", "  ", "skipping, already exists."),
            keep_traversing, 0);
          ERR_REGION_EXIT();
        }
        else {
          ERR_REGION_CMP_CHECK_CODE(
            !line_writer_write_fmt(writer, "%s%s", "  ", "overwriting... "),
            keep_traversing, 0);
        }
      }

      record = cue_traverse_record_alloc_with_paths(dst_path, src_path);
      ERR_REGION_NULL_CHECK_CODE(record, keep_traversing, 0);

      // don't need the paths any longer
      SAFE_FREE(src_path);

      // try to convert
      transformed = convert_record(self, record, self->report_only) == 0;

      ERR_REGION_CMP_CHECK_CODE(
        !line_writer_write_fmt(writer, "%s%s", "  ", transformed ? "Success." : "FAILED!"), 
        keep_traversing, 0);

      // add the appropriate report category
      added = cue_traverse_report_add_record(report, record, transformed);
      ERR_REGION_NULL_CHECK_CODE(added, keep_traversing, 0);
    }

    return keep_traversing;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(record, cue_traverse_record_free);
  SAFE_FREE(src_path);

  return keep_traversing;
}

errno_t cue_traverse_visitor_init(cue_traverse_visitor_t* self,
  cue_traverse_visitor_opts_t const *opts) {

  errno_t err = 0;

  char const* source_path_str = 0;
  cue_traverse_report_t *report = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));
    parallel_visitor_init(&self->pv_t, opts->target_path);
    self->pv_t.self = self;
    self->pv_t.visit = ctv_visit;
    self->report_only = opts->report_only;
    self->overwrite = opts->overwrite;
    self->quality = opts->quality;
    self->writer = opts->writer;

    ERR_REGION_NULL_CHECK(source_path_str = _strdup(opts->source_path), err);

    report = cue_traverse_report_alloc();
    ERR_REGION_NULL_CHECK(report, err);

    self->source_path = source_path_str;
    self->report = report;

    return err;

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(report, cue_traverse_report_free);
  SAFE_FREE(source_path_str);

  return err;
}

void cue_traverse_visitor_uninit(cue_traverse_visitor_t* self) {
  SAFE_FREE_HANDLER(self->report, cue_traverse_report_free);
  SAFE_FREE(self->source_path);
  parallel_visitor_uninit(&self->pv_t);
}

struct cue_traverse_report *cue_traverse_visitor_detach_report(cue_traverse_visitor_t* self) {
  struct cue_traverse_report* report = self->report;
  self->report = NULL;
  return report;
}

static const char s_cue_suffix[] = ".cue";

static short is_cue_file(char const* filename) {
  return cstr_ends_with(filename, s_cue_suffix);
}

static errno_t convert_record(cue_traverse_visitor_t* self, cue_traverse_record_t * record, short report_only) {
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
    if (!report_only) {
      errno_t write_err;
      write_err = write_transformed_cue(record);

      // if there was an error creating the cue, log it here as a line 0 error
      if (write_err) {
        buf = msnprintf("Failed to create cue file: %s", trg_path);
        ERR_REGION_NULL_CHECK(buf, err);

        ERR_REGION_NULL_CHECK(cue_sheet_process_result_add_error(record->result, 0, buf), err);
        SAFE_FREE(buf);
      }

      ERR_REGION_ERROR_CHECK(write_err, err);

      ERR_REGION_ERROR_CHECK(process_track_files(self, record), err);
    }

  } ERR_REGION_END()

  SAFE_FREE(buf);

  return err;
}

static errno_t write_transformed_cue(cue_traverse_record_t const *record) {
  errno_t err = 0;
  char const *dir = 0;
  cue_sheet_t const *cue = record->target_sheet;
  char const *path = record->target_path;
  cue_sheet_process_result_t *result = record->result;

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

static errno_t process_track_files(cue_traverse_visitor_t* self, cue_traverse_record_t const * record) {

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
        ERR_REGION_ERROR_CHECK(convert_file(self, src_path, src_file->type, trg_path, trg_file->type), err);
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

static errno_t convert_file(
  cue_traverse_visitor_t* self,
  char const* src_path, cue_file_type_t src_type,
  char const* trg_path, cue_file_type_t trg_type) {

  errno_t err = 0;

  if (trg_type == EWC_CFT_OGG) {
    err = convert_to_ogg(self, src_path, src_type, trg_path);
  }
  else
  {
    // unknown type
    err = -1;
  }

  return err;
}

#define FLOAT_BUF_LEN (10)

static errno_t convert_to_ogg(
  cue_traverse_visitor_t* self,
  char const *src_path, cue_file_type_t src_type, 
  char const* trg_path) {

  errno_t err = 0;
  string_vector_t *argv = 0;

  char buf[FLOAT_BUF_LEN];

  snprintf(buf, FLOAT_BUF_LEN, "%.2g", self->quality);

  ERR_REGION_BEGIN() {
    ERR_REGION_NULL_CHECK(argv = string_vector_alloc(), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "oggenc"), err);
    
    // configure the arguments for oggenc
    switch (src_type) {
      case EWC_CFT_BINARY: {
        ERR_REGION_NULL_CHECK(argv->push(argv, "-Q"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "--utf8"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "-r"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "-q"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, buf), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "-o"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, trg_path), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, src_path), err);
      }
      break;

      case EWC_CFT_WAV: {
        ERR_REGION_NULL_CHECK(argv->push(argv, "-Q"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "--utf8"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "-q"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, buf), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, "-o"), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, trg_path), err);
        ERR_REGION_NULL_CHECK(argv->push(argv, src_path), err);
      }
      break;

      default:
        // we don't know how to do this
        err = -1;
    } ERR_REGION_ERROR_BUBBLE(err);

    // invoke oggenc
    ERR_REGION_ERROR_CHECK(encode_with_arguments(argv->get_length(argv), argv->get_buffer(argv)), err);

  } ERR_REGION_END ()

  SAFE_FREE_HANDLER(argv, string_vector_free);

  return err;
}
