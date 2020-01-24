#include "test_helpers.h"

#include <string.h>

short compare_arrays(char const* const* arr1, int arr1_len, char const* const* arr2, int arr2_len) {
  if (arr1_len != arr2_len) return 0;

  for (int i = 0; i < arr1_len; ++i) {
    if (strcmp(arr1[i], arr2[i]) != 0) return 0;
  }

  return 1;
}
