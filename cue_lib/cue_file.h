#pragma once

typedef struct cue_time {
  short minutes;
  short seconds;  // 60 seconds per minute
  short frames;  // 75 frames per second
} cue_time_t;

typedef struct cue_index {
  short index;
  cue_time_t timestamp;
} cue_index_t;

typedef enum cue_track_mode {
  EWC_CTM_AUDIO = 0,
  EWC_CTM_MODE1_2352,
  EWC_CTM_MODE1_2048,
  EWC_CTM_LAST,
} cue_track_mode_t;

typedef struct cue_track {
  short track;
  cue_track_mode_t mode;
  cue_time_t pregap;
  cue_index_t *index;
  short num_indexes;
} cue_track_t;

typedef enum cue_file_type {
  EWC_CFT_BINARY = 0,
  EWC_CFT_WAV,
  EWC_CFT_MP3,
  EWC_CFT_OGG,
  EWC_CFT_LAST
} cue_file_type_t;

typedef struct cue_file {
  char *filename;
  cue_file_type_t type;
  cue_track_t **track;
  short num_tracks;
} cue_file_t;

typedef struct cue_sheet {
  cue_file_t **file;
  short num_files; 
} cue_sheet_t;

cue_sheet_t* cue_sheet_alloc(void);
cue_sheet_t* cue_sheet_alloc_copy(cue_sheet_t const *src);
void cue_sheet_init(cue_sheet_t* self);
cue_file_t *cue_sheet_new_file(cue_sheet_t *self);
void cue_sheet_copy_from(cue_sheet_t* dest, cue_sheet_t const* src);
void cue_sheet_uninit(cue_sheet_t* self);
void cue_sheet_free(cue_sheet_t* self);

cue_file_t* cue_file_alloc(void);
void cue_file_init(cue_file_t *self);
void cue_file_set_filename(cue_file_t* self, char const* filename);
void cue_file_set_filename_range(cue_file_t* self, char const* start, char const *end);
cue_track_t *cue_file_new_track(cue_file_t *self);
void cue_file_copy_from(cue_file_t* dest, cue_file_t const* src);
void cue_file_uninit(cue_file_t* self);
void cue_file_free(cue_file_t* self);

cue_track_t *cue_track_alloc(void);
void cue_track_init(cue_track_t* self);
short cue_track_has_pregap(cue_track_t const *self);
cue_index_t* cue_track_new_index(cue_track_t* self);
void cue_track_copy_from(cue_track_t *dest, cue_track_t const *src);
void cue_track_uninit(cue_track_t* self);
void cue_track_free(cue_track_t* self);

void cue_index_init(cue_index_t* self);
void cue_index_init_args(cue_index_t* self, short index, cue_time_t timestamp);

cue_time_t cue_time_from_msf(short minutes, short seconds, short frames);
void cue_time_set_msf(cue_time_t* self, short minutes, short seconds, short frames);
