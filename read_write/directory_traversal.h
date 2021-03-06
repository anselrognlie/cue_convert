#pragma once

#include <stddef.h>

struct directory_traversal_handler;
struct file_handle;
struct directory_traversal_options;
struct string_vector;

typedef struct directory_traversal_options {
  short should_descend;
  short post_visit;
} directory_traversal_options_t;

short traverse_dir(struct file_handle* directory, struct directory_traversal_handler *handler);
short traverse_dir_opts(struct file_handle* directory, struct directory_traversal_options const *opts, struct directory_traversal_handler* handler);
short traverse_dir_path(char const *path, struct directory_traversal_handler *handler);
short traverse_dir_path_opts(char const* path, struct directory_traversal_options const* opts, struct directory_traversal_handler* handler);
