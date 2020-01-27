#pragma once

#include <stdlib.h>

#include "path.h"

struct ps_path_enumerator;

typedef struct ps_path_enumerator {
  path_enumerator_i enum_i;
  char* path;
  size_t path_len;
  char* path_end;
  char* path_start;
  char path_separator;
  short has_next;
  path_enumerator_rules_i* rules;
} ps_path_enumerator_t;
