#pragma once

#include <stddef.h>

#include "directory_traversal_handler.h"

struct char_vector;
struct cue_sheet;
struct cue_traverse_record;
struct cue_traverse_record_vector;

typedef struct cue_traverse_report {
  int found_cue_count;
  int transformed_cue_count;
  int failed_cue_count;
  struct cue_traverse_record_vector* transformed_list;
  struct cue_traverse_record_vector* failed_list;
} cue_traverse_report_t;

typedef struct cue_traverse_visitor {
  directory_traversal_handler_i handler_i;
  struct char_vector *target_path;
  struct char_vector *source_path;
  short generate_report;
  short execute;
} cue_traverse_visitor_t;

errno_t ctv_init_visitor(cue_traverse_visitor_t* self,
  char const* target_path,
  char const* source_path,
  short generate_report,
  short execute);

errno_t ctv_uninit_visitor(cue_traverse_visitor_t* self);

