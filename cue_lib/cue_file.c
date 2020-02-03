#include "cue_file.h"

#include <stdlib.h>
#include <string.h>

//
// cue sheet
//

cue_sheet_t* cue_sheet_alloc(void) {
  cue_sheet_t *sheet = malloc(sizeof(cue_sheet_t));
  if (! sheet) {
    return 0;
  }

  cue_sheet_init(sheet);
  return sheet;
}

cue_sheet_t* cue_sheet_alloc_copy(cue_sheet_t const* src) {
  cue_sheet_t* sheet = malloc(sizeof(cue_sheet_t));
  if (!sheet) {
    return NULL;
  }
  memset(sheet, 0, sizeof(*sheet));

  cue_sheet_copy_from(sheet, src);
  return sheet;
}

void cue_sheet_init(cue_sheet_t* self) {
  self->num_files = 0;
  self->file = malloc(0);
}

cue_file_t* cue_sheet_new_file(cue_sheet_t* self) {
  cue_file_t* file = cue_file_alloc();
  if (!file) {
    return 0;
  }

  short num_files = self->num_files + 1;
  cue_file_t** file_array = realloc(self->file, num_files * sizeof(cue_file_t*));

  if (!file_array) {
    cue_file_free(file);
    return 0;
  }

  self->file = file_array;
  self->num_files = num_files;
  self->file[num_files - 1] = file;

  return file;
}

void cue_sheet_copy_from(cue_sheet_t *dest, cue_sheet_t const *src) {
  if (dest->file) cue_sheet_uninit(dest);

  cue_sheet_init(dest);
  if (!dest->file) return;

  for (cue_file_t const** file = src->file; file < src->file + src->num_files; ++file) {
    cue_file_t *dest_file = cue_sheet_new_file(dest);
    if (! dest_file) return;

    cue_file_copy_from(dest_file, *file);
  }
}

void cue_sheet_uninit(cue_sheet_t* self) {
  // free any owned files
  for (int i = 0; i < self->num_files; ++i) {
    cue_file_free(self->file[i]);
    self->file[i] = 0;
  }
  self->num_files = 0;

  // free the file array
  free(self->file);
  self->file = 0;
}

void cue_sheet_free(cue_sheet_t* self) {
  cue_sheet_uninit(self);

  // free the sheet itself
  free(self);
}

//
// cue file record
//

cue_file_t* cue_file_alloc(void) {
  cue_file_t* file = malloc(sizeof(cue_file_t));
  if (!file) {
    return 0;
  }

  cue_file_init(file);
  return file;
}

void cue_file_init(cue_file_t* self) {
  memset(self, 0, sizeof(*self));
  self->track = malloc(0);
  self->filename = malloc(0);
}

void cue_file_set_filename(cue_file_t* self, char const* filename) {
  if (self->filename == filename) {
    return;
  }

  size_t name_len = strlen(filename);
  cue_file_set_filename_range(self, filename, filename + name_len);
}

void cue_file_set_filename_range(cue_file_t* self, char const* start, char const* end) {
  if (self->filename == start) {
    return;
  }

  if (end < start) {
    return;
  }

  size_t name_len = end - start;
  char* new_filename = realloc(self->filename, name_len + 1);
  if (!new_filename) {
    return;
  }

  self->filename = new_filename;
  strncpy_s(self->filename, name_len + 1, start, name_len);
}

cue_track_t* cue_file_new_track(cue_file_t* self) {
  cue_track_t* track = cue_track_alloc();
  if (!track) {
    return 0;
  }

  short num_tracks = self->num_tracks + 1;
  cue_track_t** track_array = realloc(self->track, num_tracks * sizeof(cue_track_t*));

  if (!track_array) {
    cue_track_free(track);
    return 0;
  }

  self->track = track_array;
  self->num_tracks = num_tracks;
  self->track[num_tracks - 1] = track;

  return track;
}

void cue_file_copy_from(cue_file_t* dest, cue_file_t const* src) {
  if (dest->track) cue_file_uninit(dest);

  cue_file_init(dest);
  if (!dest->track) return;

  dest->type = src->type;
  cue_file_set_filename(dest, src->filename);

  for (cue_track_t const** track = src->track; track < src->track + src->num_tracks; ++track) {
    cue_track_t* dest_track = cue_file_new_track(dest);
    if (!dest_track) return;

    cue_track_copy_from(dest_track, *track);
  }
}

void cue_file_uninit(cue_file_t* self) {
  // free any owned tracks
  for (int i = 0; i < self->num_tracks; ++i) {
    cue_track_free(self->track[i]);
    self->track[i] = 0;
  }
  self->num_tracks = 0;

  // free the track array
  free(self->track);
  self->track = 0;

  // free the filename
  free(self->filename);
  self->filename = 0;
}

void cue_file_free(cue_file_t* self) {
  cue_file_uninit(self);

  // free the file itself
  free(self);
}

//
// cue track record
//

cue_track_t* cue_track_alloc(void) {
  cue_track_t* track = malloc(sizeof(cue_track_t));
  if (!track) {
    return 0;
  }

  cue_track_init(track);
  return track;
}

void cue_track_init(cue_track_t* self) {
  memset(self, 0, sizeof(*self));
  self->index = malloc(0);
}

short cue_track_has_pregap(cue_track_t const* self) {
  return (self->pregap.minutes + self->pregap.seconds + self->pregap.frames != 0);
}

cue_index_t* cue_track_new_index(cue_track_t* self) {
  short num_indexes = self->num_indexes + 1;
  cue_index_t* index_array = realloc(self->index, num_indexes * sizeof(cue_index_t));

  if (!index_array) {
    return 0;
  }

  self->index = index_array;
  self->num_indexes = num_indexes;

  cue_index_t* new_index = self->index + num_indexes - 1;
  cue_index_init(new_index);

  return new_index;
}

void cue_track_copy_from(cue_track_t* dest, cue_track_t const* src) {
  if (dest->index) cue_track_uninit(dest);

  cue_track_init(dest);
  if (!dest->index) return;

  dest->track = src->track;
  dest->mode = src->mode;
  dest->pregap = src->pregap;

  for (cue_index_t const* index = src->index; index < src->index + src->num_indexes; ++index) {
    cue_index_t* dest_index = cue_track_new_index(dest);
    if (!dest_index) return;

    *dest_index = *index;
  }
}

void cue_track_uninit(cue_track_t* self) {
  // free the index array
  free(self->index);
  self->index = 0;
}

void cue_track_free(cue_track_t* self) {
  cue_track_uninit(self);

  // free the file itself
  free(self);
}

//
// cue index record
//

void cue_index_init(cue_index_t* self) {
  memset(self, 0, sizeof(*self));
}

void cue_index_init_args(cue_index_t* self, short index, cue_time_t timestamp) {
  cue_index_init(self);
  self->index = index;
  self->timestamp = timestamp;
}

//
// cue time stamp
//

cue_time_t cue_time_from_msf(short minutes, short seconds, short frames) {
  cue_time_t time;
  cue_time_set_msf(&time, minutes, seconds, frames);
  return time;
}

void cue_time_set_msf(cue_time_t* self, short minutes, short seconds, short frames) {
  self->minutes = minutes;
  self->seconds = seconds;
  self->frames = frames;
}
