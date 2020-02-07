#include "all_tests.h"

#include <string.h>

#include "cue_file.h"
#include "cue_parser.h"
#include "cue_sheet_parse_result.h"
#include "cue_transform.h"
#include "array_line_reader.h"
#include "array_line_writer.h"
#include "null_line_writer.h"
#include "cue_traverse.h"
#include "directory_traversal.h"
#include "cue_traverse_report.h"
#include "cue_traverse_report_writer.h"
#include "cue_traverse_record.h"
#include "char_vector.h"
#include "filesystem.h"
#include "cue_options.h"
#include "string_vector.h"
#include "cue_convert.h"

#include "test_helpers.h"
#include "err_helpers.h"
#include "test_visitors.h"

static const char s_cue_src_dir[] = "..\\test_data\\cue_dir";
static const char s_cue_trg_dir[] = "..\\test_data\\new_cue_dir";

static char const* s_cue_sheet[] = {
"FILE \"track01.bin\" BINARY",
"  TRACK 01 AUDIO",
"    PREGAP 00:03:00",
"    INDEX 01 00:00:00",
"FILE track02.bin BINARY",
"  TRACK 02 MODE1/2352",
"    INDEX 01 00:03:00",
"FILE \"track03.ogg\" OGG",
"  TRACK 03 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track 04.mp3\" MP3",
"  TRACK 04 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track05.wav\" WAV",
"  TRACK 05 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track06.bin\" BINARY",
"  TRACK 06 MODE1/2048",
"    INDEX 01 00:00:00",
};

static char const* s_canonical_sheet[] = {
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
"FILE \"track 04.mp3\" MP3",
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
static const int s_canonical_sheet_num_lines = sizeof(s_canonical_sheet) / sizeof(*s_canonical_sheet);

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
"FILE \"track 04.mp3\" MP3",
"  TRACK 04 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track05.ogg\" OGG",
"  TRACK 05 AUDIO",
"    INDEX 01 00:00:00",
"FILE \"track06.bin\" BINARY",
"  TRACK 06 MODE1/2048",
"    INDEX 01 00:00:00",
};

static char const* s_error_sheet[] = {
"    PREGAP 00:03:00",  // 1
"    INDEX 01 00:00:00",  // 2
"  TRACK 01 AUDIO",  // 3
"FILE \"track01.ogg\" OGG",  // 4
"    PREGAP 00:03:00",  // 5
"    INDEX 01 00:00:00",  // 6
"  TRACK 01 BADTRACKTYPE",  // 7
"  TRACK BADINDEX AUDIO",  // 8
"  TRACK 01 AUDIO",  // 9
"    PREGAP 00:BADTIME:00",  // 10
"    INDEX 01 00:00:BADTIME",  // 11
"    INDEX DABINDEX 00:00:00",  // 12
"FILE \"track01.ogg\" BADFILETYPE",  // 13
"FILE \"BADNAME OGG",  // 14
};

typedef struct {
  size_t line_num;
  char const *line;
} error_test_record_t;

static error_test_record_t s_error_test_result[] = {
  { 1, "    PREGAP 00:03:00" },  // 1
  { 2, "    INDEX 01 00:00:00" },  // 2
  { 3, "  TRACK 01 AUDIO" },  // 3
  { 5, "    PREGAP 00:03:00" },  // 5
  { 6, "    INDEX 01 00:00:00" },  // 6
  { 7, "  TRACK 01 BADTRACKTYPE" },  // 7
  { 8, "  TRACK BADINDEX AUDIO" },  // 8
  { 10, "    PREGAP 00:BADTIME:00" },  // 10
  { 11, "    INDEX 01 00:00:BADTIME" },  // 11
  { 12, "    INDEX DABINDEX 00:00:00" },  // 12
  { 13, "FILE \"track01.ogg\" BADFILETYPE" },  // 13
  { 14, "FILE \"BADNAME OGG" },  // 14
};

static const int s_transformed_sheet_num_lines = sizeof(s_transformed_sheet) / sizeof(*s_transformed_sheet);
static const int s_error_sheet_num_lines = sizeof(s_error_test_result) / sizeof(*s_error_test_result);

