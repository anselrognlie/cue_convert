#pragma once

struct file_handle;
struct directory_entry;

typedef struct directory_traveral_handler {
  void* self;
  // return true to continue traversal, or false to halt
  short (*visit)(struct directory_traveral_handler* self, struct file_handle *directory, struct directory_entry *entry);
} directory_traveral_handler_i;