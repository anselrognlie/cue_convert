#pragma once

#include <stddef.h>

#include "directory_traversal_handler.h"
#include "parallel_visitor.h"

struct char_vector;

typedef struct dir_entry_fields {
  char const* name;
  short is_dir;
} dir_entry_fields_t;

//
// print visitor
//

typedef struct print_visitor {
  directory_traversal_handler_i handler_i;
} print_visitor_t;

void print_visitor_init(print_visitor_t* self);

//
// compare visitor
//

typedef struct compare_visitor {
  directory_traversal_handler_i handler_i;
  short passed;
  size_t line;
  dir_entry_fields_t const* result;
  size_t result_len;
} compare_visitor_t;

void compare_visitor_init(compare_visitor_t* self,
  dir_entry_fields_t const* result,
  size_t result_len);

//
// parallel traversal visitor
//

typedef struct parallel_traverse_visitor {
  parallel_visitor_t pv_t;
  short passed;
  size_t line;
  char const** result;
  size_t result_len;
} parallel_traverse_visitor_t;

errno_t parallel_traverse_visitor_init(parallel_traverse_visitor_t* self,
  char const* root_path,
  char const** result,
  size_t result_len);

void parallel_traverse_visitor_uninit(parallel_traverse_visitor_t* self);