#define GET_SIZE(arr) (arr),sizeof((arr))/sizeof(*(arr))

errno_t test_cue(void) {

  printf("Checking cue output... ");

  array_line_reader_t reader;
  array_line_reader_init_lines(&reader, GET_SIZE(s_cue_sheet));
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader, NULL);

  if (!sheet) {
    printf("FAILED!\n");
    return -1;
  }

  array_line_writer_t writer;
  array_line_writer_init(&writer);
  cue_sheet_write(sheet, &writer.line_writer);

  errno_t result = 0;
  if (compare_string_arrays(s_canonical_sheet, s_canonical_sheet_num_lines, writer.lines, writer.num_lines)) {
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
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader, NULL);

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
  if (compare_string_arrays(s_canonical_sheet, s_canonical_sheet_num_lines, writer.lines, writer.num_lines)) {
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
  cue_sheet_t* sheet = cue_sheet_parse(&reader.line_reader, NULL);

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

static short compare_result_arrays(
  cue_sheet_parse_result_t const* result,
  error_test_record_t const *output,
  size_t error_len) {

  if (error_len > 0 && ! result->has_errors) return 0;

  cue_sheet_parse_error_vector_t *errors = result->errors;
  size_t result_len = errors->get_length(errors);
  if (error_len != result_len) return 0;

  for (size_t i = 0; i < error_len; ++i) {
    error_test_record_t err_rec = output[i];
    cue_sheet_parse_error_t const *res_rec = errors->get(errors, i);

    if (err_rec.line_num != res_rec->line_num) return 0;
    if (strcmp(err_rec.line, res_rec->line) != 0) return 0;
  }

  return 1;
}

errno_t test_cue_errors(void) {
  errno_t err = 0;
  cue_sheet_parse_result_t *result = 0;
  array_line_reader_t reader;
  cue_sheet_t* sheet = 0;

  ERR_REGION_BEGIN() {

    printf("Checking cue errors... ");

    result = cue_sheet_parse_result_alloc();
    ERR_REGION_NULL_CHECK(result, err);

    array_line_reader_init_lines(&reader, GET_SIZE(s_error_sheet));
    sheet = cue_sheet_parse(&reader.line_reader, result);
    ERR_REGION_CMP_CHECK(sheet, err);

    ERR_REGION_CMP_CHECK(! compare_result_arrays(result, s_error_test_result, s_error_sheet_num_lines), err);

  } ERR_REGION_END()

  printf("%s\n", err ? "FAILED!" : "passed.");

  SAFE_FREE_HANDLER(sheet, cue_sheet_free);
  SAFE_FREE_HANDLER(result, cue_sheet_parse_result_free);

  return err;
}

typedef struct {
  char const *src;
  char const *dst;
} conversion_rec_t;

static conversion_rec_t s_traverse_transformed[] = {
  { "..\\test_data\\cue_dir\\a\\a1game\\a1game.cue", "..\\test_data\\new_cue_dir\\a\\a1game\\a1game.cue" },
  { "..\\test_data\\cue_dir\\b\\b2game\\b2game.cue", "..\\test_data\\new_cue_dir\\b\\b2game\\b2game.cue" },
  0,
};

static conversion_rec_t s_traverse_failed[] = { 0 };

typedef struct {
  conversion_rec_t const* transformed;
  conversion_rec_t const* failed;
} conversion_recs_t;

static conversion_recs_t s_traverse_results = {
  (conversion_rec_t const*)&s_traverse_transformed,
  (conversion_rec_t const*)&s_traverse_failed,
};

static const dir_entry_fields_t s_test_traverse_result[] = {
  {"a", 1},
  {"a1game", 1},
  {"a1game.cue", 0},
  {"track01.ogg", 0},
  {"track02.bin", 0},
  {"track03.ogg", 0},
  {"track04.ogg", 0},
  {"track05.mp3", 0},
  {"b", 1},
  {"b2game", 1},
  {"b2game.cue", 0},
  {"track01.ogg", 0},
  {"track02.bin", 0},
  {"track03.ogg", 0},
  {"track04.ogg", 0},
  {"track05.mp3", 0},
};

static const size_t s_test_traverse_result_len =
sizeof(s_test_traverse_result) / sizeof(*s_test_traverse_result);

static short compare_record_lists(
  cue_traverse_record_vector_t* report_recs,
  conversion_rec_t const* test_recs, size_t len) {

  short match = 1;
  for (size_t i = 0; i < len; ++i) {
    cue_traverse_record_t const* record = report_recs->get(report_recs, i);

    char const *rec, *test;
    rec = record->source_path;
    test = test_recs[i].src;
    if (!test) { match = 0; break; }
    match = strcmp(rec, test) == 0;
    if (!match) break;

    rec = record->target_path;
    test = test_recs[i].dst;
    if (!test) { match = 0; break; }
    match = strcmp(rec, test) == 0;
    if (!match) break;
  }

  return match;
}

static errno_t compare_report(cue_traverse_report_t* report, conversion_recs_t const *cmp) {
  errno_t err = 0;
  conversion_rec_t const* traverse_transformed = cmp->transformed;
  conversion_rec_t const* tt_len = traverse_transformed;
  conversion_rec_t const* traverse_failed = cmp->failed;
  conversion_rec_t const* tf_len = traverse_failed;
  int report_transformed = report->transformed_cue_count;
  int report_failed = report->failed_cue_count;
  int report_total = report->found_cue_count;

  while ((*tt_len++).src);
  while ((*tf_len++).src);
  int num_transformed = tt_len - traverse_transformed - 1;
  int num_failed = tf_len - traverse_failed - 1;

  ERR_REGION_BEGIN() {
    ERR_REGION_CMP_CHECK(report_total != report_transformed + report_failed, err);
    ERR_REGION_CMP_CHECK(report_transformed != num_transformed, err);
    ERR_REGION_CMP_CHECK(report_failed != report_failed, err);

    ERR_REGION_CMP_CHECK(!compare_record_lists(report->transformed_list, traverse_transformed, num_transformed), err);
    ERR_REGION_CMP_CHECK(!compare_record_lists(report->failed_list, traverse_failed, num_failed), err);

  } ERR_REGION_END()

  return err;
}

//#define PRINT_REPORT

errno_t test_cue_traverse(void) {
  cue_traverse_visitor_t visitor;
  cue_traverse_visitor_opts_t visitor_opts = { 0 };
  cue_traverse_report_writer_t writer;
  array_line_writer_t line_writer;
  null_line_writer_t null_line_writer;
  compare_visitor_t cv;
  errno_t err = 0;

  printf("Checking cue traversal... ");

  ERR_REGION_BEGIN() {
    array_line_writer_init(&line_writer);
    ERR_REGION_ERROR_CHECK(null_line_writer_init(&null_line_writer), err);
    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_init_params(
      &writer,
      &line_writer.line_writer), err);

    memset(&visitor_opts, 0, sizeof(visitor_opts));
    visitor_opts.target_path = s_cue_trg_dir;
    visitor_opts.source_path = s_cue_src_dir;
    visitor_opts.report_only = 0;
    visitor_opts.writer = &null_line_writer.line_writer;

    ERR_REGION_ERROR_CHECK(cue_traverse_visitor_init(
      &visitor, 
      &visitor_opts), err);

    traverse_dir_path(s_cue_src_dir, &visitor.pv_t.handler_i);

    cue_traverse_report_t *report = visitor.report;

    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_write(&writer, report), err);

    ERR_REGION_ERROR_CHECK(compare_report(report, &s_traverse_results), err);

    // make sure the expected directory structure exists
    compare_visitor_init(&cv, s_test_traverse_result, s_test_traverse_result_len);
    traverse_dir_path(s_cue_trg_dir, &cv.handler_i);
    ERR_REGION_CMP_CHECK(cv.line != s_test_traverse_result_len, err);

  } ERR_REGION_END()

  printf("%s\n", err ? "FAILED!" : "passed.");

