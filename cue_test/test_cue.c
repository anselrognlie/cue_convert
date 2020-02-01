#include "all_tests.h"

#include <string.h>

#include "cue_file.h"
#include "cue_parser.h"
#include "cue_transform.h"
#include "array_line_reader.h"
#include "array_line_writer.h"
#include "cue_traverse.h"
#include "directory_traversal.h"
#include "cue_traverse_report.h"
#include "cue_traverse_report_writer.h"
#include "cue_traverse_record.h"
#include "char_vector.h"

#include "test_helpers.h"
#include "err_helpers.h"

static const char s_cue_src_dir[] = "..\\test_data\\cue_dir";
static const char s_cue_trg_dir[] = "r:\\target_dir";

static char const *s_cue_sheet[] = {
"FILE \"track01.bin\" BINARY",
"  TRACK 01 AUDIO",
"    PREGAP 00:03:00",
"    INDEX 01 00:00:00",
"FILE \"track02.bin\" BINARY",
"  TRACK 02 MODE1/2352",
"    INDEX 01 00:03:00",
"FILE \"track03.ogg\" OGG",
"  TRACK 03 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track04.mp3\" MP3",
"  TRACK 04 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track05.wav\" WAV",
"  TRACK 05 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track06.bin\" BINARY",
"  TRACK 06 MODE1/2048",
"    INDEX 01 00:00:00",
};

static const int s_cue_sheet_num_lines = sizeof(s_cue_sheet) / sizeof(*s_cue_sheet);

static char const* s_transformed_sheet[] = {
"FILE \"track01.ogg\" OGG",
"  TRACK 01 AUDIO",
"    PREGAP 00:03:00",
"    INDEX 01 00:00:00",
"FILE \"track02.bin\" BINARY",
"  TRACK 02 MODE1/2352",
"    INDEX 01 00:03:00",
"FILE \"track03.ogg\" OGG",
"  TRACK 03 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track04.mp3\" MP3",
"  TRACK 04 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track05.ogg\" OGG",
"  TRACK 05 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track06.bin\" BINARY",
"  TRACK 06 MODE1/2048",
"    INDEX 01 00:00:00",
};

static const int s_transformed_sheet_num_lines = sizeof(s_transformed_sheet) / sizeof(*s_transformed_sheet);

#define GET_SIZE(arr) (arr),sizeof((arr))/sizeof(*(arr))

errno_t test_cue(void) {

  printf("Checking cue output... ");

  array_line_reader_t reader;
  array_line_reader_init_lines(&reader, GET_SIZE(s_cue_sheet));
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader);

  if (!sheet) {
    printf("FAILED!\n");
    return -1;
  }

  array_line_writer_t writer;
  array_line_writer_init(&writer);
  cue_sheet_write(sheet, &writer.line_writer);

  errno_t result = 0;
  if (compare_string_arrays(s_cue_sheet, s_cue_sheet_num_lines, writer.lines, writer.num_lines)) {
    printf("passed.\n");
  }
  else {
    printf("FAILED!\n");
    result = -1;
  }

  array_line_writer_uninit(&writer);
  cue_sheet_free(sheet);

  return result;
}

errno_t test_cue_copy(void) {

  printf("Checking cue copy... ");

  array_line_reader_t reader;
  array_line_reader_init_lines(&reader, GET_SIZE(s_cue_sheet));
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader);

  if (!sheet) {
    printf("FAILED!\n");
    return -1;
  }

  cue_sheet_t *copy = cue_sheet_alloc_copy(sheet);
  if (!copy) {
    printf("FAILED!\n");
    cue_sheet_free(sheet);
    return -1;
  }

  array_line_writer_t writer;
  array_line_writer_init(&writer);
  cue_sheet_write(copy, &writer.line_writer);

  errno_t result = 0;
  if (compare_string_arrays(s_cue_sheet, s_cue_sheet_num_lines, writer.lines, writer.num_lines)) {
    printf("passed.\n");
  }
  else {
    printf("FAILED!\n");
    result = -1;
  }

  array_line_writer_uninit(&writer);
  cue_sheet_free(copy);
  cue_sheet_free(sheet);

  return result;
}

