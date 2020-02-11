#include "all_tests.h"

#include "array_line_reader.h"
#include "array_line_writer.h"
#include "read_write.h"
#include "test_helpers.h"

char const *s_read_write_strs[] = {
  "line 1",
  "line 2",
  "line 3",
};

const size_t s_read_write_strs_len = sizeof(s_read_write_strs) / sizeof(*s_read_write_strs);

errno_t test_read_write_all(void) {
  errno_t err = 0;
  array_line_reader_t reader;
  array_line_writer_t writer;
  short pass;

  printf("Checking read/write behavior... ");

  array_line_reader_init_lines(&reader, s_read_write_strs, s_read_write_strs_len);
  array_line_writer_init(&writer);

  err = read_write_all_lines(&reader.line_reader, &writer.line_writer);
  pass = compare_string_arrays(s_read_write_strs, s_read_write_strs_len, writer.lines, writer.num_lines);
  if (! pass) err = -1;

  printf("%s\n", err ? "FAILED!" : "passed.");

  array_line_writer_uninit(&writer);

  return err;
}
