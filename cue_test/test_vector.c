#include "all_tests.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "string_vector.h"
#include "string_helpers.h"
#include "mem_helpers.h"
#include "err_helpers.h"
#include "char_vector.h"

errno_t test_string_stack(void) {
  errno_t result = 0;
  char* last = 0;
  string_vector_t *vec;

  printf("Checking vector stack behavior... ");

  ERR_REGION_BEGIN() {
    vec = string_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

    vec->push(vec, "str1");
    vec->push(vec, "str2");
    vec->push(vec, "str3");

    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 3, result);

    vec->pop(vec);
    vec->pop_keep(vec, &last);

    ERR_REGION_CMP_CHECK(strcmp(last, "str2") != 0, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(vec->get(vec, 0), "str1") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE(last);
  SAFE_FREE_HANDLER(vec, string_vector_free);

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
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

    vec->unshift(vec, "str1");
    vec->unshift(vec, "str2");
    vec->unshift(vec, "str3");

    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 3, result);

    vec->shift(vec);
    vec->shift_keep(vec, &last);

    ERR_REGION_CMP_CHECK(strcmp(last, "str2") != 0, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(vec->get(vec, 0), "str1") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE(last);
  SAFE_FREE_HANDLER(vec, string_vector_free);

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
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

    vec->push(vec, "str1");
    vec->push(vec, "str2");
    vec->push(vec, "str3");

    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 3, result);

    vec->shift_keep(vec, &last);
    vec->shift(vec);

    ERR_REGION_CMP_CHECK(strcmp(last, "str1") != 0, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(strcmp(vec->get(vec, 0), "str3") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE(last);
  SAFE_FREE_HANDLER(vec, string_vector_free);

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
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

    vec->push(vec, "str1");
    vec->push(vec, "str2");
    vec->push(vec, "str3");

    joined = join_cstrs(
      vec->get_buffer(vec),
      vec->get_length(vec),
      "//");
    ERR_REGION_NULL_CHECK(joined, result);

    ERR_REGION_CMP_CHECK(strcmp(joined, "str1//str2//str3") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE(joined);
  SAFE_FREE_HANDLER(vec, string_vector_free);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_string_holder_char(void) {
  errno_t result = 0;
  string_t *string = 0;
  char const *cstr = 0;

  printf("Checking string holder char behavior... ");

  ERR_REGION_BEGIN() {
    string = char_vector_alloc();
    ERR_REGION_NULL_CHECK(string, result);
    ERR_REGION_CMP_CHECK(string->get_length(string) != 0, result);

    string->push(string, 'a');
    string->push(string, 'b');
    string->push(string, 'c');
    ERR_REGION_CMP_CHECK(string->get_length(string) != 3, result);

    cstr = string->get_str(string);
    ERR_REGION_NULL_CHECK(cstr, result);

    ERR_REGION_CMP_CHECK(strcmp(cstr, "abc") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(string, char_vector_free);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_string_holder_str(void) {
  errno_t result = 0;
  string_t* string = 0;
  char cstr[4] = {0};

  printf("Checking string holder str behavior... ");

  ERR_REGION_BEGIN() {
    string = char_vector_alloc();
    ERR_REGION_NULL_CHECK(string, result);
    ERR_REGION_CMP_CHECK(string->get_length(string) != 0, result);

    string->set_str(string, "xyz");
    ERR_REGION_CMP_CHECK(string->get_length(string) != 3, result);

    string->get_str_buf(string, cstr, sizeof(cstr));

    ERR_REGION_CMP_CHECK(strcmp(cstr, "xyz") != 0, result);
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(string, char_vector_free);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_int_queue(void) {
  errno_t result = 0;
  int last = 0;
  int_vector_t* vec;

  printf("Checking int queue behavior... ");

  ERR_REGION_BEGIN() {
    vec = int_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

    vec->push(vec, 1);
    vec->push(vec, 2);
    vec->push(vec, 3);

    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 3, result);

    vec->shift_keep(vec, &last);
    vec->shift(vec);

    ERR_REGION_CMP_CHECK(last != 1, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(vec->get(vec, 0) != 3, result);
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(vec, int_vector_free);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_double_queue(void) {
  errno_t result = 0;
  double last = 0;
  double_vector_t* vec;

  printf("Checking double queue behavior... ");

  ERR_REGION_BEGIN() {
    vec = double_vector_alloc();
    ERR_REGION_NULL_CHECK(vec, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 0, result);

   vec->push(vec, 3);
   vec->push(vec, 2);
   vec->push(vec, 1);

    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 3, result);

    vec->shift_keep(vec, &last);
    vec->shift(vec);

    ERR_REGION_CMP_CHECK(last != 3, result);
    ERR_REGION_CMP_CHECK(vec->get_length(vec) != 1, result);
    ERR_REGION_CMP_CHECK(vec->get(vec, 0) != 1, result);
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(vec, double_vector_free);

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}