#ifdef PRINT_REPORT
  dump_string_array(line_writer.lines, line_writer.num_lines);
#endif

  delete_dir(s_cue_trg_dir);
  cue_traverse_report_writer_uninit(&writer);
  cue_traverse_visitor_uninit(&visitor);
  array_line_writer_uninit(&line_writer);
  null_line_writer_uninit(&null_line_writer);

  return err;
}

typedef struct cue_options_test_result {
  char const* source_dir;
  char const* target_dir;
  short generate_report;
  char const* report_path;
  short quiet;
  short test_only;
  short overwrite;
  float quality;
} cue_options_test_result_t;

static errno_t compare_options_result(cue_options_t const* opts, cue_options_test_result_t const* result) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {

    ERR_REGION_CMP_CHECK(opts->source_dir != 0 && result->source_dir == 0, err);
    if (result->source_dir) ERR_REGION_CMP_CHECK(strcmp(opts->source_dir, result->source_dir) != 0, err);
    ERR_REGION_CMP_CHECK(opts->target_dir != 0 && result->target_dir == 0, err);
    if (result->target_dir) ERR_REGION_CMP_CHECK(strcmp(opts->target_dir, result->target_dir) != 0, err);
    ERR_REGION_CMP_CHECK(opts->report_path != 0 && result->report_path == 0, err);
    if (result->report_path) ERR_REGION_CMP_CHECK(strcmp(opts->report_path, result->report_path) != 0, err);
    ERR_REGION_CMP_CHECK(opts->generate_report != result->generate_report, err);
    ERR_REGION_CMP_CHECK(opts->quiet != result->quiet, err);
    ERR_REGION_CMP_CHECK(opts->test_only != result->test_only, err);
    ERR_REGION_CMP_CHECK(opts->overwrite != result->overwrite, err);
    ERR_REGION_CMP_CHECK(opts->quality != result->quality, err);

  } ERR_REGION_END()

  return err;
}

