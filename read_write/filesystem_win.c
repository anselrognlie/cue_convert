#include "filesystem.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mem_helpers.h"

static char const s_path_wildcard[] = "\\*";
static const size_t s_path_wildcard_len = sizeof(s_path_wildcard) - 1;
static char const s_path_separator[] = "\\";
static const size_t s_path_separator_len = sizeof(s_path_separator) - 1;

typedef struct fs_file_handle {
  file_handle_i handle_i;
  HANDLE handle;
  WIN32_FIND_DATA ffd;
  char const *path;
  short eof;
} fs_file_handle_t;

static void fs_file_handle_init(fs_file_handle_t* self);
static short fs_is_eof(file_handle_i* self);
static void fs_close_dir(file_handle_i* handle);
static directory_entry_i* fs_next_dir_entry(file_handle_i* handle);
static file_handle_i* fs_open_directory(file_handle_i* handle, char const *path);
static char const* fs_get_path(file_handle_i* handle);

typedef struct fs_directory_entry {
  directory_entry_i entry_i;
  char *name;
  short is_directory;
} fs_directory_entry_t;

static void fs_entry_init(fs_directory_entry_t* self);
static short fs_is_directory(struct directory_entry* self);
static char const* fs_get_name(struct directory_entry* self);
static void fs_release(struct directory_entry* self);

static wchar_t* widen_path(char const* path) {
  // widen the path
  wchar_t* path_w = NULL;
  errno_t result;
  size_t result_size;
  size_t path_len = strlen(path);

  size_t path_w_len;
  result = mbstowcs_s(&path_w_len, NULL, 0, path, path_len);
  path_w = malloc(path_w_len * sizeof(wchar_t));
  if (!path_w) return NULL;

  result = mbstowcs_s(&result_size, path_w, path_w_len, path, path_len);
  if (result) {
    free(path_w);
    return NULL;
  }

  return path_w;
}

static char* narrow_path(wchar_t const* path) {
  char* path_n = NULL;
  errno_t result;
  size_t result_size;
  size_t path_len = wcslen(path);

  result = wcstombs_s(&result_size, NULL, 0, path, path_len);
  size_t path_n_len = (result_size + 1) * sizeof(char);
  path_n = malloc(path_n_len);
  if (!path_n) return NULL;

  result = wcstombs_s(&result_size, path_n, path_n_len, path, path_len);
  if (result) {
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

typedef struct {
  char *path_with_wildcard;
  wchar_t *path_w;
  HANDLE h;
  char *path_duplicate;
} open_dir_state_t;

static void release_open_dir_state(open_dir_state_t* state) {
  if (state) {
    if (state->path_with_wildcard) free(state->path_with_wildcard);
    if (state->path_w) free(state->path_w);
    if (state->h) CloseHandle(state->h);
    if (state->path_duplicate) free(state->path_duplicate);
  }
}

#define OPEN_DIR_NULL_CHECK(arg) \
  if (!(arg)) { \
    release_open_dir_state(&state); \
    return NULL; \
  }

#define OPEN_DIR_INVALID_CHECK(arg) \
  if (INVALID_HANDLE_VALUE == (arg)) { \
    release_open_dir_state(&state); \
    return NULL; \
  }

file_handle_i* open_dir(char const* path) {
  open_dir_state_t state;
  WIN32_FIND_DATA ffd;

  memset(&state, 0, sizeof(open_dir_state_t));

  size_t path_len = strlen(path);
  size_t path_wildcard_len = path_len + s_path_wildcard_len;
  state.path_with_wildcard = malloc(path_wildcard_len + 1);
  OPEN_DIR_NULL_CHECK(state.path_with_wildcard);

  strcpy_s(state.path_with_wildcard, path_wildcard_len + 1, path);
  strcpy_s(state.path_with_wildcard + path_len, path_wildcard_len - path_len + 1, s_path_wildcard);

  state.path_w = widen_path(state.path_with_wildcard);
  OPEN_DIR_NULL_CHECK(state.path_w);

  state.h = FindFirstFile(state.path_w, &ffd);
  OPEN_DIR_INVALID_CHECK(state.h);

  state.path_duplicate = _strdup(path);
  OPEN_DIR_NULL_CHECK(state.path_duplicate);

  fs_file_handle_t* result = malloc(sizeof(fs_file_handle_t));
  OPEN_DIR_NULL_CHECK(result);

  fs_file_handle_init(result);
  result->handle = state.h;
  result->ffd = ffd;
  result->eof = 0;
  result->path = state.path_duplicate;

  // unset state values that shouldn't be cleaned up
  state.h = 0;
  state.path_duplicate = NULL;

  release_open_dir_state(&state);

  return &result->handle_i;
}

void fs_close_dir(file_handle_i* handle) {
  fs_file_handle_t *self = (fs_file_handle_t*)handle->self;
  FindClose(self->handle);
  free((void*)self->path);
  free(self);
}

short fs_is_eof(file_handle_i* handle) {
  fs_file_handle_t* self = (fs_file_handle_t*)handle->self;
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

static char* join_path(char const* base, char const* path) {
  size_t base_len = strlen(base);
  size_t sep_len = strlen(s_path_separator);
  size_t path_len = strlen(path);

  size_t joined_len = base_len + sep_len + path_len;
  char *joined = malloc(joined_len + 1);
  if (! joined) return NULL;

  strcpy_s(joined, joined_len + 1, base);
  strcat_s(joined, joined_len + 1, s_path_separator);
  strcat_s(joined, joined_len + 1, path);

  return joined;
}

static file_handle_i* fs_open_directory(file_handle_i* handle, char const* path) {
  fs_file_handle_t *self = (fs_file_handle_t*)handle->self;

  char *new_path = join_path(self->path, path);
  if (! new_path) return NULL;

  file_handle_i *new_handle = open_dir(new_path);

  free(new_path);

  return new_handle;
}

static char const* fs_get_path(file_handle_i* handle) {
  fs_file_handle_t* self = (fs_file_handle_t*)handle->self;
  return self->path;
}

static void fs_entry_init(fs_directory_entry_t* self) {
  self->entry_i.self = self;
  self->entry_i.is_directory = fs_is_directory;
  self->entry_i.get_name = fs_get_name;
  self->entry_i.release = fs_release;
}

static short fs_is_directory(directory_entry_i* entry) {
  fs_directory_entry_t* self = (fs_directory_entry_t*)entry->self;
  return self->is_directory;
}

static char const* fs_get_name(directory_entry_i* entry) {
  fs_directory_entry_t* self = (fs_directory_entry_t*)entry->self;
  return self->name;
}

static void fs_release(directory_entry_i* entry) {
  fs_directory_entry_t* self = (fs_directory_entry_t*)entry->self;
  free(self->name);
  free(self);
}

#endif
