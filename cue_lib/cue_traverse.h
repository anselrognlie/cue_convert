#pragma once

#include <stddef.h>

#include "parallel_visitor.h"

struct cue_sheet;
struct cue_traverse_record;
struct cue_traverse_record_vector;
struct cue_traverse_report;
struct line_writer;

typedef struct cue_traverse_visitor_opts {
  char const* target_path;  // weak ref
  char const* source_path;  // weak ref
  short report_only;
  short overwrite;
  float quality;
  struct line_writer *writer;  // weak ref
  char const * const *filters;  // weak ref
  int num_filters;
} cue_traverse_visitor_opts_t;

typedef struct cue_traverse_visitor {
  parallel_visitor_t pv_t;
  struct cue_traverse_report *report;
  char const *source_path;  // owned copy
  short report_only;
  short overwrite;
  float quality;
  struct line_writer* writer;  // weak ref
  char const* const* filters;  // weak ref
  int num_filters;
} cue_traverse_visitor_t;

errno_t cue_traverse_visitor_init(cue_traverse_visitor_t* self, cue_traverse_visitor_opts_t const *opts);
struct cue_traverse_report* cue_traverse_visitor_detach_report(cue_traverse_visitor_t* self);
void cue_traverse_visitor_uninit(cue_traverse_visitor_t* self);