errno_t test_cue_transform(void) {

  printf("Checking cue transform... ");

  array_line_reader_t reader;
  array_line_reader_init_lines(&reader, GET_SIZE(s_cue_sheet));
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader);

  if (!sheet) {
    printf("FAILED!\n");
    return -1;
  }

  cue_transform_audio_options_t options;
  options.target_type = EWC_CAT_OGG;
  cue_sheet_t* transformed = cue_sheet_transform_audio(sheet, &options);
  if (!transformed) {
    printf("FAILED!\n");
    cue_sheet_free(sheet);
    return -1;
  }

  array_line_writer_t writer;
  array_line_writer_init(&writer);
  cue_sheet_write(transformed, &writer.line_writer);

  errno_t result = 0;
  if (compare_string_arrays(s_transformed_sheet, s_transformed_sheet_num_lines, writer.lines, writer.num_lines)) {
    printf("passed.\n");
  }
  else {
    printf("FAILED!\n");
    result = -1;
  }

  //dump_string_array(writer.lines, writer.num_lines);

  array_line_writer_uninit(&writer);
  cue_sheet_free(transformed);
  cue_sheet_free(sheet);

  return result;
}

static char const *s_traverse_transformed[] = {
  "..\\test_data\\cue_dir\\a\\a1game\\a1game.cue",
  "..\\test_data\\cue_dir\\b\\b2game\\b2game.cue",
  0,
};

static char const* s_traverse_failed[] = { 0 };

static char const** s_traverse_results[] = {
  s_traverse_transformed,
  s_traverse_failed,
};

static errno_t compare_report(cue_traverse_report_t* report, char const*** cmp) {
  errno_t err = 0;
  char const** traverse_transformed = cmp[0];
  char const** tt_len = traverse_transformed;
  char const** traverse_failed = cmp[1];
  char const** tf_len = traverse_failed;
  int report_transformed = report->transformed_cue_count;
  int report_failed = report->failed_cue_count;
  int report_total = report->found_cue_count;

  while (*tt_len++);
  while (*tf_len++);
  int num_transformed = tt_len - traverse_transformed - 1;
  int num_failed = tf_len - traverse_failed - 1;

  ERR_REGION_BEGIN() {
    ERR_REGION_CMP_CHECK(report_total != report_transformed + report_failed, err);
    ERR_REGION_CMP_CHECK(report_transformed != num_transformed, err);
    ERR_REGION_CMP_CHECK(report_failed != report_failed, err);

    short match = 0;
    for (int i = 0; i < num_transformed; ++i) {
      cue_traverse_record_t const* record = cue_traverse_record_vector_get(report->transformed_list, i);
      char const *rec_src = char_vector_get_str(record->source_path);
      match = strcmp(rec_src, traverse_transformed[i]) == 0;
      if (!match) break;
    }
    ERR_REGION_CMP_CHECK(!match, err);

    for (int i = 0; i < num_failed; ++i) {
      cue_traverse_record_t const* record = cue_traverse_record_vector_get(report->failed_list, i);
      char const* rec_src = char_vector_get_str(record->source_path);
      match = strcmp(rec_src, traverse_failed[i]) == 0;
      if (!match) break;
    }
    ERR_REGION_CMP_CHECK(!match, err);

  } ERR_REGION_END()

  return err;
}

errno_t test_cue_traverse(void) {
  cue_traverse_visitor_t visitor;
  cue_traverse_report_writer_t writer;
  array_line_writer_t line_writer;
  errno_t err = 0;
  short passed = 0;

  printf("Checking cue traversal... ");

  ERR_REGION_BEGIN() {
    array_line_writer_init(&line_writer);
    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_init_params(
      &writer,
      &line_writer.line_writer), err);

    ERR_REGION_ERROR_CHECK(cue_traverse_visitor_init(
      &visitor, 
      s_cue_trg_dir, 
      s_cue_src_dir, 
      1, 
      0), err);

    traverse_dir_path(s_cue_src_dir, &visitor.handler_i);

    cue_traverse_report_t *report = visitor.report;

    ERR_REGION_ERROR_CHECK(compare_report(report, s_traverse_results), err);

    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_write(&writer, report), err);

    passed = 1;

  } ERR_REGION_END()

  printf("%s\n", passed ? "passed." : "FAILED!");

  int n = line_writer.num_lines;
  for (int i = 0; i < n; ++i) {
    printf("%s\n", line_writer.lines[i]);
  }

  cue_traverse_report_writer_uninit(&writer);
  cue_traverse_visitor_uninit(&visitor);
  array_line_writer_uninit(&line_writer);

  return err;
}
