#pragma once

struct file_handle;
struct directory_entry;
struct string_vector;
struct directory_traversal_handler;
struct directory_traversal_handler_state;

typedef short (*directory_traversal_callback)(
  struct directory_traversal_handler* self,
  struct directory_traversal_handler_state const* state);

typedef struct directory_traversal_handler_state {
  struct file_handle* directory;
  struct directory_entry* entry;
  short first_entry;
  short last_entry;
  struct string_vector* history;
} directory_traversal_handler_state_t;

typedef struct directory_traversal_handler {
  void* self;
  // return true to continue traversal, or false to halt
  directory_traversal_callback visit;
  directory_traversal_callback exit;
} directory_traversal_handler_i;