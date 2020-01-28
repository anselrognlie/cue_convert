#include "string_helpers.h"

#include <string.h>
#include <stdlib.h>

#include "mem_helpers.h"

typedef struct {
  size_t *str_lens;
  char *joined;
} join_strings_t;

static void free_join_strings(join_strings_t* state) {
  if (state) {
    if (state->str_lens) SAFE_FREE(state->str_lens);
  }
}

#define EXIT_JOIN_STRINGS_IF_INVALID(arg) \
  if (!(arg)) { \
    free_join_strings(&state); \
    return NULL; \
  }

char const* join_strings(char const* strings[], size_t num_strings, char const* delim) {
  size_t delim_len = strlen(delim);
  size_t delim_total_len = delim_len * (num_strings - 1);

  // figure out total string lengths
  join_strings_t state;
  memset(&state, 0, sizeof(state));

  size_t str_len_size = num_strings * sizeof(size_t);
  state.str_lens = malloc(str_len_size);
  EXIT_JOIN_STRINGS_IF_INVALID(state.str_lens);

  size_t str_total_len = 0;
  for (size_t i = 0; i < num_strings; ++i) {
    size_t str_len = strlen(strings[i]);
    state.str_lens[i] = str_len;
    str_total_len += str_len;
  }

  size_t joined_len = str_total_len + delim_total_len;
  state.joined = malloc(joined_len + 1);
  EXIT_JOIN_STRINGS_IF_INVALID(state.joined);

  // join everything together
  size_t remaining_len = joined_len + 1;
  short first = 1;
  char *insert_at = state.joined;
  for (size_t i = 0; i < num_strings; ++i) {
    if (!first) {
      // insert delim
      memmove_s(insert_at, remaining_len, delim, delim_len);
      insert_at += delim_len;
      remaining_len -= delim_len;
    }

    size_t curr_str_len = state.str_lens[i];
    memmove_s(insert_at, remaining_len, strings[i], curr_str_len);
    insert_at += curr_str_len;
    remaining_len -= curr_str_len;

    first = 0;
  }

  state.joined[joined_len] = 0;
  char *joined = state.joined;
  state.joined = 0;

  free_join_strings(&state);

  return joined;
}