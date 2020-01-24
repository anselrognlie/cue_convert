#include "cue_transform.h"

#include <stdlib.h>

#include "cue_file.h"

// array of source types that we can convert to a target option
static short const s_types_allowed_for_target[EWC_CAT_LAST][EWC_CFT_LAST] = {
  { 1, 1, 0, 0 },  // EWC_CAT_OGG -> BINARY, WAV, !MP#, !OGG
};

static short check_convert(cue_file_t const *file, cue_audio_target_t target);

cue_sheet_t* cue_sheet_transform_audio(cue_sheet_t const* sheet, cue_transform_audio_options_t const* options) {
  cue_sheet_t *transformed = cue_sheet_alloc_copy(sheet);
  if (! transformed) return NULL;

  // consider each file entry
  for (cue_file_t **file = transformed->file; file < transformed->file + transformed->num_files; ++file) {
    if (check_convert(*file, options->target_type)) {
      // rename file
      // update type
    }
  }

  return transformed;
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
