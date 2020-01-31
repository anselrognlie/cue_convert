#include "string_helpers.h"

#include <string.h>
#include <stdlib.h>

#include "mem_helpers.h"
#include "err_helpers.h"

char * join_cstrs(char const* strings[], size_t num_strings, char const* delim) {
  size_t* str_lens = 0;
  char* joined = 0;
  size_t delim_len = strlen(delim);
  size_t delim_total_len = delim_len * (num_strings - 1);
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    size_t str_len_size = num_strings * sizeof(size_t);
    str_lens = malloc(str_len_size);
    ERR_REGION_NULL_CHECK(str_lens, err);

    size_t str_total_len = 0;
    for (size_t i = 0; i < num_strings; ++i) {
      size_t str_len = strlen(strings[i]);
      str_lens[i] = str_len;
      str_total_len += str_len;
    }

    size_t joined_len = str_total_len + delim_total_len;
    joined = malloc(joined_len + 1);
    ERR_REGION_NULL_CHECK(joined, err);

    // join everything together
    size_t remaining_len = joined_len + 1;
    short first = 1;
    char *insert_at = joined;
    for (size_t i = 0; i < num_strings; ++i) {
      if (!first) {
        // insert delim
        memmove_s(insert_at, remaining_len, delim, delim_len);
        insert_at += delim_len;
        remaining_len -= delim_len;
      }

      size_t curr_str_len = str_lens[i];
      memmove_s(insert_at, remaining_len, strings[i], curr_str_len);
      insert_at += curr_str_len;
      remaining_len -= curr_str_len;

      first = 0;
    }

    joined[joined_len] = 0;
  } ERR_REGION_END()

  SAFE_FREE(str_lens);

  return joined;
}

short cstr_ends_with(char const* cstr, char const* ending) {
  if (!cstr || !ending) return 0;

  size_t cstr_len = strlen(cstr);
  size_t ending_len = strlen(ending);

  if (cstr_len < ending_len) return 0;
  return strncmp(cstr + (cstr_len - ending_len), ending, ending_len) == 0;
}

