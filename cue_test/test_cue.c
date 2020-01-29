#include "all_tests.h"

#include <string.h>

#include "cue_file.h"
#include "cue_parser.h"
#include "cue_transform.h"
#include "array_line_reader.h"
#include "array_line_writer.h"

#include "test_helpers.h"

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

