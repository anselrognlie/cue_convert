#include "cue_options.h"

#include <stdlib.h>

#include "err_helpers.h"
#include "mem_helpers.h"

static const char k_help_message[] = 
"[-t] [-q] [-r report_path] source_directory target_directory\n"
"\n"
"-t - test mode - just examine the cues, don't convert\n"
"-q - quiet mode - no console output\n"
"-r report_path - report location - where the conversion report\n"
"                 will be written.  If not supplied, the report\n"
"                 will not be saved, but will still be written to\n"
"                 the console if not in quiet mode.\n"
"source_directory - location to start the conversion traversal\n"
"target_directory - location to replicate the source directory\n"
"                   structure, copying and converting as needed.\n"
"";

struct cue_options* cue_options_alloc() {
  cue_options_t *self = malloc(sizeof(*self));
  if (! self) return NULL;

  errno_t err = 0;
  err = cue_options_init(self);
  if (! err) return self;

  SAFE_FREE(self);
  return NULL;
}

errno_t cue_options_init(struct cue_options* self) {
  errno_t err = 0;

  memset(self, 0, sizeof(*self));

  return err;
}

void cue_options_uninit(struct cue_options* self) {
  SAFE_FREE(self->source_dir);
  SAFE_FREE(self->target_dir);
  SAFE_FREE(self->report_path);
}

void cue_options_free(struct cue_options* self) {
  cue_options_uninit(self);
  SAFE_FREE(self);
}

errno_t cue_options_load_from_args(struct cue_options* self, int argc, char const** argv) {
  errno_t err = 0;
  char const* report_path = 0;
  char const* src_dir = 0;
  char const* trg_dir = 0;
  char const* report_path_dup = 0;
  char const* src_dir_dup = 0;
  char const* trg_dir_dup = 0;
  short quiet = 0;
  short test_only = 0;

  // -q -r <report.file> <src_dir> <trg_dir>

  ERR_REGION_BEGIN() {
    short past_args = 0;

    int i = 0;
    for (; i < argc; ++i) {
      char const *arg = argv[i];

      if (*arg != '-') {
        past_args = 1;
        break;
      }

      // must be a short option
      ERR_REGION_CMP_CHECK((!*(arg + 1) || *(arg + 2)), err);

      switch (*(arg + 1)) {
        case 'q':
          quiet = 1;
        break;

        case 't':
          test_only = 1;
          break;

        case 'r':
          if (i > argc-2) {
            err = -1; 
          }
          else {
            report_path = argv[++i];
          }
        break;

        default:
        // unknown
        err = -1;

      } ERR_REGION_ERROR_BUBBLE(err);

    } ERR_REGION_ERROR_BUBBLE(err);

    // must still be two options, the src and the trg
    ERR_REGION_CMP_CHECK(i > argc-2, err);

    src_dir = argv[i++];
    trg_dir = argv[i++];

    // make sure we used all the options
    ERR_REGION_CMP_CHECK(i != argc, err);

    // allocate the string copies to use in the options
    ERR_REGION_NULL_CHECK(src_dir_dup = _strdup(src_dir), err);
    ERR_REGION_NULL_CHECK(trg_dir_dup = _strdup(trg_dir), err);
    if (report_path) ERR_REGION_NULL_CHECK(report_path_dup = _strdup(report_path), err);

    // everything we need is allocated, so release existing resources and update
    SAFE_FREE(self->source_dir);
    SAFE_FREE(self->target_dir);
    SAFE_FREE(self->report_path);

    self->source_dir = src_dir_dup;
    self->target_dir = trg_dir_dup;
    self->report_path = report_path_dup;
    self->generate_report = (report_path != 0);
    self->quiet = quiet;
    self->test_only = test_only;

    return err;

  } ERR_REGION_END()

  SAFE_FREE(src_dir_dup);
  SAFE_FREE(trg_dir_dup);
  SAFE_FREE(report_path_dup);

  return err;
}

char const* cue_options_get_help(void) {
  return k_help_message;
}
