#pragma once

#include <stddef.h>

#include "directory_traversal_handler.h"

struct char_vector;
struct cue_sheet;
struct cue_traverse_record;
struct cue_traverse_record_vector;
struct cue_traverse_report;

typedef struct cue_traverse_visitor {
  directory_traversal_handler_i handler_i;
  struct cue_traverse_report *report;
  struct char_vector *target_path;
  struct char_vector *source_path;
  short generate_report;
  short execute;
} cue_traverse_visitor_t;

errno_t cue_traverse_visitor_init(cue_traverse_visitor_t* self,
  char const* target_path,
  char const* source_path,
  short generate_report,
  short execute);

void cue_traverse_visitor_uninit(cue_traverse_visitor_t* self);

