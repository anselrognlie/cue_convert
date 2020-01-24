#include "file_helpers.h"

#include <stdio.h>
#include <stdlib.h>

// files should be opened in BINARY mode
// buf will be set to a malloc'd string that holds the read line
//   in case of error, it may be set to a NULL pointer
// bytes_read will be set to the number of bytes in buf
//   in case of error, it may be set to 0  
// the return will be the number of bytes in buf, or EOF if the end of file was reached
size_t fh_getline(char const** buf, size_t* bytes_read, FILE* fid) {
  if (buf == NULL || bytes_read == NULL) {
    return 0;
  }

  // clear out params
  *bytes_read = 0;  // literal 0
  *buf = NULL;  // null pointer

  // get the current file position
  long start = ftell(fid);

  // read from file until we get a newline
  int byte = 0;
  long byte_count = 0;
  do {
    byte = fgetc(fid);

    ++byte_count;
  } while (byte != '\n' && byte != EOF);

  // don't count the trailing end of line or EOF
  --byte_count;

  // byte_count will now contain the line length less a trailing newline/EOF

  if (!byte_count) {
    // no data, so prepare to return an empty string
    char* empty = malloc(1);
    if (empty) {
      // if malloc failed, out params will be left NULL and 0
      *buf = empty;
      empty[0] = 0;
      *bytes_read = 0;
    }

    if (byte == EOF) {
      return EOF;
    }
    else {
      return 0;
    }
  }

  // allocate memory for the string to return
  // include extra byte for end of line, which will become the null terminator
  char* bytes = malloc(byte_count + 1);
  if (!bytes) {
    // if malloc failed, out params will be left NULL and 0
    return 0;
  }

  // return to where we started and read the available bytes
  fseek(fid, start, SEEK_SET);
  fread_s(bytes, byte_count + 1, byte_count + 1, 1, fid);
  bytes[byte_count] = 0;

  // remove any other noise at the end of line (CRLF)
  for (int i = byte_count - 1; i; --i) {
    if (bytes[i] == '\n' || bytes[i] == '\r') {
      bytes[i] = 0;
      --byte_count;
    }
  }

  // update out params and return
  *buf = bytes;
  *bytes_read = byte_count;
  return byte_count;
}
