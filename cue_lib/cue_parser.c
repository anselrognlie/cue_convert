#include "cue_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cue_file.h"
#include "mem_helpers.h"
#include "format_helpers.h"
#include "line_reader.h"
#include "file_line_reader.h"
#include "line_writer.h"
#include "file_line_writer.h"
#include "cue_sheet_parse_result.h"
#include "err_helpers.h"

static char const *skip_ws(char const *buf);
static char const * skip_token(char const *buf, char const* token);
static char const * parse_path(char const* buf, char const** start, char const** end);
static char const* parse_file_type(char const* buf, cue_file_type_t *type);
static char const *cft2str(cue_file_type_t type);
static char const* parse_int_token(char const* buf, int* number);
static char const* parse_track_mode(char const* buf, cue_track_mode_t* mode);
static char const* ctm2str(cue_track_mode_t mode);
static char const* parse_time(char const* buf, cue_time_t *time);
static void safe_add_error(cue_sheet_parse_result_t *result, size_t line_num, char const *line);

cue_sheet_t* cue_sheet_parse_file(FILE* fid, struct cue_sheet_parse_result* result_opt) {
  file_line_reader_t line_reader;
  file_line_reader_init_fid(&line_reader, fid);
  return cue_sheet_parse(&line_reader.line_reader, result_opt);
}

cue_sheet_t* cue_sheet_parse_filename(char const* filename, struct cue_sheet_parse_result* result_opt) {
  FILE* cue_in;
  errno_t result = fopen_s(&cue_in, filename, "rb");
  if (!cue_in) {
    fprintf(stderr, "unable to open file: %s\n", filename);
    return NULL;
  }

  cue_sheet_t *sheet = cue_sheet_parse_file(cue_in, result_opt);

  fclose(cue_in);

  return sheet;
}

#define LOG_AND_BREAK() { safe_add_error(result_opt, line_num, line); has_errors = 1; break; }

cue_sheet_t* cue_sheet_parse(line_reader_i *reader, struct cue_sheet_parse_result* result_opt) {
  char const *line = 0;
  size_t bytes;
  size_t line_num = 0;
  short has_errors = 0;

  cue_sheet_t *sheet = cue_sheet_alloc();
  cue_file_t *curr_file = NULL;
  cue_track_t *curr_track = NULL;

  while (reader->read_line(reader, &line, &bytes) != EOF) {
    char const *parse = line;
    ++line_num;

    // skip leading whitspace
    parse = skip_ws(parse);

    // take action based on found token
    switch (*parse) {
      case 'F':
      {
        parse = skip_token(parse, "FILE");
        if (! parse) break;

        char *path = NULL, *end = NULL;
        parse = parse_path(parse, &path, &end);
        if (!parse) LOG_AND_BREAK();

        cue_file_type_t type;
        parse = parse_file_type(parse, &type);
        if (!parse) LOG_AND_BREAK();

        // got a whole file
        curr_file = cue_sheet_new_file(sheet);
        cue_file_set_filename_range(curr_file, path, end);
        curr_file->type = type;
      }
      break;

      case 'T':
      {
        parse = skip_token(parse, "TRACK");
        if (! parse) break;

        if (!curr_file) LOG_AND_BREAK();

        int track;
        parse = parse_int_token(parse, &track);
        if (! parse) LOG_AND_BREAK();

        cue_track_mode_t mode;
        parse = parse_track_mode(parse, &mode);
        if (! parse) LOG_AND_BREAK();

        // got a whole track
        curr_track = cue_file_new_track(curr_file);
        curr_track->track = track;
        curr_track->mode = mode;
      }
      break;

      case 'P':
      {
        parse = skip_token(parse, "PREGAP");
        if (!parse) break;

        if (!curr_track) LOG_AND_BREAK();

        cue_time_t time;
        parse = parse_time(parse, &time);
        if (!parse) LOG_AND_BREAK();

        // got a whole pregap
        curr_track->pregap = time;
      }
      break;

      case 'I':
      {
        parse = skip_token(parse, "INDEX");
        if (!parse) break;

        if (!curr_track) LOG_AND_BREAK();

        int index;
        parse = parse_int_token(parse, &index);
        if (!parse) LOG_AND_BREAK();

        cue_time_t time;
        parse = parse_time(parse, &time);
        if (!parse) LOG_AND_BREAK();

        // got a whole index
        cue_index_t *curr_index = cue_track_new_index(curr_track);
        curr_index->index = index;
        curr_index->timestamp = time;
      }
      break;

      default:
        // ignore line
        ;
    }
    
    SAFE_FREE(line);
  }

  SAFE_FREE(line);

  // check whether there was an error
  if (has_errors) {
    cue_sheet_free(sheet);
    sheet = NULL;
  }

  return sheet;
}

