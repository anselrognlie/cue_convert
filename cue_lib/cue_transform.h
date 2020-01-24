#pragma once

struct cue_sheet;

typedef enum cue_audio_target {
  EWC_CAT_OGG = 0,
  EWC_CAT_LAST,
} cue_audio_target_t;

typedef struct cue_transform_audio_options {
  cue_audio_target_t target_type;
} cue_transform_audio_options_t;

struct cue_sheet *cue_sheet_transform_audio(struct cue_sheet const * sheet, cue_transform_audio_options_t const *options);
