#pragma once

#include <stdio.h>

// files should be opened in BINARY mode
// buf will be set to a malloc'd string that holds the read line
//   in case of error, it may be set to a NULL pointer
// bytes_read will be set to the number of bytes in buf
//   in case of error, it may be set to 0  
// the return will be the number of bytes in buf, or EOF if the end of file was reached
size_t fh_getline(char const** buf, size_t* bytes_read, FILE* fid);