errno_t test_cue_options(void) {
  errno_t err = 0;

  ERR_REGION_BEGIN() {

    cue_options_t opts;

    printf("Checking cue options... ");

    // test 1. everything ok
    ERR_REGION_BEGIN() {
      ERR_REGION_ERROR_CHECK(cue_options_init(&opts), err);

      char const *argv[] = {
        "-Q",
        "-r",
        "report path",
        "src dir",
        "trg dir",
      };
      size_t argc = sizeof(argv) / sizeof(*argv);

      cue_options_test_result_t result = {
        .source_dir = "src dir",
        .target_dir= "trg dir",
        .generate_report = 1,
        .report_path = "report path",
        .quiet = 1,
        .test_only = 0,
        .overwrite = 0,
        .quality = 3,
      };

      ERR_REGION_ERROR_CHECK(cue_options_load_from_args(&opts, argc, argv), err);

      err = compare_options_result(&opts, &result);

      cue_options_uninit(&opts);
    } ERR_REGION_END() ERR_REGION_ERROR_BUBBLE(err);

    // test 2. only dirs
    ERR_REGION_BEGIN() {
      ERR_REGION_ERROR_CHECK(cue_options_init(&opts), err);

      char const* argv[] = {
        "src dir",
        "trg dir",
      };
      size_t argc = sizeof(argv) / sizeof(*argv);

      cue_options_test_result_t result = {
        .source_dir = "src dir",
        .target_dir = "trg dir",
        .quality = 3,
      };

      ERR_REGION_ERROR_CHECK(cue_options_load_from_args(&opts, argc, argv), err);

      err = compare_options_result(&opts, &result);

      cue_options_uninit(&opts);
    } ERR_REGION_END() ERR_REGION_ERROR_BUBBLE(err);

    // test 3. no dirs (empty)
    ERR_REGION_BEGIN() {
      ERR_REGION_ERROR_CHECK(cue_options_init(&opts), err);

      char const* argv[] = { 0 };
      size_t argc = 0;

      ERR_REGION_CMP_CHECK(! cue_options_load_from_args(&opts, argc, argv), err);

      cue_options_uninit(&opts);
    } ERR_REGION_END() ERR_REGION_ERROR_BUBBLE(err);

    // test 4. no dirs (-r path)
    ERR_REGION_BEGIN() {
      ERR_REGION_ERROR_CHECK(cue_options_init(&opts), err);

      char const* argv[] = {
        "-r",
        "report path",
      };
      size_t argc = sizeof(argv) / sizeof(*argv);

      ERR_REGION_CMP_CHECK(! cue_options_load_from_args(&opts, argc, argv), err);

      cue_options_uninit(&opts);
    } ERR_REGION_END() ERR_REGION_ERROR_BUBBLE(err);

    // test 5. no dirs (-r no path)
    ERR_REGION_BEGIN() {
      ERR_REGION_ERROR_CHECK(cue_options_init(&opts), err);

      char const* argv[] = {
        "-r",
      };
      size_t argc = sizeof(argv) / sizeof(*argv);

      ERR_REGION_CMP_CHECK(!cue_options_load_from_args(&opts, argc, argv), err);

      cue_options_uninit(&opts);
    } ERR_REGION_END() ERR_REGION_ERROR_BUBBLE(err);

  } ERR_REGION_END()

  printf("%s\n", err ? "FAILED!" : "passed.");

  return err;
}

