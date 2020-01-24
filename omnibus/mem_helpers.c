#include "mem_helpers.h"

#include <stdlib.h>

void mh_safe_free(void** m) {
  if (*m) {
    free(*m);
  }

  *m = NULL;
}
