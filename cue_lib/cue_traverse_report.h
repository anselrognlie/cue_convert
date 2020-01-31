#pragma once

#include <stddef.h>

struct cue_traverse_record;

typedef struct cue_traverse_report {
  int found_cue_count;
  int transformed_cue_count;
  int failed_cue_count;
  struct cue_traverse_record_vector* transformed_list;
  struct cue_traverse_record_vector* failed_list;
} cue_traverse_report_t;

struct cue_traverse_report* cue_traverse_report_alloc();
errno_t cue_traverse_report_init(struct cue_traverse_report* self);
struct cue_traverse_record const* cue_traverse_report_add_record(
  struct cue_traverse_report* self, 
  struct cue_traverse_record *record, 
  short transformed);
void cue_traverse_report_uninit(struct cue_traverse_report* self);
void cue_traverse_report_free(struct cue_traverse_report* self);
