#include "filesystem.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "mem_helpers.h"
#include "string_helpers.h"
#include "directory_traversal.h"
#include "directory_traversal_handler.h"
#include "path.h"
#include "err_helpers.h"

//#define PRINT_ONLY

#define PATH_SEPARATOR_CHAR '\\'
const char k_path_separator_char = PATH_SEPARATOR_CHAR;
const char k_path_separator[] = { PATH_SEPARATOR_CHAR, 0 };

static char const s_path_wildcard[] = "\\*";
static const size_t s_path_wildcard_len = sizeof(s_path_wildcard) - 1;

typedef struct fs_file_handle {
  file_handle_i handle_i;
  HANDLE handle;
  WIN32_FIND_DATA ffd;
  char const *path;
  short eof;
} fs_file_handle_t;

static void fs_file_handle_init(fs_file_handle_t* self);
static short fs_is_eof(file_handle_i const* self);
static void fs_close_dir(file_handle_i* handle);
static directory_entry_i* fs_next_dir_entry(file_handle_i* handle);
static file_handle_i* fs_open_directory(file_handle_i const* handle, char const *path);
static char const* fs_get_path(file_handle_i const* handle);

typedef struct fs_directory_entry {
  directory_entry_i entry_i;
  char *name;
  short is_directory;
} fs_directory_entry_t;

static void fs_entry_init(fs_directory_entry_t* self);
static short fs_is_directory(struct directory_entry const* self);
static char const* fs_get_name(struct directory_entry const* self);
static void fs_release(struct directory_entry* self);

static wchar_t* widen_path(char const* path) {
  // widen the path
  wchar_t* path_w = NULL;
  size_t written;

  size_t path_w_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
  path_w = malloc(path_w_len * sizeof(wchar_t));
  if (!path_w) return NULL;

  written = MultiByteToWideChar(CP_UTF8, 0, path, -1, path_w, path_w_len);
  if (!written) {
    free(path_w);
    return NULL;
  }

  return path_w;
}

static char* narrow_path(wchar_t const* path) {
  char* path_n = NULL;
  size_t result_size;
  size_t path_len = wcslen(path);

  result_size = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
  size_t path_n_len = result_size * sizeof(char);
  path_n = malloc(path_n_len);
  if (!path_n) return NULL;

  size_t written = WideCharToMultiByte(CP_UTF8, 0, path, -1, path_n, result_size, NULL, NULL);
  if (!written) {
    free(path_n);
    return NULL;
  }

  return path_n;
}

static void fs_file_handle_init(fs_file_handle_t* self) {
  self->handle_i.self = self;
  self->handle_i.is_eof = fs_is_eof;
  self->handle_i.close = fs_close_dir;
  self->handle_i.next_dir_entry = fs_next_dir_entry;
  self->handle_i.open_directory = fs_open_directory;
  self->handle_i.get_path = fs_get_path;
}

file_handle_i* open_dir(char const* path) {
  char* path_with_wildcard = 0;
  wchar_t* path_w = 0;
  HANDLE h = INVALID_HANDLE_VALUE;
  char* path_duplicate = 0;
  WIN32_FIND_DATA ffd;
  fs_file_handle_t* result = NULL;
  errno_t err = 0;

  ERR_REGION_BEGIN() {
    size_t path_len = strlen(path);
    size_t path_wildcard_len = path_len + s_path_wildcard_len;
    path_with_wildcard = malloc(path_wildcard_len + 1);
    ERR_REGION_NULL_CHECK(path_with_wildcard, err);

    strcpy_s(path_with_wildcard, path_wildcard_len + 1, path);
    strcpy_s(path_with_wildcard + path_len, path_wildcard_len - path_len + 1, s_path_wildcard);

    path_w = widen_path(path_with_wildcard);
    ERR_REGION_NULL_CHECK(path_w, err);

    h = FindFirstFile(path_w, &ffd);
    ERR_REGION_INVALID_CHECK(h, err);

    path_duplicate = _strdup(path);
    ERR_REGION_NULL_CHECK(path_duplicate, err);

    result = malloc(sizeof(fs_file_handle_t));
    ERR_REGION_NULL_CHECK(result, err);

    fs_file_handle_init(result);
    result->handle = h;
    result->ffd = ffd;
    result->eof = 0;
    result->path = path_duplicate;

    // unset state values that shouldn't be cleaned up
    h = INVALID_HANDLE_VALUE;
    path_duplicate = NULL;
  } ERR_REGION_END()

  if (path_with_wildcard) free(path_with_wildcard);
  if (path_w) free(path_w);
  if (h != INVALID_HANDLE_VALUE) FindClose(h);
  if (path_duplicate) free(path_duplicate);

  if (result) return &result->handle_i;

  return NULL;
}

void fs_close_dir(file_handle_i* handle) {
  fs_file_handle_t *self = (fs_file_handle_t*)handle->self;
  FindClose(self->handle);
  free((void*)self->path);
  free(self);
}

short fs_is_eof(file_handle_i const* handle) {
  fs_file_handle_t const* self = (fs_file_handle_t const*)handle->self;
  return self->eof;
}

