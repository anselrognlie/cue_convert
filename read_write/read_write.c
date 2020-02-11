#include "read_write.h"

#include <stdio.h>
#include <string.h>

#include "line_reader.h"
#include "line_writer.h"
#include "mem_helpers.h"

errno_t read_write_all_lines(struct line_reader* reader, struct line_writer* writer) {
  errno_t err = 0;
  char *line = 0;
  size_t bytes_read;
  size_t bytes_written;

  while (reader->read_line(reader, &line, &bytes_read) != EOF) {
    bytes_written = writer->write_line(writer, line);
    SAFE_FREE(line);
    if (bytes_written != bytes_read) {
      err = -1;
      break;
    }
  }

  SAFE_FREE(line);

  return err;
}