static const char s_file_line_format[] = "FILE \"%s\" %s";
static const char s_track_line_format[] = "  TRACK %02d %s";
static const char s_pregap_line_format[] = "    PREGAP %02d:%02d:%02d";
static const char s_index_line_format[] = "    INDEX %02d %02d:%02d:%02d";

static errno_t cue_sheet_write_indexes(cue_index_t const* indexes, short num_indexes, line_writer_i* writer) {
  char* buf = NULL;
  size_t written = 0;
  errno_t err = 0;

  for (int k = 0; k < num_indexes; ++k) {
    ERR_REGION_BEGIN() {
      cue_index_t index = indexes[k];
      buf = msnprintf(s_index_line_format, index.index,
        index.timestamp.minutes, index.timestamp.seconds, index.timestamp.frames);
      ERR_REGION_NULL_CHECK(buf, err);

      written = writer->write_line(writer->self, buf);
      ERR_REGION_CMP_CHECK(!written && *buf, err);

      SAFE_FREE(buf);
    } ERR_REGION_END()

    if (err) break;
  }

  SAFE_FREE(buf);

  return err;
}

static errno_t cue_sheet_write_tracks(cue_track_t const** tracks, short num_tracks, line_writer_i* writer) {
  char* buf = NULL;
  size_t written = 0;
  errno_t err = 0;

  for (int j = 0; j < num_tracks; ++j) {
    ERR_REGION_BEGIN() {
      cue_track_t const* track = tracks[j];
      buf = msnprintf(s_track_line_format, track->track, ctm2str(track->mode));
      ERR_REGION_NULL_CHECK(buf, err);

      written = writer->write_line(writer->self, buf);
      ERR_REGION_CMP_CHECK(!written && *buf, err);

      SAFE_FREE(buf);

      if (cue_track_has_pregap(track)) {
        buf = msnprintf(s_pregap_line_format,
          track->pregap.minutes, track->pregap.seconds, track->pregap.frames);
        ERR_REGION_NULL_CHECK(buf, err);

        written = writer->write_line(writer->self, buf);
        ERR_REGION_CMP_CHECK(!written && *buf, err);

        SAFE_FREE(buf);
      }

      ERR_REGION_ERROR_CHECK(cue_sheet_write_indexes(track->index, track->num_indexes, writer), err);

    } ERR_REGION_END()

    if (err) break;
  }

  SAFE_FREE(buf);

  return err;
}

errno_t cue_sheet_write(struct cue_sheet const* sheet, line_writer_i * writer) {
  char *buf = NULL;
  size_t written = 0;
  errno_t err = 0;

  for (int i = 0; i < sheet->num_files; ++i) {
    ERR_REGION_BEGIN() {
      cue_file_t* file = sheet->file[i];
      buf = msnprintf(s_file_line_format, file->filename, cft2str(file->type));
      ERR_REGION_NULL_CHECK(buf, err);

      written = writer->write_line(writer->self, buf);
      ERR_REGION_CMP_CHECK(!written && *buf, err);

      SAFE_FREE(buf);

      ERR_REGION_ERROR_CHECK(cue_sheet_write_tracks(file->track, file->num_tracks, writer), err);
    } ERR_REGION_END()

    if (err) break;
  }

  SAFE_FREE(buf);

  return err;
}

errno_t cue_sheet_write_file(cue_sheet_t const* sheet, FILE* fid) {
  file_line_writer_t line_writer;
  file_line_writer_init_fid(&line_writer, fid);
  return cue_sheet_write(sheet, &line_writer.line_writer);
}

errno_t cue_sheet_write_filename(cue_sheet_t const* sheet, char const *filename) {
  FILE* cue_out;
  errno_t result = fopen_s(&cue_out, filename, "wb");
  if (!cue_out) {
    fprintf(stderr, "unable to open file: %s\n", filename);
    return -1;
  }

  result = cue_sheet_write_file(sheet, cue_out);

  fclose(cue_out);

  return result;
}

//
// helper routines
//

static char const* const s_whitespace = " \t";
static char const* const s_quote= "\"";

static short char_in(char c, char const *class) {
  return strchr(class, c) != NULL;
}

static char const* skip_chars(char const* buf, char const* class) {
  while (char_in(*buf, class)) ++buf;

  return buf;
}

static char const* skip_char(char const* buf, char const* class) {
  if (char_in(*buf, class)) ++buf;

  return buf;
}

static char const* skip_ws(char const * buf) {
  return skip_chars(buf, s_whitespace);
}

static char const* skip_token(char const * buf, char const* token) {
  while (*token) {
    if (*token == *buf) {
      ++token;
      ++buf;
    }
    else {
      break;
    }
  }
 
  if (*token) {
    // token not found
    return NULL;
  }

  if (!*buf) {
    // reached end of buffer
    return buf;
  }

  char const *next = skip_ws(buf);
  if (next == buf) {
    // no whitespace separation
    return NULL;
  }

  return next;
}

