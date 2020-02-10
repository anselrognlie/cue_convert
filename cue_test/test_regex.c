#include "all_tests.h"

#include <stdio.h>

#include "regex_helper.h"
#include "err_helpers.h"

typedef struct {
  char const *subject;
  char const *pattern;
  short matches;
} regex_test_t;

#define MATCH 1
#define NO_MATCH 0

static regex_test_t s_match_tests[] = {
  {"Hello, world!", "Hello", MATCH},
  {"Hello, world!", "hello", NO_MATCH},
  {"Hello, world!", "(?i:hello)", MATCH},
  {"Hello, world!", "(?i)hello", MATCH},
  {"Parenthetical (Clause)", "\\(Clause\\)", MATCH},
  {"Some title (Japan) (Unl).cue", "^Some", MATCH},
  {"Some title (Japan) (Unl).cue", "\\.cue$", MATCH},
  {"4^2 = 16", "^4\\^2", MATCH},
  {"Dogs & Cats", "Dogs & Cats", MATCH}
};

static size_t s_match_test_len = sizeof(s_match_tests) / sizeof(*s_match_tests);

errno_t test_regex(void) {
  errno_t err = 0;

  printf("Checking regex... ");

  ERR_REGION_BEGIN() {

    for (size_t i = 0; i < s_match_test_len; ++i) {
      char const* pattern = s_match_tests[i].pattern;
      char const* subject = s_match_tests[i].subject;
      short matches = s_match_tests[i].matches;
      ERR_REGION_CMP_CHECK(regex_matches(pattern, subject) != matches, err);

    } ERR_REGION_ERROR_BUBBLE(err)

  } ERR_REGION_END()

  printf("%s\n", err ? "FAILED!" : "passed.");

  return err;
}