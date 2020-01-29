#include "all_tests.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "string_vector.h"
#include "string_helpers.h"
#include "mem_helpers.h"
#include "err_helpers.h"

errno_t test_string_stack(void) {
  errno_t result = 0;
  char* last = 0;
  string_vector_t *vec;

  printf("Checking vector stack behavior... ");

  ERR_REGION_BEGIN() {
    vec = string_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 0, result);

    string_vector_push(vec, "str1");
    string_vector_push(vec, "str2");
    string_vector_push(vec, "str3");

    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 3, result);

    string_vector_pop(vec);
    string_vector_pop_keep(vec, &last);

    ERR_REGION_CMP_CHECK(strcmp(last, "str2") != 0, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(string_vector_get(vec, 0), "str1") != 0, result);
  } ERR_REGION_END()

  if (last) free(last);
  if (vec) string_vector_free(vec);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_string_unstack(void) {
  errno_t result = 0;
  char* last = 0;
  string_vector_t* vec;

  printf("Checking vector unstack behavior... ");

  ERR_REGION_BEGIN() {

    vec = string_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 0, result);

    string_vector_unshift(vec, "str1");
    string_vector_unshift(vec, "str2");
    string_vector_unshift(vec, "str3");

    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 3, result);

    string_vector_shift(vec);
    string_vector_shift_keep(vec, &last);

    ERR_REGION_CMP_CHECK(strcmp(last, "str2") != 0, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(string_vector_get(vec, 0), "str1") != 0, result);
  } ERR_REGION_END()

  if (last) free(last);
  if (vec) string_vector_free(vec);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_string_queue(void) {
  errno_t result = 0;
  char* last = 0;
  string_vector_t* vec;

  printf("Checking vector queue behavior... ");

  ERR_REGION_BEGIN() {
    vec = string_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 0, result);

    string_vector_push(vec, "str1");
    string_vector_push(vec, "str2");
    string_vector_push(vec, "str3");

    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 3, result);

    string_vector_shift_keep(vec, &last);
    string_vector_shift(vec);

    ERR_REGION_CMP_CHECK(strcmp(last, "str1") != 0, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(string_vector_get(vec, 0), "str3") != 0, result);
  } ERR_REGION_END()

  if (last) free(last);
  if (vec) string_vector_free(vec);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_string_vector_join(void) {
  errno_t result = 0;
  string_vector_t* vec;
  char const* joined = NULL;

  printf("Checking vector join behavior... ");

  ERR_REGION_BEGIN() {
    vec = string_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(string_vector_get_length(vec) != 0, result);

    string_vector_push(vec, "str1");
    string_vector_push(vec, "str2");
    string_vector_push(vec, "str3");

    joined = join_strings(
      string_vector_get_buffer(vec), 
      string_vector_get_length(vec),
      "//");
    ERR_REGION_NULL_CHECK(joined, result);

    ERR_REGION_CMP_CHECK(strcmp(joined, "str1//str2//str3") != 0, result);
  } ERR_REGION_END()

  if (joined) SAFE_FREE(joined);
  if (vec) string_vector_free(vec);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}
