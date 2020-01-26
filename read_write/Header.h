#pragma once

#ifdef _WIN32

typedef void *HANDLE;

typedef struct directory_handle {
  HANDLE handle;
} directory_handle_t;

#endif
