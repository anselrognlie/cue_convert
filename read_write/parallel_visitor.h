#pragma once

#include <stddef.h>

#include "directory_traversal_handler.h"

struct parallel_visitor_state;
struct parallel_visitor;

typedef short (*parallel_visitor_callback)(struct parallel_visitor* self, struct parallel_visitor_state const* state);

typedef struct parallel_visitor_state {
  struct directory_traversal_handler_state const *base_state;  // weak ref
  char const *parallel_path;  // weak ref
} parallel_visitor_state_t;

typedef struct parallel_visitor {
  directory_traversal_handler_i handler_i;
  void *self;
  char const *root_path;  // owned
  parallel_visitor_callback visit;
  parallel_visitor_callback exit;
} parallel_visitor_t;

errno_t parallel_visitor_init(parallel_visitor_t* self,
  char const* root_path);

void parallel_visitor_uninit(parallel_visitor_t* self);
