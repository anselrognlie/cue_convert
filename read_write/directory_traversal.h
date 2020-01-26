#pragma once

struct directory_traveral_handler;
struct file_handle;

short traverse_dir(struct file_handle* directory, struct directory_traveral_handler *handler);
void traverse_dir_path(char const *path, struct directory_traveral_handler *handler);