directory_entry_i* fs_next_dir_entry(file_handle_i* handle) {
  char* path = NULL;

  fs_file_handle_t* self = (fs_file_handle_t*)handle->self;

  path = narrow_path(self->ffd.cFileName);
  if (!path) return NULL;

  fs_directory_entry_t *result = malloc(sizeof(fs_directory_entry_t));
  if (! result) {
    free(path);
    return NULL;
  }

  fs_entry_init(result);
  result->name = path;
  result->is_directory = !!(self->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

  self->eof = !FindNextFile(self->handle, &self->ffd);

  return &result->entry_i;
}

static char const* join_path(char const* base, char const* path) {
  char const *strings[] = { base, path };
  return join_cstrs(strings, 2, k_path_separator);
}

static file_handle_i* fs_open_directory(file_handle_i const* handle, char const* path) {
  fs_file_handle_t const *self = (fs_file_handle_t const*)handle->self;

  char const *new_path = join_path(self->path, path);
  if (! new_path) return NULL;

  file_handle_i *new_handle = open_dir(new_path);

  SAFE_FREE(new_path);

  return new_handle;
}

static char const* fs_get_path(file_handle_i const* handle) {
  fs_file_handle_t const* self = (fs_file_handle_t const*)handle->self;
  return self->path;
}

static void fs_entry_init(fs_directory_entry_t* self) {
  self->entry_i.self = self;
  self->entry_i.is_directory = fs_is_directory;
  self->entry_i.get_name = fs_get_name;
  self->entry_i.release = fs_release;
}

static short fs_is_directory(directory_entry_i const* entry) {
  fs_directory_entry_t const* self = (fs_directory_entry_t const*)entry->self;
  return self->is_directory;
}

static char const* fs_get_name(directory_entry_i const* entry) {
  fs_directory_entry_t const* self = (fs_directory_entry_t const*)entry->self;
  return self->name;
}

static void fs_release(directory_entry_i* entry) {
  fs_directory_entry_t* self = (fs_directory_entry_t*)entry->self;
  free(self->name);
  free(self);
}

errno_t delete_file(char const* path) {
  BOOL result = 1;
  wchar_t *path_w = widen_path(path);
  if (! path_w) return -1;

#ifndef PRINT_ONLY
  result = DeleteFile(path_w);
#else
  result = 1;
  printf("delete %ls\n", path_w);
#endif

  SAFE_FREE(path_w);

  return ! result;
}

static errno_t remove_directory(char const* path) {
  BOOL result = 1;
  wchar_t* path_w = widen_path(path);
  if (!path_w) return -1;

#ifndef PRINT_ONLY
  result = RemoveDirectory(path_w);
#else
  result = 1;
  printf("remove %ls\n", path_w);
#endif

  SAFE_FREE(path_w);

  return !result;
}

typedef struct delete_visitor {
  directory_traversal_handler_i handler_i;
} delete_visitor_t;

static short dv_visit(struct directory_traversal_handler* self, directory_traversal_handler_state_t const* state) {
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;

  char const *full_path = join_path(directory->get_path(directory), entry->get_name(entry));
  if (! full_path) return 0;

  errno_t result;
  if (! entry->is_directory(entry)) {
    result = delete_file(full_path);
  }
  else {
    result = remove_directory(full_path);
  }

  SAFE_FREE(full_path);

  return ! result;
}

static void dv_init_visitor(delete_visitor_t* self) {
  memset(self, 0, sizeof(*self));
  self->handler_i.self = self;
  self->handler_i.visit = dv_visit;
}

errno_t delete_dir(char const* path) {
  delete_visitor_t visitor;
  dv_init_visitor(&visitor);

  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));
  opts.should_descend = 1;
  opts.post_visit = 1;

  errno_t result = 0;
  short keep_traversing = traverse_dir_path_opts(path, &opts, &visitor.handler_i);
  if (keep_traversing) {
    result = remove_directory(path);
  }

  return result;
}

static errno_t create_directory(char const* path) {
  BOOL result = 1;
  wchar_t* path_w = widen_path(path);
  if (!path_w) return -1;

#ifndef PRINT_ONLY
  result = CreateDirectory(path_w, NULL);
  if (! result) {
    DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
      result = 1;
    }
  }
#else
  result = 1;
  printf("create %ls\n", path_w);
#endif

  SAFE_FREE(path_w);

  return !result;
}

errno_t ensure_dir(char const* path) {
  errno_t err = 0;
  char *buf = 0;
  path_enumerator_i* i = 0;

  ERR_REGION_BEGIN() {
    size_t path_len = strlen(path);
    buf = malloc(path_len + 1);
    ERR_REGION_NULL_CHECK(buf, err);
    memset(buf, 0, path_len + 1);

    // enumerate the path
    i = enumerate_path(path);
    ERR_REGION_NULL_CHECK(i, err);

    // check that each point has a directory created
    while (i->has_next(i) && !err) {
      path_enumerate_status_t status = i->next(i);
      size_t full_path_len = status.path_end - status.full_path_start;
      memmove_s(buf, path_len + 1, status.full_path_start, full_path_len);
      *(buf + full_path_len) = 0;

      err = create_directory(buf);
    }

  } ERR_REGION_END()

  if (i) i->dispose(i);
  SAFE_FREE(buf);

  return err;
}

short file_exists(char const* path) {
  WIN32_FIND_DATA ffd;
  short result = 0;

  wchar_t *path_w = widen_path(path);
  if (!path_w) return 0;

  HANDLE h = FindFirstFile(path_w, &ffd);

  if (INVALID_HANDLE_VALUE != h) {
    result = 1;
    FindClose(h);
  }

  free(path_w);
  return result;
}

#endif