errno_t test_cue_convert(void) {
  errno_t err = 0;
  string_vector_t *argv = 0;
  cue_convert_env_t env;
  compare_visitor_t cv;

  env.out = stdout; 
  env.err = stderr;

  printf("Checking cue convert... ");

  ERR_REGION_BEGIN() {
    
    ERR_REGION_NULL_CHECK(argv = string_vector_alloc(), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "some_dir\\cue_tests"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "-Q"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "-q"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "5"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, s_cue_src_dir), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, s_cue_trg_dir), err);

    ERR_REGION_ERROR_CHECK(cue_convert_with_args(
      argv->get_length(argv),
      argv->get_buffer(argv),
      &env, 0), err);

    // make sure the expected directory structure exists
    compare_visitor_init(&cv, s_test_traverse_result, s_test_traverse_result_len);
    traverse_dir_path(s_cue_trg_dir, &cv.handler_i);
    ERR_REGION_CMP_CHECK(cv.line != s_test_traverse_result_len, err);

  } ERR_REGION_END()

  delete_dir(s_cue_trg_dir);
  SAFE_FREE_HANDLER(argv, string_vector_free);

  printf("%s\n", err ? "FAILED!" : "passed.");

  return err;
}

errno_t test_cue_overwrite(void) {
  errno_t err = 0;
  string_vector_t* argv = 0;
  cue_convert_env_t env;
  cue_traverse_report_t *report = 0;

  env.out = stdout;
  env.err = stderr;

  printf("Checking cue overwrite... ");

  ERR_REGION_BEGIN() {

    // copy over the src directory to cause an overwrite conflict
    ERR_REGION_ERROR_CHECK(copy_dir(s_cue_src_dir, s_cue_trg_dir), err);

    // do a convert run without overwrite, should convert nothing
    ERR_REGION_NULL_CHECK(argv = string_vector_alloc(), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "some_dir\\cue_tests"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, "-Q"), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, s_cue_src_dir), err);
    ERR_REGION_NULL_CHECK(argv->push(argv, s_cue_trg_dir), err);

    ERR_REGION_ERROR_CHECK(cue_convert_with_args(
      argv->get_length(argv),
      argv->get_buffer(argv),
      &env, &report), err);

    // make sure nothing was converted
    ERR_REGION_NULL_CHECK(report, err);
    ERR_REGION_CMP_CHECK(report->found_cue_count != 0, err);

    // reset values to reuse
    SAFE_FREE_HANDLER(report, cue_traverse_report_free);
    report = 0;

    // do the convert again, but with force
    ERR_REGION_NULL_CHECK(argv->insert_at(argv, 2, "-f"), err);

    ERR_REGION_ERROR_CHECK(cue_convert_with_args(
      argv->get_length(argv),
      argv->get_buffer(argv),
      &env, &report), err);

    // make sure the two items were converted
    ERR_REGION_NULL_CHECK(report, err);
    ERR_REGION_CMP_CHECK(report->found_cue_count != 2, err);
    ERR_REGION_CMP_CHECK(report->transformed_cue_count != 2, err);

  } ERR_REGION_END()

  delete_dir(s_cue_trg_dir);
  SAFE_FREE_HANDLER(argv, string_vector_free);
  SAFE_FREE_HANDLER(report, cue_traverse_report_free);

  printf("%s\n", err ? "FAILED!" : "passed.");

  return err;
}
