#pragma once

#include <stddef.h>

struct file_handle;
struct directory_entry;

typedef struct file_handle {
  void *self;
  short (*is_eof)(struct file_handle const* self);
  void (*close)(struct file_handle* self);
  struct directory_entry *(*next_dir_entry)(struct file_handle* self);
  struct file_handle *(*open_directory)(struct file_handle const *self, char const *path);
  char const* (*get_path)(struct file_handle const* self);
} file_handle_i;

typedef struct directory_entry {
  void* self;
  short (*is_directory)(struct directory_entry const* self);
  char const* (*get_name)(struct directory_entry const* self);
  void (*release)(struct directory_entry* self);
} directory_entry_i;

file_handle_i* open_dir(char const* path);
errno_t delete_file(char const* path);
errno_t delete_dir(char const* path);
errno_t ensure_dir(char const* path);
short file_exists(char const* path);

extern char const *k_path_separator;
