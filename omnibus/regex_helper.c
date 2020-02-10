#include "regex_helper.h"

#include <string.h>

#include "err_helpers.h"
#include "mem_helpers.h"

#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8

#include "pcre2.h"

short regex_matches(char const* pattern, char const* str) {
  errno_t err = 0;
  short matches = 0;
  PCRE2_SPTR re_patt = (PCRE2_SPTR)pattern;
  PCRE2_SPTR subject = (PCRE2_SPTR)str;
  PCRE2_SIZE subject_length;
  pcre2_code* re = 0;
  pcre2_match_data* match_data = 0;

  subject_length = (PCRE2_SIZE)strlen(str);

  ERR_REGION_BEGIN() {
    int errornumber;
    PCRE2_SIZE erroroffset;
    int rc;

    // init the re
    re = pcre2_compile(
      re_patt,               /* the pattern */
      PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
      0,                     /* default options */
      &errornumber,          /* for error number */
      &erroroffset,          /* for error offset */
      NULL);                 /* use default compile context */
    ERR_REGION_NULL_CHECK(re, err);

    // initialize match results
    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    ERR_REGION_NULL_CHECK(match_data, err);

    // perform the match
    rc = pcre2_match(
      re,                   /* the compiled pattern */
      subject,              /* the subject string */
      subject_length,       /* the length of the subject */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      match_data,           /* block for storing the result */
      NULL);                /* use default match context */

    if (rc >= 0) matches = 1;
  } ERR_REGION_END()

  SAFE_FREE_HANDLER(match_data, pcre2_match_data_free);
  SAFE_FREE_HANDLER(re, pcre2_code_free);

  if (err) matches = 0;

  return matches;
}
