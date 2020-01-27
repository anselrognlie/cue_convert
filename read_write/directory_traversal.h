#pragma once

#include <stddef.h>

struct directory_traveral_handler;
struct file_handle;
struct directory_traversal_options;

typedef struct directory_traversal_options {
  short should_descend;
  short post_visit;
} directory_traversal_options_t;

short traverse_dir(struct file_handle* directory, struct directory_traveral_handler *handler);
short traverse_dir_opts(struct file_handle* directory, struct directory_traversal_options const *opts, struct directory_traveral_handler* handler);
short traverse_dir_path(char const *path, struct directory_traveral_handler *handler);
short traverse_dir_path_opts(char const* path, struct directory_traversal_options const* opts, struct directory_traveral_handler* handler);