static char const* parse_path(char const* buf, char const** start_out, char const** end_out) {
  // check whether the path starts with a quote
  int has_quote = 0;
  char const *end_mark = s_whitespace;
  if (*buf == '"') {
    has_quote = 1;
    end_mark = s_quote;
    ++buf;
  }

  char const *start = buf;

  // read up to the end mark
  size_t len = strcspn(buf, end_mark);
  char const *end = start + len;
  buf = end;

  // if we wanted a quote, make sure we found it
  if (has_quote) {
    if (*end != '"') {
      return NULL;
    }

    ++buf;
  }

  if (start == end) return NULL;

  *start_out = start;
  *end_out = end;

  if (!*buf) {
    // reached end of buffer
    return buf;
  }
  
  char const* next = skip_ws(buf);
  if (next == buf) {
    // no whitespace separation
    return NULL;
  }

  return next;
}

#define PARSE_FILE_TYPE(token, cft_type) \
  next = skip_token(buf, token); \
  if (next != NULL) { \
    *type = cft_type; \
    return next; \
  }

static char const* parse_file_type(char const* buf, cue_file_type_t* type) {
  // try to parse each token
  char const *next = NULL;
  PARSE_FILE_TYPE("BINARY", EWC_CFT_BINARY);
  PARSE_FILE_TYPE("WAV", EWC_CFT_WAV);
  PARSE_FILE_TYPE("MP3", EWC_CFT_MP3);
  PARSE_FILE_TYPE("OGG", EWC_CFT_OGG);

  return 0;
}

static char const* s_cue_file_type_strs[] = {
  "BINARY",
  "WAV",
  "MP3",
  "OGG",
};

static char const* cft2str(cue_file_type_t type) {
  return s_cue_file_type_strs[type];
}

static char const * const s_digits = "1234567890";
static char s_int_buf[16];
static const int s_int_buf_len = sizeof(s_int_buf);

static char const* parse_int(char const* buf, int* number) {
  size_t len = strspn(buf, s_digits);
  if (len == 0) return NULL;

  memset(s_int_buf, 0, sizeof(s_int_buf));
  memmove_s(s_int_buf, s_int_buf_len, buf, len);

  int i = atoi(s_int_buf);

  *number = i;
  return buf + len;
}

// expects whitespace to follow the value
static char const* parse_int_token(char const* buf, int* number) {
  int i;
  buf = parse_int(buf, &i);
  if (! buf) return NULL;

  *number = i;

  if (!*buf) {
    // reached end of buffer
    return buf;
  }

  char const* next = skip_ws(buf);
  if (next == buf) {
    // no whitespace separation
    return NULL;
  }

  return next;
}

#define PARSE_TRACK_MODE(token, ctm_mode) \
  next = skip_token(buf, token); \
  if (next != NULL) { \
    *mode = ctm_mode; \
    return next; \
  }

static char const* parse_track_mode(char const* buf, cue_track_mode_t* mode) {
  // try to parse each token
  char const* next = NULL;
  PARSE_TRACK_MODE("AUDIO", EWC_CTM_AUDIO);
  PARSE_TRACK_MODE("MODE1/2352", EWC_CTM_MODE1_2352);
  PARSE_TRACK_MODE("MODE1/2048", EWC_CTM_MODE1_2048);

  return 0;
}

static char const* s_cue_track_mode_strs[] = {
  "AUDIO",
  "MODE1/2352",
  "MODE1/2048",
};

static char const* ctm2str(cue_track_mode_t mode) {
  return s_cue_track_mode_strs[mode];
}

static const char s_colon_delim[] = ":";

static char const* parse_time(char const* buf, cue_time_t* time) {
  // get minutes
  int m;
  char const *next;
  buf = parse_int(buf, &m);
  if (!buf) return NULL;
  
  // skip delimeter
  next = skip_char(buf, s_colon_delim);
  if (next == buf) return NULL;

  // get seconds
  int s;
  buf = parse_int(next, &s);
  if (! buf) return NULL;

  // skip delimeter
  next = skip_char(buf, s_colon_delim);
  if (next == buf) return NULL;

  // get frames
  int f;
  buf = parse_int(next, &f);
  if (!buf) return NULL;

  cue_time_set_msf(time, m, s, f);

  if (!*buf) {
    // reached end of buffer
    return buf;
  }

  next = skip_ws(buf);
  if (next == buf) {
    // no whitespace separation
    return NULL;
  }

  return next;
}

static void safe_add_error(cue_sheet_parse_result_t* result, size_t line_num, char const* line) {
  if (! result) return;

  cue_sheet_parse_result_add_error(result, line_num, line);
}
