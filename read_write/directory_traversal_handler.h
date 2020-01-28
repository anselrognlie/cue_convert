#pragma once

struct file_handle;
struct directory_entry;

typedef struct directory_traversal_handler_state {
  struct file_handle* directory;
  struct directory_entry* entry;
  short first_entry;
  short last_entry;
} directory_traversal_handler_state_t;

typedef struct directory_traversal_handler {
  void* self;
  // return true to continue traversal, or false to halt
  short (*visit)(struct directory_traversal_handler* self, directory_traversal_handler_state_t const* state);
  short (*exit)(struct directory_traversal_handler* self, directory_traversal_handler_state_t const* state);
} directory_traversal_handler_i;