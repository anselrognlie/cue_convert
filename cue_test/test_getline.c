#include <stdlib.h>

#include "file_helpers.h"
#include "mem_helpers.h"

static char const *s_test_files[] = {
  "..\\test_data\\empty_file",
  "..\\test_data\\short_file",
  "..\\test_data\\win_file",
  "..\\test_data\\unix_file",
};

static const short s_test_file_lines[] = { 0, 1, 5, 5 };

static const size_t s_test_file_num = sizeof(s_test_files) / sizeof(s_test_files[0]);

void test_getline(void) {
  size_t bytes = 0;

  for (size_t i = 0; i < s_test_file_num; ++i) {
    char const *filename = s_test_files[i];
    FILE *in_file;
    errno_t result;

    printf("Checking length of %s... ", filename);

    result = fopen_s(&in_file, filename, "rb");
    if (in_file) {
      int line_cnt = 0;
      size_t bytes = 0;
      char *line = NULL;
      while (fh_getline(&line, &bytes, in_file) != EOF) {
        ++line_cnt;
        SAFE_FREE(line);
      }

      SAFE_FREE(line);

      if (line_cnt == s_test_file_lines[i]) {
        printf("passed.\n");
      }
      else {
        printf("FAILED!\n");
      }

      fclose(in_file);
    }
    else {
      printf("FAILED TO OPEN!\n");
    }
  }
}
