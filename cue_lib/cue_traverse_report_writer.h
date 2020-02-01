#pragma once

#include <stddef.h>

struct line_writer;
struct cue_traverse_report;

typedef struct cue_traverse_report_writer {
  struct line_writer *writer;
} cue_traverse_report_writer_t;

struct cue_traverse_report_writer* cue_traverse_report_writer_alloc_params(struct line_writer *writer);
errno_t cue_traverse_report_writer_init_params(struct cue_traverse_report_writer* self, struct line_writer* writer);
void cue_traverse_report_writer_uninit(struct cue_traverse_report_writer* self);
void cue_traverse_report_writer_free(struct cue_traverse_report_writer* self);

errno_t cue_traverse_report_writer_write(struct cue_traverse_report_writer* self, struct cue_traverse_report *report);
