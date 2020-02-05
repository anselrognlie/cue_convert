#pragma once

#include <stddef.h>

typedef struct cue_options {
  char const *source_dir;
  char const *target_dir;
  short generate_report;
  char const *report_path;
  short quiet;
  short test_only;
} cue_options_t;

struct cue_options* cue_options_alloc();
errno_t cue_options_init(struct cue_options *self);
void cue_options_uninit(struct cue_options* self);
void cue_options_free(struct cue_options* self);

errno_t cue_options_load_from_args(struct cue_options *self, int argc, char const **argv);
char const *cue_options_get_help(void);
