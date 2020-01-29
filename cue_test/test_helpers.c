#include "test_helpers.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "string_helpers.h"
#include "mem_helpers.h"

short compare_string_arrays(char const* const* arr1, int arr1_len, char const* const* arr2, int arr2_len) {
  if (arr1_len != arr2_len) return 0;

  for (int i = 0; i < arr1_len; ++i) {
    if (strcmp(arr1[i], arr2[i]) != 0) return 0;
  }

  return 1;
}

void dump_string_array(char const* const* array, int num_lines) {
  for (int i = 0; i < num_lines; ++i) {
    printf("%s\n", array[i]);
  }
}

static char const *strings[] = {
  "join", "these", "strings", "together",
};

static const size_t s_strings_len = sizeof(strings) / sizeof(*strings);
static char const *s_joined_string = "join..these..strings..together";

errno_t test_string_join(void) {
  const size_t joined_len = strlen(s_joined_string);

  printf("Checking string join... ");
  char const *joined = join_strings(strings, s_strings_len, "..");

  errno_t result = -1;
  if (joined) {
    result = 0;
    //printf("[%s] ", joined);

    if (strlen(joined) != joined_len) result = -1;
    if (result || strncmp(joined, s_joined_string, joined_len) != 0) result = -1;

    SAFE_FREE(joined);
  }

  printf("%s\n", result ? "FAILED!" : "passed.");
  return result;
}
