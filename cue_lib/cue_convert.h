#pragma once

#include <stddef.h>
#include <stdio.h>

struct cue_options;
struct cue_traverse_report;

typedef struct {
  FILE* out;
  FILE* err;
} cue_convert_env_t;

errno_t cue_convert(
  struct cue_options* opts, 
  cue_convert_env_t *env,
  struct cue_traverse_report **report_nullable);

errno_t cue_convert_with_args(
  int argc, 
  char const **argv, 
  cue_convert_env_t* env,
  struct cue_traverse_report** report_nullable);
