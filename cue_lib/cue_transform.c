#include "cue_transform.h"

#include <stdlib.h>
#include <string.h>

#include "cue_file.h"
#include "mem_helpers.h"

// array of source types that we can convert to a target option
static const short s_types_allowed_for_target[EWC_CAT_LAST][EWC_CFT_LAST] = {
  { 1, 1, 0, 0 },  // EWC_CAT_OGG -> BINARY, WAV, !MP3, !OGG
};

// array of file types given the target conversion type
static const cue_file_type_t s_type_for_target[EWC_CAT_LAST] = {
  EWC_CFT_OGG,  // EWC_CAT_OGG -> EWC_CFT_OGG
};

// array of file extensions given the target conversion type
static char const *s_ext_for_target[EWC_CAT_LAST] = {
  "ogg",  // EWC_CAT_OGG -> ogg
};

static short check_convert(cue_file_t const* file, cue_audio_target_t target);
static char *rename_file(char const* filename, cue_audio_target_t target);

cue_sheet_t* cue_sheet_transform_audio(cue_sheet_t const* sheet, cue_transform_audio_options_t const* options) {
  cue_sheet_t *transformed = cue_sheet_alloc_copy(sheet);
  if (! transformed) return NULL;

  // consider each file entry
  for (cue_file_t **file = transformed->file; file < transformed->file + transformed->num_files; ++file) {
    if (check_convert(*file, options->target_type)) {
      // rename file
      char const *renamed_file = rename_file((*file)->filename, options->target_type);
      if (! renamed_file) goto csta_unwind;

      cue_file_set_filename(*file, renamed_file);
      SAFE_FREE(renamed_file);

      // update type
      (*file)->type = s_type_for_target[options->target_type];
    }
  }

  return transformed;

csta_unwind:
  cue_sheet_free(transformed);
  return NULL;
}

//
// helpers
//

static short check_convert(cue_file_t const* file, cue_audio_target_t target_type) {
  // file type must be one we know how to convert
  cue_file_type_t src_type = file->type;
  if (!s_types_allowed_for_target[target_type][src_type]) {
    return 0;
  }

  // tracks must all be AUDIO
  for (cue_track_t *track = *file->track; track < *file->track + file->num_tracks; ++track) {
    if (track->mode != EWC_CTM_AUDIO) return 0;
  }

  return 1;
}

static char* rename_file(char const* filename, cue_audio_target_t target) {
  // find the last . to get our ext
  char const *end = strrchr(filename, '.');
  if (!end) {
    // we didn't find an extension, so just use the string end
    end = filename + strlen(filename);
  }
  char const *ext = s_ext_for_target[target];

  // calculate new buffer size
  size_t base_len = end - filename;
  size_t ext_len = strlen(ext);
  size_t buf_len = base_len + ext_len + 1 + 1; // 1 for . and one more for NULL

  char *buf = malloc(buf_len);
  if (!buf) return NULL;

  memmove_s(buf, buf_len, filename, base_len);
  memmove_s(buf + base_len, buf_len - base_len, ".", 1);
  memmove_s(buf + base_len + 1, buf_len - base_len - 1, ext, ext_len);
  buf[buf_len - 1] = 0;

  return buf;
}
